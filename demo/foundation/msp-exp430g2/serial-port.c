
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * serial-port.c is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * serial-port.c is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include "serial-port.h"

#ifdef HAL_HAS_SERIAL_PORT

#include <stdlib.h>
#include <msp430.h>
#include "anyRTOS.h"
#include "queue.h"

/** Get the value for UCSSEL register. */
static unsigned int _calcUCSSEL( hal_clkSource_t clkSrc ) {
    switch( clkSrc ) {
        case HAL_SMCLK: return UCSSEL_2;
        case HAL_ACLK:  return UCSSEL_1;
        default:;
    }
    exit(-1);
    return 0;      
}

/** Get the value for UCA0BR0 register. */
static unsigned int _calcUCA0BR0( unsigned long freq ) {
    switch( freq ) {
        case 16000000ul: return 0x82;
        case 12000000ul: return 1250;
        case  8000000ul: return 833;
        case  1000000ul: return 104;
    }
    exit(-1);
    return 0;    
}

/** Get the value for UCA0BR1 register. */
static unsigned int _calcUCA0BR1( unsigned long freq ) {
    switch( freq ) {
        case 16000000ul: return 6;
        case 12000000ul: return 0;
        case  8000000ul: return 0;
        case  1000000ul: return 0;
    }
    exit(-1);
    return 0;    
}

/** Get the value for UCA0MCTL register. */
static unsigned int _calcUCA0MCTL( unsigned long freq ) {
    switch( freq ) {
        case 16000000ul: return UCBRS_6;
        case 12000000ul: return 0;
        case  8000000ul: return UCBRS_2;
        case  1000000ul: return UCBRS_1;       
    }
    exit(-1);
    return 0;    
}

/** Queue for receive data. */
static queue_t _rx;
/** Memory for receive queue. */
static uint8_t _rxMem[4];

/** Queue for transmit data. */
static queue_t _tx;
/** Memory for transmit FIFO. */
static uint8_t _txMem[4];

/** Configure hardware and driver state variables. */
void serial_init( void ) {       
    queue_init( &_rx, _rxMem, sizeof(_rxMem) );
    queue_init( &_tx, _txMem, sizeof(_txMem) );    
    UCA0CTL1 |= UCSWRST;  
    P1SEL  |= BIT1 | BIT2;  // P1.1 <-> RXD, P1.2 <-> TXD
    P1SEL2 |= BIT1 | BIT2;  // P1.1 <-> RXD, P1.2 <-> TXD
    UCA0CTL1 |= _calcUCSSEL( HAL_SERIAL_PORT_SRC );
    unsigned long const inputFreq = hal_getClkSrcFreq( HAL_SERIAL_PORT_SRC );
    UCA0BR0 = _calcUCA0BR0( inputFreq );
    UCA0BR1 = _calcUCA0BR1( inputFreq );
    UCA0MCTL = _calcUCA0MCTL( inputFreq );
    UCA0CTL1 &= ~UCSWRST;   // Initialize USCI state machine  
    IFG2 |= UCA0TXIFG;
    IE2 |= UCA0RXIE;
}

/** USCIA0 TX ISR. */
__attribute__(( __interrupt__( USCIAB0TX_VECTOR ) ))
static void _usciAB0_tx_isr( void ) {
    uint8_t byte;
    switch( queue_get8ThdISR( &_tx, &byte, 2 ) ) {        
        case QUEUE_DOYIELD:    
            UCA0TXBUF = byte;
            task_yieldISR(); 
            break;            
        case QUEUE_DONOTYIELD:                  
            UCA0TXBUF = byte; 
            break;            
        case QUEUE_ERROR:
            IE2 &= ~UCA0TXIE;
            break;
    }    
    
}

/** USCIA0 RX ISR. */
__attribute__( ( __interrupt__( USCIAB0RX_VECTOR ) ))
static void _usciAB0_rx_isr( void ) {
    uint8_t byte = UCA0RXBUF;
    switch( queue_put8ISR( &_rx, byte ) ) {
        case QUEUE_DOYIELD: task_yieldISR(); break;
        case QUEUE_DONOTYIELD:                              
        case QUEUE_ERROR:
            break;
    }
}

/** Wait to send a character by serial port.
  * @param ch: Character to be sent. */
static void _put( char ch ) {
    queue_put8( &_tx, ch );
    IE2 |= UCA0TXIE;         
}

/** Wait to send a string by serial port. */
static void _putStr( char const* str ) { while( *str ) _put( *str++ ); }

static size_t _putStrLen( char const* str ) { 
    size_t size = (size_t)0;
    while( *str ) {
        ++size;
        _put( *str++ );
    }
    return size;
}

/** Wait to get aa received character. */
static char _get( void ) { return queue_get8( &_rx ); }

static bool _getTimeout( timer_t* timer, char* ch ) {
    return queueTimer_get8( &_rx, timer, (uint8_t*)ch );
}

/** Wait to send end of line. */
static void _endl( void ) { _putStr("\r\n"); }

static void _backSpace( void ) { _putStr("\b \b"); }

void _backSpaceLen( size_t len ) {
    for( unsigned i = len; i; --i ) _put('\b');
    for( unsigned i = len; i; --i ) _put(' ');
    for( unsigned i = len; i; --i ) _put('\b');
}

static char _nibbleToChar( uint8_t nibble ) {
    return "0123456789ABCDEF"[ nibble & 0xf ];
}

/* Waits until send a byte in hexadecimal format. */
static void _x8( uint8_t data ) {
    _put( _nibbleToChar( data >> 4 ) );
    _put( _nibbleToChar( data ) );    
}

