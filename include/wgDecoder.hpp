#ifndef WG_DECODER_HPP_
#define WG_DECODER_HPP_

// system includes
#include <string>
#include <bits/stdc++.h>

// user includes
#include "Const.hpp"

// #define DEBUG_DECODE

// Debug macros that will fill the debug histogram
#define DEBUG_NODATA 1
#define DEBUG_GOOD_SPILLGAP 2
#define DEBUG_BAD_SPILLGAP 4
#define DEBUG_BAD_CHIPNUM 8
#define DEBUG_BAD_CHIPDATA_SIZE 16
#define DEBUG_MISSING_CHIPID_TAG 32
#define DEBUG_MISSING_CHIP_HEADER 64
#define DEBUG_MISSING_CHIP_TRAILER 128
#define DEBUG_MISSING_CHIP_TRAILER_ONLY_ONE_CHIP 256

// bitset macros
#define M 16
#define x00FF bitset<M>(0x00FF)
#define x0FFF bitset<M>(0x0FFF)
#define xF000 bitset<M>(0xF000)
#define x2020 bitset<M>(0x2020)
#define x5053 bitset<M>(0x5053)
#define x4C49 bitset<M>(0x4C49)
#define x4843 bitset<M>(0x4843)
#define x5049 bitset<M>(0x5049)
#define xFFFF bitset<M>(0xFFFF)
#define xFFFE bitset<M>(0xFFFE)
#define xFFFD bitset<M>(0xFFFD)
#define xFFFC bitset<M>(0xFFFC)
#define xFFFB bitset<M>(0xFFFB)

using namespace std;

// This is needed to call the following functions from Python using ctypes
#ifdef __cplusplus
extern "C" {
#endif

  // wgDecoder
  // Main decoder function (here is were all the fun happens)
  int wgDecoder(const char * x_inputFileName,
				const char * x_calibFileName,
				const char * x_pedFileName,
				const char * x_tdcFileName,
				const char * x_outputDir,
				bool overwrite,
				unsigned maxEvt,
				unsigned dif = 0,
				unsigned n_chips = NCHIPS,
				unsigned n_channels = NCHANNELS);

  // check_ChipHeader
  /* Checks if the ChipHeader is well formed. If the number of chip is greater
	 than "n_chips" or if the size of "head" vector is less than offset + 4, the
	 0xFFFF value is returned and the "checkid_exist" flag is set to
	 false. Otherwise, if there are other missing 2Bytes in the header, the
	 "Missing_Header" counter is increased for any 2Bytes that are missing. */
  uint16_t check_ChipHeader(unsigned n_chips, vector<bitset<M>>& head, size_t offset, bool& checkid_exist, int& Missing_Header);

  // check_ChipID
  /* Check that the chip id is not less that zero and greater than 40 */ 
  bool check_ChipID(int16_t v_chipid, uint16_t n_chips);

  // tdc2time
  /* If the detector is calibrated (if the TDC coefficient file is present) this
	 function converts the raw TDC into an absolute time in nanoseconds */
  int tdc2time(d3vector &time_ns, i3vector &time, i2vector &bcid, d3vector &slope, d3vector &intcpt);

  // rd_clear
  // Clear the Raw_t rd arrays
  void rd_clear(Raw_t &rd);

#ifdef __cplusplus
}
#endif

#endif // WG_DECODER_HPP_
