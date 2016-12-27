
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * rtcc.h is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * rtcc.h is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _RTCC_
#define _RTCC_

/** @defgroup rtcc Real Time Clock Calendar
  * @{ */

/** Initializes this module. */
void rtcc_init( void );

/** Sets the real time clock calendar by terminal. */
void rtcc_enterTime( void );

/** Stops the real time clock calendar. */
void rtcc_stop( void );

/** Print the time and date by terminal. */
void rtcc_print( void );

/** Sets an alarm by terminal.
  * @param func: callback function. */
void rtcc_enterAlarm( void(*func)(void) );

/** Unsets the alarm. */
void rtcc_alarmOff( void );

/** Prints the date-time of alarm if it is set.
  * @return The pointer to callback or null if alarm is not set. */
void* rtcc_printAlarm( void );

/** Thread that runs the real time clock calendar. */
void rtcc_task( void* param );

/** @} */

#endif	/* _RTCC_ */
