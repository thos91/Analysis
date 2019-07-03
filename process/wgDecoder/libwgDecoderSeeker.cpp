// system includes
#include <bitset>
#include <istream>
#include <functional>

// user includes
#include "wgDecoder.hpp"
#include "wgDecoderSeeker.hpp"
#include "wgLogger.hpp"

namespace wg_utils = wagasci_decoder_utils;


///////////////////////////////////////////////////////////////////////////////
//                                 MarkerSeeker                              //
///////////////////////////////////////////////////////////////////////////////

MarkerSeeker::MarkerSeeker(const RawDataConfig &config) : m_config(config) {
  InitializeRing();
}

///////////////////////////////////////////////////////////////////////////////
//                               GetNextSection                              //
///////////////////////////////////////////////////////////////////////////////

MarkerSeeker::Section MarkerSeeker::GetNextSection(std::istream& is, unsigned current_chip) {
  unsigned section_type = m_last_section_type;
  bool found = false;
  do {
    section_type = NextSectionType(section_type, current_chip);
    found = m_seekers_ring[section_type](is);
  } while (found && (section_type != m_last_section_type));

  if (!found) {
    std::bitset<M> raw_data_line;
    wg_utils::ReadLine(is, raw_data_line);
    stringstream res;
    res << hex << uppercase << raw_data_line.to_ulong();
    Log.eWrite("[wgDecoder] Line \"" + res.str() + "\" not recognized. Skipping it.");
    return this->GetNextSection(is, current_chip);
  }
  
  m_last_section_type = section_type;
    
  return m_current_section;
}

///////////////////////////////////////////////////////////////////////////////
//                              NextSectionType                              //
///////////////////////////////////////////////////////////////////////////////

unsigned MarkerSeeker::NextSectionType(const unsigned last_section_type, unsigned& current_chip) {
  // Select what is the next section to look for. Only if the last
  // section whas a chip trailer we need to be cautious because we may
  // need to go back to the ChipHeader and increment the current_chip
  // by one
  if (last_section_type == ChipTrailer && current_chip < m_config.n_chips) {
    current_chip++;
    return ChipHeader;
  } else {
    return last_section_type + 1 % m_num_marker_types;
  }
}

///////////////////////////////////////////////////////////////////////////////
//                              SeekSpillNumber                              //
///////////////////////////////////////////////////////////////////////////////

bool MarkerSeeker::SeekSpillNumber(std::istream& is) {
  std::streampos start_read = is.tellg();
  std::array<std::bitset<M>, SPILL_NUMBER_LENGTH> raw_data;
  std::streampos stop_read = wg_utils::ReadChunk(is, raw_data);
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

///////////////////////////////////////////////////////////////////////////////
//                              SeekSpillHeader                              //
///////////////////////////////////////////////////////////////////////////////

bool MarkerSeeker::SeekSpillHeader(std::istream& is) {
  std::streampos start_read = is.tellg();
  std::array<std::bitset<M>, SPILL_HEADER_LENGTH> raw_data;
  std::streampos stop_read = wg_utils::ReadChunk(is, raw_data);

  bool has_header_marker, has_SP_marker, has_IL_marker, has_space_marker;
  std::size_t header_marker_pos, SP_marker_pos, IL_marker_pos, space_marker_pos;
  std::tie(has_header_marker, header_marker_pos) = wg_utils::FindInArray(raw_data, SPILL_HEADER_MARKER);
  std::tie(has_SP_marker, SP_marker_pos) = wg_utils::FindInArray(raw_data, SP_MARKER);
  std::tie(has_IL_marker, IL_marker_pos) = wg_utils::FindInArray(raw_data, IL_MARKER);
  std::tie(has_space_marker, space_marker_pos) = wg_utils::FindInArray(raw_data, SPACE_MARKER);

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
    do { wg_utils::ReadLine(is, raw_data_line); }
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
    do { wg_utils::ReadLine(is, raw_data_line); }
    while (raw_data_line != CHIP_HEADER_MARKER && i++ < 10);
    if (i < 10) {
      is.seekg(- M / 8, ios::cur);
      return false;
    }
  }
  
  // In all other cases just rewind and try with another seeker  
  is.seekg(start_read);
  return false;
}

///////////////////////////////////////////////////////////////////////////////
//                               SeekChipHeader                              //
///////////////////////////////////////////////////////////////////////////////

