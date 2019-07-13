#ifndef WG_OPTIMIZE_HPP_
#define WG_OPTIMIZE_HPP_

// user includes
#include "wgConst.hpp"

// modes
#define OP_THRESHOLD_MODE 0
#define OP_INPUTDAC_MODE  1

// This is needed to call the following functions from Python using ctypes
#ifdef __cplusplus
extern "C" {
#endif
  
  int wgOptimize(const char * x_threshold_card,
                 const char * x_calibration_card,
                 const char * x_config_xml_file,
                 const char * x_wagasci_config_dif_dir,
                 int mode      = OP_THRESHOLD_MODE,
                 int inputDAC  = 121,
                 int pe        = 2);

#ifdef __cplusplus
}
#endif

#endif // WG_OPTIMIZE_HPP_
