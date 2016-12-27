
/* ------------------------------------------------------------------------ */
/** @file hal-cfg.h
  * @brief Hardware Abstraction Layer Configuration
  * @date 17/04/2016
  * @author Rafa García.                                                    */
/* ------------------------------------------------------------------------ */

#include "stdbool.h"
#include "stdint.h"
#include "stddef.h"

#ifndef _HAL_CFG_
#define _HAL_CFG_

/** @defgroup hal-cfg Hardware Abstraction Layer Configuration
  * @{ */ 

#define HAL_HAS_TIMER0
#define HAL_HAS_TIMER1
#define HAL_HAS_SERIAL_PORT
#define HAL_HAS_SWITCH_EVENT
#define HAL_HAS_ADC


/** MSP430 clock source fot peripherals. */
typedef enum hal_clkSource_e {
    HAL_MCLK,       /**< Main clock. */        
    HAL_SMCLK,      /**< Sub-main clock. */
    HAL_ACLK,       /**< Auxiliary clock. */
} hal_clkSource_t;

/** MSP430 clock source frequencies. */
enum {
    HAL_MCLK_FREQ  = 1000000ul,
    HAL_SMCLK_FREQ = HAL_MCLK_FREQ,
    HAL_ACLK_FREQ  = 0ul,
};

/** Get the clock source frequency. */
static inline unsigned long hal_getClkSrcFreq( hal_clkSource_t clkSrc ) {
    unsigned long const freq[] = { HAL_MCLK_FREQ, HAL_SMCLK_FREQ, HAL_ACLK_FREQ };
    return freq[clkSrc];
}

/** Configuration for timer0. */
enum {
    HAL_TIMER0_FREQ    = 4,         /**< Desired frequency of timer0 ticks. */
    HAL_TIMER0_CLK_SRC = HAL_SMCLK, /**< Clock source of timer0. */
};

/** Configuration for timer1. */
enum {
    HAL_TIMER1_FREQ    = 40,        /**< Desired frequency of timer0 ticks. */
    HAL_TIMER1_CLK_SRC = HAL_SMCLK, /**< Clock source of timer1. */
};

/** configuration for uart. */
enum {
    HAL_SERIAL_PORT_SRC = HAL_SMCLK, /**< Clock source of uart. */
};


/** @} */

#endif /* _HAL_CFG_ */
