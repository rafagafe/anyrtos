
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * intervalometer.c is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * intervalometer.c is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdlib.h>
#include "msp-exp430g2/timers.h"
#include "msp-exp430g2/serial-port.h"
#include "msp-exp430g2/board-msp-exp430g2.h"

/** Commands to process by inervalometer task. */
typedef enum __attribute__((packed)) command_e { 
    CMD_NOTHING,
    CMD_INTER,
    CMD_HDR, 
    CMD_STOP
} command_t;

/** Lookup table: Timer 1 ticks for HDR speeds. */
static tick_t const _speeds[] = {
    timer1_sec(1/25.0), timer1_sec(1/20.0), timer1_sec(1/15.0),
    timer1_sec(1/13.0), timer1_sec(1/10.0), timer1_sec(1/8.0),
    timer1_sec(1/6.0),  timer1_sec(1/5.0),  timer1_sec(1/4.0),
    timer1_sec(0.3),    timer1_sec(0.4),    timer1_sec(0.5), 
    timer1_sec(0.6),    timer1_sec(0.8),    timer1_sec(1.0),
    timer1_sec(1.3),    timer1_sec(1.6),    timer1_sec(2.0), 
    timer1_sec(2.5),    timer1_sec(3.2),    timer1_sec(4.0),
    timer1_sec(5.0),    timer1_sec(6.0),    timer1_sec(8.0),
    timer1_sec(10.0),   timer1_sec(13.0),   timer1_sec(15.0),
    timer1_sec(20.0),   timer1_sec(25.0),   timer1_sec(30.0)
};

/** Lookup table: HDR speeds . */
static char const* const _speedNames[] = {
    "1/25", "1/20", "1/15", "1/13", "1/10", "1/8", "1/6", "1/5", "1/4", "0\"3",
    "0\"4", "0\"5", "0\"6", "0\"8", "1\"", "1\"3", "1\"6", "2\"", "2\"5", "3\"2",
    "4\"", "5\"", "6\"", "8\"", "10\"", "13\"", "15\"", "20\"", "25\"", "30\""
};

/** The preprocessor calculates the HDR speeds quantity. */
#define _SPEEDS_QTY (sizeof(_speedNames)/sizeof(char const*))

/** Lookup table: HDR steps names. */
static char const* const _stepNames[] = {
    "1/3 ev", "2/3 ev", "3/3 ev", "4/3 ev", "5/3 ev",
};


/** The preprocessor calculates the HDR steps quantity. */
#define _STEPS_QTY (sizeof(_stepNames)/sizeof(char const*))

/** This LED shinnes when focus is activated. */
#define _LED_FOCUS  BOARD_LED_GREEN

/** This LED shinnes when shut is activated. */
#define _LED_SHUT   BOARD_LED_RED

/** Store the running command. */
static command_t volatile _commandCode;

/** This event occours in event changes. */
static event_t _commandEvent;

/** This adjusts the time (in seconds) between “start” and the first photo. */
static uint16_t _delay = 5; 

/** This affects how long the shutter remains open. 
    This setting’s only relevant if you’re shooting 
    in bulb mode and has not effect in HDR process. */
static uint16_t _long = 1;  

/** How long between shutter clicks. */
static uint16_t _intv = 5;  

/** The total number of shots. */
static uint16_t _numb = 5;  

/** Initial HDR speed. ( 0-> 1/25, 29 -> 30" ). */
static uint8_t  _hdr = 0;   

/** 1/3 EV increases in HDR process. */
static uint8_t  _step = 1; 

/** Thread handler of intervalometer_task(). */
static thread_t* _th;

/** Activate the focus. */
static void _focusOn( void ) { board_led_on( _LED_FOCUS ); }

/** Deactivate the focus. */
static void _focusOff( void ) { board_led_off( _LED_FOCUS ); }

/** Activate the shut. */
static void _shutOn( void ) { board_led_on( _LED_SHUT ); }

/** Dectivate the shut. */
static void _shutOff( void ) { board_led_off( _LED_SHUT ); }

static void _u8( uint8_t data ) {
    if ( data < 10 ) serial_msg("  ");
    else if ( data < 100 ) serial_char(' ');
    serial_u8( data );
}

void intervalometer_printSpeeds( void ) {
    for( unsigned i = 0; i < _SPEEDS_QTY; ++i ) {
        _u8( i + 1 );
        serial_msg(") ");
        serial_msg(_speedNames[i]);
        serial_endl();
    }
}
/* Check the task interval timer is running some process. */
bool intervalometer_isRunning( void ) { return (_commandCode != CMD_NOTHING); }

/* Initializes this module. */
void interval_init( thread_t* th ) {
    _th = th;
    _commandCode = CMD_NOTHING;
    event_init( &_commandEvent );
    _shutOff();
    _focusOff();
}

/** Try to set a command. */
static void _setCommand( command_t cmd ) {
    task_enterCritical();
    if ( _commandCode == CMD_NOTHING ) {
        _commandCode = cmd;
        event_notify( &_commandEvent );        
    }
    else serial_line("Already running.");
    task_exitCritical();    
}

