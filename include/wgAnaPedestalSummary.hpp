#ifndef WG_ANAHISTSUMMARY_HPP_
#define WG_ANAHISTSUMMARY_HPP_

// system C++ includes
#include <string>

// user includes
#include "Const.h"

// errors
#define APS_SUCCESS                 0
#define ERR_CANNOT_CREATE_DIRECTORY 1
#define ERR_GET_FILE_LIST           2
#define ERR_FAILED_OPEN_XML_FILE    3
#define ERR_WRONG_DIF_VALUE         4
#define ERR_WRONG_PE_VALUE          5

#define ONE_PE 0
#define TWO_PE 1

using namespace std;

// This is needed to call the following functions from Python using ctypes
#ifdef __cplusplus
extern "C" {
#endif

  vector<string> GetIncludeFileName(const string& inputDirName);

  int AnaPedestalSummary(const char * inputDirName,
						 const char * outputXMLDirName,
						 const char * outputIMGDirName,
						 unsigned n_difs = NDIFS,
						 unsigned n_chips = NCHIPS,
						 unsigned n_chans = NCHANNELS);

#ifdef __cplusplus
}
#endif

#endif // WG_ANAHISTSUMMARY_HPP_
