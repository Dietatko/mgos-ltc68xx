#include "mgos.h"
#include "mgos_spi.h"
#include "mgos_system.h"

#include "crc15.h"

#include "mgos_ltc68xx.h"
#include "mgos_ltc68xx_regs_internal.h"

#define LAMBDA(c_) ({ c_ _;})

// N/A, 27 kHz (Fast), 7 kHz (Normal), 26 Hz (Filtered), N/A, 14 kHz, 3 kHz, 2kHz
uint32_t adc_time_single[] = {
   0, 201, 405, 33568, 0, 230, 521, 754
};

uint32_t adc_time_multi4[] = {
   0, 748, 1563, 134218, 0, 865, 2028, 2959
};

uint32_t adc_time_multi6[] = {
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
   
   handle->chainLength = 16;

   if (!mgos_ltc68xx1_wake_up(handle))
   {
      handle->chainLength = 0;
      return 0;
   }

   struct mgos_ltc68xx_data *data = mgos_ltc68xx_create_data(16, 6);
   mgos_ltc68xx1_read_reg(handle, READ_STATUS_REGISTER_B, data);
   for(size_t i = 0; i < 16; i++)
   {
      uint8_t *chipData = mgos_ltc68xx_get_chip_data(data, i);
      if (chipData == NULL || chipData[5] == 0xFF)
      {
         handle->chainLength = i;
         break;
      }
   }
   mgos_ltc68xx_free_data(data);

   return handle->chainLength;
}

bool mgos_ltc68xx1_start_ref(struct mgos_ltc68xx1 *handle)
{
   struct mgos_ltc68xx_data *data = mgos_ltc68xx_create_data(handle->chainLength, 6);

   if (!mgos_ltc68xx1_read_reg(handle, READ_CONFIG_REGISTER, data))
      return false;

   for(size_t i = 0; i < handle->chainLength; i++)
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

   for(size_t i = 0; i < handle->chainLength; i++)
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
   //LOG(LL_INFO, ("setup_config_reg"));
   return read_update_write_config_reg(handle, data,
      LAMBDA(void _(struct mgos_ltc68xx1 *h, struct mgos_ltc68xx_data *d) {
         for (size_t i = 0; i < d->chainLength; i++)
         {
            uint8_t *chipData = mgos_ltc68xx_get_chip_data(d, i);
            chipData[0] = (0x1F << BIT_GPIO1) | (1 << BIT_REFON);
            chipData[0] |= GET_ADC_OPTION(h->adcMode);

            chipData[4] = 0;
            chipData[5] = 0;
         }
      }));
}

bool clean_config_reg(struct mgos_ltc68xx1 *handle, struct mgos_ltc68xx_data *data)
{
   return read_update_write_config_reg(handle, data,
      LAMBDA(void _(DO_NOT_WARN_UNUSED struct mgos_ltc68xx1 *h, struct mgos_ltc68xx_data *d) {
         for (size_t i = 0; i < d->chainLength; i++)
         {
            uint8_t *chipData = mgos_ltc68xx_get_chip_data(d, i);
            chipData[0] |= (0x1F << BIT_GPIO1);
            CLEAR_BIT(chipData[0], 2);    // Stop reference

            chipData[4] = 0;
            chipData[5] = 0;
         }
      }));
}

bool measure_cell_voltages(struct mgos_ltc68xx1 *handle, uint16_t cells)
{
   //LOG(LL_INFO, ("measure_cell_voltages"));
   if (cells == 0)
      return true;
   
   int convCount = 0;
   uint16_t mask = 0x41;
   for (size_t i = 0; i < 6; i++)
   {
      if ((cells & mask) != 0)
         convCount++;
      mask <<= 1;
   }

   uint8_t adcMode = handle->adcMode;
   bool result = false;
   if (convCount > 5)
   {
      uint16_t cmd = START_CELL_CONVERSION(GET_ADC_MODE(adcMode), false, 0x00);
      result = mgos_ltc68xx1_exec_cmd(handle, cmd);

      if (result)
         mgos_usleep(adc_time_multi6[adcMode]);
   }
   else
   {
      uint16_t mask = 0x41;
      for (size_t i = 1; i <= 6; i++, mask <<= 1)
      {
         if ((cells & mask) == 0)
            continue;
         
         uint16_t cmd = START_CELL_CONVERSION(GET_ADC_MODE(adcMode), false, i);
         result = mgos_ltc68xx1_exec_cmd(handle, cmd);
         if (!result)
            break;
         
         mgos_usleep(adc_time_single[adcMode]);
      }
   }

   return result;
}

bool measure_aux_voltages(struct mgos_ltc68xx1 *handle, uint8_t aux)
{
   if (aux == 0)
      return true;

   mgos_usleep(100);
   
   uint8_t adcMode = handle->adcMode;
   bool result = false;
   if ((aux & 0x3F) == 0x3F)
   {
      uint16_t cmd = START_AUX_CONVERSION(GET_ADC_MODE(adcMode), 0x00);
      //uint16_t cmd = START_AUX_SELF_TEST(GET_ADC_MODE(adcMode), 0x01);
      result = mgos_ltc68xx1_exec_cmd(handle, cmd);

      if (result)
         mgos_usleep(adc_time_multi6[adcMode] + 100);
   }
   else
   {
      uint16_t mask = 0x01;
      for (size_t i = 1; i <= 6; i++, mask <<= 1)
      {
         if ((aux & mask) == 0)
            continue;
         
         uint16_t cmd = START_AUX_CONVERSION(GET_ADC_MODE(adcMode), i);
         result = mgos_ltc68xx1_exec_cmd(handle, cmd);
         if (!result)
            break;
         
         mgos_usleep(adc_time_single[adcMode]);
      }
   }

   return result;
}

