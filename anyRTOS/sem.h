
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * sem.h is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * sem.h is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _SEM_
#define	_SEM_

#include <stdbool.h>
#include "anyRTOS-conf.h"
#include "timer.h"
#include "src/thread-list.h"

#ifdef __cplusplus
extern "C" {
#endif
    
/** @defgroup semaphore Semaphore
  * @{ */ 

/** Structure to handle semaphores. */
typedef struct sem_s {
    priorList_t list;
    enum { SEM_RED, SEM_GREEN } volatile state;
} sem_t;

/** Initializes an event.
  * @param sem: Semaphore handler. */
void sem_init( sem_t* sem );

/** Checks if a semaphore is busy.
  * @param sem: Semaphore handler. */
bool sem_isBusy( sem_t const* sem );

/** Waits until a semaphore is not busy.
  * @param sem: Semaphore handler. */
void sem_wait( sem_t* sem );

/** Signals a semaphore.
  * @param sem: Semaphore handler. */
void sem_signal( sem_t* sem );

/** Signals a semaphore.
  * @param sem: Semaphore handler.
  * @retval true: If yield is suggested.
  * @retval false: If yield is not necessary. */
bool sem_signalISR( sem_t* sem );

#if !defined( ANYRTOS_BASIC_MODE ) || ( !ANYRTOS_BASIC_MODE )

/** Waits until a semaphore is not busy or until the 
  * tick counter of a timer gets the task tick.
  * @param sem: Semaphore handler.
  * @param timer: Timer handler.
  * @retval true:  The semaphore occurs before the timer gets the task tick.
  * @retval false: The timer gets the task tick before the semaphore occurs. */
bool semTimer_wait( sem_t* sem, timer_t* timer );

#else

/** Waits until a semaphore is not busy.
  * @param sem: Semaphore handler. 
  * @param timer: Timer handler, ignored.
  * @return It always returns true. */
static inline bool semTimer_wait( sem_t* sem, timer_t* timer ) {
    (void)timer;
    sem_wait( sem );
    return true;
}

#endif /* ANYRTOS_BASIC_MODE */

/** @} */

#ifdef __cplusplus
}
#endif

#endif	/* _JOIN_ */

