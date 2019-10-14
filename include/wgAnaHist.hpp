#ifndef WG_ANAHIST_HPP_
#define WG_ANAHIST_HPP_

// system C++ includes
#include <string>
#include <bitset>

// system C includes
//#include <bits/stdc++.h>

// user includes
#include "wgConst.hpp"

// flags
enum ANAHIST_FLAGS {
 SELECT_OVERWRITE     = 0,
 SELECT_CONFIG        = 1,
 SELECT_PRINT         = 2,
 SELECT_DARK_NOISE    = 3,
 SELECT_CHARGE        = 4,
 SELECT_PEDESTAL      = 5,
 SELECT_CHARGE_HG     = 6,
 SELECT_COMPATIBILITY = 7,
 ANAHIST_NFLAGS       = 8
};


// Set the flags according to the mode
void ModeSelect(int mode, std::bitset<ANAHIST_NFLAGS>& flag);

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
