#ifndef WG_SCURVE_HPP_
#define WG_SCURVE_HPP_

// system C++ includes
#include <string>

// system C includes
#include <cstdbool>

// ROOT includes
#include <TGraphErrors.h>

// user includes
#include "wgConst.hpp"
#include "wgTopology.hpp"

// #define SCURVE_SUCCESS              0
// #define ERR_CANNOT_CREATE_DIRECTORY 1
// #define ERR_WRONG_MODE              2
// #define ERR_FAILED_CREATE_XML_FILE  3
// #define ERR_WG_SCURVE               4

// Errors
#define SCURVE_SUCCESS              0
#define ERR_CANNOT_CREATE_DIRECTORY 1
#define ERR_FAILED_OPEN_XML_FILE    2
#define ERR_FAILED_WRITE            3
#define ERR_FAILED_OPEN_HIST_FILE   4
#define ERR_FAILED_GET_BISTREAM     5
#define ERR_WRONG_DIF_VALUE         6
#define ERR_WRONG_CHIP_VALUE        7
#define ERR_WRONG_CHANNEL_VALUE     8
#define ERR_EMPTY_INPUT_FILE        9
#define ERR_TOPOLOGY                10
#define ERR_WG_SCURVE               11

using namespace std;


// This is needed to call the following functions from Python using ctypes
#ifdef __cplusplus
extern "C" {
#endif
  int wgScurve(const char* x_inputDirName,
               const char* x_outputXMLDirName = "",
               const char* x_outputIMGDirName = "");
#ifdef __cplusplus
}
#endif

// Fit the noise rate s-curve for each inputDAC, chip "ichip" and channel "ichan".
void fit_scurve(TGraphErrors* Scurve, 
                double& pe1_t, 
                double& pe2_t, 
                unsigned idif_id, 
                unsigned ichip_id, 
                unsigned ichan_id, 
                unsigned inputDAC,
                string outputIMGDir, 
                bool print_flag = false);

#endif // WG_SCURVE_HPP_
