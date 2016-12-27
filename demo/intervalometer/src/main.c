
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * main.c is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * main.c is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdint.h>
#include "anyRTOS.h"
#include "msp-exp430g2/board-msp-exp430g2.h"
#include "msp-exp430g2/timers.h"
#include "msp-exp430g2/serial-port.h"
#include "msp-exp430g2/adc.h"
#include "prompt/prompt.h"
#include "rtcc/rtcc.h"
#include "intervalometer/intervalometer.h"
#include "switch/switch.h"

enum { MIN_STACK = 20 };

static stack_t _rtcc_stack[MIN_STACK];
static stack_t _prompt_stack[MIN_STACK+5];
static stack_t _inter_stack[MIN_STACK];
static stack_t _switch_stack[MIN_STACK];
static thread_t _th[4];

  static threadInfo_t const _schInfo[] = { 
    { // intervalometer
        .process = intervalometer_task, 
        .param   = (void*)0, 
        .stack   = _inter_stack, 
        .size    = sizeof _inter_stack,
        .prior   = 0, 
        .th      = _th + 0
    },       
    { // Terminal
        .process = prompt_task, 
        .param   = (void*)0, 
        .stack   = _prompt_stack, 
        .size    = sizeof _prompt_stack,
        .prior   = 2, 
        .th      = _th + 1
    },
    { // RTCC
        .process = rtcc_task, 
        .param   = (void*)0, 
        .stack   = _rtcc_stack, 
        .size    = sizeof _rtcc_stack,
        .prior   = 1, 
        .th      = _th + 2
    },
    { // Switch
        .process = switch_task, 
        .param   = (void*)0, 
        .stack   = _switch_stack, 
        .size    = sizeof _switch_stack,
        .prior   = 2, 
        .th      = _th + 3
    },

};

/** Entry point of application. */
int main( void ) {    
    
    /* Configures common drivers: */
    board_init(); 
    scheduler_init();  
    timer_allInit();    
    serial_init();
    adc_init();
    rtcc_init();
    interval_init( _th );
        
    /* Create task section: */  
    unsigned const threadsQty = sizeof _schInfo / sizeof *_schInfo;
    for( unsigned i = 0; i < threadsQty; ++i )
        scheduler_add( _schInfo + i );
    
    /* Run scheduler: */
    scheduler_run();
    
    /* This is the task with the lowest priority: */
    LPM0; 
    
    return 0;   
} 

/* ------------------------------------------------------------------------ */
