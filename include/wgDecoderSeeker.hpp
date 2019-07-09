#ifndef WGDECODERNEW_H
#define WGDECODERNEW_H

// system includes
#include <bitset>
#include <istream>
#include <functional>

// user includes
#include "wgConst.hpp"
#include "wgDecoder.hpp"
#include "wgDecoderUtils.hpp"

///////////////////////////////////////////////////////////////////////////////
//                             MarkerSeeker class                            //
///////////////////////////////////////////////////////////////////////////////

class MarkerSeeker {

 public:

  // Used for array indexes!  Don't change the numbers!
  enum MarkerType {
    SpillHeader = 0,
    ChipHeader,
    RawData,
    ChipTrailer,
    SpillTrailer,
    SpillNumber
  };

  struct Section {
    std::streampos start;
    std::streampos stop;
    unsigned ichip = 0;
    unsigned ispill = 0;
    unsigned type;
  };

  MarkerSeeker(const RawDataConfig &config);

  Section SeekNextSection(std::istream& is);

 private:

  std::size_t m_num_marker_types;

  typedef std::function<bool(std::istream& is)> seeker;
  std::array<seeker, NUM_MARKER_TYPES> m_seekers_ring;

  RawDataConfig m_config;
  Section m_current_section;
  unsigned m_last_ispill = 0;
  unsigned m_last_ichip = 0;
  
  bool SeekSpillNumber (std::istream& is);
  bool SeekSpillHeader (std::istream& is);
  bool SeekChipHeader  (std::istream& is);
  bool SeekChipTrailer (std::istream& is);
  bool SeekSpillTrailer(std::istream& is);
  bool SeekRawData     (std::istream& is);

  unsigned NextSectionType(unsigned last_section_type, bool last_section_was_found);
  
  void InitializeRing();
};

unsigned GetNumberOfLinesToRead(const MarkerSeeker::Section & section);

#endif /* WGDECODERNEW_H */
