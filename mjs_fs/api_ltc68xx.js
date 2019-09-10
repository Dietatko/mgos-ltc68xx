load("mjs_array.js");

let LTC68XX = {
   ALL_CELLS: 0x0FFF,
   ALL_AUXES: 0x3F,
   ALL_SYSTEM: 0x0F,

   configureSPI: function(params) {
      return this._create_txn_config(params.cs, params.mode, params.freq);
   },

   create: function(spi, spiConfig) {
      let handle = this._create(spi, spiConfig);
      if (handle === null) {
         return null;
      }

      let result = Object.create(this._prot);
      result._handle = handle;
      return result;
   },

   _prot: {
      _handle: undefined,

      chainLength: undefined,

      close: function() {
         LTC68XX._close(this._handle);
      },

      determineLength: function() {
         this.chainLength = LTC68XX._determine_length(this._handle);
         return this.chainLength;
      },

      wakeUp: function() {
         LTC68XX._wake_up(this._handle);
      },

      measure: function(cells, aux, system) {
         let resultsPtr = LTC68XX._create_results(this.chainLength);
         let success = LTC68XX._measure(this._handle, 
            cells || 0,
            aux || 0,
            system || 0,
            resultsPtr);
         if (!success) {
            LTC68XX._free_results(resultsPtr);
            return null;
         }
         
         let s2oResults = s2o(resultsPtr, LTC68XX._results_descr());
         let results = Array.create(s2oResults.chipResults)
            .map(function (chipResult) {
               return {
                  cells: Array.create(chipResult.cells).map(function(x) { return x / 10000; }),
                  gpios: Array.create(chipResult.gpios).map(function(x) { return x / 10000; }),
                  internalRef2: chipResult.internalRef2 / 10000,
                  sumOfCells: chipResult.sumOfCells / 500,
                  dieTemp: chipResult.dieTemp / 75 - 273,
                  analogSupply: chipResult.analogSupply / 10000,
                  digitalSupply: chipResult.digitalSupply / 10000
               };
            });

         s2oResults = null;
         LTC68XX._free_results(resultsPtr);
         
         return results;
      }
   },
   
   _create_results: ffi("void* mgos_ltc68xx_create_results(int)"),
   _free_results: ffi("void mgos_ltc68xx_free_results(void*)"),

   _create: ffi("void* mgos_ltc68xx1_create(void*,void*)"),
   _create_txn_config: ffi("void* mgos_ltc68xx1_create_txn_config(int,int,int)"),
   _close: ffi("void mgos_ltc68xx1_close(void*)"),
   _wake_up: ffi("bool mgos_ltc68xx1_wake_up(void*)"),
   _exec_cmd: ffi("bool mgos_ltc68xx1_exec_cmd(void*,int)"),
   _read_reg: ffi("bool mgos_ltc68xx1_read_reg(void*,int,void*)"),
   _write_reg_same: ffi("bool mgos_ltc68xx1_write_reg_same(void*,int,void*,int)"),
   _write_reg_diff: ffi("bool mgos_ltc68xx1_write_reg_diff(void*,int,void*)"),

   _determine_length: ffi("int mgos_ltc68xx1_determine_length(void*)"),
   _measure: ffi("bool mgos_ltc68xx1_measure(void*,int,int,int,void*);"),

   _results_descr: ffi("void *get_mgos_ltc68xx_measure_results_descr(void)")
};