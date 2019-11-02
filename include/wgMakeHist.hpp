#ifndef WG_MAKEHIST_HPP_
#define WG_MAKEHIST_HPP_

// system includes
#include <bitset>

// user includes
#include "wgConst.hpp"

enum WG_MAKEHIST_MODE {
  SELECT_DARK_NOISE = 0,
  SELECT_CHARGE,
  SELECT_PEDESTAL,
  SELECT_TIME,
  N_MODES
};

// Set the flags according to the mode
void ModeSelect(int mode, std::bitset<N_MODES>& flag);

// This is needed to call the following functions from Python using ctypes
#ifdef __cplusplus
extern "C" {
#endif

int wgMakeHist(const char * x_input_file_name,
               const char * x_pyrame_config_file,
               const char * x_output_dir,
               const int mode,
               bool overwrite = false,
               unsigned dif = 0);
  
#ifdef __cplusplus
}
#endif

#endif // WG_MAKEHIST_HPP_
