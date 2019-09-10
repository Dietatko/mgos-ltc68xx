#ifdef MGOS_HAVE_MJS

#include "mjs.h"
#include "mgos_ltc68xx_data.h"

mjs_val_t voltage_array_converter(struct mjs *mjs, const void *field_ptr, size_t arrayLength)
{
   uint16_t *srcArray = (uint16_t*)field_ptr;

   mjs_val_t result = mjs_mk_array(mjs);
   for (size_t i = 0; i < arrayLength; i++)
      mjs_array_push(mjs, result, mjs_mk_number(mjs, srcArray[i]));
   
   return result;
}

mjs_val_t cell_voltage_converter(struct mjs *mjs, const void *field_ptr)
{
   return voltage_array_converter(mjs, field_ptr, 12);
}

mjs_val_t gpio_voltage_converter(struct mjs *mjs, const void *field_ptr)
{
   return voltage_array_converter(mjs, field_ptr, 5);
}

static const struct mjs_c_struct_member mgos_ltc68xx_chip_results_descr[] = {
   {"cells",         offsetof(struct mgos_ltc68xx_chip_results, cells),         MJS_STRUCT_FIELD_TYPE_CUSTOM, cell_voltage_converter},
   {"gpios",         offsetof(struct mgos_ltc68xx_chip_results, gpios),         MJS_STRUCT_FIELD_TYPE_CUSTOM, gpio_voltage_converter},
   {"internalRef2",  offsetof(struct mgos_ltc68xx_chip_results, internalRef2),  MJS_STRUCT_FIELD_TYPE_UINT16, NULL},
   {"sumOfCells",    offsetof(struct mgos_ltc68xx_chip_results, sumOfCells),    MJS_STRUCT_FIELD_TYPE_UINT16, NULL},
   {"dieTemp",       offsetof(struct mgos_ltc68xx_chip_results, dieTemp),       MJS_STRUCT_FIELD_TYPE_UINT16, NULL},
   {"analogSupply",  offsetof(struct mgos_ltc68xx_chip_results, analogSupply),  MJS_STRUCT_FIELD_TYPE_UINT16, NULL},
   {"digitalSupply", offsetof(struct mgos_ltc68xx_chip_results, digitalSupply), MJS_STRUCT_FIELD_TYPE_UINT16, NULL},
   {NULL, 0, MJS_STRUCT_FIELD_TYPE_INVALID, NULL},
};

const struct mjs_c_struct_member *get_mgos_ltc68xx1_chip_results_descr(void) {
   return mgos_ltc68xx_chip_results_descr;
};


mjs_val_t convert_chip_results_array(struct mjs *mjs, const void *field_ptr)
{
   struct mgos_ltc68xx_measure_results *results = (struct mgos_ltc68xx_measure_results*)field_ptr;
   
   mjs_val_t val = mjs_mk_array(mjs);
   for (size_t i = 0; i < results->chipCount; i++)
      mjs_array_push(mjs, val, mjs_struct_to_obj(mjs, &results->chipResults[i], mgos_ltc68xx_chip_results_descr));
   
   return val;
}

static const struct mjs_c_struct_member mgos_ltc68xx_measure_results_descr[] = {
   {"chipCount",   offsetof(struct mgos_ltc68xx_measure_results, chipCount),   MJS_STRUCT_FIELD_TYPE_INT,    NULL},
   //{"chipResults", offsetof(struct mgos_ltc68xx_measure_results, chipResults), MJS_STRUCT_FIELD_TYPE_STRUCT_PTR, mgos_ltc68xx_chip_results_descr},
   {"chipResults", 0,                                                          MJS_STRUCT_FIELD_TYPE_CUSTOM, convert_chip_results_array},
   {NULL, 0, MJS_STRUCT_FIELD_TYPE_INVALID, NULL},
};

const struct mjs_c_struct_member *get_mgos_ltc68xx_measure_results_descr(void) {
   return mgos_ltc68xx_measure_results_descr;
};

#endif /* MGOS_HAVE_MJS */