#include "mgos.h"
#include "mgos_spi.h"

#include "crc15.h"
#include "mgos_ltc68xx.h"

#define SET_BIT(a,i)    ((a) |= 1<<(i))
#define CLEAR_BIT(a,i)  ((a) &= ~(1<<(i)))
#define IS_SET(a,i)     ((a) & (1<<(i)))
#define IS_CLEAR(a,i)   (((a) & (1<<(i))) == 0)

#define WRITE_CONFIG_REGISTER           (0x0001)
#define READ_CONFIG_REGISTER            (0x0002)
#define READ_CELL_REGISTER_A            (0x0004)
#define READ_CELL_REGISTER_B            (0x0006)
#define READ_CELL_REGISTER_C            (0x0008)
#define READ_CELL_REGISTER_D            (0x000A)
#define READ_AUX_REGISTER_A             (0x000C)
#define READ_AUX_REGISTER_B             (0x000E)
#define READ_STATUS_REGISTER_A          (0x0010)
#define READ_STATUS_REGISTER_B          (0x0012)
#define CLEAR_CELL_REGISTER             (0x0711)
#define CLEAR_AUX_REGISTER              (0x0712)
#define CLEAR_STATUS_REGISTER           (0x0713)
#define POLL_CONVERSION_STATUS          (0x0714)
#define DIAGNOSE_MUX                    (0x0715)
#define WRITE_COMM_REGISTER             (0x0721)
#define READ_COMM_REGISTER              (0x0722)
#define START_COMM                      (0x0723)
#define START_CELL_CONVERSION(m, d, c)  (0x0260 | ((uint16_t)m << 7) | ((d ? 1 : 0) << 4) | ((uint16_t)c))
#define START_AUX_CONVERSION(m, c)      (0x0460 | ((uint16_t)m << 7) | ((uint16_t)c))
#define START_STATUS_CONVERSION(m, c)   (0x0468 | ((uint16_t)m << 7) | ((uint16_t)c))

// Configuration Register Group
#define ADCOPT          0
#define DTEN            1
#define REFON           2
#define GPIO            3
#define GPIO1           3
#define GPIO2           4
#define GPIO3           5
#define GPIO4           6
#define GPIO5           7

unsigned int mgos_ltc68xx1_determine_length(struct mgos_ltc68xx1* handle)
{
   if (handle == NULL)
      return 0;
   
   handle->chainLength = 1;
   return handle->chainLength;
}

bool mgos_ltc68xx1_start_ref(struct mgos_ltc68xx1* handle)
{
   struct mgos_ltc68xx_data* data = mgos_ltc68xx_create_data(handle->chainLength, 6);

   if (!mgos_ltc68xx1_read_reg(handle, READ_CONFIG_REGISTER, data))
      return false;

   for(int i = handle->chainLength - 1; i >= 0; i--)
   {
      uint8_t* chipData = mgos_ltc68xx_get_chip_data(data, i);
      SET_BIT(chipData[0], 2);
   }

   bool success = mgos_ltc68xx1_write_reg_diff(handle, WRITE_CONFIG_REGISTER, data);
   free(data);
   return success;
}

bool mgos_ltc68xx1_stop_ref(struct mgos_ltc68xx1* handle)
{
   struct mgos_ltc68xx_data* data = mgos_ltc68xx_create_data(handle->chainLength, 6);

   if (!mgos_ltc68xx1_read_reg(handle, READ_CONFIG_REGISTER, data))
      return false;

   for(int i = handle->chainLength - 1; i >= 0; i--)
   {
      uint8_t* chipData = mgos_ltc68xx_get_chip_data(data, i);
      CLEAR_BIT(chipData[0], 2);
   }

   bool success = mgos_ltc68xx1_write_reg_diff(handle, WRITE_CONFIG_REGISTER, data);
   free(data);
   return success;
}