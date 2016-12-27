
/* ------------------------------------------------------------------------ */
/** @file  switch.c
  * @date   04/11/2015
  * @author Rafa García.                                                    */
/* ------------------------------------------------------------------------ */

#include "../intervalometer/intervalometer.h"
#include "msp-exp430g2/timers.h"
#include "msp-exp430g2/board-msp-exp430g2.h"

static void(*volatile _func)(void) = interval_run;

/* Set a callback funcion invoked when button is pressed. */
void switch_setFunc( void(*func)(void) ) { _func = func; }

/* Thread that controls the switch state. */
thread void switch_task( void* param ) {    
    for(;;) {        
        board_switch_waitToPressed();
        if ( intervalometer_isRunning() ) interval_stop();
        else _func();
        board_switch_waitToReleased(); 
        timer_on( &timer1 );
        timer_delay( &timer1, 2 );
        timer_off( &timer1 );
    }
}

/* ------------------------------------------------------------------------ */

