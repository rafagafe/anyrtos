
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * board-msp-exp430g2.c is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * board-msp-exp430g2.c is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "board-msp-exp430g2.h"
#include "anyRTOS.h"
#include <stdlib.h>
#include <stdbool.h>


#if defined( HAL_HAS_SWITCH_EVENT ) 
/** This event occurs in the state changes of switch. */
static event_t _switch;
#endif    

/** Get the value for BCSCTL1 register. */
static unsigned int _getBCSCTL1( unsigned long freq ) {
    switch( freq ) {
        case 16000000ul: return CALBC1_16MHZ;
        case 12000000ul: return CALBC1_12MHZ;
        case  8000000ul: return CALBC1_8MHZ;
        case  1000000ul: return CALBC1_1MHZ;
    }
    exit(-1);
    return 0;
}

/** Get the value for DCOCTL register. */
static unsigned int _getDCOCTL( unsigned long freq ) {
    switch( freq ) {
        case 16000000ul: return CALDCO_16MHZ;
        case 12000000ul: return CALDCO_12MHZ;
        case  8000000ul: return CALDCO_8MHZ;
        case  1000000ul: return CALDCO_1MHZ;
    }
    exit(-1);
    return 0;    
}

/* Initializes all. */
void board_init( void ) { 
    /* Configures oscilator and watch dog timer: */
    WDTCTL = WDTPW | WDTHOLD;
    BCSCTL1 = _getBCSCTL1( HAL_MCLK_FREQ );
    DCOCTL = _getDCOCTL( HAL_MCLK_FREQ );         
    board_led_init();
    board_switch_init();
#if defined( HAL_HAS_SWITCH_EVENT ) 
    event_init( &_switch );
#endif         
}

#if defined( HAL_HAS_SWITCH_EVENT ) 

/** The switch interrpts when is pressed. */
static void _interruptWhenPress( void ) {
    P1IFG &= ~BOARD_SWITCH_BIT;
    P1IES |= BOARD_SWITCH_BIT;
    P1IE  |= BOARD_SWITCH_BIT;
} 

/** The switch interrpts when is released. */
static void _interruptWhenRelease( void ) {
    P1IFG &= ~BOARD_SWITCH_BIT;
    P1IES &= ~BOARD_SWITCH_BIT;
    P1IE  |= BOARD_SWITCH_BIT;
}

/** Waits until the switch is pressed. */
void board_switch_waitToPressed( void ) {
    task_enterCritical();
    _interruptWhenPress();
    event_wait( &_switch );
    task_exitCritical();    
}

/** Waits until the switch is released. */
void board_switch_waitToReleased( void ) {
    task_enterCritical();
    _interruptWhenRelease();
    event_wait( &_switch );
    task_exitCritical();    
}

/** Port 1 ISR: */  
__attribute__(( __interrupt__( PORT1_VECTOR ) )) 
static void _port1_isr( void ) {
    P1IE  &= ~BOARD_SWITCH_BIT;
    P1IFG &= ~BOARD_SWITCH_BIT;
    event_notify( &_switch );
}

#endif 

/* ------------------------------------------------------------------------ */
