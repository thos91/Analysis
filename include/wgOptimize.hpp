#ifndef WG_OPTIMIZE_HPP_
#define WG_OPTIMIZE_HPP_

namespace optimize {
// modes
enum OPTIMIZE_MODES {
  OP_THRESHOLD_MODE = 0,
  OP_INPUTDAC_MODE = 1,
  OP_WALL_MRD = 2,
  NUM_OPTIMIZE_MODES
};  
}

// This is needed to call the following functions from Python using ctypes
#ifdef __cplusplus
extern "C" {
#endif
  
  int wgOptimize(const char * x_threshold_card,
                 const char * x_calibration_card,
                 const char * x_config_xml_file,
                 const char * x_wagasci_config_dif_dir,
                 unsigned long ul_flags = 0,
                 unsigned pe        = 2,
                 unsigned inputDAC  = 121);

#ifdef __cplusplus
}
#endif

#endif // WG_OPTIMIZE_HPP_
