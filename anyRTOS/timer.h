
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * timer.h is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * timer.h is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _TIMER_
#define	_TIMER_

#include <stdbool.h>
#include "src/thread-list.h"

#ifdef __cplusplus
extern "C" {
#endif
    
/** @defgroup timer Timer Control
  * @{ */

/** Structure for handle timers. */
typedef struct timer_s {
    tickList_t list;
    tick_t volatile tick;
} timer_t;

/** Initializes a timer handler.
  * @param timer: The timer handler. */
void timer_init( timer_t* timer );

/** Increases the tick counter of a timer.
  * It has to be invoked perodictly in a by user timer driver.  
  * @param timer: The timer handler.
  * @retval true: If yield is suggested.
  * @retval false: If yield is not necessary. */
bool timer_tick( timer_t* timer );

/** Waits until the tick counter of a timer gets the task tick.
  * @see task_setTimeout().
  * @param timer: Timer handler. */
void timer_wait( timer_t* timer );

/** Wait N ticks of a timer from the task tick.
  * @see task_updateTick(). 
  * @param timer: The timer handler.
  * @param ticks: Ticks quantity to wait. */
void timer_shift( timer_t* timer, tick_t ticks );

/** Wait N ticks of a timer from the task tick.
  * After the waiting the task tick is increased N ticks.
  * @see task_updateTick(). 
  * @see timer_shift(). 
  * @param timer: The timer handler.
  * @param ticks: Ticks quantity to wait. */
void timer_period( timer_t* timer, tick_t ticks );

/** Waits N ticks from now on.
  * @param timer: Timer handler.
  * @param ticks: Ticks quantity to wait. */
void timer_delay( timer_t* timer, tick_t ticks );

/** Resume a thread blocked by a timer.
  * @param timer: Timer handler.
  * @param th: Thread handler.
  * @retval true: The thread was blocked.
  * @retval false: The thread was not blocked. */
bool timer_abort( timer_t* timer, thread_t* th );

/** Turn on the timer and uodate the tick.
  * It has to be defined by user driver.
  * @param timer: The timer handler. */
void timer_on( timer_t const* timer );

/** Turn off the timer.
  * It has to be defined by user driver.
  * @param timer: The timer handler. */
void timer_off( timer_t const* timer );

/** @} */

#ifdef __cplusplus
}
#endif

#endif	/* _TIMER_ */

