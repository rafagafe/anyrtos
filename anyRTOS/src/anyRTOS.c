
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * anyRTOS.c is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * anyRTOS.c is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "../anyRTOS.h"
#include "anyRTOS-conf.h"
#include "thread-list.h"
#include "port.h"

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------- Scheduler Control: --- */
/* ------------------------------------------------------------------------ */

#ifndef ANYRTOS_PRIORYTIES_QTY
#define ANYRTOS_PRIORYTIES_QTY 3
#warning "Defined ANYRTOS_PRIORYTIES_QTY 3"
#endif

/* One priority level else is necessary for a background thread. */
#define REALY_PRIOR_QTY ( ANYRTOS_PRIORYTIES_QTY + 1 )

/** The priority of background thread. */
#define LOWEST_PRIOR    ANYRTOS_PRIORYTIES_QTY

/** Thread handle for background thread. */
static thread_t _background;

/** Pointer to running thread. */
static thread_t *volatile _running = &_background;

/** Array of lists by priority of ready threads. */
static threadQueue_t _ready[REALY_PRIOR_QTY];

/* Initializes the scheduler. */
void scheduler_init( void ) {
    portable_dint();
    _running = (thread_t *volatile)&_background; 
    _running->prior = LOWEST_PRIOR;
    _running->critical = 1;      
    threadQueueArray_flush( _ready, REALY_PRIOR_QTY );     
}

/* Adds a new thread to scheduler. */
void scheduler_add( threadInfo_t const* info ) {
    portable_initContext( info );
    thread_init( info->th, info->prior );      
    threadQueueArray_put( _ready, info->th );    
}

/** Change context to the highest priority ready thread. */
static void _jump( void ) { 
    thread_t* th = threadQueueArray_get( _ready, REALY_PRIOR_QTY );
    portable_changeContext( &_running, th );
}

/** Sets the running thread in ready list and jump. */
static void _yieldISR( void ) {   
    threadQueueArray_put( _ready, _running );
    _jump();    
}

/** Enables and disables IRQ. It must be called inside of critical section. */
static void _checkIRQ( void ) {
    portable_eint();
    portable_dint();    
}

/** Sets the running thread in ready list and jump. */
static void _yield( void ) {   
    _yieldISR();  
    _checkIRQ();    
}

/** Puts a thread in a ready queue and yileds if necesary. */
static void _resume( thread_t* th ) {
    threadQueueArray_put( _ready, th );
    if ( th->prior < _running->prior ) _yield();    
}

/** Sets the running thread blocked in a list sorted by prioruty.
  * @param list: The list handler. */
static void _waitInPriorList( priorList_t* list ) {
    priorList_put( list, _running );
    _jump();
    _checkIRQ();
}

/** Resumes the first thread of a list sorted by prioriry.
  * @param list: The list handler. */
static void _resumeFromPriorList( priorList_t* list ) {
    thread_t* th = priorList_get( list );
    if ( th ) _resume( th );   
}

/** Resumes all threads of a list sorted by prioriry.
  * @param list: The list handler. */
static void _resumeFullPriorList( priorList_t* list ) {
    if ( !priorList_isEmpty( list ) ) {
        uint8_t prior = list->first->prior;
        threadQueueArray_putList( _ready, list );
        if ( prior < _running->prior ) _yield();        
    }
}

/** Sets the first thread of a list sorted in ready 
  * state in a interrupt service routine. It does not yield.
  * @param list: List handle
  * @retval true: If the priority of this thread is higher than the running thread.
  * @retval false: In other case.*/
static bool _resumeFromPriorListISR( priorList_t* list ) {
    thread_t* th = priorList_get( list );
    if( !th ) return false;
    threadQueueArray_put( _ready, th );
    return ( th->prior < _running->prior );          
}

/** Sets all threads of a list sorted in ready 
  * state in a interrupt service routine. It does not yield.
  * @param list: List handle
  * @retval true: If the priority of this thread is higher than the running thread.
  * @retval false: In other case.*/
