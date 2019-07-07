// system includes
#include <bitset>
#include <istream>
#include <functional>

// user includes
#include "wgConst.hpp"
#include "wgDecoder.hpp"
#include "wgDecoderSeeker.hpp"
#include "wgDecoderUtils.hpp"
#include "wgLogger.hpp"

namespace wg_utils = wagasci_decoder_utils;

///////////////////////////////////////////////////////////////////////////////
//                                 MarkerSeeker                              //
///////////////////////////////////////////////////////////////////////////////

MarkerSeeker::MarkerSeeker(const RawDataConfig &config) : m_config(config) {
  if (m_config.has_spill_number) {
    m_num_marker_types = NUM_MARKER_TYPES;
  } else {
    m_num_marker_types = NUM_MARKER_TYPES - 1;
  }
  m_current_section.type = MarkerType::SpillTrailer;
  InitializeRing();
}

///////////////////////////////////////////////////////////////////////////////
//                               GetNextSection                              //
///////////////////////////////////////////////////////////////////////////////

MarkerSeeker::Section MarkerSeeker::SeekNextSection(std::istream& is) {
  bool found = false;
  unsigned last_section_type = m_current_section.type;
  
  do {
    m_current_section.type = NextSectionType(last_section_type);
    found = m_seekers_ring[m_current_section.type](is);
  } while (!found && (m_current_section.type != last_section_type));

  if (!found) {
    std::bitset<BITS_PER_LINE> raw_data_line;
    wg_utils::ReadLine(is, raw_data_line);
    stringstream res;
    res << hex << uppercase << raw_data_line.to_ulong();
    Log.eWrite("[wgDecoder] Line \"" + res.str() + "\" not recognized. Skipping it.");
    return this->SeekNextSection(is);
  }
  
  return m_current_section;
}

///////////////////////////////////////////////////////////////////////////////
//                              NextSectionType                              //
///////////////////////////////////////////////////////////////////////////////

unsigned MarkerSeeker::NextSectionType(const unsigned last_section_type) {
  // Select what is the next section to look for. Only if the last
  // section whas a chip trailer we need to be cautious because we may
  // need to go back to the ChipHeader and increment the current_chip
  // by one
  if ((last_section_type == ChipTrailer) && (m_current_section.ichip < m_config.n_chips - 1))
    return ChipHeader;
  else
    return m_current_section.type + 1 % m_num_marker_types;
}

///////////////////////////////////////////////////////////////////////////////
//                              SeekSpillNumber                              //
///////////////////////////////////////////////////////////////////////////////

