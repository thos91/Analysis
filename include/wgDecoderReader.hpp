#ifndef WGDECODERREADER_HPP_
#define WGDECODERREADER_HPP_

// system includes
#include <bitset>
#include <istream>
#include <functional>

// user includes
#include "wgConst.hpp"
#include "wgDecoder.hpp"
#include "wgDecoderSeeker.hpp"

// Debug macros that will fill the debug histogram
// spill
#define DEBUG_SPILL_MODE        0
#define DEBUG_SAME_SPILL_NUMBER 1
#define DEBUG_SPILL_NUMBER_GAP  2
#define DEBUG_SAME_SPILL_COUNT  3
#define DEBUG_SPILL_COUNT_GAP   4
#define DEBUG_SPILL_TRAILER     5
#define DEBUG_WRONG_NCHIPS      6

// chip
#define DEBUG_WRONG_BCID        0
#define DEBUG_WRONG_HIT_BIT     1
#define DEBUG_WRONG_GAIN_BIT    2
#define DEBUG_WRONG_ADC         3
#define DEBUG_WRONG_TDC         4
#define DEBUG_WRONG_CHIPID      5

///////////////////////////////////////////////////////////////////////////////
//                             EventReader class                             //
///////////////////////////////////////////////////////////////////////////////

class EventReader {
 public:

  EventReader(const RawDataConfig& config, TTree * tree, Raw_t & rd);
  
  void ReadNextSection(std::istream& is, const MarkerSeeker::Section section);
  
 private:

  RawDataConfig m_config;
  TTree * m_tree;
  std::reference_wrapper<Raw_t> m_rd;
  unsigned m_last_spill_number = 0;
  unsigned m_last_spill_count = 0;
  
  std::size_t m_num_marker_types;
  typedef std::function<void(std::istream& is, const MarkerSeeker::Section& section)> reader;
  std::array<reader, NUM_MARKER_TYPES> m_readers_ring;
  
  void ReadSpillNumber (std::istream& is, const MarkerSeeker::Section& section);
  void ReadSpillHeader (std::istream& is, const MarkerSeeker::Section& section);
  void ReadChipHeader  (std::istream& is, const MarkerSeeker::Section& section);
  void ReadChipTrailer (std::istream& is, const MarkerSeeker::Section& section);
  void ReadSpillTrailer(std::istream& is, const MarkerSeeker::Section& section);
  void ReadRawData     (std::istream& is, const MarkerSeeker::Section& section);

  void InitializeRing();

  void FillTree();
};

#endif /* WGDECODERREADER_HPP_ */
