#include "mgos.h"
#include "mgos_ltc68xx_data.h"

struct mgos_ltc68xx_data *mgos_ltc68xx_create_data(size_t chainLength, size_t dataLength)
{
   struct mgos_ltc68xx_data *data = (struct mgos_ltc68xx_data*)malloc(sizeof(*data));
   data->chainLength = chainLength;
   data->dataLength = dataLength;
   data->buffer = (uint8_t*)calloc(4 + chainLength  *(dataLength + 2), sizeof(uint8_t));

   return data;
}

uint8_t *mgos_ltc68xx_get_chip_data(struct mgos_ltc68xx_data *data, size_t chipIndex)
{
   if (data == NULL || chipIndex >= data->chainLength)
      return NULL;

   return data->buffer + 4 + (chipIndex * (data->dataLength + 2));
}

bool mgos_ltc68xx_set_chip_data(struct mgos_ltc68xx_data *data, size_t chipIndex, void *chipData)
{
   if (data == NULL || chipIndex >= data->chainLength || chipData == NULL)
      return false;

   memcpy(data->buffer + 4 + (chipIndex * (data->dataLength + 2)), chipData, data->dataLength);
   return true;
}

void mgos_ltc68xx_free_data(struct mgos_ltc68xx_data *data)
{
   free(data->buffer);
   free(data);
}


struct mgos_ltc68xx_measure_results *mgos_ltc68xx_create_results(size_t chipCount)
{
   struct mgos_ltc68xx_measure_results *results = (struct mgos_ltc68xx_measure_results*)malloc(sizeof(*results));
   results->chipCount = chipCount;
   results->chipResults = (struct mgos_ltc68xx_chip_results*)calloc(chipCount, sizeof(struct mgos_ltc68xx_chip_results));

   return results;
}

void mgos_ltc68xx_free_results(struct mgos_ltc68xx_measure_results *results)
{
   free(results->chipResults);
   free(results);
}
