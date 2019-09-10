#pragma once

#include "mgos.h"

void crc15_init();
uint16_t crc15_calculate(uint8_t* buffer, int byteCount);

void add_pec(uint8_t *buffer, size_t byteCount);
bool validate_pec(uint8_t *buffer, size_t byteCount);
