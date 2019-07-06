#ifndef WGDECODERNEW_H
#define WGDECODERNEW_H

// system includes
#include <bitset>
#include <istream>
#include <functional>

// user includes
#include "wgConst.hpp"
#include "wgDecoder.hpp"

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
    unsigned ichip;
    MarkerType type;
  };

  MarkerSeeker(const RawDataConfig &config);

  Section SeekNextSection(std::istream& is, unsigned current_chip);

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

  unsigned NextSectionType(const unsigned last_section_type, const unsigned ichip);
  
  void InitializeRing();
};

unsigned GetNumberOfLinesToRead(const MarkerSeeker::Section & section);

#endif /* WGDECODERNEW_H */
