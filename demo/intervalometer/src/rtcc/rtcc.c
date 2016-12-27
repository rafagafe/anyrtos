
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * rtcc.c is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * rtcc.c is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "anyRTOS.h"
#include "rtcc.h"
#include "msp-exp430g2/serial-port.h"
#include "../date-time/date-time.h"
#include "msp-exp430g2/timers.h"

/** Invoked callback function when an alarm occurs. Null when no alarm. */
static void(* volatile _func)(void);
/** Event that the threads waits whe is not running. */
static event_t _event;
/** Mutex to access to date-time. */
static mutex_t _mutex;
/** Current day of week. */
static weekDay_t _weekDay;
/** Current day light saving time. */
static season_t _season;
/** Current time. */
static dateTime_t _time;
/** Alarm. */
static dateTime_t _alarm;
/** Indicates if the real time clock calendar is running. */
static bool volatile _go;

/* Initializes this module. */
void rtcc_init( void ) {
    _go = false;
    event_init( &_event );
    mutex_init( &_mutex );
    dateTime_init( &_time );
    dateTime_init( &_alarm );
    _weekDay = dateTime_calcWeekDay( &_time );
    _season = dateTime_calcSeason( &_time, _weekDay );
    _func = (void(*)(void))0;
}

/* Stops the real time clock calendar. */
void rtcc_stop( void ) { _go = false; }

/* Unsets the alarm. */
void rtcc_alarmOff( void ) { _func = (void(* )(void))0; }

/** Enter a date-time by terminal.
  * @param dt: Destination. */
static void _enter( dateTime_t* dt ) {
    do{
        char str[5];
        serial_msg("      Year: ");
        serial_getNum( str, 5 );
        dateTime_setYear( dt, atoi( str ) );
        do {
            serial_msg("     Month: ");
            serial_getNum( str, 3 );
            dt->month = (month_t)atoi( str );
        } while( dt->month < JANURY || dt->month > DECEMBER );
        do {
            serial_msg("       Day: ");
            serial_getNum( str, 3 );
            dt->day = (month_t)atoi( str );
        } while( !dt->day && dt->day > dateTime_daysInMonth( dt ) );
        do {
            serial_msg("      Hour: ");
            serial_getNum( str, 3 );
            dt->hour = atoi( str );
        } while( dt->hour > 23 );
        do {
            serial_msg("    Minute: ");
            serial_getNum( str, 3 );
            dt->minute = atoi( str );
        } while( dt->minute > 59 );
        do {
            serial_msg("    Second: ");
            serial_getNum( str, 3 );
            dt->second = atoi( str );
        } while( dt->second > 59 );
    } while( !dateTime_checkRanges( dt ) );
}

/* Sets the real time clock calendar by terminal. */
void rtcc_enterTime( void ) {
    mutex_enter( &_mutex );
    _enter( &_time ); 
    _weekDay = dateTime_calcWeekDay( &_time );
    _season = dateTime_calcSeason( &_time, _weekDay );    
    mutex_exit( &_mutex );
    rtcc_print();
    _go = true;
    event_notify( &_event );
}

/* Sets an alarm by terminal. */
void rtcc_enterAlarm( void(*func)(void) ) {
    mutex_enter( &_mutex );
    _func = func;
    _enter( &_alarm );
    mutex_exit( &_mutex );
}

/** Prints an unsigned 8-bits with at least two digits
  * @param byte: Data to print. */
static void _u08( uint8_t byte ) {
    if( byte < 10 ) serial_x8( byte );
    else serial_u8( byte );
}

/** Print in terminal a time of a date-time.
  * @param dt: date-time handler. */
static void _printTime( dateTime_t const* dt ) {
    _u08( dt->hour );
    serial_char(':');
    _u08( dt->minute );
    serial_char('.');
    _u08( dt->second );
}

/** Get the name of a month.
  * @param month The month code.
  * @return A null-terminaed string with the name. */
