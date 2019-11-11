#ifndef WG_ANAHIST_HPP_
#define WG_ANAHIST_HPP_

// system C++ includes
#include <string>
#include <bitset>

// system C includes
//#include <bits/stdc++.h>

// user includes
#include "wgConst.hpp"

// all doubles are cast to int when saving to XML files
// the unphysical values are stored as -1

namespace anahist {

// flags
enum ANAHIST_FLAGS {
 SELECT_OVERWRITE     = 0, // 7
 SELECT_CONFIG        = 1, // 6
 SELECT_PRINT         = 2, // 5
 SELECT_DARK_NOISE    = 3, // 4
 SELECT_PEDESTAL      = 4, // 3
 SELECT_CHARGE_HG     = 5, // 2
 SELECT_CHARGE_LG     = 6, // 1
 SELECT_COMPATIBILITY = 7, // 0
 NFLAGS               = 8
};

}

#ifdef __cplusplus
extern "C" {
#endif

  int wgAnaHist(const char * inputFileName,
                const char * configFileName,
                const char * outputDir,
                const char * outputIMGDir,
                const unsigned long flags_ulong,
                unsigned idif = 1);

#ifdef __cplusplus
}
#endif

#endif // WG_ANAHIST_HPP_
