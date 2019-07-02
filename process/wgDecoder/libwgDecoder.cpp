// system includes
#include <bitset>
#include <istream>
#include <functional>

// user includes
#include "wgDecoder.hpp"
#include "wgLogger.hpp"

///////////////////////////////////////////////////////////////////////////////
//                            RawDataConfig class                            //
///////////////////////////////////////////////////////////////////////////////

RawDataConfig::RawDataConfig() {}
RawDataConfig::RawDataConfig(unsigned n_chips, unsigned n_channels, unsigned n_chip_id)
    : n_chips(n_chips), n_channels(n_channels), n_chip_id(n_chip_id) {}

///////////////////////////////////////////////////////////////////////////////
//                             MarkerSeeker class                            //
///////////////////////////////////////////////////////////////////////////////

MarkerSeeker::MarkerSeeker(const RawDataConfig &config) : m_config(config) {
  InitializeRing();
}

MarkerSeeker::Section MarkerSeeker::GetNextSection(std::istream& is, unsigned current_chip) {
  unsigned section_type = m_last_section_type;
  bool found = false;
  do {
    // Choose what is the next section. In case the current_chip section couldn't be found
    section_type = NextSectionType(section_type, current_chip);
    seeker iseeker = m_seekers_ring[section_type];
    found = iseeker(is);
  } while (found);

  m_last_section_type = section_type;
    
  return m_current_section;
}

unsigned MarkerSeeker::NextSectionType(const unsigned last_section_type, unsigned& current_chip) {
  // Select what is the next section to look for. Only if the last
  // section whas a chip trailer we need to be cautious because we
  // may need to go back to the ChipHeader and increment the current_chip by one
  if (last_section_type == ChipTrailer && current_chip < m_config.n_chips) {
    current_chip++;
    return ChipHeader;
  } else {
    return last_section_type + 1 % m_num_marker_types;
  }
}

std::streampos ReadLine(std::istream& is, std::bitset<M>& raw_data) {
  is.read((char*) &raw_data, M / 8);
  if (is) throw wgInvalidFile("EOF");
  return is.tellg();
}

template<std::size_t SIZE>
std::streampos ReadChunk(std::istream& is, std::array<std::bitset<M>, SIZE>& raw_data) {
  for (unsigned i = 0; i < raw_data.size(); ++i) {
    is.read((char*) &raw_data[i], M / 8);
    if (is) throw wgInvalidFile("EOF");
  }
  return is.tellg();
}



bool MarkerSeeker::SeekSpillNumber(std::istream& is) {
  std::streampos start_read = is.tellg();
  std::array<std::bitset<M>, SPILL_NUMBER_LENGTH> raw_data;
  std::streampos stop_read = ReadChunk(is, raw_data);
  // Everything should be fine
  if ((raw_data[0] == SPILL_NUMBER_MARKER) &&
      (raw_data[2] != SPILL_HEADER_MARKER)) {
    m_current_section.start = start_read;
    m_current_section.stop = stop_read;
    m_current_section.type = SpillNumber;
    return true;
  }
  // This means that the spill number of spill flag was skipped. In
  // that case we just ignore the spill number section and go on with
  // reading the spill header section
  else if (raw_data[SPILL_NUMBER_LENGTH - 1] == SPILL_HEADER_MARKER) {
    // rewind just before the spill header
    is.seekg(- M / 8, ios::cur);
    return false;
  }
  // In any other case just rewind to the beginning and try again with
  // another seeker
  is.seekg(start_read);
  return false;
}

template < typename T, std::size_t SIZE>
std::pair<bool, std::size_t> FindInArray(const std::array<T, SIZE>& array_of_elements, const T& element) {
  std::pair<bool, std::size_t > result;
 
  // Find given element in std::array
  auto it = std::find(array_of_elements.begin(), array_of_elements.end(), element);
 
  if (it != array_of_elements.end()) {
    result.second = distance(array_of_elements.begin(), it);
    result.first = true;
  }
  else {
    result.first = false;
    result.second = -1;
  }
  return result;
}

