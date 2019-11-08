#ifndef WG_GAIN_CALIB_UGLY_HPP_
#define WG_GAIN_CALIB_UGLY_HPP_

// system includes
#include <thread>

// user includes
#include "wgConst.hpp"
#include "wgGainCalib.hpp"

#ifdef __cplusplus
extern "C" {
#endif

  int wgGainCalibUgly(const char * x_input_run_dir,
                      const char * x_xml_config_file,
                      const char * x_output_xml_dir,
                      const char * x_output_img_dir,
                      const bool ignore_wagasci );
  
#ifdef __cplusplus
}
#endif

namespace gain_calib {

namespace ugly {

const unsigned PEU_LEVEL = 1;

typedef ChargeVector GainVector;

//               iDAC               DIF
typedef std::map<unsigned, std::map<unsigned, GainVector>> Gain;

}

}

#endif // WG_GAIN_CALIB_UGLY_HPP_
