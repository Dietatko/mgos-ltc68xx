#pragma once

#include "mgos.h"

void crc15_init();
uint16_t crc15_calculate(uint8_t* buffer, int byteCount);
