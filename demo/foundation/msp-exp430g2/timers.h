
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * times.h is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * timers.h is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _TIMERS_
#define _TIMERS_

#include "anyRTOS.h"
#include "msp-exp430g2-conf.h"

/** @defgroup timers Timers
  * This module controls the hardware timers so that tasks can make 
  * measurements of time.
  * @{ */ 



/** Configure this module and hardware timer. */
void timer_allInit( void );


unsigned int timer_status( timer_t const* timer );


#ifdef HAL_HAS_TIMER0

/** anyRTOS timer instance for timer 0. */
extern timer_t timer0;

/** Convert to timer 0 ticks a time in seconds. */
#define timer0_sec( x ) \
    ((tick_t)(HAL_TIMER0_FREQ*(x))?(tick_t)(HAL_TIMER0_FREQ*(x)):(tick_t)1)


#endif


#ifdef HAL_HAS_TIMER1

/** anyRTOS timer instance for timer 1. */
extern timer_t timer1;

/** Convert to timer 1 ticks a time in seconds. */
#define timer1_sec( x ) \
    ((tick_t)(HAL_TIMER1_FREQ*(x))?(tick_t)(HAL_TIMER1_FREQ*(x)):(tick_t)1)

#endif


/** @} */

#endif /* _TIMERS_ */

