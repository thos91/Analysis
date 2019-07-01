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

std::streampos ReadChunk(std::istream& is, std::vector<std::bitset<M>>& raw_data) {
  if (raw_data.size() == 0)
    throw std::invalid_argument("need a non zero size vector");
  for (unsigned i = 0; i < raw_data.size(); ++i) {
    is.read((char*) &raw_data[i], M / 8);
    if (is) throw wgInvalidFile("EOF");
  }
  return is.tellg();
}



bool MarkerSeeker::SeekSpillNumber(std::istream& is) {
  std::streampos start_read = is.tellg();
  std::vector<std::bitset<M>> raw_data(SPILL_NUMBER_LENGTH);
  std::streampos stop_read = ReadChunk(is, raw_data);
  if (raw_data[0] == SPILL_NUMBER_MARKER) {
    m_current_section.start = start_read;
    m_current_section.stop = stop_read;
    m_current_section.type = SpillNumber;
    return true;
  }
  is.seekg(start_read);
  return false;
}

  
bool MarkerSeeker::SeekSpillHeader(std::istream& is) {
  std::streampos start_read = is.tellg();
  std::vector<std::bitset<M>> raw_data(SPILL_HEADER_LENGTH);
  std::streampos stop_read = ReadChunk(is, raw_data);

  bool has_header_marker; // find in vector
  // position header
  bool has_SP_marker;  // find in vector
  // position SP
  bool has_IL_marker;  // find in vector
  bool has_space_marker; // find in vector
  if (has_header_marker && has_SP_marker && has_IL_marker && has_space_marker) {
    m_current_section.start = start_read;
    m_current_section.stop = stop_read;
    m_current_section.type = SpillHeader;
    return true;
  } else if (has_header_marker && has_SP_marker && (/* position header - position SP == 2*/)) {
    // we can still work with that
    is.seekg(start_read);
    unsigned i = 0;
    std::bitset<M> raw_data_line;
    do {
      stop_read = ReadLine(is, raw_data_line);
    } while (raw_data_line != CHIP_HEADER_MARKER && i < 10);
    if (i < 10) {
        m_current_section.start = start_read;
        m_current_section.stop = stop_read; // -1
        m_current_section.type = SpillHeader;
        return true;
      }
    }
  } 

  
  is.seekg(start_read);
  return false;
}

bool MarkerSeeker::SeekChipHeader(std::istream& is) {
  return true;
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
