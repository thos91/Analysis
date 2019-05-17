#ifndef WG_MAKEHIST_HPP_
#define WG_MAKEHIST_HPP_

// user includes
#include "Const.h"

#define DEBUG_MAKEHIST

#define MH_SUCCESS                       0
#define ERR_CANNOT_OVERWRITE_OUTPUT_FILE 1

using namespace std;

// This is needed to call the following functions from Python using ctypes
#ifdef __cplusplus
extern "C" {
#endif

  int MakeHist(const string& inputFileName,
			   const string& outputDir,
			   bool overwrite,
			   unsigned n_chips = NCHIPS,
			   unsigned n_channels = NCHANNELS);

#ifdef __cplusplus
}
#endif

#endif // WG_MAKEHIST_HPP_
