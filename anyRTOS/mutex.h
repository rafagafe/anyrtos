
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * mutex.h is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mutex.h is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _MUTEX_
#define	_MUTEX_

#include <stdbool.h>
#include "anyRTOS-conf.h"
#include "timer.h"
#include "src/thread-list.h"

#ifdef __cplusplus
extern "C" {
#endif
    
/** @defgroup mutex Mutual Exclusion Control
  * @{ */ 

/** Structure to handle mutual exclusion sections. */
typedef struct mutex_s {    
    priorList_t list;
    thread_t* volatile busy;
} mutex_t;

/** Initializes a mutual exclusion handle.
  * @param mutex: Mutual exclusion handle. */
void mutex_init( mutex_t* mutex );

/** Checks if mutex section is busy. */
static inline bool mutex_isBusy( mutex_t const* mutex ) { return mutex->busy; }

/** Waits until enters in mutual exclusion section.
  * @param mutex: Mutual exclusion handle. */
void mutex_enter( mutex_t* mutex );

/** Exits of mutual exclusion section.
  * @param mutex: Mutual exclusion handle. */
void mutex_exit( mutex_t* mutex );

/** Enters in critical section and waits until enters in  mutual exclusion section.
  * @param mutex: Mutual exclusion handle. */ 
void mutex_enterCritical( mutex_t* mutex );

/** Exits of mutual exclusion and critical sections.
  * @param mutex: Mutual exclusion handle. */ 
void mutex_exitCritical( mutex_t* mutex );

#if !defined( ANYRTOS_BASIC_MODE ) || ( !ANYRTOS_BASIC_MODE )

/** Waits until enters in mutual exclusion section or
  * until the tick counter of a timer gets the task tick.
  * @param mutex: Mutual exclusion handle.
  * @param timer: Timer handler.
  * @retval true:  The mutex is got before the timer gets the task tick.
  * @retval false: The timer gets the task tick before the mutex is got. */
bool mutexTimer_enter( mutex_t* mutex, timer_t* timer );

/** Enters in critical section and waits until enters in mutual exclusion 
  * section or until the tick counter of a timer gets the task tick.
  * @param mutex: Mutual exclusion handle.
  * @param timer: Timer handler.
  * @retval true:  The mutex is got before the timer gets the task tick.
  * @retval false: The timer gets the task tick before the mutex is got. */
bool mutexTimer_enterCritical( mutex_t* mutex, timer_t* timer );

#else

/** Waits until enters in mutual exclusion section.
  * @param mutex: Mutual exclusion handle.
  * @param timer: Timer handler, ignored.
  * @return It always returns true. */
static inline bool mutexTimer_enter( mutex_t* mutex, timer_t* timer ) {
    (void)timer;
    mutex_enter( mutex );
    return true;    
}

/** Enters in critical section and waits until enters in mutual exclusion section.
  * @param mutex: Mutual exclusion handle.
  * @param timer: Timer handler, ignored.
  * @return It always returns true. */
static inline bool mutexTimer_enterCritical( mutex_t* mutex, timer_t* timer ) {
    (void)timer;
    mutex_enter( mutex );
    return true;     
}

#endif /* ANYRTOS_BASIC_MODE */

/** @} */ 

#ifdef __cplusplus
}
#endif

#endif	/* _MUTEX_ */

