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

// markers
#define SPILL_NUMBER_MARKER  bitset<M>(0xFFFB)
#define SPILL_HEADER_MARKER  bitset<M>(0xFFFC)
#define CHIP_HEADER_MARKER   bitset<M>(0xFFFD)
#define CHIP_TRAILER_MARKER  bitset<M>(0xFFFE)
#define SPILL_TRAILER_MARKER bitset<M>(0xFFFF)
#define SPACE_MARKER         bitset<M>(0x2020)
#define SP_MARKER            bitset<M>(0x5053)
#define IL_MARKER            bitset<M>(0x4C49)
#define CH_MARKER            bitset<M>(0x4843)
#define IP_MARKER            bitset<M>(0x5049)

// masks
#define x000F                bitset<M>(0x00FF)
#define x00FF                bitset<M>(0x00FF)
#define x0FFF                bitset<M>(0x0FFF)
#define xFFF0                bitset<M>(0x0FFF)
#define xFF00                bitset<M>(0xFF00)
#define xF000                bitset<M>(0xF000)
#define x0000                bitset<M>(0x0000)

///////////////////////////////////////////////////////////////////////////////
//                            RawDataConfig class                            //
///////////////////////////////////////////////////////////////////////////////

class RawDataConfig {

 public:
  const unsigned n_chips = NCHIPS;
  const unsigned n_channels = NCHANNELS;
  const unsigned n_chip_id = 1;
  const unsigned max_event = MAX_EVENT;

  RawDataConfig() {}
  RawDataConfig(unsigned n_chips, unsigned n_channels, unsigned n_chip_id)
      : n_chips(n_chips), n_channels(n_channels), n_chip_id(n_chip_id) {}
};

#include "wgDecoderUtils.tpp"

#endif /* WGDECODER_HPP_ */
