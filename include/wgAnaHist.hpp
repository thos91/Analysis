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

#define SELECT_CONFIG          0
#define SELECT_DARK_NOISE      1
#define SELECT_CHARGE_LOW      2
#define SELECT_PEDESTAL        3
#define SELECT_CHARGE_HG_LOW   4
#define SELECT_CHARGE_HG_HIGH  5
#define SELECT_PRINT           6
#define OVERWRITE              7

// Errors
#define AH_SUCCESS                  0
#define ERR_CANNOT_CREATE_DIRECTORY 1
#define ERR_FAILED_OPEN_XML_FILE    2
#define ERR_WRONG_MODE              3
#define ERR_FAILED_WRITE            4
#define ERR_FAILED_OPEN_HIST_FILE   5
#define ERR_FAILED_GET_BISTREAM     6

// This is needed to call the following functions from Python using ctypes
#ifdef __cplusplus
extern "C" {
#endif

  void ModeSelect(int mode, bitset<M>& flag);
  int AnaHist(const char * inputFileName,
			  const char * configFileName,
			  const char * outputDir,
			  const char * outputIMGDir,
			  int mode,
			  unsigned long flags_ulong,
			  unsigned idif    = 1,
			  unsigned n_chips = NCHIPS,
			  unsigned n_chans = NCHANNELS);

#ifdef __cplusplus
}
#endif

#endif // WG_ANAHIST_HPP_
