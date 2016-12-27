
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * prompt.c is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * prompt.c is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <msp430.h>
#include "anyRTOS.h"
#include "msp-exp430g2/timers.h"
#include "msp-exp430g2/serial-port.h"
#include "msp-exp430g2/board-msp-exp430g2.h"
#include "msp-exp430g2/adc.h"
#include "../rtcc/rtcc.h"
#include "../intervalometer/intervalometer.h"
#include "../switch/switch.h"

/* ------------------------------------------- Command line prototypes: --- */
static void _help_command( void );
static void _clear_command( void );
static void _alarm_command( void );
static void _printAlarm_command( void );
static void _switch_command( void );
static void _temp_command( void );

/** Command line. */
static struct {
    char const* line;     /**< Indentifier. */
    char const* descript; /**< Explains what command do. */
    void(*command)(void); /**< Pointer to command. */
} const _commands[] = {
    { "help",      "Print this text.",                   _help_command                   },
    { "clear",     "Clear screen.",                      _clear_command                  },
    { "reset",     "Reset the device.",                  ((void(*)(void))(RESET_VECTOR)) },
    { "set time",  "Set the time by terminal.",          rtcc_enterTime                  },
    { "time",      "Print the time.",                    rtcc_print                      },
    { "time stop", "Stop the real time clock calendar.", rtcc_stop                       },
    { "set alarm", "Set an alarm by terminal.",          _alarm_command                  },
    { "alarm",     "Print the alarm.",                   _printAlarm_command             },
    { "alarm off", "Unset the alarm.",                   rtcc_alarmOff                   },
    { "set",       "Enter intervalometer settings.",     interval_enterSettings          },
    { "get",       "Print intervalometer settings.",     interval_printSettings          },
    { "run",       "Start the intervalometer process.",  interval_run                    },
    { "hdr",       "Start the HDR process.",             interval_hdr                    },
    { "stop",      "Stop any running process.",          interval_stop                   },
    { "speeds",    "Print HDR speeds.",                  intervalometer_printSpeeds      },
    { "set sw",    "Set switch function.",               _switch_command                 },
    { "temp",      "Print MCU temperature.",             _temp_command                   },
}; 

static unsigned int const _commandsQty = sizeof _commands / sizeof *_commands;

/* Receives commands by serial port. */
thread void prompt_task( void* param ) {    
    static char const* hello =
        "                   ____ _____ ___  ____\n\r"
        "  __ _ _ __  _   _|  _ \\_   _/ _ \\/ ___|\n\r"
        " / _` | '_ \\| | | | |_) || || | | \\___ \\\n\r"
        "| (_| | | | | |_| |  _ < | || |_| |___) |\n\r"
        " \\__,_|_| |_|\\__, |_| \\_\\|_| \\___/|____/\n\r"
        "             |___/\n\r"
        "                            Version 1.0\n\r";    
    task_enterCritical();
    timer_on( &timer0 );
    timer_delay( &timer0, 1 );
    timer_off( &timer0 );
    serial_line( "\x1B[2J" );
    serial_line( hello );    
    for(;;) {
        serial_msg( "$ " );
        static char str[10];
        serial_getStr( str, sizeof str );
        if ( str[0] == '\0' ) continue;
        unsigned i;
        for( i = 0; i < _commandsQty; ++i ) {
            if ( !strcmp( _commands[i].line, str ) ) {
                _commands[i].command();                
                break;
            } 
        }        
        if ( i == _commandsQty ) {
            if ( !strcmp( "hello", str ) ) serial_line( hello ); 
            else serial_line( "Unknown command." );        
        }
    }
}

/** Command that prints all available commands. */
static void _help_command( void ) {    
    for( unsigned int i = 0; i < _commandsQty; ++i ) {
        serial_char(' ');
        serial_msg( _commands[i].line );
        serial_msg(": ");
        serial_line( _commands[i].descript );      
    }      
}

/** Command that clears the screen. */
static void _clear_command( void ) { serial_line( "\x1B[2J"); }

/** Alarm funcions names. */
static char const* const _funcNames[] = { "Intervalometer", "HDR" };

/** Alarm functions pointers. */
static void (*_funcPtr[])(void) = { interval_run, interval_hdr };

/** Defines the alram function quantity. */
#define _FUNC_QTY ( sizeof(_funcNames) / sizeof(char const*) )

/** Command that sets an alarm by terminal. */
static void _alarm_command( void ) { 
    serial_msg("Select an alarm function [+/-]: ");
    unsigned opt = serial_option( _funcNames, _FUNC_QTY );
    rtcc_enterAlarm( _funcPtr[ opt ] );
}

/** Command that prints the alarm by terminal. */
static void _printAlarm_command( void ) { 
    void* ptr = rtcc_printAlarm();
    if ( !ptr ) return;
    for( unsigned i = 0; i < _FUNC_QTY; ++i ) {
        if ( _funcPtr[i] == ptr ) {
            serial_line( _funcNames[i] );
            return;
        }
    }
}

/** Command that selects a function for switch. */
static void _switch_command( void ) {
    serial_msg("Select a switch function [+/-]: ");
    unsigned opt = serial_option( _funcNames, _FUNC_QTY );
    switch_setFunc( _funcPtr[ opt ] );    
}

/** Command that prints the MCU temperature. */
static void _temp_command( void ) {
    serial_s16( adc_getTemp() );
    serial_line("C.");
}

/* ------------------------------------------------------------------------ */

