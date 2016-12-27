
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * main.h is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * main.h is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "anyRTOS.h"
#include "queue.h"
#include "msp-exp430g2/board-msp-exp430g2.h"
#include "msp-exp430g2/board-msp-exp430g2.h"
#include "msp-exp430g2/timers.h"
#include "msp-exp430g2/serial-port.h"
#include "msp-exp430g2/adc.h"
#include "date-time.h"
#include <stdio.h>

#define _delay( x ) __delay_cycles( (unsigned long int)( HAL_MCLK_FREQ * x ) )

/* --------------------------------------------------- Task prototypes: --- */
static void _led_task( void* param );
static void _term_task( void* param );
static void _clock_task( void* param );
static void _queue_task( void* param );


/* ------------------------------------------- Command line prototypes: --- */
static void _help_command( void );
static void _clear_command( void );
static void _time_command( void );
static void _setTime_command( void );
static void _queueTest_command( void );
static void _timerTest_command( void );
static void _temp_command( void );
static void _switch_command( void );

/* -------------------------------------------------- Memory for tasks: --- */
enum {
    _MIN_STACK   = 20, 
    _LED_STACK   = _MIN_STACK,
    _TERM_STACK  = _MIN_STACK + 5,
    _CLOCK_STACK = _MIN_STACK,
    _QUEUE_STACK = _MIN_STACK,
};
static stack_t _led_stack[_LED_STACK];
static stack_t _term_stack[_TERM_STACK];
static stack_t _clock_stack[_CLOCK_STACK];
static stack_t _queue_stack[_QUEUE_STACK];
static thread_t _th[4];


/* ------------------------------- State and communication between task:--- */
/** This event is used to join _queue_task() and _queueTest_command(). */                                                                              
static event_t _event;
/** This queue is used for _queue_task() to send data to _queueTest_command(). */
static queue_t _queue;
/** Memoru space for _queue. */
static uint8_t _queue_data[4];
/** Bufer to transmit strings by _term_task() and some of its commands. */
static char _str[10];
/** Type of param of _clock_task(). */
typedef struct clock_s {
    dateTime_t dateTime; /**< The current time. */
    bool volatile go;    /**< Indicates if the clock is running. */
} clock_t;
/** Instance of param of _clock_task(). */
static clock_t _clock;



/* --------------------------------------------------------- Constants: --- */
/** Command line. */
typedef struct command_s {
    char const* line;     /**< Indentifier. */
    char const* descript; /**< Explains what command do. */
    void(*command)(void); /**< Pointer to command. */
} command_t;

/** Command line array. */
static command_t const _commands[] = {
    { "help",     "Print this text.",  _help_command                    },
    { "clear",    "Clear screen.",     _clear_command                   },
    { "reset",    "Reset the device.", ((void(*)(void))(RESET_VECTOR))  },
    { "time",     "Print time.",       _time_command                    },
    { "set time", "Enter time.",       _setTime_command                 },
    { "queue",    "Test queue.",       _queueTest_command               },
    { "timer",    "Test timer.",       _timerTest_command               },
    { "temp",     "MCU temperature.",  _temp_command                    },
    { "switch",   "Switch test.",      _switch_command                  },
    { 0 }
};

/** Information for adding threads to scheduler. */
static threadInfo_t const _schInfo[] = {
    { // Blinky led
        .process = _led_task, 
        .param = (void*)BOARD_LED_RED, 
        .stack = _led_stack, 
        .size = sizeof(_led_stack),
        .prior = 0, 
        .th = &_th[0]
    },
    { // Clock
        .process = _clock_task, 
        .param = (void*)&_clock, 
        .stack = _clock_stack, 
        .size = sizeof(_clock_stack),
        .prior = 1, 
        .th = &_th[1]
    },
    { // Terminal
        .process = _term_task, 
        .param = (void*)0, 
        .stack = _term_stack, 
        .size = sizeof(_term_stack),
        .prior = 2, 
        .th = &_th[2]
    },
    { // Queue test
        .process = _queue_task, 
        .param = (void*)0, 
        .stack = _queue_stack, 
        .size = sizeof(_queue_stack),
        .prior = 1, 
        .th = &_th[3]
    },
};

/* ---------------------------------------------- Functions definition: --- */
/** Entry point of application. */
int main( void ) {    
    
    /* Configures common drivers: */
    board_init(); 
    scheduler_init();  
    timer_allInit();
    serial_init();
    adc_init();
    
    /* Configures common objects: */
    event_init( &_event );
    queue_init( &_queue, _queue_data, sizeof(_queue_data) );
    
    /* Create task section: */
    unsigned const threadsQty = sizeof(_schInfo) / sizeof(*_schInfo);
    for( unsigned i = 0; i < threadsQty; ++i )
        scheduler_add( &_schInfo[i] );
    
    /* Run scheduler: */
    scheduler_run();
    
    /* This is the task with the lowest priority: */
    if ( false ) LPM0; 
    else for(;;) {  
        task_enterCritical();
        board_led_toggle( BOARD_LED_GREEN );
        task_exitCritical();
        _delay( 0.1 );
    }
       
    return 0;   
} 

