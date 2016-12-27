
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * thread-list.h is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * thread-list.h is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _THREAD_LIST_
#define _THREAD_LIST_

#include <stddef.h>
#include <stdbool.h>
#include "port-def.h"
#include "anyRTOS-conf.h"

/** Type for priority of tasks. */
typedef uint_fast8_t prior_t ;

/** Type for nested critical section counter. */
typedef uint_fast8_t crtcl_t;

/** Checks if a timer tick is later than another timer tick.
  * @retval true a >= b
  * @retval false a < b  */
 static inline bool tick_isOver( tick_t a, tick_t b ) {
    return ( ( a - b ) > ( (tick_t)-1 >> 1 ) )? false: true;
}

#if !( defined(ANYRTOS_BASIC_MODE) && ANYRTOS_BASIC_MODE )

/** Structure that the scheduler uses to can handle threads. */
typedef struct thread_s {
    struct thread_s* nextPr;
    struct thread_s* nextTk;
    struct thread_s** prevPr;
    struct thread_s** prevTk;
    tick_t tick;
    port_t portable;
    crtcl_t critical;
    prior_t prior;
} thread_t;

/** Initializes a thread handler.
  * @param th: Thread handle.
  * @param prior: Priority of thread. */
static inline void thread_init( thread_t* th, prior_t prior ) {
    th->prior = prior;
    th->tick = (tick_t)0;
    th->critical = 0;
    th->nextPr = (thread_t*)0;
    th->nextTk = (thread_t*)0;
    th->prevPr = (thread_t**)0;
    th->prevTk = (thread_t**)0;
}

/** Checks if a timer tick is later than another timer tick of two threads.
  * @retval true a >= b
  * @retval false a < b  */
static inline bool thread_isOver( thread_t const* a, thread_t const* b ) {
    return tick_isOver( a->tick, b->tick );
}

/** A thread is pointed inside a list sorted by tick. 
  * @param th: Thread handle.
  * @param ptr: Previous pointer. */
static inline void thread_pointedByTickList( thread_t* th, thread_t** ptr ) {
    *ptr = th;
    th->prevTk = ptr;
}

/** A thread is not pointed inside a list sorted by tick. 
  * @param th: Thread handle. */
static inline void thread_unPointedByTickList( thread_t* th ) {
    th->prevTk = (thread_t**)0;
}

/** Removes a thread form a list sorted by tick. 
  * @param th: Thread handle. */ 
static inline void thread_removeFromTickList( thread_t* th ) {
    if ( th->prevTk <= (thread_t**)1 ) return;
    *th->prevTk = th->nextTk;
    th->prevTk = (thread_t**)1;
    return;
}


/** Checks if a thread has been removed form a tick list.
  * @param th: Thread handle. */ 
static inline bool thread_isRemovedFromTickList( thread_t* th ) {
    return th->prevTk == (thread_t**)1;
}

/** A thread is pointed inside a list sorted by priority. 
  * @param th: Thread handle.
  * @param ptr: Previous pointer. */
static inline void thread_pointedByPriorList( thread_t* th, thread_t** ptr ) {
    *ptr = th;
    th->prevPr = ptr;
}

/** A thread is not pointed inside a list sorted by priority. 
  * @param th: Thread handle. */
static inline void thread_unPointedByPriorList( thread_t* th ) {
    th->prevPr = (thread_t**)0;
}

/** Removes a thread form a list sorted by priority. 
  * @param th: Thread handle. */ 
static inline void thread_removeFromPriorList( thread_t* th ) {
    if ( !th->prevPr ) return;
    *th->prevPr = th->nextPr;
    th->prevPr = (thread_t**)0;
    return;
}

#else

/** Structure that the scheduler uses to can handle threads. */
typedef struct thread_s {
    union {
        struct thread_s* nextPr;
        struct thread_s* nextTk;
    };
    tick_t tick;
    port_t portable;
    uint8_t critical;
    uint8_t prior;
} thread_t;

/** Initializes a thread handler.
  * @param th: Thread handle.
  * @param prior: Priority of thread. */
static inline void thread_init( thread_t* th, prior_t prior ) {
    th->prior = prior;
    th->tick = (tick_t)0;
    th->critical = 0;
    th->nextPr = (thread_t*)0;
}

/** Checks if a timer tick is later than another timer tick of two threads.
  * @retval true a >= b
  * @retval false a < b  */
static inline bool thread_isOver( thread_t const* a, thread_t const* b ) {
    return tick_isOver( a->tick, b->tick );
}

/** A thread is not pointed inside a list sorted by tick. 
  * @param th: Thread handle. */
static inline void thread_pointedByTickList( thread_t* th, thread_t** ptr ) {
    *ptr = th;
}

/** A thread is not pointed inside a list sorted by tick. 
  * @param th: Thread handle. */
static inline void thread_unPointedByTickList( thread_t* th ) { (void)th; }

