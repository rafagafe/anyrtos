
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * scheduler.h is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * scheduler.h is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _SCHEDULER_
#define	_SCHEDULER_

#include <stdint.h>
#include <stddef.h>
#include "src/thread-list.h"

#ifdef __cplusplus
extern "C" {
#endif
    
/** @defgroup scheduler Scheduler Control
  * @{ */

/** Initializes the scheduler. */
void scheduler_init( void );

/** Information for adding threads. */
typedef struct threadInfo_s {
    thread_t* th;          /**< Thread handler. */
    void(*process)(void*); /**< Pointer to thread function. */
    void* param;           /**< Parameter for thread. */
    stack_t* stack;        /**< Pointer to stack. */
    size_t size;           /**< Size in bytes of stack. */
    prior_t prior;         /**< Priority of thread. 0 is the highest. */
} threadInfo_t;

/** Adds a new thread to scheduler. */
void scheduler_add( threadInfo_t const* info );

/** Starts the scheduler. */
void scheduler_run( void );

/** @} */             

#ifdef __cplusplus
}
#endif

#endif	/* _SCHEDULER_ */