static bool _resumeFullPriorListISR( priorList_t* list ) {
    if ( priorList_isEmpty( list ) ) return false;
    uint8_t prior = list->first->prior;
    threadQueueArray_putList( _ready, list );
    return ( prior < _running->prior );       
}

/** Enter in a critical section in the context of the running thread.*/
static void _enterCritical( void ) {
    portable_dint();
    ++_running->critical;
}

/** Exit of a critical section in the context of the running thread. */
static void _exitCritical( void ) {
    if( !--_running->critical ) portable_eint();
}

/* Starts the scheduler. */
void scheduler_run( void ) {
    _yield();
    _running->critical = 0; 
    portable_eint();
}



/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------ Task Control: --- */
/* ------------------------------------------------------------------------ */

/* Disable interrupts to enter in a critical section in the 
 * context of the running thread. Nested critical sections are allow. */
void task_enterCritical( void ) { _enterCritical(); }

/* Enable interrupts, if required, to leave a critical section. */
void task_exitCritical( void ) { _exitCritical(); }

/* Updates the timestamp of the running thread with a timer. */
void task_updateTick( timer_t const* timer ) {
    _enterCritical();
    _running->tick = timer->tick;
    _exitCritical();    
}

/* Icreases the task tick. */
void task_increaseTimeout( tick_t ticks ) {
    _enterCritical();    
    _running->tick += ticks;    
    _exitCritical();       
}

/* Updates the timestamp of the running thread with a timer. */
void task_setTimeout( timer_t const* timer, tick_t ticks ) {
    _enterCritical();
    _running->tick = timer->tick + ticks;
    _exitCritical();    
}

/* Checks if the counter tick of a timer has been got the task tick. */
bool task_isOver( timer_t const* timer ) {
    _enterCritical();
    bool retVal = tick_isOver( timer->tick, _running->tick );
    _exitCritical();  
    return retVal;
}

/* Gets the priority of the task. */
prior_t task_getPriority( void ) {
    _enterCritical();
    prior_t retVal = _running->prior;
    _exitCritical();  
    return retVal;
}

/* Set a new priority to task. */
prior_t task_setPriority( prior_t prior ) { 
    _enterCritical();
    prior_t retVal = _running->prior;
    _running->prior = prior;
    _exitCritical();  
    return retVal;
}

/* Yields the flow of execution to threads of greater than or equal priority. */
void task_yield( void ) {
    _enterCritical();    
    _yield();
    _exitCritical();
}

/* Yields the flow of execution in interrupt service routines. */
void task_yieldISR( void ) { _yieldISR(); }

/* Sets the running thread in suspended state. */
void task_suspend( void ) {
    _enterCritical();
    _jump(); 
    _exitCritical();    
}

/* Sets a thread in suspended state in ready state. */
void task_resume( thread_t* th ) {
    _enterCritical();
    _resume( th );
    _exitCritical();    
}



/* ------------------------------------------------------------------------ */
/* ----------------------------------------------------- Event Control: --- */
/* ------------------------------------------------------------------------ */

/* Initializes an event. */
void event_init( event_t* event ) { 
    priorList_flush( &event->list );
}

/* Sets the running thread in blocked state until an event occurs. */
void event_wait( event_t* event ) {
    _enterCritical();
    _waitInPriorList( &event->list );
    _exitCritical();
}

/* Sets the highest priority thread blocked by an event in ready state.
 * If the priority of this thread is higher than the running thread it yields.*/
void event_notify( event_t* event ) {
    _enterCritical();
    _resumeFromPriorList( &event->list );
    _exitCritical();
}

/* Sets the highest priority thread blocked by an event in ready 
 * state in a interrupt service routine. It does not yield. */
bool event_notifyISR( event_t* event ) {
    return _resumeFromPriorListISR( &event->list );
}

/* Sets all threads blocked by an event in ready state. If the priority of 
 * the highest priority thread is higher than  the running thread it yields. */
void event_notifyAll( event_t *event ) {
    _enterCritical();
    _resumeFullPriorList( &event->list );
    _exitCritical();
}

/* Sets all threads blocked by an event in ready state in an 
 * interrupt service routine. It does not yield. */
bool event_notifyAllISR( event_t *event ) {
    return _resumeFullPriorListISR( &event->list );
}



