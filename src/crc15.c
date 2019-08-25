#include "mgos.h"

uint16_t crc15Table[256];
const uint16_t CRC15_POLY = 0x4599;

void crc15_init()
{
    for (int i = 0; i < 256; i++)
    {
        uint16_t remainder = i << 7;
        for (int bit = 8; bit > 0; --bit)
        {
            if (remainder & 0x4000)
            {
                remainder = remainder << 1;
                remainder = remainder ^ CRC15_POLY;
            }
            else
                remainder = remainder << 1;
        }
        crc15Table[i] = remainder & 0xFFFF;
    }
}

uint16_t crc15_calculate(uint8_t* buffer, int byteCount)
{
    int address;
    uint16_t remainder;

    remainder = 16;   //PEC seed
    for (int i = 0; i < byteCount; i++)
    {
        address = ((remainder >> 7) ^ buffer[i]) & 0xff;
        remainder = (remainder << 8) ^ crc15Table[address];
    }
    return remainder * 2;     //The CRC15 has a 0 in the LSB so the final value must be multiplied by 2
}
