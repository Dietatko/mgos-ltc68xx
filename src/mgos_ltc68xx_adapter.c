#include "mgos.h"
#include "mgos_spi.h"
#include "mgos_system.h"

#include "crc15.h"

#include "mgos_ltc68xx.h"
#include "mgos_ltc68xx_regs_internal.h"

#define LAMBDA(c_) ({ c_ _;})

uint32_t adc_time_single[] = {
   0, 201, 405, 33568, 0, 230, 521, 754
};

uint32_t adc_time_multi[] = {
   0, 1113, 2335, 201317, 0, 1288, 3033, 4430
};

uint16_t read_cell_voltage_reg[] = {
   READ_CELL_REGISTER_A,
   READ_CELL_REGISTER_B,
   READ_CELL_REGISTER_C,
   READ_CELL_REGISTER_D,
};

uint16_t read_aux_reg[] = {
   READ_AUX_REGISTER_A,
   READ_AUX_REGISTER_B,
};

uint16_t read_status_reg[] = {
   READ_STATUS_REGISTER_A,
   READ_STATUS_REGISTER_B,
};

int mgos_ltc68xx1_determine_length(struct mgos_ltc68xx1 *handle)
{
   if (handle == NULL)
      return 0;
   
   handle->chainLength = 1;
   return handle->chainLength;
}

bool mgos_ltc68xx1_start_ref(struct mgos_ltc68xx1 *handle)
{
   struct mgos_ltc68xx_data *data = mgos_ltc68xx_create_data(handle->chainLength, 6);

   if (!mgos_ltc68xx1_read_reg(handle, READ_CONFIG_REGISTER, data))
      return false;

   for(int i = handle->chainLength - 1; i >= 0; i--)
   {
      uint8_t *chipData = mgos_ltc68xx_get_chip_data(data, i);
      SET_BIT(chipData[0], 2);
   }

   bool success = mgos_ltc68xx1_write_reg_diff(handle, WRITE_CONFIG_REGISTER, data);
   free(data);
   return success;
}

bool mgos_ltc68xx1_stop_ref(struct mgos_ltc68xx1 *handle)
{
   struct mgos_ltc68xx_data *data = mgos_ltc68xx_create_data(handle->chainLength, 6);

   if (!mgos_ltc68xx1_read_reg(handle, READ_CONFIG_REGISTER, data))
      return false;

   for(int i = handle->chainLength - 1; i >= 0; i--)
   {
      uint8_t *chipData = mgos_ltc68xx_get_chip_data(data, i);
      CLEAR_BIT(chipData[0], 2);
   }

   bool success = mgos_ltc68xx1_write_reg_diff(handle, WRITE_CONFIG_REGISTER, data);
   free(data);
   return success;
}

bool read_update_write_reg(
   struct mgos_ltc68xx1 *handle, 
   struct mgos_ltc68xx_data *data, 
   uint16_t readReg, 
   uint16_t writeReg, 
   void (*updateFunc)(struct mgos_ltc68xx1*, struct mgos_ltc68xx_data*))
{
   if (!mgos_ltc68xx1_read_reg(handle, readReg, data))
      return false;

   updateFunc(handle, data);

   return mgos_ltc68xx1_write_reg_diff(handle, writeReg, data);
}

bool read_update_write_config_reg(struct mgos_ltc68xx1 *handle, struct mgos_ltc68xx_data *data, void (*updateFunc)(struct mgos_ltc68xx1*, struct mgos_ltc68xx_data*))
{
   return read_update_write_reg(handle, data, READ_CONFIG_REGISTER, WRITE_CONFIG_REGISTER, updateFunc);
}

bool setup_config_reg(struct mgos_ltc68xx1 *handle, struct mgos_ltc68xx_data *data)
{
   return read_update_write_config_reg(handle, data,
      LAMBDA(void _(struct mgos_ltc68xx1 *h, struct mgos_ltc68xx_data *d) {
         for (int i = d->chainLength - 1; i >= 0; i--)
         {
            uint8_t *chipData = mgos_ltc68xx_get_chip_data(d, i);

            // Start reference
            SET_BIT(chipData[0], 2);

            // Set ADC Mode option
            CLEAR_BIT(chipData[0], 0);
            chipData[0] |= GET_ADC_OPTION(h->adcMode);
         }
      }));
}

bool clean_config_reg(struct mgos_ltc68xx1 *handle, struct mgos_ltc68xx_data *data)
{
   return read_update_write_config_reg(handle, data,
      LAMBDA(void _(DO_NOT_WARN_UNUSED struct mgos_ltc68xx1 *h, struct mgos_ltc68xx_data *d) {
         for (int i = d->chainLength - 1; i >= 0; i--)
         {
            uint8_t *chipData = mgos_ltc68xx_get_chip_data(d, i);
            CLEAR_BIT(chipData[0], 2);    // Stop reference
         }
      }));
}

bool measure_cell_voltages(struct mgos_ltc68xx1 *handle, uint16_t cells)
{
   if (cells == 0)
      return true;
   
   uint8_t adcMode = handle->adcMode;
   uint8_t cmd = START_CELL_CONVERSION(GET_ADC_MODE(adcMode), false, 0x0);
   bool result = mgos_ltc68xx1_exec_cmd(handle, cmd);

   if (result)
      mgos_usleep(adc_time_multi[adcMode]);

   return result;
}

