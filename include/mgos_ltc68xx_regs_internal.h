#pragma once

#define SET_BIT(a,i)    ((a) |= 1<<(i))
#define CLEAR_BIT(a,i)  ((a) &= ~(1<<(i)))
#define IS_SET(a,i)     ((a) & (1<<(i)))
#define IS_CLEAR(a,i)   (((a) & (1<<(i))) == 0)

// Commands
#define WRITE_CONFIG_REGISTER           (0x0001)
#define READ_CONFIG_REGISTER            (0x0002)
#define READ_CELL_REGISTER_A            (0x0004)
#define READ_CELL_REGISTER_B            (0x0006)
#define READ_CELL_REGISTER_C            (0x0008)
#define READ_CELL_REGISTER_D            (0x000A)
#define READ_AUX_REGISTER_A             (0x000C)
#define READ_AUX_REGISTER_B             (0x000E)
#define READ_STATUS_REGISTER_A          (0x0010)
#define READ_STATUS_REGISTER_B          (0x0012)
#define CLEAR_CELL_REGISTER             (0x0711)
#define CLEAR_AUX_REGISTER              (0x0712)
#define CLEAR_STATUS_REGISTER           (0x0713)
#define POLL_CONVERSION_STATUS          (0x0714)
#define DIAGNOSE_MUX                    (0x0715)
#define WRITE_COMM_REGISTER             (0x0721)
#define READ_COMM_REGISTER              (0x0722)
#define START_COMM                      (0x0723)
#define START_CELL_CONVERSION(m, d, c)  (0x0260 | ((uint16_t)m << 7) | ((d ? 1 : 0) << 4) | ((uint16_t)c))
#define START_AUX_CONVERSION(m, c)      (0x0460 | ((uint16_t)m << 7) | ((uint16_t)c))
#define START_STATUS_CONVERSION(m, c)   (0x0468 | ((uint16_t)m << 7) | ((uint16_t)c))

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
