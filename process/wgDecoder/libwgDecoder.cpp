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
std::pair<bool, int> FindInArray(const std::array<T, SIZE>& array_of_elements, const T& element) {
  std::pair<bool, int > result;
 
  // Find given element in vector
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

  std::pair<bool, int> header_marker = FindInArray(raw_data, SPILL_HEADER_MARKER);
  std::pair<bool, int> SP_marker     = FindInArray(raw_data, SP_MARKER);
  std::pair<bool, int> IL_marker     = FindInArray(raw_data, IL_MARKER);
  std::pair<bool, int> space_marker  = FindInArray(raw_data, SPACE_MARKER);

  // If everything is in place just return and call it a day
  if (header_marker.first && SP_marker.first && IL_marker.first && space_marker.first &&
      (space_marker.second - header_marker.second == SPILL_HEADER_LENGTH) ) {
    m_current_section.start = start_read;
    m_current_section.stop = stop_read;
    m_current_section.type = SpillHeader;
    return true;
    // we can still work with a partially corrupted header, if it has
    // the header marker and the SP marker and they are correctly
    // spaced.
  } else if (header_marker.first && SP_marker.first &&
             (SP_marker.second - header_marker.second == 2)) {
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
  // In all other cases just rewind and try with another seeker  
  is.seekg(start_read);
  return false;
}

bool MarkerSeeker::SeekChipHeader(std::istream& is) {
  std::streampos start_read = is.tellg();
  std::vector<std::bitset<M>> raw_data(CHIP_HEADER_LENGTH);
  std::streampos stop_read = ReadChunk(is, raw_data);

  std::pair<bool, int> header_marker = FindInVector(raw_data, SPILL_HEADER_MARKER);
  std::pair<bool, int> SP_marker     = FindInVector(raw_data, SP_MARKER);
  std::pair<bool, int> IL_marker     = FindInVector(raw_data, IL_MARKER);
  std::pair<bool, int> space_marker  = FindInVector(raw_data, SPACE_MARKER);

  // If everything is in place just return and call it a day
  if (header_marker.first && SP_marker.first && IL_marker.first && space_marker.first &&
      (space_marker.second - header_marker.second == SPILL_HEADER_LENGTH) ) {
    m_current_section.start = start_read;
    m_current_section.stop = stop_read;
    m_current_section.type = SpillHeader;
    return true;
    // we can still work with a partially corrupted header, if it has
    // the header marker and the SP marker and they are correctly
    // spaced.
  } else if (header_marker.first && SP_marker.first &&
             (SP_marker.second - header_marker.second == 2)) {
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
  // In all other cases just rewind and try with another seeker  
  is.seekg(start_read);
  return false;
}

bool MarkerSeeker::SeekChipTrailer(std::istream& is) {
  return true;
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
