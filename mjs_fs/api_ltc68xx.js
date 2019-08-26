let LTC68XX = {
  _create: ffi('void* mgos_ltc68xx1_create(void*,void*)'),
  _create_txn_config: ffi('void* mgos_ltc68xx1_create_txn_config(int,int,int)'),

  configureSPI: function(params) {
    return _create_txn_config(params.cs, params.mode, params.freq);
  },

  create: function(spi, spiConfig) {
    let handle = _create(spi, spiConfig);
    return {
      _close: ffi('void mgos_ltc68xx1_close(void*)'),
      _wake_up: ffi('bool mgos_ltc68xx1_wake_up(void*)'),
      _exec_cmd: ffi('bool mgos_ltc68xx1_exec_cmd(void*,int)'),
      _read_reg: ffi('bool mgos_ltc68xx1_read_reg(void*,int,void*)'),
      _write_reg_same: ffi('bool mgos_ltc68xx1_write_reg_same(void*,int,void*,int)'),
      _write_reg_diff: ffi('bool mgos_ltc68xx1_write_reg_diff(void*,int,void*)'),
      _measure: ffi('void* mgos_ltc68xx1_measure(void*,int,int,int);'),

      _results_descr: ffi('void *get_mgos_ltc68xx1_measure_result_descr(void)'),
      
      _handle: handle,

      close: function() {
        _close(_handle);
      },

      wakeUp: function() {
        _wake_up(_handle);
      },

      measure: function() {
        let resultsPtr = _measure(_handle, 0x0FFF, 0b111111, 0b1111);
        let sd = this._results_descr();
        print("_results_descr:", sd);
        let results = s2o(resultsPtr, sd);
        return results;
      }
    };
  }
};