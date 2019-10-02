#ifndef WG_MAKEHIST_HPP_
#define WG_MAKEHIST_HPP_

// user includes
#include "wgConst.hpp"

// This is needed to call the following functions from Python using ctypes
#ifdef __cplusplus
extern "C" {
#endif

int wgMakeHist(const char * x_input_file_name,
               const char * x_pyrame_config_file,
               const char * x_output_dir,
               bool overwrite = false,
               unsigned dif = 0);
  
#ifdef __cplusplus
}
#endif

#endif // WG_MAKEHIST_HPP_
