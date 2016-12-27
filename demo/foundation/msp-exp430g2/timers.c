

/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * timers.c is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * timers.c is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <msp430.h>
#include <intrinsics.h>
#include <stdlib.h>
#include "timers.h"
#include "board-msp-exp430g2.h"

/** Defines the timer configuration. */
typedef struct {
    bool enabled;             /**< Indicates if the timer is or not used. */
    unsigned long outputFreq; /**< The interrupt frequency. */
    hal_clkSource_t clkSrc;   /** The clock soure option. */
    timer_t* timer;           /**< Pointer to public anyRTOS timer. */
} timerCfg_t;


#ifdef HAL_HAS_TIMER0

/** anyRTOS timer instance for Timer0. */
timer_t timer0;

/** Configuration for timer0. */
static timerCfg_t const _timer0 = {
    .enabled    = true,
    .outputFreq = (unsigned long)HAL_TIMER0_FREQ,
    .clkSrc     = HAL_TIMER0_CLK_SRC,
    .timer      = &timer0,
};

#else

/** Configuration for timer0. */
static timerCfg_t const _timer0 = { .enabled = false };

#endif


#ifdef HAL_HAS_TIMER1

/** anyRTOS timer instance for Timer0. */
timer_t timer1;

/** Configuration for timer1. */
static timerCfg_t const _timer1 = {
    .enabled    = true,
    .outputFreq = (unsigned long)HAL_TIMER1_FREQ,
    .clkSrc     = HAL_TIMER1_CLK_SRC,
    .timer      = &timer1,
};

#else

/** Configuration for timer1. */
static timerCfg_t const _timer1 = { .enabled = false };

#endif


/** Defines the timer prescaler options. */
typedef enum {
    DIV_1, DIV_2,
    DIV_4, DIV_8,
    DIV_ERR
} prescaler_t;

/** Get the flag bits to be set in the control register to config the prescaler.
  * @param pre: A valid prescaler option. */
static unsigned int _calcPrescalerFlags( prescaler_t pre ) {    
    unsigned int const array[4] = { ID_0, ID_1, ID_2, ID_3 };
    return array[ pre ];
}

/** Calculate the flag bits to be set in the 
  * control register to config the clock source.
  * @param A valid clock soure option. */ 
unsigned int _calcClkSelFlags( hal_clkSource_t clkSrc ) {
    switch( clkSrc ) {
        case HAL_SMCLK: return TASSEL_2;
        case HAL_ACLK:  return TASSEL_1;
        default:        exit(-1); return 0;
    }
}

/** Calculate the flag bits to be set in the control 
  * register to config the prescaler and the clock source.
  * @param A valid clkSrc option.
  * @param pre: A valid prescaler option. */
unsigned int _calCtrlFlags( hal_clkSource_t clkSrc, prescaler_t pre ) {
    return _calcPrescalerFlags( pre ) | _calcClkSelFlags( clkSrc );
}

/** Calculate the value to be set in perdiod register.
  * @param inputFreq: The clock source frequency.
  * @param outputFreq: The interrupt frequency.
  * @param pre: A valid prescaler option.
  * @return The register period value. It can be greater than the register. */
static unsigned long _calcPeriod( unsigned long inputFreq, unsigned long outputFreq, prescaler_t pre ) {
    unsigned int const factor = 1 << pre;
    return ( inputFreq / ( factor * outputFreq ) ) - 1;
}

/** Calclutate the best prescaler option.
  * @param inputFreq: The clock source frequency.
  * @param outputFreq: The interrupt frequency.
  * @return The prescaler option. */
static prescaler_t _calcPrescaler( unsigned long inputFreq, unsigned long outputFreq ) {   
    prescaler_t pre;
    for( pre = 0; pre < DIV_ERR; ++pre ) {
        unsigned long const period = _calcPeriod( inputFreq, outputFreq, pre );
        if ( period <= 0xfffful ) break;
    }
    return pre;
}

/** Number of tasks that need to Timer0. */
static uint8_t _timer0_Qty;

