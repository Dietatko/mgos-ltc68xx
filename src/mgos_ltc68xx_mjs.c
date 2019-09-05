#ifdef MGOS_HAVE_MJS

#include "mjs.h"
#include "mgos_ltc68xx.h"

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

static const struct mjs_c_struct_member mgos_ltc68xx1_measure_result_descr[] = {
  {"cells",         offsetof(struct mgos_ltc68xx1_measure_result, cells),         MJS_STRUCT_FIELD_TYPE_CUSTOM, cell_voltage_converter},
  {"gpios",         offsetof(struct mgos_ltc68xx1_measure_result, gpios),         MJS_STRUCT_FIELD_TYPE_CUSTOM, gpio_voltage_converter},
  {"internalRef2",  offsetof(struct mgos_ltc68xx1_measure_result, internalRef2),  MJS_STRUCT_FIELD_TYPE_UINT16, NULL},
  {"sumOfCells",    offsetof(struct mgos_ltc68xx1_measure_result, sumOfCells),    MJS_STRUCT_FIELD_TYPE_UINT16, NULL},
  {"dieTemp",       offsetof(struct mgos_ltc68xx1_measure_result, dieTemp),       MJS_STRUCT_FIELD_TYPE_UINT16, NULL},
  {"analogSupply",  offsetof(struct mgos_ltc68xx1_measure_result, analogSupply),  MJS_STRUCT_FIELD_TYPE_UINT16, NULL},
  {"digitalSupply", offsetof(struct mgos_ltc68xx1_measure_result, digitalSupply), MJS_STRUCT_FIELD_TYPE_UINT16, NULL},
  {NULL, 0, MJS_STRUCT_FIELD_TYPE_INVALID, NULL},
};

const struct mjs_c_struct_member *get_mgos_ltc68xx1_measure_result_descr(void) {
  return mgos_ltc68xx1_measure_result_descr;
};

#endif /* MGOS_HAVE_MJS */