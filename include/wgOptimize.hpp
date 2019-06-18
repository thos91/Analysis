#ifndef WG_OPTIMIZE_HPP_
#define WG_OPTIMIZE_HPP_

// user includes
#include "wgConst.hpp"

// modes
#define OP_THRESHOLD_MODE 0
#define OP_INPUTDAC_MODE  1

// errors
#define OP_SUCCESS                     0
#define ERR_CALIBRATION_CARD_NOT_FOUND 1
#define ERR_THRESHOLD_CARD_NOT_FOUND   2
#define ERR_WRONG_INPUTDAC_VALUE       3
#define ERR_WRONG_PE_VALUE             4
#define ERR_WRONG_DIF_VALUE            5
#define ERR_WRONG_CHIP_VALUE           6
#define ERR_WRONG_MODE                 7
#define ERR_THRESHOLD_CARD_READ        8
#define ERR_CALIBRATION_CARD_READ      9
#define ERR_BITSTREAM_FILE_NOT_FOUND   10
#define ERR_INPUTDAC_WRITE             11
#define ERR_THRESHOLD_WRITE            12
#define ERR_OPTIMIZE_GENERIC_ERROR     13

//#define DEBUG_OPTIMIZE

using namespace std;

// This is needed to call the following functions from Python using ctypes
#ifdef __cplusplus
extern "C" {
#endif
  
int wgOptimize(const char * x_threshold_card,
			   const char * x_calibration_card,
			   const char * x_wagasci_config_dif_dir,
			   int mode            = OP_THRESHOLD_MODE,
			   int inputDAC        = 121,
			   int pe              = 2,
			   unsigned n_difs     = NDIFS,
			   unsigned n_chips    = NCHIPS,
			   unsigned n_channels = NumChipCh);

#ifdef __cplusplus
}
#endif

#endif // WG_OPTIMIZE_HPP_
