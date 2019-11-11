#ifndef WG_GAIN_CALIB_HPP_
#define WG_GAIN_CALIB_HPP_

// system includes
#include <vector>
#include <bitset>
#include <map>

// user includes
#include "wgConst.hpp"

#ifdef __cplusplus
extern "C" {
#endif

  int wgGainCalib(const char * x_inputDir,
                  const char * x_outputXMLDir,
                  const char * x_outputIMGDir,
                  const bool only_wallmrd,
                  const bool only_wagasci);
  
#ifdef __cplusplus
}
#endif

namespace gain_calib {

const unsigned MAX_GAIN = 150;
const unsigned MIN_GAIN = 10;
const unsigned MAX_SIGMA = MAX_GAIN;
const unsigned MIN_SIGMA = 2;

//           CHIP         CHANNEL
typedef std::vector <std::vector <int>> ChargeVector;
//               iDAC           PEU              DIF
typedef std::map<unsigned, std::array <std::map <unsigned, ChargeVector>, NUM_PE>> Charge;
//               DIF            CHIP         CHANNEL
typedef std::map<unsigned, std::vector <std::bitset <NCHANNELS>>> BadChannels;
//               DIF            CHIP         CHANNEL
typedef std::map<unsigned, std::vector <std::vector <double>>> LinearFit;

typedef std::vector<std::string> DirList;

} // gain_calib

#endif // WG_GAIN_CALIB_HPP_
