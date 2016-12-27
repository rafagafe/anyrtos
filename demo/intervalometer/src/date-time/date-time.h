
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * date-time.h is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * date-time.h is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _DATE_TIME_
#define _DATE_TIME_

#include <stdint.h>
#include <stdbool.h>

/** @defgroup date-time Date and Time  @{ */

/** Enumerates the months. */
typedef enum __attribute__(( packed )) month_e {
    JANURY    = 1,
    FEBRUARY  = 2,
    MARCH     = 3,
    APRIL     = 4,
    MAY       = 5,
    JUNE      = 6,
    JULY      = 7,
    AUGUST    = 8,
    SEPTEMBER = 9,
    OCTOBER   = 10,
    NOVEMBER  = 11,
    DECEMBER  = 12
} month_t;

/** Structure to handle times and dates. */
typedef struct dateTime_s {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    month_t month;
    uint8_t year;    /**< Inside a century, from 0 to 99. */
    uint8_t century; /**< From 1 to 255 */
} dateTime_t;

/** Sets a date-time a initial value.
  * @param dt: date-time handler.*/
void dateTime_init( dateTime_t* dt );

/** Checks that all members are in range.
  * @param dt: date-time handler.*/
bool dateTime_checkRanges( dateTime_t const* dt );

/** Sets to date-time the century and year from a long format year.
  * @param dt: date-time handler.
  * @param year: long format year value. */
void dateTime_setYear( dateTime_t* dt, unsigned year );

/** Reads the centure the year inside century and translate in long format year.
  * @param dt: date-time handler.
  * @return Long format year. */
unsigned dateTime_getYear( dateTime_t const* dt );

/** Checks if a date-time is later or equal than other.
  * @param a: date-time handler.
  * @param b: date-time handler.
  * @retval true: if a is later or equal than b.
  * @retval false: if a is earlier than b. */
bool dateTime_isLaterOrEqual( dateTime_t const* a, dateTime_t const* b );

/** Calculates if the year in a date-time is a leap-year.
  * @param dt: date-time handler.
  * @retval true: if it is leap-year.
  * @retval false: if it is not leap-year. */
bool dateTime_isLeapYear( dateTime_t const* dt );

/** Calculates how many days has the month in a date-time.
  * @param dt: date-time handler.
  * @return The days quantity. */
unsigned dateTime_daysInMonth( dateTime_t const* dt );

/** Enumerates the days of week. */
typedef enum __attribute__(( packed )) weekDay_e {
    MONDAY    = 1,
    TUESDAY   = 2,
    WEDNESDAY = 3,
    THURSDAY  = 4,
    FRIDAY    = 5,
    SATURDAY  = 6,
    SUNDAY    = 7
} weekDay_t;

/** Calculates the day of week of a date-time.
  * @param dt: date-time handler.
  * @return The day of week. */
weekDay_t dateTime_calcWeekDay( dateTime_t const* dt );

/** Indicate of daylight saving time status. */
typedef enum __attribute__(( packed )) season_e {
  NO_KNOWN_SEASON = 0, /**< Between 2:00 and 3:00 of the last Sunday in October. */
  WINTER = 1,          /**< Out of daylight saving time. */
  SUMMER = 2           /**< In daylight saving time. */
} season_t;

/** Calculate de season of a date-time.
  * @param dt: date-time handler.
  * @return The season. */
season_t dateTime_calcSeason( dateTime_t const* dt, weekDay_t weekDay );

/** @ } */

#endif /* _DATE_TIME_ */