
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

/* ------------------------------------------------------------------------ */
/** @file  date-time.c
  * @date   04/11/2015
  * @author Rafa García.                                                    */
/* ------------------------------------------------------------------------ */

#include <stdlib.h>
#include "date-time.h"

/* Sets a date-time a initial value. */
void dateTime_init( dateTime_t* dt ) {
    /* 18:34.59 29/01/2016 */
    static dateTime_t const defaultDateTime = {
        .century = 21, .year = 16, .month = JANURY, .day = 29,
        .hour = 18, .minute = 34, .second  = 59        
    };
    *dt = defaultDateTime;
}

/* Checks that all members are in range. */
bool dateTime_checkRanges( dateTime_t const* dt ) {
    if ( dt->second >= 60 ) return false;
    if ( dt->minute >= 60 ) return false;
    if ( dt->hour >= 24 ) return false;
    if ( dt->month > DECEMBER ) return false;
    if ( dt->month < JANURY ) return false;
    if ( !dt->day ) return false;
    if ( dt->year >= 99 ) return false;
    if ( !dt->century ) return false;
    if ( dt->day > dateTime_daysInMonth( dt ) ) return false;
    return true;
}

/* Sets to date-time the century and year from a long format year. */
void dateTime_setYear( dateTime_t* dt, unsigned year ) {
    div_t tmp = div( year, 100 );
    dt->year = tmp.rem;
    dt->century = 1 + tmp.quot;    
}

/* Reads the centure the year inside century and translate in long format year. */
unsigned dateTime_getYear( dateTime_t const* dt ) {
    return dt->year + 100 * ( dt->century - 1 );
}

/* Checks if a date-time is later or equal than other. */
bool dateTime_isLaterOrEqual( dateTime_t const* a, dateTime_t const* b ) {
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
    if ( a->minute > b->minute ) return true;
    if ( a->minute < b->minute ) return false;
    if ( a->second >= b->second ) return true;
    return false;
}

/* Calculates if the year in a date-time is a leap-year. */
bool dateTime_isLeapYear( dateTime_t const* dt ) {
    unsigned year = dateTime_getYear( dt );
    if ( year % 4 ) return false;
    if ( (year % 100) && !(year % 400) ) return false;
    return true;
}

/** Lookup table days quantity of months. */
static uint8_t const _daysInMonth[] = {
    0,  /**< It is a padding. */
    31, /**< Days in JANURY. */
    28, /**< Days in FEBRUARY. */
    31, /**< Days in MARCH. */
    30, /**< Days in APRIL. */
    31, /**< Days in MAY. */
    30, /**< Days in JUNE. */
    31, /**< Days in JULY. */
    31, /**< Days in AUGUST. */
    30, /**< Days in SEPTEMBER. */
    31, /**< Days in OCTOBER. */
    30, /**< Days in NOVEMBER. */
    31  /**< Days in DICEMBER. */
};

/* Calculates how many days has the month in a date-time. */
unsigned dateTime_daysInMonth( dateTime_t const* dt ) {
    if ( dt->month != FEBRUARY ) return (unsigned)_daysInMonth[dt->month];
    if ( dateTime_isLeapYear( dt ) ) return 29;
    return 28;
}

/* Calculates the day of week of a date-time. */
weekDay_t dateTime_calcWeekDay( dateTime_t const* dt ) {
    unsigned year = dateTime_getYear( dt );
    unsigned long days = 365ul * year;
    days += year / 4;
    days -= year / 100;
    days += year / 400;
    for( month_t m = JANURY; m < dt->month; ++m ) days += _daysInMonth[m];
    if ( ( dt->month > FEBRUARY ) && dateTime_isLeapYear( dt ) ) ++days;
    days += dt->day;
    return (weekDay_t)(1+((days+4)%7));
}

/* Calculate the season, Day Light Saving Time. */
season_t dateTime_calcSeason( dateTime_t const* dt, weekDay_t weekday ) {
    if ( ( dt->month < MARCH ) || ( dt->month > OCTOBER ) ) return WINTER;
    if ( ( dt->month > MARCH ) && ( dt->month < OCTOBER ) ) return SUMMER;
    if ( dt->month == MARCH ) {
        unsigned daysToApril = 31 + 1 - dt->day;
        if ( daysToApril >= 7 ) return WINTER;
        unsigned daysToSunday = SUNDAY - weekday;
        if ( daysToSunday > daysToApril  ) return SUMMER;
        if ( weekday != SUNDAY ) return WINTER;
        if ( dt->hour < 2 ) return WINTER;
        return SUMMER;
    }
    /* It's October: */
    unsigned daysToNovember = 31 + 1 - dt->day;
    if ( daysToNovember >= 7 ) return SUMMER;
    unsigned daysToSunday = SUNDAY - weekday;
    if ( daysToSunday > daysToNovember  ) return SUMMER;
    if ( weekday != SUNDAY ) return SUMMER;
    if ( dt->hour < 2 ) return SUMMER;
    if ( dt->hour >= 3 ) return WINTER;
    return NO_KNOWN_SEASON;
}

/* ------------------------------------------------------------------------ */

