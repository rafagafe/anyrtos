
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * adc.c is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * adc.c is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "msp-exp430g2-conf.h"

#ifdef HAL_HAS_ADC

#include <msp430.h>
#include "anyRTOS.h"

static event_t _endOfConversion;
static mutex_t _busy;

/* Initilizes this module. */
void adc_init( void ) {
    event_init( &_endOfConversion );
    mutex_init( &_busy );
}

/* Starts an ADC conversion and wait until the converion finishes. */
uint16_t adc_get( unsigned channel ) {
    mutex_enterCritical( &_busy );
    channel <<= 12;
    ADC10CTL0 = ADC10SHT_3 | ADC10ON | ADC10IE;
    ADC10CTL1 = ADC10SSEL_0 + channel;
    ADC10CTL0 |= ENC | ADC10SC; 
    event_wait( &_endOfConversion );
    uint16_t result = ADC10MEM;
    mutex_exitCritical( &_busy );
    return result;      
}

/** ADC ISR: */  
__attribute__( ( __interrupt__( ADC10_VECTOR ) ) ) 
static void _isr( void ) {
    ADC10CTL0 &= ~ADC10ON;
    event_notify( &_endOfConversion );    
}

#endif /* HAL_HAS_ADC */

