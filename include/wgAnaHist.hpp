#ifndef WG_ANAHIST_HPP_
#define WG_ANAHIST_HPP_

// system C++ includes
#include <string>

// system C includes
#include <bits/stdc++.h>

// user includes
#include "Const.h"

using namespace std;

// Number of flags
#define M 8

#define SELECT_OVERWRITE       0
#define SELECT_CONFIG          1
#define SELECT_PRINT           2
#define SELECT_DARK_NOISE      3
#define SELECT_CHARGE_LOW      4
#define SELECT_PEDESTAL        5
#define SELECT_CHARGE_HG_LOW   6
#define SELECT_CHARGE_HG_HIGH  7

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

// Set the flags according to the mode
void ModeSelect(int mode, bitset<M>& flag);

#ifdef __cplusplus
extern "C" {
#endif

  int AnaHist(const char * inputFileName,
			  const char * configFileName,
			  const char * outputDir,
			  const char * outputIMGDir,
			  unsigned long flags_ulong,
			  unsigned idif    = 1,
			  unsigned n_chips = NCHIPS,
			  unsigned n_chans = NCHANNELS);

#ifdef __cplusplus
}
#endif

#endif // WG_ANAHIST_HPP_
