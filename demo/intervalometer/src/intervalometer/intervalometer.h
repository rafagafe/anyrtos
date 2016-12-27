
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * intervalometer.h is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * intervalometer.h is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _INTERVALOMETER_
#define _INTERVALOMETER_

#include "anyRTOS.h"

/** @defgroup interval Intervalometer
  * @{ */

/** Initializes this module.
  * @param th: The thread handler of intervalometer_task(). */
void interval_init( thread_t* th );

/** Enter the intervalometer settings by terminal. */
void interval_enterSettings( void );

/** Print the the intervalometer settings. */
void interval_printSettings( void );

/** Try to run a intervalometer process. */
void interval_run( void );

/** Try to run a HDR process. */
void interval_hdr( void );

/** Stop a running process. */
void interval_stop( void );

/** Check the task interval timer is running some process. */
bool intervalometer_isRunning( void ) ;

void intervalometer_printSpeeds( void );

/** Thread that runs the intervalometer and HDR processes. */
void intervalometer_task( void* param );

/** @} */

#endif /* _INTERVALOMETER_ */