#ifndef WG_PEDESTAL_CALIB_HPP_
#define WG_PEDESTAL_CALIB_HPP_

// system C++ includes
#include <string>

// user includes
#include "wgConst.hpp"
#include "wgTopology.hpp"

#define N_PE            2  // number of photo electron macros
const unsigned ONE_PE = 0; // threshold at 0.5 p.e.
const unsigned TWO_PE = 1; // threshold at 1.5 p.e.

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