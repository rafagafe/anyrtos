
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * serial-port.h is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * serial-port.h is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _SERIAL_PORT_
#define _SERIAL_PORT_

#include "msp-exp430g2-conf.h"

#ifdef HAL_HAS_SERIAL_PORT

#include "stdint.h"
#include "stddef.h"
#include "timer.h"

/** @defgroup serial Serial Port
  * This module implements methods to send and receive text by serial port. 
  * @{ */ 

/** Configure hardware and driver state variables. */
void serial_init( void );

/**  Waits until send a character. */
void serial_char( char ch );

/** Waits until send a string. */
void serial_msg( char const* msg );

void serial_bool( bool cond );

/** Waits until send a string and new line. */
void serial_line( char const* line );

/** Waits until send a byte in hexadecimal format. */
void serial_x8( uint8_t data );

/** Waits until send a word in hexadecimal format. */
void serial_x16( uint16_t data );

/** Waits until send a byte in decimal format. */
void serial_u8( uint8_t data );

/** Waits until send a word in decimal format. */
void serial_u16( uint16_t data );

/** Waits until send a word in decimal format. */
void serial_s16( int16_t data );

/** Waits until send a new line code. */
void serial_endl( void );

static inline void serial_u08( uint8_t byte ) {
    if ( byte < 10 ) serial_x8( byte );
    else serial_u8( byte );
}

/** Waits until receive a character. */
char serial_get( void );

/** Waits until receive a character or timeout.
  * @param timer: Timer handler.
  * @param ch: Reference to destination.
  * @retval true:  A character is received before the timer gets the task tick.
  * @retval false: The timer gets the task tick before a character is received. */
bool serialTimer_get( timer_t* timer, char* ch );

/** Waits until receive a string. 
  * @param str: Destination buffer.
  * @param size: Size of destination buffer.
  * @return The length of string. */
size_t serial_getStr( char* str, size_t size );

/** Waits until receive a numeric string. 
  * @param str: Destination buffer.
  * @param size: Size of destination buffer.
  * @return The length of string. */
size_t serial_getNum( char* str, size_t size );

/** Waits until receive a numeric string or timeout.
  * @param timer: Timer handler.
  * @param str: Destination buffer.
  * @param size: Size of destination buffer.
  * @return The length of string when numeric string is received before the 
  *         timer gets the task tick. Zero when the timer gets the task tick
  *         before a numeric string is received. */
size_t serialTimer_getNum( timer_t* timer, char* str, size_t size );

/** This makes the user to choose an option.
  * @param opt: The list of names of options.
  * @param qty: Quantity of options.
  * @return The chosen option index. */  
size_t serial_option( char const* const opt[], size_t qty );

#endif /* HAL_HAS_SERIAL_PORT */

/** @} */

#endif /* _SERIAL_PORT_ */