bool measure_system_voltages(struct mgos_ltc68xx1 *handle, uint8_t system)
{
   if (system == 0)
      return true;
      
   uint8_t adcMode = handle->adcMode;
   bool result = false;
   if ((system & 0x0F) == 0x0F)
   {
      uint16_t cmd = START_STATUS_CONVERSION(GET_ADC_MODE(adcMode), 0x00);
      result = mgos_ltc68xx1_exec_cmd(handle, cmd);

      if (result)
         mgos_usleep(adc_time_multi4[adcMode]);
   }
   else
   {
      uint16_t mask = 0x01;
      for (size_t i = 1; i <= 4; i++, mask <<= 1)
      {
         if ((system & mask) == 0)
            continue;
         
         uint16_t cmd = START_STATUS_CONVERSION(GET_ADC_MODE(adcMode), i);
         result = mgos_ltc68xx1_exec_cmd(handle, cmd);
         if (!result)
            break;
         
         mgos_usleep(adc_time_single[adcMode]);
      }
   }

   return result;
}

bool read_cell_voltages(struct mgos_ltc68xx1 *handle, struct mgos_ltc68xx_data *data, struct mgos_ltc68xx_measure_results *results, uint16_t cells)
{
   if (cells == 0)
      return true;
   
   uint16_t valueMask = 0x07;
   for (int reg = 0; reg < 4; reg++, valueMask <<= 3)
   {
      if ((cells & valueMask) == 0)
         continue;

      bool result = mgos_ltc68xx1_read_reg(handle, read_cell_voltage_reg[reg], data);
      if (!result)
         return false;

      for (size_t i = 0; i < results->chipCount; i++)
      {
         int cell = reg * 3;
         uint8_t *chipData = mgos_ltc68xx_get_chip_data(data, i);
         results->chipResults[i].cells[cell++] = (chipData[1] << 8) | chipData[0];
         results->chipResults[i].cells[cell++] = (chipData[3] << 8) | chipData[2];
         results->chipResults[i].cells[cell++] = (chipData[5] << 8) | chipData[4];
      }
   }
   
   return true;
}

bool read_aux_voltages(struct mgos_ltc68xx1 *handle, struct mgos_ltc68xx_data *data, struct mgos_ltc68xx_measure_results *results, uint16_t aux)
{
   if (aux == 0)
      return true;
   
   uint16_t valueMask = 0x07;
   for (int reg = 0; reg < 2; reg++, valueMask <<= 3)
   {
      if ((aux & valueMask) == 0)
         continue;

      bool result = mgos_ltc68xx1_read_reg(handle, read_aux_reg[reg], data);
      if (!result)
         return false;

      for (size_t i = 0; i < results->chipCount; i++)
      {
         uint8_t *chipData = mgos_ltc68xx_get_chip_data(data, i);
         if (reg == 0)
         {
            results->chipResults[i].gpios[0] = (chipData[1] << 8) | chipData[0];
            results->chipResults[i].gpios[1] = (chipData[3] << 8) | chipData[2];
            results->chipResults[i].gpios[2] = (chipData[5] << 8) | chipData[4];
         }
         else
         {
            results->chipResults[i].gpios[3] =     (chipData[1] << 8) | chipData[0];
            results->chipResults[i].gpios[4] =     (chipData[3] << 8) | chipData[2];
            results->chipResults[i].internalRef2 = (chipData[5] << 8) | chipData[4];
         }
      }
   }
   
   return true;
}

bool read_system_voltages(struct mgos_ltc68xx1 *handle, struct mgos_ltc68xx_data *data, struct mgos_ltc68xx_measure_results *results, uint16_t system)
{
   if (system == 0)
      return true;
   
   uint16_t valueMask = 0x07;
   for (int reg = 0; reg < 2; reg++, valueMask <<= 3)
   {
      if ((system & valueMask) == 0)
         continue;

      bool result = mgos_ltc68xx1_read_reg(handle, read_status_reg[reg], data);
      if (!result)
         return false;

      for (size_t i = 0; i < results->chipCount; i++)
      {
         uint8_t *chipData = mgos_ltc68xx_get_chip_data(data, i);
         if (reg == 0)
         {
            results->chipResults[i].sumOfCells =    (chipData[1] << 8) | chipData[0];
            results->chipResults[i].dieTemp =       (chipData[3] << 8) | chipData[2];
            results->chipResults[i].analogSupply =  (chipData[5] << 8) | chipData[4];
         }
         else
         {
            results->chipResults[i].digitalSupply = (chipData[1] << 8) | chipData[0];
         }
      }
   }
   
   return true;
}

bool mgos_ltc68xx1_diagnose(struct mgos_ltc68xx1 *handle)
{
   bool result = mgos_ltc68xx1_exec_cmd(handle, DIAGNOSE_MUX);
   if (result)
      mgos_usleep(450);

   return result;
}

bool mgos_ltc68xx1_measure(struct mgos_ltc68xx1 *handle, uint16_t cells, uint8_t aux, uint8_t system, struct mgos_ltc68xx_measure_results *results)
{
   if (!mgos_ltc68xx1_wake_up(handle))
      return NULL;

   struct mgos_ltc68xx_data *data = mgos_ltc68xx_create_data(results->chipCount, 6);

   bool success = !setup_config_reg(handle, data) ||
      !measure_cell_voltages(handle, cells) ||
      !read_cell_voltages(handle, data, results, cells) ||
      !measure_aux_voltages(handle, aux) ||
      !read_aux_voltages(handle, data, results, aux) ||
      !measure_system_voltages(handle, system) ||
      !read_system_voltages(handle, data, results, system) ||
      !clean_config_reg(handle, data);

   mgos_ltc68xx_free_data(data);

   return success;
}
