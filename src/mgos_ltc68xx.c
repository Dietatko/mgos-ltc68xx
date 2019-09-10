#include "mgos.h"
#include "mgos_spi.h"

#include "crc15.h"
#include "mgos_ltc68xx.h"

bool mgos_ltc68xx_init(void)
{
   crc15_init();
   return true;
}

struct mgos_spi_txn_config *mgos_ltc68xx1_create_txn_config(int cs, int mode, int freq)
{
  struct mgos_spi_txn_config *txn = malloc(sizeof(*txn));
  txn->cs = cs;
  txn->mode = mode;
  txn->freq = freq;

  return txn;
}
