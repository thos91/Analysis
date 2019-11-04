#ifndef WG_GAIN_CALIB_UGLY_HPP_
#define WG_GAIN_CALIB_UGLY_HPP_

// user includes
#include "wgConst.hpp"

#ifdef __cplusplus
extern "C" {
#endif

  int wgGainCalibUgly(const char * x_hist_file,
                      const char * x_xml_config_file,  
                      const char * x_output_xml_dir,
                      const char * x_output_img_dir);
  
#ifdef __cplusplus
}
#endif

#endif // WG_GAIN_CALIB_UGLY_HPP_
