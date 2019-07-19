#ifndef WG_MAKEHIST_HPP_
#define WG_MAKEHIST_HPP_

// user includes
#include "wgConst.hpp"

// This is needed to call the following functions from Python using ctypes
#ifdef __cplusplus
extern "C" {
#endif

  int wgMakeHist(const char * inputFileName,
                 const char * outputDir,
                 bool overwrite,
                 unsigned dif = 1,
                 unsigned n_chips = NCHIPS);

#ifdef __cplusplus
}
#endif

#endif // WG_MAKEHIST_HPP_