bool MarkerSeeker::SeekSpillNumber(std::istream& is) {
  std::streampos start_read = is.tellg();
  std::vector<std::bitset<BITS_PER_LINE>> raw_data(SPILL_NUMBER_LENGTH);
  std::streampos stop_read = wg_utils::ReadChunk(is, raw_data);
  // Everything should be fine
  if (((m_current_section.ispill != 0 && raw_data[0] == SPILL_NUMBER_MARKER) ||
      (m_current_section.ispill == 0 && raw_data[0] == FIRST_SPILL_MARKER)) &&
      raw_data[2] != SPILL_HEADER_MARKER) {
    m_current_section.start = start_read;
    m_current_section.stop = stop_read;
    m_current_section.type = SpillNumber;
    return true;
  } else {
    // Else the spill number section is corrupted or missing. In that
    // case we just ignore it and go on with reading the spill header
    // section if we found it in the following 10 lines
    unsigned i = 0;
    std::bitset<BITS_PER_LINE> raw_data_line;
    do { wg_utils::ReadLine(is, raw_data_line); }
    while (raw_data_line != SPILL_HEADER_MARKER && i++ < 10);
    if (i < 10) {
      // rewind just before the spill header
      is.seekg(- BYTES_PER_LINE, ios::cur);
      m_current_section.type = SpillNumber;
      return false;
    }
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
  std::vector<std::bitset<BITS_PER_LINE>> raw_data(SPILL_HEADER_LENGTH);
  std::streampos stop_read = wg_utils::ReadChunk(is, raw_data);

  bool has_header_marker, has_SP_marker, has_IL_marker, has_space_marker;
  std::size_t header_marker_pos, SP_marker_pos, IL_marker_pos, space_marker_pos;
  std::tie(has_header_marker, header_marker_pos) = wg_utils::FindInVector(raw_data, SPILL_HEADER_MARKER);
  std::tie(has_SP_marker, SP_marker_pos) = wg_utils::FindInVector(raw_data, SP_MARKER);
  std::tie(has_IL_marker, IL_marker_pos) = wg_utils::FindInVector(raw_data, IL_MARKER);
  std::tie(has_space_marker, space_marker_pos) = wg_utils::FindInVector(raw_data, SPACE_MARKER);

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
    std::bitset<BITS_PER_LINE> raw_data_line;
    do { wg_utils::ReadLine(is, raw_data_line); }
    while (raw_data_line != CHIP_HEADER_MARKER && i++ < 10);
    if (i < 10) {
      is.seekg(- BYTES_PER_LINE, ios::cur);
      m_current_section.start = start_read;
      m_current_section.stop = is.tellg();
      m_current_section.type = SpillHeader;
      return true;
    }
  }
  // else all is lost and try to find the next section
  else if (has_header_marker) {
    unsigned i = 0;
    std::bitset<BITS_PER_LINE> raw_data_line;
    do { wg_utils::ReadLine(is, raw_data_line); }
    while (raw_data_line != CHIP_HEADER_MARKER && i++ < 10);
    if (i < 10) {
      is.seekg(- BYTES_PER_LINE, ios::cur);
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
  std::vector<std::bitset<BITS_PER_LINE>> raw_data(CHIP_HEADER_LENGTH);
  std::streampos stop_read = wg_utils::ReadChunk(is, raw_data);

  bool has_header_marker, has_CH_marker, has_IP_marker, has_space_marker;
  std::size_t header_marker_pos, CH_marker_pos, IP_marker_pos, space_marker_pos;
  std::tie(has_header_marker,header_marker_pos) = wg_utils::FindInVector(raw_data, CHIP_HEADER_MARKER);
  std::tie(has_CH_marker, CH_marker_pos) = wg_utils::FindInVector(raw_data, CH_MARKER);
  std::tie(has_IP_marker, IP_marker_pos) = wg_utils::FindInVector(raw_data, IP_MARKER);
  std::tie(has_space_marker, space_marker_pos) = wg_utils::FindInVector(raw_data, SPACE_MARKER);

  // If everything is in place just return true and call it a day
  if (raw_data[0] == CHIP_HEADER_MARKER &&
      raw_data[2] == CH_MARKER &&
      raw_data[3] == IP_MARKER &&
      raw_data[4] == SPACE_MARKER) {
    m_current_section.start = start_read;
    m_current_section.stop = stop_read;
    m_current_section.type = ChipHeader;
    return true;
    // we can still work with a partially corrupted header, if it has
    // the header marker and the CH marker and they are correctly
    // spaced.
  } else if (raw_data[0] == CHIP_HEADER_MARKER &&
             raw_data[2] == CH_MARKER) {
    // XOR between IP_marker and space_marker (execute only when one
    // is true and the other is false). In that case we just have to
    // rewind one line and we are good to go.
    if (has_IP_marker != has_space_marker) {
      is.seekg(- BYTES_PER_LINE, ios::cur);
      m_current_section.start = start_read;
      m_current_section.stop = is.tellg();
      m_current_section.type = ChipHeader;
      return true;
    }
    // If both are false rewind two lines and go on.
    else if (!has_IP_marker && !has_space_marker ) {
      is.seekg(- 2 * BYTES_PER_LINE, ios::cur);
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
    is.seekg(start_read + std::streampos(space_marker_pos * BYTES_PER_LINE));
    return false;
  } else if (has_header_marker && has_IP_marker) {
    is.seekg(start_read + std::streampos(IP_marker_pos * BYTES_PER_LINE));
    return false;
  } else if (has_header_marker && has_CH_marker) {
    is.seekg(start_read + std::streampos(CH_marker_pos * BYTES_PER_LINE));
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
  std::vector<std::bitset<BITS_PER_LINE>> raw_data(CHIP_TRAILER_LENGTH);
  std::streampos stop_read = wg_utils::ReadChunk(is, raw_data);
  // Everything is good
  if (raw_data[0] == CHIP_TRAILER_MARKER &&
      raw_data[2] == SPACE_MARKER &&
      raw_data[3] == SPACE_MARKER) {
    m_current_section.start = start_read;
    m_current_section.stop = stop_read;
    m_current_section.type = ChipTrailer;
    ++m_current_section.ichip;
    return true;
  }
  // If the chip trailer is corrupted we may as well skip it and go
  // straightly to the next section
  else if (raw_data[0] == CHIP_TRAILER_MARKER) {
    unsigned i = 0;
    std::bitset<BITS_PER_LINE> raw_data_line;
    do { wg_utils::ReadLine(is, raw_data_line); }
    while (raw_data_line != CHIP_HEADER_MARKER &&
           raw_data_line != SPILL_NUMBER_MARKER &&
           raw_data_line != SPILL_HEADER_MARKER &&
           i++ < 10);
    if (i < 10) {
      Log.eWrite("[wgDecoder] A spill trailer was corrupted or not found at byte " + to_string(start_read));
      is.seekg(- BYTES_PER_LINE, ios::cur);
      ++m_current_section.ichip;
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
  std::vector<std::bitset<BITS_PER_LINE>> raw_data(SPILL_TRAILER_LENGTH);
  std::streampos stop_read = wg_utils::ReadChunk(is, raw_data);
  // Everything is good
  if ((raw_data[0] == SPILL_TRAILER_MARKER) &&
      (raw_data[1] == raw_data[4]) &&
      (raw_data[2] == raw_data[5]) &&
      ((raw_data[3] & xFF00) == x0000) &&      
      (raw_data[6] == SPACE_MARKER)) {
    m_current_section.start = start_read;
    m_current_section.stop = stop_read;
    m_current_section.type = SpillTrailer;
    ++m_current_section.ispill;
    return true;
  } else {
    Log.eWrite("[wgDecoder] A spill trailer was corrupted or not found at byte " + to_string(start_read));
    bool has_trailer_marker, has_space_marker;
    std::size_t trailer_marker_pos, space_marker_pos;
    std::tie(has_trailer_marker, trailer_marker_pos) = wg_utils::FindInVector(raw_data, SPILL_TRAILER_MARKER);
    std::tie(has_space_marker, space_marker_pos) = wg_utils::FindInVector(raw_data, SPACE_MARKER);
    if (has_trailer_marker && has_space_marker) {

      is.seekg(start_read);
      unsigned i = 0;
      std::bitset<BITS_PER_LINE> raw_data_line;
      
      if (m_config.has_spill_number) {
      // Just make sure that the spill number is following in the next
      // 10 lines.
      do { wg_utils::ReadLine(is, raw_data_line); }
      while ((raw_data_line != SPILL_NUMBER_MARKER) && i++ < 10);
      
      } else {
        // Just make sure that the spill header is following in the next
      // 10 lines.
      do { wg_utils::ReadLine(is, raw_data_line); }
      while ((raw_data_line != SPILL_HEADER_MARKER) && i++ < 10);
      }
      
      if (i < 10) {
        is.seekg(- BYTES_PER_LINE, ios::cur);
        // If the trailer marker and the space x2020 marker are spaced
        // at least 3 positions there is still hope to extract
        // something meaningful from the spill trailer
        if (space_marker_pos - trailer_marker_pos > 3) {
          m_current_section.start = start_read;
          m_current_section.stop = is.tellg();
          m_current_section.type = SpillTrailer;
          ++m_current_section.ispill;
          return true;
        } else {
          // The spill trailer is beyond any hope of recovery. Skip it.
          ++m_current_section.ispill;
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
  std::streampos start_read = is.tellg();
  std::bitset<BITS_PER_LINE> raw_data_line;
  unsigned n_raw_data = 0;

  is.seekg(- BYTES_PER_LINE, ios::cur);
  wg_utils::ReadLine(is, raw_data_line);
  if (raw_data_line != SPACE_MARKER || raw_data_line != IP_MARKER) {
    return false;
  }
  
  do {
    wg_utils::ReadLine(is, raw_data_line);
    n_raw_data++;
  }
  while (raw_data_line != CHIP_TRAILER_MARKER);

  if (n_raw_data - m_config.n_chip_id % ONE_COLUMN_LENGTH == 0) {
    m_current_section.start = start_read;
    m_current_section.stop = is.tellg();
    m_current_section.type = RawData;
  }
  // In all other cases just rewind and try with another seeker  
  is.seekg(start_read);
  return false;
}

///////////////////////////////////////////////////////////////////////////////
//                               InitializeRing                              //
///////////////////////////////////////////////////////////////////////////////

void MarkerSeeker::InitializeRing() {
  m_seekers_ring[MarkerType::SpillHeader]  = std::bind(&MarkerSeeker::SeekSpillHeader,  this, std::placeholders::_1);
  m_seekers_ring[MarkerType::ChipHeader]   = std::bind(&MarkerSeeker::SeekChipHeader,   this, std::placeholders::_1);
  m_seekers_ring[MarkerType::RawData]      = std::bind(&MarkerSeeker::SeekRawData,      this, std::placeholders::_1);
  m_seekers_ring[MarkerType::ChipTrailer]  = std::bind(&MarkerSeeker::SeekChipTrailer,  this, std::placeholders::_1);
  m_seekers_ring[MarkerType::SpillTrailer] = std::bind(&MarkerSeeker::SeekSpillTrailer, this, std::placeholders::_1);
  if (m_config.has_spill_number)
    m_seekers_ring[MarkerType::SpillNumber]  = std::bind(&MarkerSeeker::SeekSpillNumber,  this, std::placeholders::_1);
}

///////////////////////////////////////////////////////////////////////////////
//                          GetNumberOfLinesToRead                           //
///////////////////////////////////////////////////////////////////////////////

unsigned GetNumberOfLinesToRead(const MarkerSeeker::Section & section) {
  return (section.stop - section.start) / BYTES_PER_LINE;
}
