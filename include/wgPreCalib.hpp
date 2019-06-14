#ifndef WG_PRE_CALIB_HPP_
#define WG_PRE_CALIB_HPP_

// system C++ includes
#include <string>

// system C includes
#include <cstdbool>

// user includes
#include "Const.hpp"

#define TWIN_PEAKS      1
#define LONELY_MOUNTAIN 0

#define ONE_PE 0
#define TWO_PE 1

#define PC_SUCCESS                  0
#define ERR_CANNOT_CREATE_DIRECTORY 1

using namespace std;

template < typename T>
pair<bool, int > findInVector(const vector<T>& vecOfElements, const T& element)
{
  pair<bool, int > result;
  // Find given element in vector
  auto it = find(vecOfElements.begin(), vecOfElements.end(), element);
  if (it != vecOfElements.end()) {
	result.second = distance(vecOfElements.begin(), it);
	result.first = true;
  }
  else {
	result.first = false;
	result.second = -1;
  }
  return result;
}
vector<string> ListFiles(const string& inputDirName);
double cal_mean(vector<double>);

// This is needed to call the following functions from Python using ctypes
#ifdef __cplusplus
extern "C" {
#endif

  int wgPreCalib(const char * x_inputDir,
				 const char * x_outputXMLDir,
				 const char * x_outputIMGDir,
				 int mode,
				 unsigned n_difs = NDIFS,
				 unsigned n_chips = NCHIPS,
				 unsigned n_chans = NCHANNELS);
  
#ifdef __cplusplus
}
#endif

#endif // WG_PRE_CALIB_HPP_
