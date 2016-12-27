
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * queue.c is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * queue.c is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "queue.h"
#include "anyRTOS-conf.h"

#if defined(ANYRTOS_USE_QUEUE) && ANYRTOS_USE_QUEUE

/* Initializes a queue.  */
void queue_init( queue_t* queue, uint8_t memory[], size_t size ) {
    event_init( &queue->input );
    event_init( &queue->output );
    mutex_init( &queue->putting );
    mutex_init( &queue->getting );
    queue->first = queue->last = queue->qty = (size_t)0;
    queue->size = size;
    queue->data = memory;
}

/** Checks if a queue is full.
  * @param queue: Queue handler.
  * @retval true: If queue is full;
  * @retval false: If queue is not full. */
static bool _isFull( queue_t const* queue ) {
    return (queue->qty >= queue->size);
}

/* Checks if a queue is full. */
bool queue_isFull( queue_t const* queue ) {
    task_enterCritical();
    bool retVal = _isFull( queue );
    task_exitCritical();
    return retVal;
}

/** Tries to put a byte in a queue.
  * @param queue: Queue handler.
  * @param data: Byte to be put.
  * @retval true If success;
  * @retval false If queue was full. */
static bool _put( queue_t* queue, uint8_t data ) {
    if ( _isFull( queue ) ) return false;
    queue->data[ queue->last ] = data;
    if( ++queue->last >= queue->size )
        queue->last = (size_t)0;
    ++queue->qty;
    return true;
}

/* Waits until put a block of memory. */
void queue_put( queue_t* queue, void const* src, size_t size ) {
    if ( !size ) return;
    mutex_enterCritical( &queue->putting );
    for(;; ++src ) {
        while ( !_put( queue, *(uint8_t*)src ) ) event_wait( &queue->output );
        if ( !--size ) break;
        if ( _isFull( queue ) ) event_notify( &queue->input );
    }
    event_notify( &queue->input );
    mutex_exitCritical( &queue->putting );
}

/** Checks if a queue is empty.
  * @param queue: Queue handler.
  * @retval true: If queue is empty;
  * @retval false: If queue is not empty. */
static bool _isEmpty( queue_t const* queue ) { return !queue->qty; }

/* Checks if a queue is empty. */
bool queue_isEmpty( queue_t const* queue ) {
    task_enterCritical();
    bool retVal = _isEmpty( queue );
    task_exitCritical();
    return retVal;
}

/** Tries to get a byte from a queue.
  * @param queue: Queue handler.
  * @param data: Destination byte.
  * @retval true If success;
  * @retval false If queue was empty. */
static bool _get( queue_t* queue, uint8_t* data ) {
    if ( _isEmpty( queue ) ) return false;
    *data = queue->data[ queue->first ];
    if( ++queue->first >= queue->size ) queue->first = (size_t)0;
    --queue->qty;
    return true;
}

/* Waits until get a block of memory. */
void queue_get( queue_t* queue, void* dst, size_t size ) {
    if ( !size ) return;
    mutex_enterCritical( &queue->getting );
    for(;; ++dst ) {
        while( !_get( queue, dst) ) event_wait( &queue->input );
        if ( !--size ) break;
        if ( _isEmpty( queue ) ) event_notify( &queue->output );
    }
    event_notify( &queue->output );
    mutex_exitCritical( &queue->getting );
}

/* Waits until put a byte. */
void queue_put8( queue_t* queue, uint8_t data ) {
    mutex_enterCritical( &queue->putting );
    while ( !_put( queue, data ) ) event_wait( &queue->output );
    event_notify( &queue->input );
    mutex_exitCritical( &queue->putting );
}

/* Waits until get a byte. */
uint8_t queue_get8( queue_t* queue ) {
    mutex_enterCritical( &queue->getting );
    uint8_t retVal;
    while ( !_get( queue, &retVal ) ) event_wait( &queue->input );
    event_notify( &queue->output );
    mutex_exitCritical( &queue->getting );
    return retVal;
}

/* Waits until put a null-terminated string. */
void queue_putStr( queue_t* queue, char const* src ) {
    mutex_enterCritical( &queue->putting );
    for(;; ++src ) {
        while ( !_put( queue, *src ) ) event_wait( &queue->output );
        if( !*src ) break;
        if ( _isFull( queue ) ) event_notify( &queue->input );
    }
    event_notify( &queue->input );
    mutex_exitCritical( &queue->putting );
}

/* Waits until get a null-terminated string. */
void queue_getStr( queue_t* queue, char* dst ) {
    mutex_enterCritical( &queue->getting );
    for(;; ++dst ) {
        while( !_get( queue, (uint8_t *)dst) ) event_wait( &queue->input );
        if ( !*dst ) break;
        if ( _isEmpty( queue ) ) event_notify( &queue->output );
    }
    event_notify( &queue->output );
    mutex_exitCritical( &queue->getting );
}

