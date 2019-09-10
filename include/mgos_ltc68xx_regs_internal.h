#pragma once

#define SET_BIT(a,i)    ((a) |= 1<<(i))
#define CLEAR_BIT(a,i)  ((a) &= ~(1<<(i)))
#define IS_SET(a,i)     ((a) & (1<<(i)))
#define IS_CLEAR(a,i)   (((a) & (1<<(i))) == 0)

// Configuration Register Group
#define BIT_ADCOPT                      0
#define BIT_DTEN                        1
#define BIT_REFON                       2
#define BIT_GPIO1                       3
#define BIT_GPIO2                       4
#define BIT_GPIO3                       5
#define BIT_GPIO4                       6
#define BIT_GPIO5                       7
#define BIT_GPIO                        BIT_GPIO1

// ADC
#define GET_ADC_OPTION(m)              ((m) >> 2)
#define GET_ADC_MODE(m)                ((m) & 3)
