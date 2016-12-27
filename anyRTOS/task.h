
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * task.h is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * task.h is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _TASK_
#define	_TASK_

#include <stdbool.h>
#include "timer.h"

#ifdef __cplusplus
extern "C" {
#endif
    
/** @defgroup task Task Control 
  * @{ */

/** Disables interrupts to enter in a critical section in the 
  * context of the running thread. Nested critical sections are allow. */
void task_enterCritical( void );

/** Enables interrupts, if required, to leave a critical section. */
void task_exitCritical( void );

/** Sets the task tick with a timer tick counter.
  * @param timer: The timer handler. */
void task_updateTick( timer_t const* timer );

/** Icreases the task tick.
  * @param ticks: Increasing number of ticks.  */
void task_increaseTimeout( tick_t ticks );

/** Sets the task tick with a timer tick counter and icreases the task tick.
  * This is equivalent: task_updateTick( timer ); task_increaseTimeout( ticks );
  * @param timer: The timer handler.
  * @param ticks: Increasing number of ticks. */
void task_setTimeout( timer_t const* timer, tick_t ticks );

/** Checks if the counter tick of a timer has been got the task tick.
  * @param timer: The timer handler. */
bool task_isOver( timer_t const* timer );

/** Gets the priority of the task. 
  * @return The priority value. */
prior_t task_getPriority( void );

/** Set a new priority to task.
  * @return The old priority. */
prior_t task_setPriority( prior_t prior );

/** Yields the flow of execution to threads of greater than or equal priority. */
void task_yield( void );

/** Yields the flow of execution in interrupt service routines. */
void task_yieldISR( void );

/** Sets the running thread in suspended state. */
void task_suspend( void );

/** Sets a thread in suspended state in ready state.
  * @param th: Thread handler. */
void task_resume( thread_t* th );

/** @} */

#ifdef __cplusplus
}
#endif

#endif	/* _TASK_ */