bool MarkerSeeker::SeekSpillHeader(std::istream& is) {
  std::streampos start_read = is.tellg();
  std::array<std::bitset<M>, SPILL_HEADER_LENGTH> raw_data;
  std::streampos stop_read = ReadChunk(is, raw_data);

  bool has_header_marker, has_SP_marker, has_IL_marker, has_space_marker;
  std::size_t header_marker_pos, SP_marker_pos, IL_marker_pos, space_marker_pos;
  std::tie(has_header_marker, header_marker_pos) = FindInArray(raw_data, SPILL_HEADER_MARKER);
  std::tie(has_SP_marker, SP_marker_pos) = FindInArray(raw_data, SP_MARKER);
  std::tie(has_IL_marker, IL_marker_pos) = FindInArray(raw_data, IL_MARKER);
  std::tie(has_space_marker, space_marker_pos) = FindInArray(raw_data, SPACE_MARKER);

  // If everything is in place just return and call it a day
  if (has_header_marker && has_SP_marker && has_IL_marker && has_space_marker &&
      (space_marker_pos - header_marker_pos == SPILL_HEADER_LENGTH) ) {
    m_current_section.start = start_read;
    m_current_section.stop = stop_read;
    m_current_section.type = SpillHeader;
    return true;
    // we can still work with a partially corrupted header, if it has
    // the header marker and the SP marker and they are correctly
    // spaced.
  } else if (has_header_marker && has_SP_marker &&
             (SP_marker_pos - header_marker_pos == 2)) {
    // Just make sure that the chip header is following in the next 10
    // lines.
    is.seekg(start_read);
    unsigned i = 0;
    std::bitset<M> raw_data_line;
    do { ReadLine(is, raw_data_line); }
    while (raw_data_line != CHIP_HEADER_MARKER && i++ < 10);
    if (i < 10) {
      is.seekg(- M / 8, ios::cur);
      m_current_section.start = start_read;
      m_current_section.stop = is.tellg();
      m_current_section.type = SpillHeader;
      return true;
    }
  }
  // else all is lost and try to find the next section
  else if (has_header_marker) {
     unsigned i = 0;
    std::bitset<M> raw_data_line;
    do { ReadLine(is, raw_data_line); }
    while (raw_data_line != CHIP_HEADER_MARKER && i++ < 10);
    if (i < 10) {
      is.seekg(- M / 8, ios::cur);
      m_current_section.start = start_read;
      m_current_section.stop = is.tellg();
      m_current_section.type = SpillHeader;
      return true;
    }
  }
  
  // In all other cases just rewind and try with another seeker  
  is.seekg(start_read);
  return false;
}

bool MarkerSeeker::SeekChipHeader(std::istream& is) {
  std::streampos start_read = is.tellg();
  std::array<std::bitset<M>, CHIP_HEADER_LENGTH> raw_data;
  std::streampos stop_read = ReadChunk(is, raw_data);

  bool has_header_marker, has_CH_marker, has_IP_marker, has_space_marker;
  std::size_t header_marker_pos, CH_marker_pos, IP_marker_pos, space_marker_pos;
  std::tie(has_header_marker,header_marker_pos) = FindInArray(raw_data, CHIP_HEADER_MARKER);
  std::tie(has_CH_marker, CH_marker_pos) = FindInArray(raw_data, CH_MARKER);
  std::tie(has_IP_marker, IP_marker_pos) = FindInArray(raw_data, IP_MARKER);
  std::tie(has_space_marker, space_marker_pos) = FindInArray(raw_data, SPACE_MARKER);

  // If everything is in place just return true and call it a day
  if (has_header_marker && has_CH_marker && has_IP_marker && has_space_marker &&
      (space_marker_pos - header_marker_pos == CHIP_HEADER_LENGTH)) {
    m_current_section.start = start_read;
    m_current_section.stop = stop_read;
    m_current_section.type = ChipHeader;
    return true;
    // we can still work with a partially corrupted header, if it has
    // the header marker and the CH marker and they are correctly
    // spaced.
  } else if (has_header_marker && has_CH_marker &&
             (CH_marker_pos - header_marker_pos == 2)) {
    // XOR between IP_marker and space_marker (execute only when one
    // is true and the other is false). In that case we just have to
    // rewind one line and we are good to go.
    if (has_IP_marker != has_space_marker) {
      is.seekg(- M / 8, ios::cur);
      m_current_section.start = start_read;
      m_current_section.stop = is.tellg();
      m_current_section.type = ChipHeader;
      return true;
    }
    // If both are false rewind two lines and go on.
    else if (!has_IP_marker && !has_CH_marker ) {
      is.seekg(- 2 * M / 8, ios::cur);
      m_current_section.start = start_read;
      m_current_section.stop = is.tellg();
      m_current_section.type = ChipHeader;
      return true;
    }
    // as a last resort try to recover from an heavily corrupted chip
    // header by skipping it altogether. Try to guess the end of the
    // corrupted chip header by looking for the first and last
    // markers.
  } else if (has_header_marker && has_space_marker) {
    is.seekg(start_read + std::streampos(space_marker_pos * M / 8));
    return false;
  } else if (has_header_marker && has_IP_marker) {
    is.seekg(start_read + std::streampos(IP_marker_pos * M / 8));
    return false;
  } else if (has_header_marker && has_CH_marker) {
    is.seekg(start_read + std::streampos(CH_marker_pos * M / 8));
    return false;
  }
  
  // In all other cases just rewind and try with another seeker  
  is.seekg(start_read);
  return false;
}