static char const* _monthName( month_t month ) {
    if ( month < JANURY || month > DECEMBER ) return "Wrong Month";
    static char const* const monthNames[] = {
        0, "Janury", "Febrary", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };    
    return monthNames[ month ];
}

/** Print in terminal a date of a date-time.
  * @param dt: date-time handler. */
static void _printDate( dateTime_t const* dt ) {    
    _u08( dt->day );
    serial_char(' ');
    serial_msg( _monthName( dt->month ) );
    serial_char(' ');
    serial_u16( dateTime_getYear( dt ) );
}

/* Prints the date-time of alarm if it is set. */
void* rtcc_printAlarm( void ) {
    mutex_enter( &_mutex );
    if ( _func ) {
        _printTime( &_alarm );
        serial_char(' ');
        _printDate( &_alarm );
        serial_char('.');
        serial_endl();
    }
    void* retVal = _func;
    mutex_exit( &_mutex );
    return retVal;
}

/* Print the time and date by terminal. */
void rtcc_print( void ) {           
    mutex_enter( &_mutex );
    _printTime( &_time );
    serial_char(' ');
    static char const* const weekDaysNames[] = {
      0, 
      "Monday", "Tuesday", "Wednesday",
      "Thursday", "Friday", "Saturday", "Sunday"
    }; 
    serial_msg( weekDaysNames[_weekDay] );
    serial_char(' ');
    _printDate( &_time );
    serial_msg(" (");
    static char const* const seasonNames[] = {
        "Ottom", "Winter", "Summer"
    };    
    serial_msg( seasonNames[_season] );
    serial_line(" time).");
    mutex_exit( &_mutex );
}


/** Icreases by one hour the time. */
static void _incHour( void ) {
    if ( ++_time.hour < 24 ) return;
    _time.hour = 0;
    if ( ++_weekDay > SUNDAY ) _weekDay = MONDAY;
    if ( ++_time.day < dateTime_daysInMonth( &_time ) ) return;
    _time.day = 1;
    if ( ++_time.month <= DECEMBER ) return;
    _time.month = JANURY;
    if ( ++_time.year <= 99 ) return;
    _time.year = 0;
    ++_time.century;
}

/** Decreases by one hour the time. */
static void _decHour( void ) {
    if ( _time.hour ) {
        --_time.hour;
        return;
    }
    _time.hour = 23;
    if ( --_weekDay < MONDAY ) _weekDay = SUNDAY;
    if ( --_time.day ) return;
    if ( --_time.month < JANURY ) {
        _time.month = DECEMBER;
        if ( !_time.year ) {
            _time.year = 99;
            --_time.century;
        }
    }
    _time.day = dateTime_daysInMonth( &_time );
}

/** Icreases by one second the time. */
static void _incSecond( void ) {
    if ( ++_time.second < 60 ) return;
    _time.second = 0;
    if ( ++_time.minute < 60 ) return;
    _time.minute = 0;
    _incHour();
    season_t newSeason = dateTime_calcSeason( &_time, _season );
    if ( newSeason == _season ) return;
    _season = newSeason;
    switch( newSeason ) {
       case WINTER: _decHour(); break;
       case SUMMER: _incHour(); break;
       case NO_KNOWN_SEASON:    break;
    }
}

/** Defines one second in timer 0 ticks. */
#define _ONE_SECOND      timer0_sec(1.0)

/* Thread that runs the real time clock calendar. */
thread void rtcc_task( void* param ) {
    task_enterCritical();
    while( !_go ) event_wait( &_event );
    timer_on( &timer0 );
    /* main loop: */
    for(;;) {
        timer_period( &timer0, _ONE_SECOND );
        while( !_go ) {
            timer_off( &timer0 );
            event_wait( &_event );
            timer_on( &timer0 );
        }
        mutex_enter( &_mutex );
        _incSecond();
        if ( _func && dateTime_isLaterOrEqual( &_time, &_alarm ) ) {
          _func();
          _func = 0;
        }
        mutex_exit( &_mutex );
    } /* end main loop */
}

/* ------------------------------------------------------------------------ */