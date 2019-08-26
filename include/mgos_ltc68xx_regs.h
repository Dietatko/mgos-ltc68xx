#pragma once

#define LTC68XX_ADC_OPTION_FAST         0
#define LTC68XX_ADC_OPTION_SLOW         1
#define LTC68XX_ADC_MODE_FAST           1
#define LTC68XX_ADC_MODE_NORMAL         2
#define LTC68XX_ADC_MODE_FILTERED       3
#define LTC68XX_ADC_MODE(o, m)          (((o) << 2) | (m))