/* Waits until send a word in hexadecimal format. */
static void _x16( uint16_t data ) {
    _x8( ((uint8_t*)&data)[1] );
    _x8( ((uint8_t*)&data)[0] );    
}

int putchar(int c) {
    task_enterCritical();
    _put( c );
    task_exitCritical();
    return c;
}

/* Waits until send a character. */
void serial_char( char ch ) { 
    task_enterCritical();
    _put( ch );
    task_exitCritical();
}

/* Waits until send a new line code. */
void serial_endl( void ) {
    task_enterCritical();
    _endl();
    task_exitCritical();
}

/* Waits until send a string and new line. */
void serial_line( char const* line ) {
    task_enterCritical();
    _putStr( line );
    _endl();
    task_exitCritical();
}

/* Waits until send a string. */
void serial_msg( char const* str ) {
    task_enterCritical();
    _putStr( str );
    task_exitCritical();
}

void serial_bool( bool cond ) {
    task_enterCritical();
    _putStr( cond? "true": "false" );
    task_exitCritical();
}
/* Waits until send a byte in hexadecimal format. */
void serial_x8( uint8_t data ) {
    task_enterCritical();
    _x8( data );
    task_exitCritical();
}
    
/* Waits until send a word in hexadecimal format. */
void serial_x16( uint16_t data ) { 
    task_enterCritical();
    _x16( data );
    task_exitCritical();
}

/** Waits until send a word in decimal format. */
static void _u16( uint16_t data ) {     
    static uint_fast16_t const denominators[] = { 10000, 1000, 100, 10 };
    unsigned i;
    for( i = 0; ( i < 4 ) && ( data < denominators[i] ); ++i );
    for( ; i < 4; ++i ) {  
        uint_fast16_t const denominator = denominators[i];
        uint_fast16_t const digit = data / denominator;
        data -= digit * denominator;
        _put( digit + '0' );
    }
    _put( data + '0' );
}

/* Waits until send a word in decimal format. */
void serial_u16( uint16_t data ) {
    task_enterCritical();
    _u16( data );
    task_exitCritical();
}

/** Waits until send a word in decimal format. */
void serial_s16( int16_t data ) {
    task_enterCritical();
    if ( data < 0 ) {
        data *= -1;
        _put('-');
    }
    _u16( data );
    task_exitCritical();
}

/* Waits until send a byte in decimal format. */
void serial_u8( uint8_t data ) { 
    task_enterCritical();
    _u16( data );
    task_exitCritical();
}

/* Waits until receive a character. */
char serial_get( void ) {
    task_enterCritical();
    char ch = _get();
    task_exitCritical();
    return ch;
}

/* Waits until receive a character or timeout. */
bool serialTimer_get( timer_t* timer, char* ch ) {
    task_enterCritical();
    bool retVal = _getTimeout( timer, ch );
    task_exitCritical();
    return retVal;
}

/* Waits until receive a string. */
size_t serial_getStr( char* str, size_t size ) {
    task_enterCritical();
    size_t count = (size_t)0;
    --size;
    for(;;) {
        *str = _get();
        if( *str >= ' ' ) { 
            if ( count < size ) {
                _put( *str );
                ++str;
                ++count;
            }
        }
        else if ( *str == '\b' ) {
            if ( count ) {
               _backSpace();
               --str;
               --count;
            }
        }
        else {
            _endl();            
            *str = '\0';
            break;                
        }
    }
    task_exitCritical();
    return count;
}

/* Waits until receive a numeric string. */
size_t serial_getNum( char* str, size_t size ) {
    task_enterCritical();
    size_t count = (size_t)0;
    --size;
    for(;;) {
        *str = _get();
        if( ( *str >= '0' ) && ( *str <= '9') ) { 
            if ( count < size ) {
                _put( *str );
                ++str;
                ++count;
            }
        }
        else if ( *str == '\b' ) {
            if ( count ) {
               _backSpace();
               --str;
               --count;
            }
        }
        else if ( *str < ' ' ) {
            _endl();            
            *str = '\0';
            break;                
        }
    }
    task_exitCritical();
    return count;
}

size_t serialTimer_getNum( timer_t* timer, char* str, size_t size ) {
    char* ptr = str;
    task_enterCritical();
    size_t count = (size_t)0;
    --size;
    for(;;) {
        if ( !_getTimeout( timer, str ) ) {
            count = (size_t)0;
            *ptr = '\0';
            break;
        }
        if( ( *str >= '0' ) && ( *str <= '9') ) { 
            if ( count < size ) {
                _put( *str );
                ++str;
                ++count;
            }
        }
        else if ( *str == '\b' ) {
            if ( count ) {
               _backSpace();
               --str;
               --count;
            }
        }
        else if ( *str < ' ' ) {
            _endl();            
            *str = '\0';
            break;                
        }
    }
    task_exitCritical();
    return count;
}

/* This makes the user to choose an option. */
size_t serial_option( char const* const opt[], size_t qty ) {
    task_enterCritical();   
    size_t i = 0;
    size_t length = _putStrLen( opt[i] );
    for(;;) {
        char ch = _get();
        if ( ch < ' ' ) {
            _endl();
            return i;
        }
        if ( ( ch == '+' ) || ( ch == ' ' ) ) {
            if (++i >= qty ) i = 0;
            _backSpaceLen( length );
            length = _putStrLen( opt[i] );
        }
        else if ( ch == '-' ) {
            if ( !i ) i = qty;
            --i;
            _backSpaceLen( length );
            length = _putStrLen( opt[i] );
        }
    }
    task_exitCritical();
}


#endif /* HAL_HAS_SERIAL_PORT */

/* ------------------------------------------------------------------------ */
