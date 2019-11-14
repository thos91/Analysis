#ifndef WG_ANAHISTSUMMARY_HPP_
#define WG_ANAHISTSUMMARY_HPP_

// system C++ includes
#include <string>

// user includes
#include "wgConst.hpp"

// This is needed to call the following functions from Python using ctypes
#ifdef __cplusplus
extern "C" {
#endif

  int wgAnaHistSummary(const char * x_inputDir,
                       const char * x_outputXMLDir,
                       const char * x_outputIMGDir,
                       const unsigned long ul_flags);
  
#ifdef __cplusplus
}
#endif

#endif // WG_ANAHISTSUMMARY_HPP_
