#ifndef WGDECODER_HPP_
#define WGDECODER_HPP_

// system includes
#include <bitset>
#include <istream>
#include <functional>
#include <vector>

// user includes
#include "wgConst.hpp"
#include "wgExceptions.hpp"

#define NUM_MARKER_TYPES 6

// bitset macros
#define M 16
#define BYTES_PER_LINE 2

// markers
#define SPILL_NUMBER_MARKER  std::bitset<M>(0xFFFB)
#define SPILL_HEADER_MARKER  std::bitset<M>(0xFFFC)
#define CHIP_HEADER_MARKER   std::bitset<M>(0xFFFD)
#define CHIP_TRAILER_MARKER  std::bitset<M>(0xFFFE)
#define SPILL_TRAILER_MARKER std::bitset<M>(0xFFFF)
#define SPACE_MARKER         std::bitset<M>(0x2020)
#define SP_MARKER            std::bitset<M>(0x5053)
#define IL_MARKER            std::bitset<M>(0x4C49)
#define CH_MARKER            std::bitset<M>(0x4843)
#define IP_MARKER            std::bitset<M>(0x5049)

// masks
#define x000F                std::bitset<M>(0x00FF)
#define x00FF                std::bitset<M>(0x00FF)
#define x0FFF                std::bitset<M>(0x0FFF)
#define xFFF0                std::bitset<M>(0x0FFF)
#define xFF00                std::bitset<M>(0xFF00)
#define xF000                std::bitset<M>(0xF000)
#define x0000                std::bitset<M>(0x0000)

// Debug macros that will fill the debug histogram
// spill
#define DEBUG_SPILL_MODE        0
#define DEBUG_SAME_SPILL_NUMBER 1
#define DEBUG_SPILL_NUMBER_GAP  2
#define DEBUG_SAME_SPILL_COUNT  3
#define DEBUG_SPILL_COUNT_GAP   4
#define DEBUG_WRONG_CHIPID      5
#define DEBUG_SPILL_TRAILER     6
#define DEBUG_WRONG_NCHIPS      7

// chip
#define DEBUG_WRONG_BCID        0
#define DEBUG_WRONG_HIT_BIT     1
#define DEBUG_WRONG_GAIN_BIT    2
#define DEBUG_WRONG_ADC         3
#define DEBUG_WRONG_TDC         4

// Error codes
#define DE_SUCCESS                       0
#define ERR_CANNOT_CREATE_DIRECTORY      1
#define ERR_CANNOT_OVERWRITE_OUTPUT_FILE 2
#define ERR_WRONG_DIF_VALUE              3
#define ERR_FAILED_OPEN_RAW_FILE         4
#define ERR_INPUT_FILE_NOT_FOUND         5

///////////////////////////////////////////////////////////////////////////////
//                            RawDataConfig class                            //
///////////////////////////////////////////////////////////////////////////////

class RawDataConfig {

 public:
  const unsigned n_chips     = NCHIPS;
  const unsigned n_channels  = NCHANNELS;
  const unsigned n_chip_id   = 1;
  const unsigned max_event   = MAX_EVENT;
  const bool spill_insert_flag = false;
  const bool adc_is_calibrated = false;
  const bool tdc_is_calibrated = false;

  unsigned last_spill_number = 0;
  unsigned last_spill_count  = 0;
  
  RawDataConfig() {}
  RawDataConfig(unsigned n_chips, unsigned n_channels, unsigned n_chip_id, unsigned max_event,
                bool spill_insert_flag, bool adc_is_calibrated, bool tdc_is_calibrated)
      : n_chips(n_chips), n_channels(n_channels), n_chip_id(n_chip_id), max_event(max_event),
        spill_insert_flag(spill_insert_flag), adc_is_calibrated(adc_is_calibrated),
        tdc_is_calibrated(tdc_is_calibrated) {}
};

#include "wgDecoderUtils.tpp"

#endif /* WGDECODER_HPP_ */