bool MarkerSeeker::SeekChipHeader(std::istream& is) {
  std::streampos start_read = is.tellg();
  std::array<std::bitset<M>, CHIP_HEADER_LENGTH> raw_data;
  std::streampos stop_read = wg_utils::ReadChunk(is, raw_data);

  bool has_header_marker, has_CH_marker, has_IP_marker, has_space_marker;
  std::size_t header_marker_pos, CH_marker_pos, IP_marker_pos, space_marker_pos;
  std::tie(has_header_marker,header_marker_pos) = wg_utils::FindInArray(raw_data, CHIP_HEADER_MARKER);
  std::tie(has_CH_marker, CH_marker_pos) = wg_utils::FindInArray(raw_data, CH_MARKER);
  std::tie(has_IP_marker, IP_marker_pos) = wg_utils::FindInArray(raw_data, IP_MARKER);
  std::tie(has_space_marker, space_marker_pos) = wg_utils::FindInArray(raw_data, SPACE_MARKER);

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

///////////////////////////////////////////////////////////////////////////////
//                              SeekChipTrailer                              //
///////////////////////////////////////////////////////////////////////////////

bool MarkerSeeker::SeekChipTrailer(std::istream& is) {
  std::streampos start_read = is.tellg();
  std::array<std::bitset<M>, CHIP_TRAILER_LENGTH> raw_data;
  std::streampos stop_read = wg_utils::ReadChunk(is, raw_data);
  // Everything is good
  if (raw_data[0] == CHIP_TRAILER_MARKER &&
      raw_data[2] == SPACE_MARKER &&
      raw_data[3] == SPACE_MARKER) {
    m_current_section.start = start_read;
    m_current_section.stop = stop_read;
    m_current_section.type = ChipTrailer;
    return true;
  }
  // If the chip trailer is corrupted we may as well skip it and go
  // straightly to the next section
  else if (raw_data[0] == CHIP_TRAILER_MARKER) {
    unsigned i = 0;
    std::bitset<M> raw_data_line;
    do { wg_utils::ReadLine(is, raw_data_line); }
    while (raw_data_line != CHIP_HEADER_MARKER &&
           raw_data_line != SPILL_NUMBER_MARKER &&
           raw_data_line != SPILL_HEADER_MARKER &&
           i++ < 10);
    if (i < 10) {
      Log.eWrite("[wgDecoder] A spill trailer was corrupted or not found at byte " + to_string(start_read));
      is.seekg(- M / 8, ios::cur);
      return false;
    }
  }   
  // In all other cases just rewind and try with another seeker  
  is.seekg(start_read);
  return false;
}

///////////////////////////////////////////////////////////////////////////////
//                              SeekSpillTrailer                             //
///////////////////////////////////////////////////////////////////////////////

bool MarkerSeeker::SeekSpillTrailer(std::istream& is) {

  std::streampos start_read = is.tellg();
  std::array<std::bitset<M>, SPILL_TRAILER_LENGTH> raw_data;
  std::streampos stop_read = wg_utils::ReadChunk(is, raw_data);
  // Everything is good
  if (raw_data[0] == SPILL_TRAILER_MARKER &&
      raw_data[1] == raw_data[4] &&
      raw_data[2] == raw_data[5] &&
      raw_data[3] & xFF00 == x0000 &&      
      raw_data[6] == SPACE_MARKER) {
    m_current_section.start = start_read;
    m_current_section.stop = stop_read;
    m_current_section.type = SpillTrailer;
    return true;
  } else {
    Log.eWrite("[wgDecoder] A spill trailer was corrupted or not found at byte " + to_string(start_read));
    bool has_trailer_marker, has_space_marker;
    std::size_t trailer_marker_pos, space_marker_pos;
    std::tie(has_trailer_marker, trailer_marker_pos) = wg_utils::FindInArray(raw_data, SPILL_TRAILER_MARKER);
    std::tie(has_space_marker, space_marker_pos) = wg_utils::FindInArray(raw_data, SPACE_MARKER);
    if (has_trailer_marker && has_space_marker) {
      // Just make sure that the chip header is following in the next 10
      // lines.
      is.seekg(start_read);
      unsigned i = 0;
      std::bitset<M> raw_data_line;
      do { wg_utils::ReadLine(is, raw_data_line); }
      while (raw_data_line != SPILL_NUMBER_MARKER &&
             raw_data_line != SPILL_HEADER_MARKER &&
             i++ < 10);
      if (i < 10) {
        is.seekg(- M / 8, ios::cur);
        // If the header marker and the space x2020 marker are spaced
        // at least 3 positions there is still hope to extract
        // something meaningful from the spill trailer
        if (space_marker_pos - header_marker_pos > 3) {
        m_current_section.start = start_read;
        m_current_section.stop = is.tellg();
        m_current_section.type = SpillHeader;
        return true;
        } else {
          return false;
        }
      }
    }
  }
  // In all other cases just rewind and try with another seeker  
  is.seekg(start_read);
  return false;
}

///////////////////////////////////////////////////////////////////////////////
//                                SeekRawData                                //
///////////////////////////////////////////////////////////////////////////////

bool MarkerSeeker::SeekRawData(std::istream& is) {
  return true;
}

///////////////////////////////////////////////////////////////////////////////
//                               InitializeRing                              //
///////////////////////////////////////////////////////////////////////////////

void MarkerSeeker::InitializeRing() {
  m_seekers_ring[MarkerType::SpillNumber]  = std::bind(&MarkerSeeker::SeekSpillNumber,  this, std::placeholders::_1);
  m_seekers_ring[MarkerType::SpillHeader]  = std::bind(&MarkerSeeker::SeekSpillHeader,  this, std::placeholders::_1);
  m_seekers_ring[MarkerType::ChipHeader]   = std::bind(&MarkerSeeker::SeekChipHeader,   this, std::placeholders::_1);
  m_seekers_ring[MarkerType::RawData]      = std::bind(&MarkerSeeker::SeekRawData,      this, std::placeholders::_1);
  m_seekers_ring[MarkerType::ChipTrailer]  = std::bind(&MarkerSeeker::SeekChipTrailer,  this, std::placeholders::_1);
  m_seekers_ring[MarkerType::SpillTrailer] = std::bind(&MarkerSeeker::SeekSpillTrailer, this, std::placeholders::_1);
}