/** Blinking LED task. */
thread static void _led_task( void* param ) {    
    boardLED_t led = (boardLED_t)param;
    task_enterCritical();
    timer_on( &timer0 );
    for(;;) {
        board_led_on( led );
        timer_shift( &timer0, timer0_sec(0.1) );
        board_led_off( led );
        timer_period( &timer0, timer0_sec(1.0) );        
    }     
}

/** Increases the variable seconds periodically. */
thread static void _clock_task( void* param ) {   
    clock_t *clock = (clock_t *)param;
    task_enterCritical();
    dateTime_init( &clock->dateTime );
    clock->go = false;
    timer_on( &timer0 );
    for(;;) {        
        timer_period( &timer0, timer0_sec(1.0) );   
        if ( clock->go ) dateTime_incSec( &clock->dateTime );
    }    
}

/** Receives commands by serial port. */
thread static void _term_task( void* param ) {
    (void)param;
    task_enterCritical();
    serial_line( "\x1B[2J" );
    static char const* const hello =
        "                   ____ _____ ___  ____\n\r"
        "  __ _ _ __  _   _|  _ \\_   _/ _ \\/ ___|\n\r"
        " / _` | '_ \\| | | | |_) || || | | \\___ \\\n\r"
        "| (_| | | | | |_| |  _ < | || |_| |___) |\n\r"
        " \\__,_|_| |_|\\__, |_| \\_\\|_| \\___/|____/\n\r"
        "             |___/\n\r"
        "                            Version 1.0\n\r";    
    serial_line( hello );     
    for(;;) {
        serial_msg( "$ " );        
        serial_getStr( _str, sizeof(_str) );
        if ( _str[0] == '\0' ) continue;
        unsigned i;
        for( i = 0; _commands[i].line; ++i ) {
            if ( !strcmp( _commands[i].line, _str ) ) {
                _commands[i].command();                
                break;
            } 
        }        
        if ( !_commands[i].line ) {
            if ( !strcmp( _str, "hello" ) ) puts( hello );
            else serial_line( "Unknown command." );        
        }
    }
}


/** Send messages by a queue when an event occurs.
  * One of four times fails. */
thread static void _queue_task( void* param ) {
    (void)param;
    unsigned cnt = 0;
    task_enterCritical();
    for( ;; ) {
        /* Wait until queue test command. */
        event_wait( &_event );
        /* Send to queue test command an alphabet char by char. */
        for( char a = 'a'; a <= 'z'; ++a ) queue_put8( &_queue, a ); 
        /* One of four times fails. */
        if ( !( ++cnt & 3 ) ) continue; 
        /* Wait to send a pointer to string. */
        static char const* const message = "Message";
        queue_put( &_queue, &message, sizeof(message) );
        /* Wait to send a string. */
        queue_putStr( &_queue, "anyRTOS" );     
    }    
}

/** Command that communicates with _queue_task(). */
static void _queueTest_command( void ) { 
    serial_line("This is a queue and timeout test.\r\n"
                "It does the test five times.\r\n"
                "One of each four times will fail.");
    
    timer_on( &timer0 );
    task_increaseTimeout( timer0_sec(5.0) ); 
    serial_msg("Press any key to start ... ");
    char ch;
    if ( !serialTimer_get( &timer0, &ch ) ) {
        serial_line("timeout!!");  
        timer_off( &timer0 );
        return;
    }
    serial_endl();
        
    for( unsigned j = 5; j; --j ) {                
        
        /* Notify start new test to _queue_task(). */
        event_notify( &_event );
                    
        /* Receive an alphabet char by char. */
        char a;
        do {
            a = queue_get8( &_queue );
            serial_char( a );
        } while( a != 'z' );
        serial_endl();
        task_setTimeout( &timer0, timer0_sec(0.2) );  
        bool timeout = true;
        do {
            /* Wait to receive a pointer to string or timeout. */
            char const* ptr;
            if ( !queueTimer_get( &_queue, &timer0, &ptr, sizeof(ptr) ) ) break;
            serial_line( ptr );
            /* Wait to receive a string or timeout. */
            if ( !queueTimer_getStr( &_queue, &timer0, _str ) ) break;
            serial_line( _str );
            /* Everything is received in time. */
            timeout = false;
        } while(0);  

        if ( timeout ) serial_line("\t\tTimeout!!");          
                        
    }
    timer_off( &timer0 );
}

