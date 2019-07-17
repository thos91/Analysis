#ifndef WG_ANAHISTSUMMARY_HPP_
#define WG_ANAHISTSUMMARY_HPP_

// system C++ includes
#include <string>

// user includes
#include "wgConst.hpp"

#define M 5

#define SELECT_NOISE        0
#define SELECT_DIFF         1
#define SELECT_CHARGE_NOHIT 2
#define SELECT_CHARGE_HIT   3
#define SELECT_PRINT        4

// This is needed to call the following functions from Python using ctypes
#ifdef __cplusplus
extern "C" {
#endif

  int wgAnaHistSummary(const char * x_inputDir,
                       const char * x_outputXMLDir,
                       const char * x_outputIMGDir,
                       int mode,
                       bool overwrite = false,
                       bool print = false);
  
#ifdef __cplusplus
}
#endif

void ModeSelect(int mode);

void MakeSummaryXmlFile(const std::string& str,
                        bool overwrite,
                        unsigned n_chips = NCHIPS,
                        unsigned n_chans = NCHANNELS);

#endif // WG_ANAHISTSUMMARY_HPP_
