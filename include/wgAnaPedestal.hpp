#ifndef WG_ANAPEDESTAL_HPP_
#define WG_ANAPEDESTAL_HPP_

// system C++ includes
#include <string>

// system C includes
#include <cstdbool>

// user includes
#include "Const.h"

// Errors
#define AP_SUCCESS                  0
#define ERR_CANNOT_CREATE_DIRECTORY 1
#define ERR_FAILED_OPEN_XML_FILE    2
#define ERR_WRONG_PE_VALUE          3

using namespace std;

// This is needed to call the following functions from Python using ctypes
#ifdef __cplusplus
extern "C" {
#endif

  // This little function tries to guess the threshold level (0.5, 1.5 or 2.5)
  // given the value of the noise rate.
  double NoiseToPe(double);

  void MakeSummaryXmlFile(const string& str,
						  bool overwrite,
						  unsigned n_chips = NCHIPS,
						  unsigned n_chans = NCHANNELS);

  int AnaPedestal(const char * x_inputDir,
				  const char * x_outputXMLDir,
				  const char * x_outputIMGDir,
				  bool pre_calibration_mode = false,
				  bool overwrite = false,
				  unsigned n_chips = NCHIPS,
				  unsigned n_chans = NCHANNELS);

#ifdef __cplusplus
}
#endif

#endif // WG_ANAPEDESTAL_HPP_
