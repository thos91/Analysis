#ifndef WG_MAKEHIST_HPP_
#define WG_MAKEHIST_HPP_

// system includes
#include <bitset>

// user includes
#include "wgConst.hpp"

namespace makehist {
enum WG_MAKEHIST_FLAGS {
  SELECT_DARK_NOISE = 0, // 6
  SELECT_CHARGE_HG  = 1, // 5
  SELECT_CHARGE_LG  = 2, // 4
  SELECT_PEU        = 3, // 3
  SELECT_PEDESTAL   = 4, // 2
  SELECT_TIME       = 5, // 1
  OVERWRITE         = 6, // 0
  NFLAGS = 7
};
}

// This is needed to call the following functions from Python using ctypes
#ifdef __cplusplus
extern "C" {
#endif

int wgMakeHist(const char * x_input_file_name,
               const char * x_pyrame_config_file,
               const char * x_output_dir,
               const unsigned long ul_flags,
               unsigned dif = 0);
  
#ifdef __cplusplus
}
#endif

#endif // WG_MAKEHIST_HPP_
