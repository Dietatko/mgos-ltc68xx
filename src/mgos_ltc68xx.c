#include "mgos.h"
#include "mgos_spi.h"

#include "crc15.h"
#include "mgos_ltc68xx.h"

void add_pec(uint8_t* buffer, size_t byteCount);
bool write_data(struct mgos_ltc68xx1* handle, void* buffer, size_t length);

bool mgos_ltc68xx_init(void)
{
   crc15_init();
   return true;
}

struct mgos_ltc68xx1* mgos_ltc68xx1_create(struct mgos_spi* spi, struct mgos_spi_txn_config* txn_config)
{
   if (spi == NULL || txn_config == NULL)
      return NULL;
   
   struct mgos_ltc68xx1* handle = (struct mgos_ltc68xx1*)calloc(1, sizeof(*handle));
   handle->spi = spi;

   struct mgos_spi_txn* txn = (struct mgos_spi_txn*)calloc(1, sizeof(*txn));
   txn->cs = txn_config->cs;
   txn->mode = txn_config->mode;
   txn->freq = txn_config->freq;
   handle->txn = txn;

   handle->chainLength = 0;

   return handle;
}

void mgos_ltc68xx1_close(struct mgos_ltc68xx1* handle)
{
   free(handle);
}

bool mgos_ltc68xx1_wake_up(struct mgos_ltc68xx1* handle)
{
   if (handle == NULL)
      return false;

   uint8_t tx_data[1] = { 0x99 };

   handle->txn->hd.tx_data = tx_data;
   handle->txn->hd.tx_len = 1;
   handle->txn->hd.dummy_len = 0;
   handle->txn->hd.rx_data = NULL;
   handle->txn->hd.rx_len = 0;

   return mgos_spi_run_txn(handle->spi, false, handle->txn);
}

bool mgos_ltc68xx1_exec_cmd(struct mgos_ltc68xx1* handle, uint16_t command)
{
   if (handle == NULL)
      return false;

   unsigned int data_len = 4 * handle->chainLength;
   // Prepare data for one chip
   uint8_t tx_data[data_len];
   tx_data[0] = (uint8_t)(command >> 8);
   tx_data[1] = (uint8_t)command;
   add_pec(tx_data, 2);

   // Multiply data for all chips
   for (unsigned int i = 1; i < handle->chainLength; i++)
      memcpy(&tx_data[4 * i], tx_data, 4);
   
   handle->txn->hd.tx_data = tx_data;
   handle->txn->hd.tx_len = data_len;
   handle->txn->hd.dummy_len = 0;
   handle->txn->hd.rx_data = NULL;
   handle->txn->hd.rx_len = 0;

   return mgos_spi_run_txn(handle->spi, false, handle->txn);
}

bool mgos_ltc68xx1_read_reg(struct mgos_ltc68xx1* handle, uint16_t command, struct mgos_ltc68xx_data* data)
{
   if (handle == NULL || data == NULL || data->chainLength < handle->chainLength)
      return false;

   uint8_t* buffer = data->buffer;

   buffer[0] = (uint8_t)(command >> 8);
   buffer[1] = (uint8_t)command;
   add_pec(buffer, 2);

   handle->txn->hd.tx_data = buffer;
   handle->txn->hd.tx_len = 4;
   handle->txn->hd.dummy_len = 0;
   handle->txn->hd.rx_data = buffer + 4;
   handle->txn->hd.rx_len = handle->chainLength * (data->dataLength + 2);

   return mgos_spi_run_txn(handle->spi, false, handle->txn);
}

bool mgos_ltc68xx1_write_reg_same(struct mgos_ltc68xx1* handle, uint16_t command, uint8_t* registerData, size_t dataLength)
{
   if (handle == NULL || registerData == NULL)
      return false;

   unsigned int chunkLength = dataLength + 2;
   unsigned int totalLength = 4 + handle->chainLength * chunkLength;
   uint8_t* buffer = (uint8_t*)calloc(totalLength, sizeof(uint8_t));
   
   // Prepare command
   buffer[0] = (uint8_t)(command >> 8);
   buffer[1] = (uint8_t)command;
   add_pec(buffer, 2);

   // Prepare data for one chip
   uint8_t* srcPointer = buffer + 4;
   memcpy(srcPointer, registerData, dataLength);
   add_pec(srcPointer, dataLength);

   // Multiply data for all chips
   uint8_t* copyPointer = srcPointer;
   for(int i = handle->chainLength; i > 1; i--)
   {
      copyPointer = copyPointer + chunkLength;
      memcpy(copyPointer, srcPointer, chunkLength);
   }

   bool success = write_data(handle, buffer, totalLength);
   free(buffer);
   return success;
}

bool mgos_ltc68xx1_write_reg_diff(struct mgos_ltc68xx1* handle, uint16_t command, struct mgos_ltc68xx_data* data)
{
   if (handle == NULL || data == NULL || data->chainLength < handle->chainLength)
      return false;
   
   uint8_t* buffer = data->buffer;
    
   // Prepare command
   buffer[0] = (uint8_t)(command >> 8);
   buffer[1] = (uint8_t)command;
   add_pec(buffer, 2);

   // Calculate PEC for each chunk
   uint8_t* bufferPtr = buffer + 4;
   for(unsigned int i = handle->chainLength; i > 0; i--)
   {
      add_pec(bufferPtr, data->dataLength);
      bufferPtr += (data->dataLength + 2);
   }

   return write_data(handle, buffer, 4 + handle->chainLength * (data->dataLength + 2));
}

void add_pec(uint8_t* buffer, size_t byteCount)
{
    uint16_t pec = crc15_calculate(buffer, byteCount);
    buffer[byteCount] = (uint8_t)(pec >> 8);
    buffer[byteCount + 1] = (uint8_t)pec;
}

bool write_data(struct mgos_ltc68xx1* handle, void* buffer, size_t length)
{
   // Send data
   handle->txn->hd.tx_data = buffer;
   handle->txn->hd.tx_len = length;
   handle->txn->hd.dummy_len = 0;
   handle->txn->hd.rx_data = NULL;
   handle->txn->hd.rx_len = 0;

   return mgos_spi_run_txn(handle->spi, false, handle->txn);
}
