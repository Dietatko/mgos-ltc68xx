#pragma once

#include "mgos.h"

struct mgos_ltc68xx_data
{
    size_t chainLength;
    size_t dataLength;
    uint8_t *buffer;
};

struct mgos_ltc68xx_data *mgos_ltc68xx_create_data(size_t chainLength, size_t dataLength);
uint8_t *mgos_ltc68xx_get_chip_data(struct mgos_ltc68xx_data *data, size_t chipIndex);
bool mgos_ltc68xx_set_chip_data(struct mgos_ltc68xx_data *data, size_t chipIndex, void *chipData);
void mgos_ltc68xx_free_data(struct mgos_ltc68xx_data *data);

struct mgos_ltc68xx_chip_results
{
    uint16_t cells[12];
    uint16_t gpios[5];
    uint16_t internalRef2;
    uint16_t sumOfCells;
    uint16_t dieTemp;
    uint16_t analogSupply;
    uint16_t digitalSupply;
};

struct mgos_ltc68xx_measure_results
{
    size_t chipCount;
    struct mgos_ltc68xx_chip_results *chipResults;
};

struct mgos_ltc68xx_measure_results *mgos_ltc68xx_create_results(size_t chipCount);
void mgos_ltc68xx_free_results(struct mgos_ltc68xx_measure_results *results);
