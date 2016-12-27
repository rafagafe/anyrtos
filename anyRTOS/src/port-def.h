
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * port-def.h is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * port-def.h is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _PORTABLE_DEF_
#define _PORTABLE_DEF_

#ifdef __MSP430__

#include <setjmp.h>

/** Attribute to increase the effectiveness of the threads. */
#define thread __attribute__(( naked ))

/** Type for stack memory. */
typedef unsigned int stack_t;

/** Type for timer tick. */
typedef unsigned int tick_t;

/** Structure with data portable in threads. */
typedef struct port_s {
    jmp_buf context; /**< MCU context */
} port_t;

#else

#error "Unknown MCU" 

#endif

#endif /* _PORTABLE_DEF_ */
