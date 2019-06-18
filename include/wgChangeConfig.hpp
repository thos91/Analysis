#ifndef WG_CHANGE_CONFIG_HPP_
#define WG_CHANGE_CONFIG_HPP_

// user includes
#include "wgConst.hpp"

// flags
#define WG_CHANGE_CONFIG_FLAGS 3
#define EDIT_FLAG      0
#define OVERWRITE_FLAG 1
#define MPPC_DATA_FLAG 2

// error codes
#define EC_SUCCESS                  0
#define ERR_EMPTY_INPUT_FILE        1
#define ERR_INPUT_FILE_NOT_FOUND    2
#define ERR_OVERWRITE_FLAG_NOT_SET  3
#define ERR_VALUE_OUT_OF_RANGE      4
#define ERR_CHANNEL_OUT_OF_RANGE    5
#define ERR_WRONG_MODE              6
#define ERR_FAILED_WRITE            7
#define ERR_CANNOT_CREATE_DIRECTORY 8
// modes
#define EC_TRIGGER_THRESHOLD     0
#define EC_GAIN_SELECT_THRESHOLD 1
#define EC_INPUT_DAC             2
#define EC_HG_LG_AMPLIFIER       3
#define EC_THRESHOLD_ADJUSTMENT  4
#define EC_INPUT_DAC_REFERENCE   5

//#define DEBUG_CHANGECONFIG
#define M 4

using namespace std;

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
