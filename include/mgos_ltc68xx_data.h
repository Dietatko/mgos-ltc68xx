#pragma once

#include "mgos.h"

struct mgos_ltc68xx_data
{
    int chainLength;
    int dataLength;
    uint8_t* buffer;
};

struct mgos_ltc68xx_data* mgos_ltc68xx_create_data(int chainLength, int dataLength);
uint8_t* mgos_ltc68xx_get_chip_data(struct mgos_ltc68xx_data* data, int chipIndex);
bool mgos_ltc68xx_set_chip_data(struct mgos_ltc68xx_data* data, int chipIndex, void* chipData);
void mgos_ltc68xx_free_data(struct mgos_ltc68xx_data* data);
