
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * adc.h is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * adc.h is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _ADC_
#define _ADC_

#include "msp-exp430g2-conf.h"

/** @defgroup adc ADC
  * This module implements methods to get information from ADC.
  * @{ */ 

/** Initilizes this module. */
void adc_init( void );

/** Starts an ADC conversion and wait until the converion finishes.
  * @param channel: The ADC channel.
  * @return The conversion in raw value. */
uint16_t adc_get( unsigned channel );

/** Waits until get the CPU temperature.
  * @return Grades centigrades.*/
static inline int16_t adc_getTemp( void ) {
    return -278 + adc_get( 10 );   
}

/** @} */

#endif /* _ADC_ */