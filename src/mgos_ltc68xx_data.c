#include "mgos.h"
#include "mgos_ltc68xx_data.h"

struct mgos_ltc68xx_data *mgos_ltc68xx_create_data(int chainLength, int dataLength)
{
   if (chainLength <= 0 || dataLength <= 0)
      return NULL;
   
   struct mgos_ltc68xx_data *data = (struct mgos_ltc68xx_data*)calloc(1, sizeof(*data));
   data->chainLength = chainLength;
   data->dataLength = dataLength;
   data->buffer = (uint8_t*)calloc(4 + chainLength  *(dataLength + 2), sizeof(uint8_t));

   return data;
}

uint8_t *mgos_ltc68xx_get_chip_data(struct mgos_ltc68xx_data *data, int chipIndex)
{
   if (data == NULL || chipIndex >= data->chainLength)
      return NULL;

   return data->buffer + 4 + (chipIndex  *(data->dataLength + 2));
}

bool mgos_ltc68xx_set_chip_data(struct mgos_ltc68xx_data *data, int chipIndex, void *chipData)
{
   if (data == NULL || chipIndex >= data->chainLength || chipData == NULL)
      return false;

   memcpy(data->buffer + 4 + (chipIndex  *(data->dataLength + 2)), chipData, data->dataLength);
   return true;
}

void mgos_ltc68xx1_free_data(struct mgos_ltc68xx_data *data)
{
   free(data->buffer);
   free(data);
}
