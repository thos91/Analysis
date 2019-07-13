#ifndef WG_CHANGE_CONFIG_HPP_
#define WG_CHANGE_CONFIG_HPP_

// user includes
#include "wgConst.hpp"

// flags
#define WG_CHANGE_CONFIG_FLAGS 3
#define EDIT_FLAG      0
#define OVERWRITE_FLAG 1
#define MPPC_DATA_FLAG 2

// modes
#define EC_TRIGGER_THRESHOLD     0
#define EC_GAIN_SELECT_THRESHOLD 1
#define EC_INPUT_DAC             2
#define EC_HG_LG_AMPLIFIER       3
#define EC_THRESHOLD_ADJUSTMENT  4
#define EC_INPUT_DAC_REFERENCE   5

//#define DEBUG_CHANGECONFIG
#define M 4

// This is needed to call the following functions from Python using ctypes
#ifdef __cplusplus
extern "C" {
#endif

  // edit bitstream txt file (see documentation for further info)
  int wgChangeConfig(const char * x_inputFile,
                     const char * x_outputFile,
                     unsigned long x_flags,
                     int value,
                     int mode,
                     int chip    = 0,
                     int channel = NCHANNELS);

#ifdef __cplusplus
}
#endif

#endif // WG_CHANGE_CONFIG_HPP_