/** Removes a thread form a list sorted by tick. 
  * @param th: Thread handle. */
static inline void thread_removeFromTickList( thread_t* th ) { (void)th; }

static inline void thread_pointedByPriorList( thread_t* th, thread_t** ptr ) {
    *ptr = th;
}

/** A thread is pointed inside a list sorted by priority. 
  * @param th: Thread handle.
  * @param ptr: Previous pointer. */
static inline void thread_unPointedByPriorList( thread_t* th ) { (void)th; }

/** Removes a thread form a list sorted by priority. 
  * @param th: Thread handle. */ 
static inline void thread_removeFromPriorList( thread_t* th ) { (void)th; }

#endif


/* ------------------------------------------------------------------------ */

/** @defgroup thread-prior-list  Thread Priority List.
  * Linked list sorted by priority. 0 is the highest priority. 
  * @{ */

/** A list is defined by a pointer to first element.
  * If it is a null pointer indicates that the list is empty. */
typedef struct priorList_s {
    thread_t* first;
} priorList_t;

/** Empties a thread list.
  * @param list: The list handler. */
static inline void priorList_flush( priorList_t* list ) {
    list->first = (thread_t *)0;
}

/** Checks if a list is empty.
  * @param list: The list handler.
  * @retval true if the list is empty.
  * @retval false in other cases. */
static inline bool priorList_isEmpty( priorList_t const* list ) {
    return !list->first;
}

/** Calculates how many threads are in a list.
  * @param list: The list handler.
  * @return The threads quantity. */
static inline unsigned priorList_calcQty( priorList_t const* list ) {
    unsigned qty = 0;
    for( thread_t* i = list->first; i; ++qty, i = i->nextPr );
    return qty;
}

/** Puts a thread in a list sorted by priority.
  * @param list: The list handler.
  * @param th: Thread handler to be put. */
static inline void priorList_put( priorList_t* list, thread_t* th ) {
    if ( priorList_isEmpty( list ) )  {
        list->first = th;
        thread_pointedByPriorList( th, &list->first );
    }
    else {
        thread_t* i;
        for( i = list->first; i->nextPr && (i->nextPr->prior < th->prior); i = i->nextPr );
        th->nextPr = i->nextPr;
        thread_pointedByPriorList( th, &i->nextPr );
    }
}

/** Gets the first thread of a list.
  * @param list: The list handler.
  * @retval Pointer to gotten thread if success.
  * @retval Null pointer if the list was empty. */
static inline thread_t* priorList_get( priorList_t *list ) {
    if ( priorList_isEmpty( list ) ) return (thread_t *)0;
    thread_t *retVal = list->first;
    list->first = list->first->nextPr;
    thread_unPointedByPriorList( retVal );
    thread_removeFromTickList( retVal );
    return retVal;
}


/** @ } */

/* ------------------------------------------------------------------------ */

/** @defgroup thread-tick-list  Thread Tick List.
  * Linked list sorted by timer tick.
  * @{ */

/** A list is defined by a pointer to first element.
  * If it is a null pointer indicates that the list is empty. */
typedef struct tickList_s {
    thread_t* first;
} tickList_t;

/** Empties a thread list.
  * @param list: The list handler. */
static inline void tickList_flush( tickList_t* list ) {
    list->first = (thread_t *)0;
}

/** Checks if a list is empty.
  * @param list: The list handler.
  * @retval true if the list is empty.
  * @retval false in other cases. */
static inline bool tickList_isEmpty( tickList_t const* list ) {
    return !list->first;
}

/** Calculates how many threads are in a list.
  * @param list: The list handler.
  * @return The threads quantity. */
static inline unsigned tickList_calcQty( tickList_t const* list ) {
    unsigned qty = 0;
    for( thread_t* i = list->first; i; ++qty, i = i->nextTk );
    return qty;
}

/** Puts a thread in a list sorted by timer tick.
  * @param list: The list handler.
  * @param th: Thread handler to be put. */
static inline void tickList_put( tickList_t* list, thread_t* th ) {  
    if ( tickList_isEmpty( list ) || thread_isOver( list->first, th ) ) {
        th->nextTk = list->first;
        thread_pointedByTickList( th, &list->first ); 
        return;            
    }    
    for( thread_t* i = list->first;; i = i->nextTk )    
        if ( !i->nextTk || thread_isOver( i->nextTk, th ) ) {
            th->nextTk = i->nextTk;
            thread_pointedByTickList( th, &i->nextTk ); 
            return;            
        }           
}

/** Gets the first thread of a list if its timer tick matches. 
  * @param list: The list handler.
  * @retval Pointer to gotten thread if success.
  * @retval Null if the list was empty or its timer tick does not match. */
