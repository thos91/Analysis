#ifndef WGDECODERNEW_H
#define WGDECODERNEW_H

// system includes
#include <bitset>
#include <istream>
#include <functional>

// user includes
#include "wgConst.hpp"

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

///////////////////////////////////////////////////////////////////////////////
//                            RawDataConfig class                            //
///////////////////////////////////////////////////////////////////////////////

class RawDataConfig {

 public:
  const unsigned n_chips = NCHIPS;
  const unsigned n_channels = NCHANNELS;
  const unsigned n_chip_id = 1;
  const unsigned max_event = MAX_EVENT;
  RawDataConfig();
  RawDataConfig(unsigned n_chips, unsigned n_channels, unsigned n_chip_id);
};

///////////////////////////////////////////////////////////////////////////////
//                             MarkerSeeker class                            //
///////////////////////////////////////////////////////////////////////////////

class MarkerSeeker {

 public:

  // Used for array indexes!  Don't change the numbers!
  enum MarkerType {
    SpillNumber = 0,
    SpillHeader,
    ChipHeader,
    RawData,
    ChipTrailer,
    SpillTrailer
  };

  struct Section {
    std::streampos start;
    std::streampos stop;
    MarkerType type;
  };

  MarkerSeeker(const RawDataConfig &config);

  Section GetNextSection(std::istream& is, unsigned current_chip);

 private:

  static const std::size_t m_num_marker_types = NUM_MARKER_TYPES;
  typedef std::function<bool(std::istream& is)> seeker;
  std::array<seeker, m_num_marker_types> m_seekers_ring;
  RawDataConfig m_config;
  unsigned m_last_section_type = MarkerType::SpillNumber;
  Section m_current_section;

  bool SeekSpillNumber (std::istream& is);
  bool SeekSpillHeader (std::istream& is);
  bool SeekChipHeader  (std::istream& is);
  bool SeekChipTrailer (std::istream& is);
  bool SeekSpillTrailer(std::istream& is);
  bool SeekRawData     (std::istream& is);

  unsigned NextSectionType(const unsigned last_section_type, unsigned& current_chip);
  
  void InitializeRing();
};

///////////////////////////////////////////////////////////////////////////////
//                             EventReader class                             //
///////////////////////////////////////////////////////////////////////////////

class EventReader {

 private:

  Raw_t rd;

  void ReadSpillNumber (std::istream& is);
  void ReadSpillHeader (std::istream& is);
  void ReadChipHeader  (std::istream& is);
  void ReadChipTrailer (std::istream& is);
  void ReadSpillTrailer(std::istream& is);
};

#endif /* WGDECODERNEW_H */