/** Configure timer 0. */
static void _timer0_init( void ) {
    unsigned long const inputFreq = hal_getClkSrcFreq( _timer0.clkSrc );
    prescaler_t const pre = _calcPrescaler( inputFreq, _timer0.outputFreq );
    if ( pre == DIV_ERR ) exit(-1);
    TA0CCTL0 = CCIE;
    TA0CCR0 = _calcPeriod( inputFreq, _timer0.outputFreq, pre );;
    TA0CTL = _calCtrlFlags( _timer0.clkSrc, pre );;
    _timer0_Qty = 0;
    timer_init( _timer0.timer );        
}

/** Turn on the timer 0 and uodate the tick. */
static void _timer0_on( void ) { 
    task_enterCritical();
    task_updateTick( &timer0 );
    if ( !_timer0_Qty++ ) TA0CTL |= MC_1 | TACLR;
    task_exitCritical();
}

/** Turn off the timer 0. */
static void _timer0_off( void ) {
    task_enterCritical();
    if ( !--_timer0_Qty ) TA0CTL &= ~MC_1;
    task_exitCritical();
}

/** Periodic ISR. */
__attribute__( ( __interrupt__( TIMER0_A0_VECTOR ) ) )
static void _timerA0_isr( void ) {
    if ( !_timer0.enabled ) return;
    if ( timer_tick( _timer0.timer ) ) task_yieldISR();
    if ( 0 ) {
        static unsigned cntr = 1;
        if ( !--cntr ) {
            cntr = _timer0.outputFreq;
            board_led_toggle( BOARD_LED_GREEN );
        }
    }           
}

/** Number of tasks that need to Timer0. */
static uint8_t _timer1_Qty;

/** Configure timer 1. */
static void _timer1_init( void ) {   
    unsigned long const inputFreq = hal_getClkSrcFreq( _timer1.clkSrc );
    prescaler_t const pre = _calcPrescaler( inputFreq, _timer1.outputFreq );
    if ( pre == DIV_ERR ) exit(-1);
    TA1CCTL0 = CCIE;
    TA1CCR0 = _calcPeriod( inputFreq, _timer1.outputFreq, pre );;
    TA1CTL = _calCtrlFlags( _timer1.clkSrc, pre );;
    _timer1_Qty = 0;
    timer_init( _timer1.timer );     
}

/** Turn on the timer 0 and uodate the tick. */
static void _timer1_on( void ) { 
    task_enterCritical();
    task_updateTick( _timer1.timer );
    if ( !_timer1_Qty++ ) TA1CTL |= MC_1 | TACLR;
    task_exitCritical();
}

/** Turn off the timer 1. */
static void _timer1_off( void ) {
    task_enterCritical();
    if ( !--_timer1_Qty ) TA1CTL &= ~MC_1;
    task_exitCritical();
}

/** Periodic ISR. */
__attribute__( ( __interrupt__( TIMER1_A0_VECTOR ) ) )
static void _timerA1_isr( void ) {
    if ( !_timer1.enabled ) return;
    if ( timer_tick( _timer1.timer ) ) task_yieldISR();
    if ( 0 ) {
        static unsigned cntr = 1;
        if ( !--cntr ) {
            cntr = _timer1.outputFreq;
            board_led_toggle( BOARD_LED_GREEN );
        }
    } 
}

/* Configure this module and hardware timer. */
void timer_allInit( void ) {
    if ( _timer0.enabled ) _timer0_init();
    if ( _timer1.enabled ) _timer1_init();
   
}

/* Turn on the timer and uodate the tick. */
void timer_on( timer_t const* timer ) {
    if ( _timer0.enabled && timer == _timer0.timer ) {
        _timer0_on();
        return;
    }   
    if ( _timer1.enabled && timer == _timer1.timer ) {
        _timer1_on();
        return;
    }           
}

/* Turn off the timer. */
void timer_off( timer_t const* timer ) {
    if ( _timer0.enabled && timer == _timer0.timer ) {
        _timer0_off();
        return;
    }    
    if ( _timer1.enabled && timer == _timer1.timer ) {
        _timer1_off();
        return;
    }          
}

unsigned int timer_status( timer_t const* timer ) {
    if ( _timer0.enabled && timer == _timer0.timer ) return _timer1_Qty;
    if ( _timer1.enabled && timer == _timer1.timer ) return _timer1_Qty;
    return 0;
}

/* ------------------------------------------------------------------------ */
