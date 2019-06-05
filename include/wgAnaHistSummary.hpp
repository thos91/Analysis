#ifndef WG_ANAHISTSUMMARY_HPP_
#define WG_ANAHISTSUMMARY_HPP_

// system C++ includes
#include <string>

// system C includes
#include <cstdbool>

// user includes
#include "Const.hpp"

#define M 5

#define SELECT_NOISE     0
#define SELECT_GAIN      1
#define SELECT_PEDESTAL  2
#define SELECT_RAWCHARGE 3
#define SELECT_PRINT     4

#define AHS_SUCCESS                 0
#define ERR_CANNOT_CREATE_DIRECTORY 1
#define ERR_WRONG_MODE              2
#define ERR_FAILED_CREATE_XML_FILE  3
#define ERR_WG_ANA_HIST_SUMMARY     4

using namespace std;

// This is needed to call the following functions from Python using ctypes
#ifdef __cplusplus
extern "C" {
#endif

  void ModeSelect(int mode);

  void MakeSummaryXmlFile(const string& str,
						  bool overwrite,
						  unsigned n_chips = NCHIPS,
						  unsigned n_chans = NCHANNELS);

  int wgAnaHistSummary(const char * x_inputDirName,
					   const char * x_outputXMLDir,
					   const char * x_outputIMGDir,
					   int mode,
					   bool overwrite = false,
					   bool print = false,
					   unsigned n_chips = NCHIPS,
					   unsigned n_chans = NCHANNELS);
  
#ifdef __cplusplus
}
#endif

#endif // WG_ANAHISTSUMMARY_HPP_
