
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * port.h is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * port.h is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _PORTABLE_
#define _PORTABLE_

#include "scheduler.h"

#ifdef __MSP430__

#include <msp430.h>
#include <intrinsics.h>

#if __MSP430X__ & (__MSP430_CPUX_TARGET_SR20__ | __MSP430_CPUX_TARGET_ISR20__)
typedef unsigned long int __attribute__((__a20__)) register_t;
#else /* any SR20 */
typedef unsigned int register_t;
#endif /* any SR20 */

typedef struct context_s {
    register_t sp; /**< r1 stack pointer */
    register_t sr; /**< r2 status register */
    register_t r4; /**< Used by gcc as pointer to stack frame. */
    register_t r5;
    register_t r6;
    register_t r7;
    register_t r8;
    register_t r9;
    register_t r10;
    register_t r11;
    register_t pc;  /**< return address */
} context_t;

#if 0

/** API function that change the context. */
static inline void portable_changeContext(  thread_t* volatile* running, thread_t* th  ) {
    if ( !setjmp( (*running)->portable.context ) ) {
        *running = th;
        longjmp( (*running)->portable.context, 1 );
    }   
}

/** API function that prepares a thread to be invoked. */
static inline void portable_initContext( threadInfo_t const* info ) {
    if ( setjmp( info->th->portable.context ) ) {
        __asm__ __volatile__ ( "mov.w r10, r15   \n" );
        __asm__ __volatile__ ( "mov.w r11, r0    \n" );                        
        for(;;);
    }
    context_t *context = (context_t *)info->th->portable.context;
    context->r10 = (unsigned)info->param;
    context->r11 = (unsigned)info->process;
    context->sr = GIE;
    context->sp = (unsigned)(&info->stack[info->size/sizeof(stack_t)]); 
}

#else

#define portable_changeContext( running, th ) {         \
    if ( !setjmp( (*running)->portable.context ) ) {    \
        *running = th;                                  \
        longjmp( (*running)->portable.context, 1 );     \
    }                                                   \
}

#define portable_initContext( info ) {                                  \
    if ( setjmp( info->th->portable.context ) ) {                       \
        __asm__ __volatile__ ( "mov.w r10, r15   \n" );                 \
        __asm__ __volatile__ ( "mov.w r11, r0    \n" );                 \
        for(;;);                                                        \
    }                                                                   \
    context_t *context = (context_t *)info->th->portable.context;       \
    context->r10 = (unsigned)info->param;                               \
    context->r11 = (unsigned)info->process;                             \
    context->sr = GIE;                                                  \
    context->sp = (unsigned)(&info->stack[info->size/sizeof(stack_t)]); \
}

#endif

/** API function that enable IRQ. */
static inline void portable_eint( void ) { __eint(); }

/** API function that disable IRQ. */
static inline void portable_dint( void ) { __dint(); }

#else

#error "Unknown MCU" 

#endif

#endif /* _PORTABLE_ */
