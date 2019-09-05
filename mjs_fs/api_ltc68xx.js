let LTC68XX = {
  _create: ffi('void* mgos_ltc68xx1_create(void*,void*)'),
  _create_txn_config: ffi('void* mgos_ltc68xx1_create_txn_config(int,int,int)'),

  allCells: 0x0FFF,
  allAuxes: 0x3F,
  allSystem: 0x0F,

  configureSPI: function(params) {
    return this._create_txn_config(params.cs, params.mode, params.freq);
  },

  create: function(spi, spiConfig) {
    let handle = this._create(spi, spiConfig);
    return {
      _close: ffi('void mgos_ltc68xx1_close(void*)'),
      _wake_up: ffi('bool mgos_ltc68xx1_wake_up(void*)'),
      _exec_cmd: ffi('bool mgos_ltc68xx1_exec_cmd(void*,int)'),
      _read_reg: ffi('bool mgos_ltc68xx1_read_reg(void*,int,void*)'),
      _write_reg_same: ffi('bool mgos_ltc68xx1_write_reg_same(void*,int,void*,int)'),
      _write_reg_diff: ffi('bool mgos_ltc68xx1_write_reg_diff(void*,int,void*)'),

      _determine_length: ffi('int mgos_ltc68xx1_determine_length(void*)'),
      _measure: ffi('void* mgos_ltc68xx1_measure(void*,int,int,int);'),

      _results_descr: ffi('void *get_mgos_ltc68xx1_measure_result_descr(void)'),
      
      _handle: handle,

      _map: function(array, func) {
        let result = [];
        for (let i = 0; i < array.length; i++) { 
          result.push(func(array[i]));
        }
        return result;
      },

      close: function() {
        this._close(this._handle);
      },

      determineLength: function() {
        return this._determine_length(this._handle);
      },

      wakeUp: function() {
        this._wake_up(this._handle);
      },

      measure: function(cells, aux, system) {
        let resultsPtr = this._measure(this._handle, 
          cells || 0,
          aux || 0,
          system || 0);
        if (resultsPtr === null)
          return null;
        
        let s2oResults = s2o(resultsPtr, this._results_descr());
        let results = {
          cells: this._map(s2oResults.cells, function(x) { return x / 10000; }),
          gpios: this._map(s2oResults.gpios, function(x) { return x / 10000; }),
          internalRef2: s2oResults.internalRef2 / 10000,
          sumOfCells: s2oResults.sumOfCells / 500,
          dieTemp: s2oResults.dieTemp / 75 - 273,
          analogSupply: s2oResults.analogSupply / 10000,
          digitalSupply: s2oResults.digitalSupply / 10000
        };

        return results;
      }
    };
  }
};