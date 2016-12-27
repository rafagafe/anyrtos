
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * event.h is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * event.h is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _EVENT_
#define	_EVENT_

#include <stdbool.h>
#include "anyRTOS-conf.h"
#include "timer.h"
#include "src/thread-list.h"

#ifdef __cplusplus
extern "C" {
#endif
    
/** @defgroup event Event Control 
  * @{ */ 

/** Structure to handle events. */
typedef struct event_s {
    priorList_t list;
} event_t;

/** Initializes an event.
  * @param event: Event handler. */
void event_init( event_t* event );

/** Sets the running thread in blocked state until an event occurs.
  * More than one thread can be blocked waiting the same event.
  * @param event: Event handler. */
void event_wait( event_t* event );

/** Sets the highest priority thread blocked by an event in ready state.
  * If the priority of this thread is higher than the running thread it yields.
  * @param event: Event handler. */
void event_notify( event_t* event );

/** Sets the highest priority thread blocked by an event in ready 
  * state in a interrupt service routine. It does not yield.
  * @param event: Event handler.
  * @retval true: If the priority of this thread is higher than the running thread.
  * @retval false: In other case.*/
bool event_notifyISR( event_t* event );

/** Sets all threads blocked by an event in ready state. If the priority of 
  * the highest priority thread is higher than  the running thread it yields.
  * @param event: Event handler. */
void event_notifyAll( event_t* event );

/** Sets all threads blocked by an event in ready state in an 
  * interrupt service routine. It does not yield.
  * @param event: Event handler.
  * @retval true: If the priority of the highest priority thread 
  *               was higher than the running thread.
  * @retval false: In other case.*/
bool event_notifyAllISR( event_t* event );

#if !defined( ANYRTOS_BASIC_MODE ) || ( !ANYRTOS_BASIC_MODE )

/** Waits until an event occurs or until the 
  * tick counter of a timer gets the task tick.
  * @param event: Event handler.
  * @param timer: Timer handler.
  * @retval true:  The event occurs before the timer gets the task tick.
  * @retval false: The timer gets the task tick before the event occurs. */
bool eventTimer_wait( event_t* event, timer_t* timer );

#else

/** Waits until an event occurs.
  * @param event: Event handler.
  * @param timer: Timer handler, ignored.
  * @return It always returns true. */
static inline bool eventTimer_wait( event_t* event, timer_t* timer ) {
    (void)timer;
    event_wait( event );
    return true;
}

#endif /* ANYRTOS_BASIC_MODE */

/** @} */

#ifdef __cplusplus
}
#endif

#endif	/* _EVENT_ */