bool MarkerSeeker::SeekChipTrailer(std::istream& is) {
  std::streampos start_read = is.tellg();
  std::array<std::bitset<M>, CHIP_TRAILER_LENGTH> raw_data;
  std::streampos stop_read = ReadChunk(is, raw_data);
  // Everything is good
  if (raw_data[0] == CHIP_TRAILER_MARKER &&
     raw_data[2] == SPACE_MARKER &&
     raw_data[3] == SPACE_MARKER) {
    m_current_section.start = start_read;
    m_current_section.stop = stop_read;
    m_current_section.type = ChipTrailer;
    return true;
  }
  // If one of the x2020 spaces is missing
  else if (raw_data[0] == CHIP_TRAILER_MARKER &&
           raw_data[2] == SPACE_MARKER) {
    is.seekg(- M / 8, ios::cur);
    m_current_section.start = start_read;
    m_current_section.stop = is.tellg();
    m_current_section.type = ChipTrailer;
    return true;
  }
  else if (raw_data[0] == CHIP_TRAILER_MARKER &&
           raw_data[1] == SPACE_MARKER &&
           raw_data[2] == SPACE_MARKER) {
    is.seekg(- M / 8, ios::cur);
    return false;
  }
  else if (raw_data[0] == CHIP_TRAILER_MARKER &&
           raw_data[1] == SPACE_MARKER) {
    is.seekg(- 2 * M / 8, ios::cur);
    return false;
  }
  // In all other cases just rewind and try with another seeker  
  is.seekg(start_read);
  return false;
}

bool MarkerSeeker::SeekSpillTrailer(std::istream& is) {
  
  return true;
}

bool MarkerSeeker::SeekRawData(std::istream& is) {
  return true;
}

void MarkerSeeker::InitializeRing() {
  m_seekers_ring[MarkerType::SpillNumber]  = std::bind(&MarkerSeeker::SeekSpillNumber,  this, std::placeholders::_1);
  m_seekers_ring[MarkerType::SpillHeader]  = std::bind(&MarkerSeeker::SeekSpillHeader,  this, std::placeholders::_1);
  m_seekers_ring[MarkerType::ChipHeader]   = std::bind(&MarkerSeeker::SeekChipHeader,   this, std::placeholders::_1);
  m_seekers_ring[MarkerType::RawData]      = std::bind(&MarkerSeeker::SeekRawData,      this, std::placeholders::_1);
  m_seekers_ring[MarkerType::ChipTrailer]  = std::bind(&MarkerSeeker::SeekChipTrailer,  this, std::placeholders::_1);
  m_seekers_ring[MarkerType::SpillTrailer] = std::bind(&MarkerSeeker::SeekSpillTrailer, this, std::placeholders::_1);
}

///////////////////////////////////////////////////////////////////////////////
//                             EventReader class                             //
///////////////////////////////////////////////////////////////////////////////

void EventReader::ReadSpillNumber(std::istream& is) {}
void EventReader::ReadSpillHeader(std::istream& is) {}
void EventReader::ReadChipHeader(std::istream& is) {}
void EventReader::ReadChipTrailer(std::istream& is) {}
void EventReader::ReadSpillTrailer(std::istream& is) {}
