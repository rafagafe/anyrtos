
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * queue.h is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * queue.h is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _QUEUE_
#define _QUEUE_

#include <stddef.h>
#include <stdint.h>
#include "anyRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup queue Queue of Bytes
  * @{ */

/** Structure to handle queues. */
typedef struct queue_s {
    event_t input, output;
    mutex_t putting, getting;
    size_t first, last, size;
    size_t volatile qty;
    uint8_t* data;
} queue_t;

/** Initializes a queue.
  * @param queue: Queue handler.
  * @param memory: Pointer to memory space for queue.
  * @param size: Size in bytes of memory space. */
void queue_init( queue_t* queue, uint8_t memory[], size_t size );

/** Checks if a queue is full.
  * @param queue: Queue handler.
  * @retval true: If queue is full;
  * @retval false: If queue is not full. */
bool queue_isFull( queue_t const* queue );

/** Checks if a queue is empty.
  * @param queue: Queue handler.
  * @retval true: If queue is empty;
  * @retval false: If queue is not empty. */
bool queue_isEmpty( queue_t const* queue );

/** Waits until put a block of memory in a queue.
  * @param queue: Queue handler.
  * @param src: Pointer to block of memory source.
  * @param size: Size of block of memory. */
void queue_put( queue_t* queue, void const* src, size_t size );

/** Waits until get a block of memory from a queue.
  * @param queue: Queue handler.
  * @param dst: Pointer to block of memory destination.
  * @param size: Size of block of memory. */
void queue_get( queue_t* queue, void* dst, size_t size );

/** Waits until put a byte.
  * @param queue: Queue handler.
  * @param data: Byte to be put. */
void queue_put8( queue_t* queue, uint8_t data );

/** Waits until get a byte.
  * @param queue: Queue handler.
  * @return The got byte. */
uint8_t queue_get8( queue_t* queue );

/** Waits until put a null-terminated string.
  * @param queue: Queue handler.
  * @param src: Pointer to source string. */
void queue_putStr( queue_t* queue, char const* src );

/** Waits until get a null-terminated string.
  * @param queue: Queue handler.
  * @param dst: Pointer to destination string. */
void queue_getStr( queue_t* queue, char* dst );

/** Tries to put a block of memory in a queue before a timeout.
  * @param queue: Queue handler.
  * @param timer: Timer handler.
  * @param src: Pointer to block of memory source.
  * @param size: Size of block of memory.
  * @retval true:  The first byte is put before the timer gets the task tick.
  * @retval false: The timer gets the task tick before the first byte is put. */
bool queueTimer_put( queue_t* queue, timer_t* timer, void const* src, size_t size );

/** Tries to get a block of memory from a queue before a timeout.
  * @param queue: Queue handler.
  * @param timer: Timer handler.
  * @param dst: Pointer to block of memory destination.
  * @param size: Size of block of memory.
  * @retval true:  The first byte is got before the timer gets the task tick.
  * @retval false: The timer gets the task tick before the first byte is got. */
bool queueTimer_get( queue_t* queue, timer_t* timer, void* dst, size_t size );

/** Tries to put a byte in a queue before a timeout.
  * @param queue: Queue handler.
  * @param timer: Timer handler.
  * @retval true:  The byte is put before the timer gets the task tick.
  * @retval false: The timer gets the task tick before the byte is put. */
bool queueTimer_put8( queue_t* queue, timer_t* timer, uint8_t data );

/** Tries to get a byte from a queue before a timeout.
  * @param queue: Queue handler.
  * @param timer: Timer handler.
  * @retval true:  The byte is got before the timer gets the task tick.
  * @retval false: The timer gets the task tick before the byte is got. */
bool queueTimer_get8( queue_t* queue, timer_t* timer, uint8_t *data );

/** Tries to put a null-terminated string before a timeout.
  * @param queue: Queue handler.
  * @param timer: Timer handler.
  * @param src: Pointer to source string.
  * @retval true:  The first chracter is put before the timer gets the task tick.
  * @retval false: The timer gets the task tick before the first chracter is put. */
bool queueTimer_putStr( queue_t* queue, timer_t* timer, char const* src );

/** Tries to get a null-terminated string before a timeout.
  * @param queue: Queue handler.
  * @param timer: Timer handler.
  * @param dst: Pointer to destination string.
  * @retval true:  The first chracter is got before the timer gets the task tick.
  * @retval false: The timer gets the task tick before the first chracter is got. */
bool queueTimer_getStr( queue_t* queue, timer_t* timer, char* dst );

/** Return code when ISR operation. */
typedef enum queueCode_e {
    QUEUE_ERROR, QUEUE_DOYIELD, QUEUE_DONOTYIELD
} queueCode_t;

/** Puts a byte in an interrupt service routine.
  * @param queue: Queue handler.
  * @param data: Byte to be put.
  * @retval QUEUE_ERROR: The FIFO is full.
  * @retval QUEUE_DOYIELD: Success, yield is suggested.
  * @retval QUEUE_DONOTYIELD: Success, no yield is suggested. */
queueCode_t queue_put8ISR( queue_t* queue, uint8_t data );

/** Gets a byte in an interrupt service routine.
  * @param queue: Queue handler.
  * @param data: Destination byte.
  * @retval QUEUE_ERROR: The FIFO is empty.
  * @retval QUEUE_DOYIELD: Success, yield is suggested.
  * @retval QUEUE_DONOTYIELD: Success, no yield is suggested. */
queueCode_t queue_get8ISR( queue_t* queue, uint8_t* data );

/** Gets a byte in an interrupt service routine. Only an input event is
  * suggested when the data quantity in queue is less or equal than a threshold.
  * @param queue: Queue handler.
  * @param data: Destination byte.
  * @param thd: Threshold to generate an input event.
  * @retval QUEUE_ERROR: The queue is empty.
  * @retval QUEUE_DOYIELD: Success, yield is suggested.
  * @retval QUEUE_DONOTYIELD: Success, no yield is suggested. */
queueCode_t queue_get8ThdISR( queue_t* fifo, uint8_t* data, unsigned thd );

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* _QUEUE_ */
