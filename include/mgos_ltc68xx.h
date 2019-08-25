#pragma once

#include "mgos.h"
#include "mgos_spi.h"
#include "mgos_ltc68xx_data.h"

struct mgos_spi_txn_config
{
    char cs;
    char mode;
    int freq;
};

struct mgos_ltc68xx1
{
    struct mgos_spi* spi;
    struct mgos_spi_txn* txn;

    unsigned int chainLength;
};

struct mgos_ltc68xx1* mgos_ltc68xx1_create(struct mgos_spi* spi, struct mgos_spi_txn_config* txn);
void mgos_ltc68xx1_close(struct mgos_ltc68xx1* handle);

bool mgos_ltc68xx1_wake_up(struct mgos_ltc68xx1* handle);
bool mgos_ltc68xx1_exec_cmd(struct mgos_ltc68xx1* handle, uint16_t command);
bool mgos_ltc68xx1_read_reg(struct mgos_ltc68xx1* handle, uint16_t command, struct mgos_ltc68xx_data* data);
bool mgos_ltc68xx1_write_reg_same(struct mgos_ltc68xx1* handle, uint16_t command, uint8_t* registerData, size_t dataLength);
bool mgos_ltc68xx1_write_reg_diff(struct mgos_ltc68xx1* handle, uint16_t command, struct mgos_ltc68xx_data* data);

unsigned int mgos_ltc68xx1_determine_length(struct mgos_ltc68xx1* handle);
bool mgos_ltc68xx1_start_ref(struct mgos_ltc68xx1* handle);
bool mgos_ltc68xx1_stop_ref(struct mgos_ltc68xx1* handle);
/*
bool mgos_ltc68xx1_start_cell_conversion(struct mgos_ltc68xx1* handle, ConversionMode mode, bool dischargePermitted, int cellIndex);
bool mgos_ltc68xx1_start_aux_conversion(struct mgos_ltc68xx1* handle, ConversionMode mode, int channelIndex);
bool mgos_ltc68xx1_start_status_conversion(struct mgos_ltc68xx1* handle, ConversionMode mode, int channelIndex);

bool mgos_ltc68xx1_read_cell_voltages(struct mgos_ltc68xx1* handle, int cellIndex, cell_voltages* data);
bool mgos_ltc68xx1_read_aux_voltages(struct mgos_ltc68xx1* handle, int cellIndex, aux_voltages* data);
*/