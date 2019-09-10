#pragma once

#include "mgos.h"
#include "mgos_spi.h"
#include "mgos_ltc68xx_data.h"
#include "mgos_ltc68xx_regs.h"

struct mgos_spi_txn_config
{
    int cs;
    int mode;
    int freq;
};
