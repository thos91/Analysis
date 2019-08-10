#ifndef WG_PEDESTAL_CALIB_HPP_
#define WG_PEDESTAL_CALIB_HPP_

// system C++ includes
#include <string>

// user includes
#include "wgConst.hpp"
#include "wgTopology.hpp"

#define PEDESTAL_DIFFERENCE_WARNING_THRESHOLD 0.1

// This is needed to call the following functions from Python using ctypes
#ifdef __cplusplus
extern "C" {
#endif

  int wgPedestalCalib(const char * inputDirName,
                      const char * outputXMLDirName,
                      const char * outputIMGDirName);

#ifdef __cplusplus
}
#endif

#endif // WG_PEDESTAL_CALIB_HPP_