/** Command that prints all available commands. */
static void _help_command( void ) {    
    for( command_t const* i = _commands; i->line; ++i ) {
        serial_char(' ');
        serial_msg( i->line );
        serial_msg(": ");
        serial_line( i->descript );      
    }      
}

/** Command that clears the screen. */
static void _clear_command( void ) { serial_line( "\x1B[2J\n\r"); }

/** Command that prints the time. */
static void _time_command( void ) {
    serial_msg("Active: ");
    serial_bool( _clock.go );
    serial_endl();
    static char const* monthNames[] = {
        "January", "February", "March", "April", "May", "June", "July", 
        "August", "September", "October", "November", "December"
    };    
    serial_u08( _clock.dateTime.hour );
    serial_char(':');
    serial_u08( _clock.dateTime.min );
    serial_char(':');
    serial_u08( _clock.dateTime.sec );
    serial_char(' ');
    serial_msg( monthNames[ _clock.dateTime.month] );
    serial_char(' ');
    serial_u8( _clock.dateTime.day );
    serial_msg(", ");
    serial_u16( dateTime_getYear( &_clock.dateTime ) );
    serial_endl();
}
    
/* Command that set the time. */ 
static void _setTime_command( void ) {
    _clock.go = false;
    serial_msg("Year: ");
    serial_getNum( _str, 5 );
    dateTime_setYear( &_clock.dateTime, atoi( _str ) );
    do {        
        serial_msg("Month: ");
        serial_getNum( _str, 3 ); 
        _clock.dateTime.month = atoi( _str ) - 1; 
    } while( _clock.dateTime.month > december );   
    do {   
        serial_msg("Day: ");
        serial_getNum( _str, 3 ); 
        _clock.dateTime.day = atoi( _str );
    } while( !dateTime_check( &_clock.dateTime ) );     
    do {
        serial_msg("Hour: ");
        serial_getNum( _str, 3 ); 
        _clock.dateTime.hour = atoi( _str );
    } while( _clock.dateTime.hour > 23 );
    do {
        serial_msg("Minute: ");
        serial_getNum( _str, 3 ); 
        _clock.dateTime.min = atoi( _str );
    } while( _clock.dateTime.min > 59 );    
    do {
        serial_msg("Second: ");
        serial_getNum( _str, 3 ); 
        _clock.dateTime.sec = atoi( _str );
    } while( _clock.dateTime.sec > 59 );    
    _clock.go = true;
    _time_command();
}

typedef struct {
    unsigned long freq;
    tick_t ticks;
    timer_t* timer;
} timerTest_t;

#ifdef HAL_HAS_TIMER1 

static timerTest_t const _timerTest = {
    .freq = HAL_TIMER1_FREQ,
    .ticks = HAL_TIMER1_FREQ * 0.1,
    .timer = &timer1
};

#else

static timerTest_t const _timerTest = {
    .freq = HAL_TIMER0_FREQ,
    .ticks = HAL_TIMER0_FREQ * 0.1,
    .timer = &timer0
};
#endif

/** Command that tests the timer. */
static void _timerTest_command( void ) {    
    serial_line("This is a timer test.\r\n"
                "It prints the timer tick each 100mS.");     
    serial_msg("Timer Freq: ");
    serial_u16( _timerTest.freq );
    serial_line("Hz.");
    serial_msg("Timer ticks in 100mS: ");
    serial_u16( _timerTest.ticks );
    serial_line(".");
    serial_msg("Enter test time in seconds: ");   
    timer_on( &timer0 );
    task_increaseTimeout( timer0_sec(5.0) );
    size_t len = serialTimer_getNum( &timer0, _str, 4 );
    timer_off( &timer0 );
    if ( !len ) {
        serial_endl();
        return;    
    }
    timer_on( _timerTest.timer );
    for( unsigned j = atoi( _str ); j; --j ) {
        serial_msg("->"); serial_x16( _timerTest.ticks ); serial_endl();
        for( unsigned i = 9; i; --i ) {
            timer_period( _timerTest.timer, _timerTest.ticks );
            serial_x16( _timerTest.timer->tick ); serial_endl();
        }
        timer_period( _timerTest.timer, _timerTest.ticks );
    }
    serial_msg("->"); serial_x16( _timerTest.timer->tick ); serial_endl();
    timer_off( _timerTest.timer );
}

/** Command that prints the MCU temperature. */
static void _temp_command( void ) {
    serial_s16( adc_getTemp() );
    serial_line("C.");
}

/** Copmmad that tests the switch. */
static void _switch_command( void ) {
    if ( board_switch_isReleased() ) {
        serial_msg("The switch is released, wait until press ... ");
        board_switch_waitToPressed();        
        serial_line("OK.");
    }
    serial_msg("The switch is pressed, wait until release ... ");
    board_switch_waitToReleased();
    serial_line("OK.");
}

/* ------------------------------------------------------------------------ */
