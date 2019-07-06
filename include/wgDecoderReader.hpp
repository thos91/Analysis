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

///////////////////////////////////////////////////////////////////////////////
//                             EventReader class                             //
///////////////////////////////////////////////////////////////////////////////

class EventReader {
 public:

  EventReader(const RawDataConfig& config);
  
  void ReadNextSection(std::istream& is, MarkerSeeker::Section section);
  
 private:

  RawDataConfig m_config;
  Raw_t m_rd;

  static const std::size_t m_num_marker_types = NUM_MARKER_TYPES;
  typedef std::function<void(std::istream& is, const MarkerSeeker::Section& section)> reader;
  std::array<reader, m_num_marker_types> m_readers_ring;
  
  void ReadSpillNumber (std::istream& is, const MarkerSeeker::Section& section);
  void ReadSpillHeader (std::istream& is, const MarkerSeeker::Section& section);
  void ReadChipHeader  (std::istream& is, const MarkerSeeker::Section& section);
  void ReadChipTrailer (std::istream& is, const MarkerSeeker::Section& section);
  void ReadSpillTrailer(std::istream& is, const MarkerSeeker::Section& section);
  void ReadRawData     (std::istream& is, const MarkerSeeker::Section& section);

  void InitializeRing();
};

#endif /* WGDECODERREADER_HPP_ */