static inline thread_t* tickList_get( tickList_t *list, tick_t tick ) {
    if ( tickList_isEmpty( list ) ) return (thread_t *)0;
    if ( !tick_isOver( tick, list->first->tick ) ) return (thread_t *)0;
    thread_t *retVal = list->first;
    list->first = list->first->nextTk;
    thread_unPointedByTickList( retVal );
    thread_removeFromPriorList( retVal );
    return retVal;    
}

/** Remove a thread of a list.
  * @param list: The list handler.
  * @param th: Thread handler to be removed.
  * @retval true: If success.
  * @retval false: The trhead is not in list. */
static inline bool threadList_remove( tickList_t* list, thread_t* th ) {
    if ( tickList_isEmpty( list ) ) return false;
    if ( list->first == th ) {
        list->first = th->nextTk;
        thread_unPointedByTickList( th );
        return true;
    }
    for( thread_t* i = list->first; i->nextTk; i = i->nextTk )
        if ( i->nextTk == th ) {
            i->nextTk = th->nextTk;
            thread_unPointedByTickList( th );
            return true;
        }    
    return false;
}

/** @ } */


/* ------------------------------------------------------------------------ */

/** @defgroup thread-queue Linked Queue of Threads
  * A queue is defined by a structure with two pointers.
  * The putting is as fast as the getting.  @{ */

/** Structure to handle queue of threads.
  * It has two pointers for a quick putting and getting. */
typedef struct threadQueue_s {
    thread_t* first; /*< Pointer to the first thread. Null pointer if empty. */
    thread_t* last;  /*< Pointer to the last thread. Null pointer if empty. */
} threadQueue_t;

/** It empties a thread queue.
  * @param list: Thread queue handler. */
static inline void threadQueue_flush( threadQueue_t* queue ) {
    queue->first = queue->last = (thread_t *)0;
}

/** Checks if a thread queue is empty.
  * @param queue: Thread queue handler.
  * @retval true: The queue is empty.
  * @retval false: The queue is not empty. */
static inline bool threadQueue_isEmpty( threadQueue_t const* queue ) {
    return ( queue->first == (thread_t *)0 );
}

/** Checks if a thread queue has less than two threads.
  * @param queue: Thread queue handler.
  * @retval true: The queue has less than two threads.
  * @retval false: The queue has two threads or more. */
static inline bool threadQueue_hasOneOrNone( threadQueue_t const* queue ) {
    return ( queue->first == queue->last );
}

/** Tries to get the first thread of a queue.
  * @param queue: Thread queue handler.
  * @retval The thread pointer if success.
  * @retval Null pointer if the queue was empty. */
static inline thread_t* threadQueue_get( threadQueue_t *queue ) {
    thread_t* first = queue->first;
    if ( !threadQueue_isEmpty( queue ) ) {
        if ( threadQueue_hasOneOrNone( queue ) ) threadQueue_flush( queue );
        else queue->first = queue->first->nextPr;
    }
    return first;
}

/** Puts a thread at the end of a thread queue.
  * @param queue: Thread queue handler.
  * @param th: Thread handler.  */
static inline void threadQueue_put( threadQueue_t *queue, thread_t *th ) {
    th->nextPr = (thread_t *)0;
    if ( threadQueue_isEmpty( queue ) ) queue->first = queue->last = th;
    else queue->last = queue->last->nextPr = th;
}

/** @ } */



/* ------------------------------------------------------------------------ */

/** @defgroup thread-queue-array Array Of Queue Of Threads
  * It defines an array of queue of threads.
  * Each queue has threads with the same priority.
  * The priority of the threads in a list matches with its index in array.
  *  @{ */

/** It empties all thread queues in an array.
  * @param array: Thread queue array.
  * @param size: Lists quantity of array. It cannot be 0. */
static inline void threadQueueArray_flush( threadQueue_t array[], size_t size ) {
    for( ; ; ++array ) {
        threadQueue_flush( array );
        if ( !--size ) return;
    }
}

/** Puts a thread in a queue array.
  * @param array: Thread queue array.
  * @param th: Thread to put.  */
static inline void threadQueueArray_put( threadQueue_t array[], thread_t* th ) {
    threadQueue_put( &array[th->prior], th );
}

/** Puts a list of threads in a queue array.
  * @param array: Thread queue array.
  * @param thl: Thread list to put.  */
static inline void threadQueueArray_putList( threadQueue_t array[], priorList_t* list ) {
    thread_t* th;
    while(( th = priorList_get( list ) ))
        threadQueueArray_put( array, th );
}

/** Gets a thread from an thread list vector.
  * @param array: Thread queue array.
  * @retval The thread if success.
  * @retval Null pointer if the thread list vector was empty. */
static inline thread_t* threadQueueArray_get( threadQueue_t array[], size_t size ) {
    thread_t* th;
    for( ; !( th = threadQueue_get( array ) ); ++array )
        if( !--size ) return (thread_t *)0;
    return th;
}

/** @ } */

#endif /* _THREAD_LIST_ */
