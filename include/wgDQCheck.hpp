#ifndef WG_DQCHECK_HPP_
#define WG_DQCHECK_HPP_

// This is needed to call the following functions from Python using ctypes
#ifdef __cplusplus
extern "C" {
#endif

  int wgDQCheck(const char * x_summary_dir,
                const char * x_calib_dir,
                const char * x_output_dir);
  
#ifdef __cplusplus
}
#endif

#endif // WG_DQCHECK_HPP_
