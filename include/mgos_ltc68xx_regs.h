#pragma once

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
#define START_CELL_CONVERSION(ad, d, c) (0x0260 | ((uint16_t)(ad) << 7) | (((d) ? 1 : 0) << 4) | ((uint16_t)(c)))
#define START_CELL_SELF_TEST(ad, st)    (0x0207 | ((uint16_t)(ad) << 7) | ((uint16_t)(st) << 5))
#define START_AUX_CONVERSION(ad, c)     (0x0460 | ((uint16_t)(ad) << 7) | ((uint16_t)(c)))
#define START_AUX_SELF_TEST(ad, st)     (0x0407 | ((uint16_t)(ad) << 7) | ((uint16_t)(st) << 5))
#define START_STATUS_CONVERSION(ad, c)  (0x0468 | ((uint16_t)(ad) << 7) | ((uint16_t)(c)))
#define START_STATUS_SELF_TEST(ad, st)  (0x040F | ((uint16_t)(ad) << 7) | ((uint16_t)(st) << 5))
#define CLEAR_CELL_REGISTER             (0x0711)
#define CLEAR_AUX_REGISTER              (0x0712)
#define CLEAR_STATUS_REGISTER           (0x0713)
#define POLL_CONVERSION_STATUS          (0x0714)
#define DIAGNOSE_MUX                    (0x0715)
#define WRITE_COMM_REGISTER             (0x0721)
#define READ_COMM_REGISTER              (0x0722)
#define START_COMM                      (0x0723)

#define LTC68XX_ADC_OPTION_FAST         0
#define LTC68XX_ADC_OPTION_SLOW         1
#define LTC68XX_ADC_MODE_FAST           1
#define LTC68XX_ADC_MODE_NORMAL         2
#define LTC68XX_ADC_MODE_FILTERED       3
#define LTC68XX_ADC_MODE(o, m)          (((o) << 2) | (m))