/* ------------------------------------------------------------------------ */
/* ------------------------------------------ Mutual Exclusion Control: --- */
/* ------------------------------------------------------------------------ */

/** Tries to enter in mutual exclusion section.
  * @param event: Mutual exclusion handler. */ 
static inline void _enterMutex( mutex_t* mutex ) {
    while( mutex->busy ) _waitInPriorList( &mutex->list );
    mutex->busy = _running;
}

/** Exits of mutual exclusion.
  * @param event: Mutual exclusion handler. */
static inline void _exitMutex( mutex_t* mutex ) {
    mutex->busy = (thread_t*)0;
    _resumeFullPriorList( &mutex->list );    /* FIXME: full? */
}

/* Initializes a mutual exclusion handler. */
void mutex_init( mutex_t* mutex ) {
    priorList_flush( &mutex->list );
    mutex->busy = (thread_t*)0;
}

/* Enters in critical section and tries to enter in mutual exclusion section. */
void mutex_enterCritical( mutex_t* mutex ) {
    _enterCritical();
    _enterMutex( mutex );
}

/* Exits of mutual exclusion and critical sections. */
void mutex_exitCritical( mutex_t* mutex ) {
    _exitMutex( mutex );    
    _exitCritical();    
}

/* Tries to enter in mutual exclusion section. */
void mutex_enter( mutex_t* mutex ) {
    _enterCritical();
    _enterMutex( mutex );
    _exitCritical();    
}

/* Exits of mutual exclusion section. */
void mutex_exit( mutex_t* mutex ) {
    _enterCritical();
    _exitMutex( mutex );
    _exitCritical();        
}



/* ------------------------------------------------------------------------ */
/* ----------------------------------------------------- Timer Control: --- */
/* ------------------------------------------------------------------------ */

/* Initializes a timer handler. */
void timer_init( timer_t* timer ) {
    timer->tick = 0;
    tickList_flush( &timer->list );
} 

/* Increases a tick a timer handler. */
bool timer_tick( timer_t* timer ) {
    ++timer->tick;
    bool yield = false;
    thread_t* th;
    while(( th = tickList_get( &timer->list, timer->tick ) )) {
        threadQueueArray_put( _ready, th );
        yield |= ( th->prior < _running->prior );        
    }
    return yield;
}

/** Sets the running thread in blocked state until an timer event occurs.
  * More than one thread can be blocked waiting the same timer event.
  * @param timer: Timer handler. */
static void _waitTimer( timer_t* timer ) {
    tickList_put( &timer->list, _running );
    _jump();     
    _checkIRQ();
}

/* Waits until the tick counter of a timer gets the task tick. */
void timer_wait( timer_t* timer ) {
    _enterCritical();
    _waitTimer( timer );
    _exitCritical();    
}

/* Wait N ticks of a timer from last timestamp updating. */
void timer_shift( timer_t* timer, tick_t ticks ) {
    _enterCritical();
    tick_t tmp = _running->tick;
    _running->tick += ticks;
    _waitTimer( timer );
    _running->tick = tmp;
    _exitCritical();
}

/* Wait N ticks of a timer from last timestamp updating.
 * After the waiting. Increase the timestamp N ticks. */
void timer_period( timer_t* timer, tick_t ticks ) {
    _enterCritical();
    _running->tick += ticks;
    _waitTimer( timer );
    _exitCritical();
}

/* Waits N ticks from now on. */
void timer_delay( timer_t* timer, tick_t ticks ) {
    _enterCritical();
    tick_t tmp = _running->tick;
    _running->tick = ticks + timer->tick;
    _waitTimer( timer );
    _running->tick = tmp;
    _exitCritical();
}

/* Resume a thread blocked by a timer. */
bool timer_abort( timer_t* timer, thread_t* th ) {
    _enterCritical();
    bool retVal = threadList_remove( &timer->list, th );
    if ( retVal ) _resume( th );
    _exitCritical();
    return retVal;    
}


/* Turn on the timer and uodate the tick.*/
__attribute__(( weak )) void timer_on( timer_t const* timer ) {
    (void)timer;
}

