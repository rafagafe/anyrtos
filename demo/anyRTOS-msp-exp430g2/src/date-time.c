
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * date-time.c is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * date-time.c is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include <stdlib.h>
#include "date-time.h"


/* ------------------------------------------------------------------------ */
/* ----------------------------------------------- Internal prototypes: --- */
/* ------------------------------------------------------------------------ */
static uint8_t _getMonthDaysQty( dateTime_t const* dt );


/* ------------------------------------------------------------------------ */
/* -------------------------------------------------- Public functions: --- */
/* ------------------------------------------------------------------------ */
/* Set initial values.  */
void dateTime_init( dateTime_t*  dt ) {
    static dateTime_t const init  = {
        .century = 21, .year = 15, .month = january, 
        .day = 1, .hour = 0, .min = 0, .sec = 0
    };
    *dt = init;
}

/* Calculate the year in long format from dateTime_t. */
uint16_t dateTime_getYear( dateTime_t const* dt ) { 
    return dt->year + 100*(dt->century-1);
}

/* Set the century and year from a long format year. ( 2015 -> XXI 15 ). */
void dateTime_setYear( dateTime_t* dt, uint16_t year ) {
    div_t aux = div( year, 100 );
    dt->year = aux.rem;
    dt->century = aux.quot + 1;    
}

/* Check the date and time data are consistent. */
bool dateTime_check( dateTime_t const* dt ) { 
    if ( dt->century == 0 ) return false; 
    if ( dt->year > 99 ) return false;       
    if ( dt->month > december ) return false;             
    if ( dt->day > _getMonthDaysQty( dt ) ) return false;           
    if ( dt->hour > 23 ) return false;                                            
    if ( dt->min > 59 ) return false;                                          
    if ( dt->sec > 59 ) return false;                                         
    return true;
}

/* Increase the time one second. */
void dateTime_incSec( dateTime_t*  dt ) { 
    if ( ++dt->sec <= 59 ) return;
    dt->sec = 0;
    if ( ++dt->min <= 59 ) return;
    dt->min = 0;
    if ( ++dt->hour <= 23 ) return;
    dt->hour = 0;
    if ( ++dt->day <= _getMonthDaysQty( dt ) ) return;
    dt->day = 1;
    if ( ++dt->month <= december ) return;
    dt->month = january;
    if ( ++dt->year <= 99 ) return;
    dt->year = 0;
    dt->century++;
}

/* Check whether a date is later than another date. */
bool dateTime_isGreaterOrEqual( dateTime_t const* a, dateTime_t const* b  ) {
    if ( a->century > b->century ) return true;
    if ( a->century < b->century ) return false;
    if ( a->year > b->year ) return true;
    if ( a->year < b->year ) return false;
    if ( a->month > b->month ) return true;
    if ( a->month < b->month ) return false;
    if ( a->day > b->day ) return true;
    if ( a->day < b->day ) return false;
    if ( a->hour > b->hour ) return true;
    if ( a->hour < b->hour ) return false;
    if ( a->min > b->min ) return true;
    if ( a->min < b->min ) return false;
    if ( a->sec > b->sec ) return true;
    if ( a->sec < b->sec ) return false;
    return true;
}



/* ------------------------------------------------------------------------ */
/* ------------------------------------------------- Private functions: --- */
/* ------------------------------------------------------------------------ */
/** Check whether it is a leap year. */
static bool _isLeapYear( dateTime_t const* dt ) {
    uint16_t year = dateTime_getYear( dt );
    if ( year & 0x3 )      return false; 
    if ( !( year % 400 ) ) return true;
    if ( !( year % 100 ) ) return false;
    return true;    
}

/** Gets how many days in the month. */
static uint8_t _getMonthDaysQty( dateTime_t const* dt ) {
    static uint8_t const monthDaysQty[] = { 
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 
    };    
    if ( ( dt->month == february ) && _isLeapYear( dt ) ) return 29;	
    return monthDaysQty[dt->month];  
}

/* ------------------------------------------------------------------------ */