/* Try to run a intervalometer process. */
void interval_run( void ) { _setCommand( CMD_INTER ); }

/* Try to run a HDR process. */
void interval_hdr( void ) { _setCommand( CMD_HDR ); }

/** Stop a running process. */
void interval_stop( void ) {
    task_enterCritical(); 
    if ( _commandCode == CMD_NOTHING ) serial_line("Already stopped.");
    else  {
        _commandCode = CMD_STOP;
        timer_abort( &timer1, _th );
        while( _commandCode == CMD_STOP ) event_wait( &_commandEvent );
    }
    task_exitCritical();    
}

/* Thread that runs the intervalometer and HDR processes. */
thread void intervalometer_task( void* param ) {
    task_enterCritical();
    for(;;) {
        _commandCode = CMD_NOTHING;  
        event_notify( &_commandEvent );        
        while( _commandCode == CMD_NOTHING ) event_wait( &_commandEvent );
        timer_on( &timer1 );
        tick_t delay, firstFocus;
        if (_delay >= 3 ) {
            delay = timer1_sec( _delay - 3 );
            firstFocus = timer1_sec( 3 );
        }
        else {
            firstFocus = timer1_sec( _delay );
            delay = (tick_t)0;
        }
        if ( _commandCode == CMD_STOP ) continue;
        tick_t intv, focus;
        if ( _intv >= 3 ) {
            intv = timer1_sec( _intv - 3 );
            focus = timer1_sec( 3 );
        }
        else {
            focus = timer1_sec( _intv );
            intv = (tick_t)0;
        }
        if ( _commandCode == CMD_STOP ) continue;
        if ( delay ) timer_period( &timer1, delay );
        if ( _commandCode == CMD_STOP ) continue;
        if ( firstFocus ) {
            _focusOn();
            timer_period( &timer1, firstFocus );
            _focusOff();
        }   
        if ( _commandCode == CMD_STOP ) continue;
        if ( _commandCode == CMD_INTER ) {    
            _shutOn();        
            tick_t Long = timer1_sec( _long );
            for( unsigned i = 0;;) {
                timer_period( &timer1, Long );
                _shutOff();
                if ( _commandCode == CMD_STOP ) break;
                if ( ++i >= _numb ) break;  
                if ( intv ) timer_shift( &timer1, intv );
                if ( _commandCode == CMD_STOP ) break;
                if ( focus ) {
                    _focusOn();
                    timer_period( &timer1, focus );
                    _focusOff();
                }
                if ( _commandCode == CMD_STOP ) break;
                _shutOn();             
            }
        }
        else {
            unsigned speed = _hdr;        
            for( unsigned i = 0;;) {
                _shutOn(); 
                timer_period( &timer1, _speeds[ speed ] );
                _shutOff();
                if ( _commandCode == CMD_STOP ) break;
                if ( ++i >= _numb ) break;
                speed += _step;
                if ( speed >= _SPEEDS_QTY ) break;
                if ( intv ) timer_period( &timer1, intv );
                if ( _commandCode == CMD_STOP ) break;
                if ( focus ) {
                    _focusOn();
                    timer_period( &timer1, focus );
                    _focusOff();
                }
                if ( _commandCode == CMD_STOP ) break;
            }
        }
        timer_off( &timer1 ); 
    }
}

/* Enter the intervalometer settings by terminal. */
void interval_enterSettings( void ) {
    if ( intervalometer_isRunning() ) {
        serial_line("Intervalometer is running." );    
        return;
    }
    char str[5];
    serial_line("Enter intervalometer parameters:" );
    serial_msg("        Delay, (seconds): ");
    serial_getNum( str, sizeof(str) );
    _delay = atoi( str );
    do {
        serial_msg("         Long, (seconds): ");
        serial_getNum( str, sizeof(str) );
        _long = atoi( str ); 
    } while( !_long );
    do {
        serial_msg("         Intv, (seconds): ");
        serial_getNum( str, sizeof(str) );
        _intv = atoi( str );
    } while( !_intv );
    do {
        serial_msg("                       N: ");
        serial_getNum( str, sizeof(str) );
        _numb = atoi( str );    
    } while( !_numb );
    serial_msg("               HDR [+/-]: ");
    _hdr = serial_option( _speedNames, _SPEEDS_QTY );
    serial_msg("              Step [+/-]: +");
    _step = 1 + serial_option( _stepNames, _STEPS_QTY );
    serial_endl();        
}

/* Print the the intervalometer settings. */
void interval_printSettings( void ) {
    serial_msg("    Delay: ");
    serial_u16( _delay );
    serial_msg("\"\n\r     Long: ");
    serial_u16( _long );
    serial_msg("\"\n\r     Intv: ");
    serial_u16( _intv );
    serial_msg("\"\n\r        N: ");
    serial_u16( _numb );
    serial_msg("\n\r      HDR: ");
    serial_msg( _speedNames[_hdr] );
    serial_msg(" ev\n\r     Step: +");
    serial_msg( _stepNames[_step-1] );
    serial_endl();    
}

/* ------------------------------------------------------------------------ */

