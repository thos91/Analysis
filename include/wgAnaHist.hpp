#ifndef WG_ANAHIST_HPP_
#define WG_ANAHIST_HPP_

// system C++ includes
#include <string>

// system C includes
#include <bits/stdc++.h>

// user includes
#include "wgConst.hpp"

using namespace std;

// Number of flags
#define M 7

#define SELECT_OVERWRITE   0
#define SELECT_CONFIG      1
#define SELECT_PRINT       2
#define SELECT_DARK_NOISE  3
#define SELECT_CHARGE      4
#define SELECT_PEDESTAL    5
#define SELECT_CHARGE_HG   6

// Errors
#define AH_SUCCESS                  0
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

// Set the flags according to the mode
void ModeSelect(int mode, bitset<M>& flag);

#ifdef __cplusplus
extern "C" {
#endif

  int wgAnaHist(const char * inputFileName,
                const char * configFileName,
                const char * outputDir,
                const char * outputIMGDir,
                int mode,
                unsigned long flags_ulong,
                unsigned idif = 1);

#ifdef __cplusplus
}
#endif

#endif // WG_ANAHIST_HPP_