/* Tries to get a block of memory from a queue before a timeout. */
bool queueTimer_get( queue_t* queue, timer_t* timer, void* dst, size_t size ) {
    if ( !size ) return true;
    if ( !mutexTimer_enterCritical( &queue->getting, timer ) ) return false;
    bool timeout = false;
    while ( !timeout &&  !_get( queue, dst)  )
        timeout = !eventTimer_wait( &queue->input, timer );
    if ( !timeout ) {
        if ( --size ) for( ++dst;; ++dst ) {
            while ( !_get( queue, dst) ) event_wait( &queue->input );
            if ( !--size ) break;
            if ( _isEmpty( queue ) ) event_notify( &queue->output );
        }
        event_notify( &queue->output );
    }
    mutex_exitCritical( &queue->getting );
    return !timeout;
}

/* Tries to put a block of memory in a queue before a timeout. */
bool queueTimer_put( queue_t* queue, timer_t* timer, void const* src, size_t size ) {
    if ( !size ) return true;
    if ( !mutexTimer_enterCritical( &queue->putting, timer ) ) return false;
    bool timeout = false;
    while ( !timeout &&  !_put( queue, *(uint8_t*)src ) )
        timeout = !eventTimer_wait( &queue->output, timer );
    if ( !timeout ) {
        if ( --size ) for( ++src;; ++src ) {
            while ( !_put( queue, *(uint8_t*)src ) ) event_wait( &queue->output );
            if ( !--size ) break;
            if ( _isFull( queue ) ) event_notify( &queue->input );
        }
        event_notify( &queue->input );
    }
    mutex_exitCritical( &queue->putting );
    return !timeout;
}

/* Waits until put a byte in a queue or until
 * the tick counter of a timer gets the task tick. */
bool queueTimer_put8( queue_t* queue, timer_t* timer, uint8_t data ) {
    if ( !mutexTimer_enterCritical( &queue->putting, timer ) ) return false;
    bool timeout = false;
    while ( !timeout &&  !_put( queue, data ) )
        timeout = !eventTimer_wait( &queue->output, timer );
    if ( !timeout ) event_notify( &queue->input );
    mutex_exitCritical( &queue->getting );
    return !timeout;
}

/* Waits until get a byte from a queue or until
 * the tick counter of a timer gets the task tick. */
bool queueTimer_get8( queue_t* queue, timer_t* timer, uint8_t *data ) {
   if ( !mutexTimer_enterCritical( &queue->getting, timer ) ) return false;
   bool timeout = false;
   while ( !timeout && !_get( queue, data )  )
       timeout = !eventTimer_wait( &queue->input, timer );
   if ( !timeout ) event_notify( &queue->output );
   mutex_exitCritical( &queue->getting );
   return !timeout;
}

/* Tries to put a null-terminated string before a timeout. */
bool queueTimer_putStr( queue_t* queue, timer_t* timer, char const* src ) {
    if ( !*src ) return true;
    if ( !mutexTimer_enterCritical( &queue->putting, timer ) ) return false;
    bool timeout = false;
    while ( !timeout &&  !_put( queue, *src ) )
        timeout = !eventTimer_wait( &queue->output, timer );
    if ( !timeout ) {
        if ( *src ) for( ++src;; ++src ) {
            while ( !_put( queue, *src ) ) event_wait( &queue->output );
            if( !*src ) break;
            if ( _isFull( queue ) ) event_notify( &queue->input );
        }
        event_notify( &queue->input );
    }
    mutex_exitCritical( &queue->putting );
    return !timeout;
}

/* Tries to get a null-terminated string before a timeout. */
bool queueTimer_getStr( queue_t* queue, timer_t* timer, char* dst ) {
    if ( !mutexTimer_enterCritical( &queue->getting, timer ) ) return false;
    bool timeout = false;
    while ( !timeout &&  !_get( queue, (uint8_t*)dst )  )
        timeout = !eventTimer_wait( &queue->input, timer );
    if ( !timeout ) {
        if ( *dst ) for( ++dst;; ++dst ) {
            while( !_get( queue, (uint8_t*)dst) ) event_wait( &queue->input );
            if ( !*dst ) break;
            if ( _isEmpty( queue ) ) event_notify( &queue->output );
        }
        event_notify( &queue->output );
    }
    mutex_exitCritical( &queue->getting );
    return !timeout;
}

/* Puts a byte in an interrupt service routine. */
queueCode_t queue_put8ISR( queue_t* queue, uint8_t data ) {
    if ( !_put( queue, data ) ) return QUEUE_ERROR;
    if ( event_notifyISR( &queue->input ) ) return QUEUE_DOYIELD;
    return QUEUE_DONOTYIELD;
}

/* Gets a byte in an interrupt service routine. */
queueCode_t queue_get8ISR( queue_t* queue, uint8_t *data ) {
    if ( !_get( queue, data ) ) return QUEUE_ERROR;
    if ( event_notifyISR( &queue->output ) ) return QUEUE_DOYIELD;
    return QUEUE_DONOTYIELD;
}

/* Gets a byte in an interrupt service routine.
 * Only an input event is generated when the data quantity in queue
 * is less or equal than a threshold. */
queueCode_t queue_get8ThdISR( queue_t* queue, uint8_t* data, unsigned thd ) {
    if ( !_get( queue, data ) ) return QUEUE_ERROR;
    if ( (queue->qty <= thd) && event_notifyISR( &queue->output ) )
        return QUEUE_DOYIELD;
    return QUEUE_DONOTYIELD;
}

#endif /* ANYRTOS_USE_QUEUE */

/* ------------------------------------------------------------------------ */