/* Turn off the timer.*/
__attribute__(( weak )) void timer_off( timer_t const* timer ) {
    (void)timer;    
}

/* ------------------------------------------------------------------------ */
/* -------------------------------------------------- Smaphore Control: --- */
/* ------------------------------------------------------------------------ */

#if defined(ANYRTOS_USE_SEM) && ANYRTOS_USE_SEM

void sem_init( sem_t* sem ) {
    priorList_flush( &sem->list );
    sem->state = SEM_GREEN;
}

bool sem_isBusy( sem_t const* sem ) {
    _enterCritical();
    bool retVal = sem->state == SEM_RED;
    _exitCritical();    
    return retVal;
}

void sem_wait( sem_t* sem ) {
    _enterCritical();
    if ( sem->state == SEM_RED )
        _waitInPriorList( &sem->list );
    sem->state = SEM_RED;
    _exitCritical();    
}

void sem_signal( sem_t* sem ) {
    _enterCritical();
    sem->state = SEM_GREEN;
    _resumeFromPriorList( &sem->list );
    _exitCritical();    
}

bool sem_signalISR( sem_t* sem ) {
    _enterCritical();
    sem->state = SEM_GREEN;
    bool retVal = _resumeFromPriorListISR( &sem->list );
    _exitCritical();
    return retVal;    
}

#endif /* ANYRTOS_USE_SEM */


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

#if ANYRTOS_BASIC_MODE == 0

static bool _waitPriorTickLists( priorList_t* priorList, tickList_t* tickList ) {
    priorList_put( priorList, _running );
    tickList_put( tickList, _running );
    _jump();  
    return thread_isRemovedFromTickList( _running );    
}

static bool _waitEventTimer( event_t* event, timer_t* timer ) {
    return _waitPriorTickLists( &event->list, &timer->list );
}

static bool _waitMutexTimer( mutex_t* mutex, timer_t* timer ) {
    return _waitPriorTickLists( &mutex->list, &timer->list );
}

/* Waits until an event occurs or until the tick counter of a timer gets the 
 * task tick. */
bool eventTimer_wait( event_t* event, timer_t* timer ) {
    _enterCritical();  
    bool retVal = _waitEventTimer( event, timer );
    _exitCritical();    
    return retVal;
}

/** Enters in critical section and tries to enter in mutual exclusion section.
  * @param event: Mutual exclusion handle. */ 
static inline bool _enterMutexTimer( mutex_t* mutex, timer_t* timer ) {
    if ( !mutex->busy ) {
        mutex->busy = _running;
        return true;
    }
    if ( _waitMutexTimer( mutex, timer ) && !mutex->busy ) {
        mutex->busy = _running;
        return true;
    }
    return false;    
}

/* Waits until enters in mutual exclusion section or
   until the tick counter of a timer gets the task tick. */
bool mutexTimer_enter( mutex_t* mutex, timer_t* timer ) {
    _enterCritical();     
    bool retVal = _enterMutexTimer( mutex, timer );
    _exitCritical();      
    return retVal;
}

/* Enters in critical section and waits until enters in mutual exclusion 
   section or until the tick counter of a timer gets the task tick. */
bool mutexTimer_enterCritical( mutex_t* mutex, timer_t* timer ) {
    _enterCritical();     
    if ( _enterMutexTimer( mutex, timer ) ) return true;
    _exitCritical();      
    return false;    
}

#if defined(ANYRTOS_USE_SEM) && ANYRTOS_USE_SEM

static bool _waitSemTimer( sem_t* sem, timer_t* timer ) {
    return _waitPriorTickLists( &sem->list, &timer->list );
}

bool semTimer_wait( sem_t* sem, timer_t* timer ) {
    bool retVal;
    _enterCritical();
    if ( ( sem->state == SEM_GREEN ) || _waitSemTimer( sem, timer ) ) {
        sem->state = SEM_RED;
        retVal = true;
    }
    else retVal = false;
    _exitCritical();    
    return retVal;
}

#endif /* ANYRTOS_USE_SEM */

#endif

/* ------------------------------------------------------------------------ */

