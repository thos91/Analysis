#ifndef WG_ANAHISTSUMMARY_HPP_
#define WG_ANAHISTSUMMARY_HPP_

// system C++ includes
#include <string>

// system C includes
#include <cstdbool>

// user includes
#include "wgConst.hpp"

#define M 5

#define SELECT_NOISE        0
#define SELECT_DIFF         1
#define SELECT_CHARGE_NOHIT 2
#define SELECT_CHARGE_HIT   3
#define SELECT_PRINT        4

#define AHS_SUCCESS                 0
#define ERR_CANNOT_CREATE_DIRECTORY 1
#define ERR_WRONG_MODE              2
#define ERR_FAILED_CREATE_XML_FILE  3
#define ERR_WG_ANA_HIST_SUMMARY     4
#define ERR_EMPTY_INPUT_FILE        5
#define ERR_FAILED_OPEN_XML_FILE    6

using namespace std;

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

void MakeSummaryXmlFile(const string& str,
                        bool overwrite,
                        unsigned n_chips = NCHIPS,
                        unsigned n_chans = NCHANNELS);

#endif // WG_ANAHISTSUMMARY_HPP_