bool measure_aux_voltages(struct mgos_ltc68xx1 *handle, uint8_t aux)
{
   if (aux == 0)
      return true;
   
   uint8_t adcMode = handle->adcMode;
   uint8_t cmd = START_AUX_CONVERSION(GET_ADC_MODE(adcMode), 0x0);
   bool result = mgos_ltc68xx1_exec_cmd(handle, cmd);

   if (result)
      mgos_usleep(adc_time_multi[adcMode]);

   return result;
}

bool measure_system_voltages(struct mgos_ltc68xx1 *handle, uint8_t system)
{
   if (system == 0)
      return true;
   
   uint8_t adcMode = handle->adcMode;
   uint8_t cmd = START_STATUS_CONVERSION(GET_ADC_MODE(adcMode), 0x0);
   bool result = mgos_ltc68xx1_exec_cmd(handle, cmd);

   if (result)
      mgos_usleep(adc_time_multi[adcMode]);

   return result;
}

bool read_cell_voltages(struct mgos_ltc68xx1 *handle, struct mgos_ltc68xx_data *data, struct mgos_ltc68xx1_measure_result *results, uint16_t cells)
{
   if (cells == 0)
      return true;
   
   uint16_t valueMask = 0x07;
   for (int reg = 0; reg < 4; reg++)
   {
      if ((cells & valueMask) == 0)
         continue;

      bool result = mgos_ltc68xx1_read_reg(handle, read_cell_voltage_reg[reg], data);
      if (!result)
         return false;

      for (int i = 0; i < data->chainLength; i++)
      {
         int cell = reg  *3;
         uint8_t *chipData = mgos_ltc68xx_get_chip_data(data, i);
         results[i].cells[cell++] = (chipData[1] << 8) | chipData[0];
         results[i].cells[cell++] = (chipData[3] << 8) | chipData[2];
         results[i].cells[cell++] = (chipData[5] << 8) | chipData[4];
      }

      valueMask <<= 3;
   }
   
   return true;
}

bool read_aux_voltages(struct mgos_ltc68xx1 *handle, struct mgos_ltc68xx_data *data, struct mgos_ltc68xx1_measure_result *results, uint16_t aux)
{
   if (aux == 0)
      return true;
   
   uint16_t valueMask = 0x07;
   for (int reg = 0; reg < 2; reg++)
   {
      if ((aux & valueMask) == 0)
         continue;

      bool result = mgos_ltc68xx1_read_reg(handle, read_aux_reg[reg], data);
      if (!result)
         return false;

      for (int i = 0; i < data->chainLength; i++)
      {
         uint8_t *chipData = mgos_ltc68xx_get_chip_data(data, i);
         if (reg == 0)
         {
            results[i].gpios[0] = (chipData[1] << 8) | chipData[0];
            results[i].gpios[1] = (chipData[3] << 8) | chipData[2];
            results[i].gpios[2] = (chipData[5] << 8) | chipData[4];
         }
         else
         {
            results[i].gpios[3] =     (chipData[1] << 8) | chipData[0];
            results[i].gpios[4] =     (chipData[3] << 8) | chipData[2];
            results[i].internalRef2 = (chipData[5] << 8) | chipData[4];
         }
      }

      valueMask <<= 3;
   }
   
   return true;
}

bool read_system_voltages(struct mgos_ltc68xx1 *handle, struct mgos_ltc68xx_data *data, struct mgos_ltc68xx1_measure_result *results, uint16_t system)
{
   if (system == 0)
      return true;
   
   uint16_t valueMask = 0x07;
   for (int reg = 0; reg < 2; reg++)
   {
      if ((system & valueMask) == 0)
         continue;

      bool result = mgos_ltc68xx1_read_reg(handle, read_status_reg[reg], data);
      if (!result)
         return false;

      for (int i = 0; i < data->chainLength; i++)
      {
         uint8_t *chipData = mgos_ltc68xx_get_chip_data(data, i);
         if (reg == 0)
         {
            results[i].sumOfCells =    (chipData[1] << 8) | chipData[0];
            results[i].internalTemp =  (chipData[3] << 8) | chipData[2];
            results[i].analogSupply =  (chipData[5] << 8) | chipData[4];
         }
         else
         {
            results[i].digitalSupply = (chipData[1] << 8) | chipData[0];
         }
      }

      valueMask <<= 3;
   }
   
   return true;
}

struct mgos_ltc68xx1_measure_result *mgos_ltc68xx1_measure(struct mgos_ltc68xx1 *handle, uint16_t cells, uint8_t aux, uint8_t system)
{
   struct mgos_ltc68xx_data *data = mgos_ltc68xx_create_data(handle->chainLength, 6);

   if (!setup_config_reg(handle, data) ||
      !measure_cell_voltages(handle, cells) ||
      !measure_aux_voltages(handle, aux) ||
      !measure_system_voltages(handle, system) ||
      !clean_config_reg(handle, data))
   {
      free(data);
      return NULL;
   }

   // Read results
   struct mgos_ltc68xx1_measure_result *results = (struct mgos_ltc68xx1_measure_result*)calloc(handle->chainLength, sizeof(struct mgos_ltc68xx1_measure_result));

   if (!read_cell_voltages(handle, data, results, cells) ||
      !read_aux_voltages(handle, data, results, aux) ||
      !read_system_voltages(handle, data, results, system))
   {
      free(results);
      results = NULL;
   }

   free(data);
   return results;
}
