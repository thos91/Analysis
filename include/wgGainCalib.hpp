#ifndef WG_GAIN_CALIB_HPP_
#define WG_GAIN_CALIB_HPP_

// user includes
#include "wgConst.hpp"

#ifdef __cplusplus
extern "C" {
#endif

  int wgGainCalib(const char * x_inputDir,
                  const char * x_outputXMLDir,
                  const char * x_outputIMGDir);
  
#ifdef __cplusplus
}
#endif

#endif // WG_GAIN_CALIB_HPP_
