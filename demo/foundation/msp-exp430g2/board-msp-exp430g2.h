
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * board-msp-exp430g2.h is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * board-msp-exp430g2.h is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _BOARD_
#define _BOARD_

#include <stdbool.h>
#include <stdint.h>
#include <msp430.h>
#include "msp-exp430g2-conf.h"

/** @defgroup board Board
  * This module that controls the switch and leds of exp430g2 board.
  * And It control the ADC of MCU.
  * @{ */ 

#define BOARD_LEDS_QTY      2
#define BOARD_LED0_POUT     P1OUT
#define BOARD_LED0_DIR      P1DIR
#define BOARD_LED0_BIT      BIT0
#define BOARD_LED1_POUT     P1OUT
#define BOARD_LED1_DIR      P1DIR
#define BOARD_LED1_BIT      BIT6
#define BOARD_SWITCH_PIN    P1IN
#define BOARD_SWITCH_DIR    P1DIR
#define BOARD_SWITCH_REN    P1REN
#define BOARD_SWITCH_BIT    BIT3

/** Initializes all. */
void board_init( void );

typedef enum board_led_e { BOARD_LED_RED = 0, BOARD_LED_GREEN = 1 } boardLED_t;

/** Turns off a LED.
  * @param led The index of LED. */
static inline void board_led_off( boardLED_t led ) {
    switch( led ) {
        case BOARD_LED_RED:   BOARD_LED0_POUT &= ~BOARD_LED0_BIT; break;
        case BOARD_LED_GREEN: BOARD_LED1_POUT &= ~BOARD_LED1_BIT; break;
    }
}

/** Turns on a LED.
  * @param led The index of LED. */
static inline void board_led_on( boardLED_t led ) {
    switch( led ) {
        case BOARD_LED_RED:   BOARD_LED0_POUT |= BOARD_LED0_BIT; break;
        case BOARD_LED_GREEN: BOARD_LED1_POUT |= BOARD_LED1_BIT; break;
    }
}
/** Toggles a LED.
  * @param led The index of LED. */
static inline void board_led_toggle( boardLED_t led ) {
    switch( led ) {
        case BOARD_LED_RED:   BOARD_LED0_POUT ^= BOARD_LED0_BIT; break;
        case BOARD_LED_GREEN: BOARD_LED1_POUT ^= BOARD_LED1_BIT; break;
    }
}

/** Initializes all LED pins. */
static inline void board_led_init( void ) {
    board_led_off( BOARD_LED_GREEN );
    board_led_off( BOARD_LED_RED );
    BOARD_LED0_DIR |= BOARD_LED0_BIT;
    BOARD_LED1_DIR |= BOARD_LED1_BIT;
}

/** Checks the state if switch.
  * @retval true if released.
  * @retval false if pressed.   */
static inline bool board_switch_isReleased( void ) {
    if ( BOARD_SWITCH_PIN & BOARD_SWITCH_BIT ) return true;
    return false;
}

/** Checks the state if switch.
  * @retval true if pressed.
  * @retval false if released.   */
static inline bool board_switch_isPressed( void ) {
    return !board_switch_isReleased();
}

/** Initializes the pin of switch. */
static inline void board_switch_init( void ) {
    BOARD_SWITCH_DIR &= ~BOARD_SWITCH_BIT;
    BOARD_SWITCH_REN |= BOARD_SWITCH_BIT;
}

#ifdef HAL_HAS_SWITCH_EVENT

/** Waits until the switch is pressed. */
void board_switch_waitToPressed( void );

/** Waits until the switch is released. */
void board_switch_waitToReleased( void );

#endif /* HAL_HAS_SWITCH_EVENT */

/** @} */

#endif /* _BOARD_ */
