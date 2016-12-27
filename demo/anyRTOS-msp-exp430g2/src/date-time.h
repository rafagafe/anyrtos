
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

#include <stdbool.h>
#include <stdint.h>

/** @defgroup date-time Date Time
  * This module defines the type date and implements methods to work with it.
  * @{ */

typedef enum __attribute__ ((packed)) month_e {
    january, february, march, april, may, june, july, 
    august, september, october, november, december
} month_t;

typedef struct dateTime_s {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    month_t month;
    uint8_t year;       /**< Year in century, [0..99]. */
    uint8_t century;
} dateTime_t;

/** Set initial values. */
void dateTime_init( dateTime_t*  dt );

/** Check the date and time data are consistent.
  * @param dt[in]: Data to be checked.
  * @retval true: when consistent.
  * @retval false: In other case. */
bool dateTime_check( dateTime_t const* dt );

/** Increase the time one second.
  * @param dt[inout]: Time to be increased. */
void dateTime_incSec( dateTime_t*  dt );

/** Check whether a date is later than another date.  
  * @param a[in]:
  * @param b[in]:
  * @retval true: when a >= b.
  * @retval false: In other case. */
bool dateTime_isGreaterOrEqual( dateTime_t const* a, dateTime_t const* b  );

/** Calculate the year in long format from dateTime_t.
  * @param dt[in]: Source.
  * @return The year. ( Century XXI year 15 -> 2015 ).  */
uint16_t dateTime_getYear( dateTime_t const* const dt );

/** Set the century and year from a long format year. ( 2015 -> XXI 15 ).
  * @param dt[out] Destination date-time.
  * @param year: long format year. */
void dateTime_setYear( dateTime_t* dt, uint16_t year );


/** @} */

#endif /* _DATE_TIME_ */
