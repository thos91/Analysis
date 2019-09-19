// system includes
#include <bitset>
#include <istream>
#include <functional>
#include <iomanip>

// system C includes
#include <csignal>

// user includes
#include "wgConst.hpp"
#include "wgDecoder.hpp"
#include "wgDecoderSeeker.hpp"
#include "wgDecoderUtils.hpp"
#include "wgLogger.hpp"

namespace wg_utils = wagasci_decoder_utils;

///////////////////////////////////////////////////////////////////////////////
//                                 SectionSeeker                              //
///////////////////////////////////////////////////////////////////////////////

SectionSeeker::SectionSeeker(const RawDataConfig &config) : m_config(config) {
  if (m_config.has_spill_number && m_config.has_phantom_menace) {
    m_num_section_types = NUM_SECTION_TYPES;
  } else if (m_config.has_spill_number ^ m_config.has_phantom_menace) {
    m_num_section_types = NUM_SECTION_TYPES - 1;
  } else {
    m_num_section_types = NUM_SECTION_TYPES - 2;
  }
  m_current_section.type = SectionType::SpillTrailer;
  InitializeRing();
}

///////////////////////////////////////////////////////////////////////////////
//                               GetNextSection                              //
///////////////////////////////////////////////////////////////////////////////

SectionSeeker::Section SectionSeeker::SeekNextSection(std::istream& is) {
  bool found = true;
  unsigned last_section_type = m_current_section.type;
  m_current_section.ichip = m_last_ichip;
  m_current_section.ispill = m_last_ispill;
  
  do {
    m_current_section.type = NextSectionType(m_current_section.type, found);
    found = m_seekers_ring[m_current_section.type](is);
  } while (!found && (m_current_section.type != last_section_type));

  if (!found) {
    std::bitset<BITS_PER_LINE> raw_data_line;
    wg_utils::ReadLine(is, raw_data_line);
    std::stringstream res;
    res << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << raw_data_line.to_ulong();
    Log.eWrite("[wgDecoder] Line \"" + res.str() + "\" not recognized at byte " + to_string(is.tellg()) + ". Skipping it.");
    return this->SeekNextSection(is);
  }

  m_current_section.lines = GetNumberOfLines(m_current_section);
  return m_current_section;
}

///////////////////////////////////////////////////////////////////////////////
//                              NextSectionType                              //
///////////////////////////////////////////////////////////////////////////////

unsigned SectionSeeker::NextSectionType(const unsigned last_section_type, const bool last_section_was_found) {
  // Select what is the next section to look for. Only if the last
  // section whas a chip trailer we need to be cautious because we may
  // need to go back to the ChipHeader and increment the current_chip
  // by one
  if (last_section_was_found && last_section_type == ChipTrailer && m_last_ichip < m_config.n_chips) {
    m_last_ichip %= m_config.n_chips;
    return ChipHeader;
  } else if (!last_section_was_found && last_section_type == ChipHeader) {
    m_last_ichip %= m_config.n_chips;
    return SpillTrailer;
  } else {
    m_last_ichip %= m_config.n_chips;
    return (m_current_section.type + 1) % m_num_section_types;
  }
}

///////////////////////////////////////////////////////////////////////////////
//                              SeekSpillNumber                              //
///////////////////////////////////////////////////////////////////////////////

bool SectionSeeker::SeekSpillNumber(std::istream& is) {
  std::streampos start_read = is.tellg();
  std::vector<std::bitset<BITS_PER_LINE>> raw_data(SPILL_NUMBER_LENGTH);
  std::streampos stop_read = wg_utils::ReadChunk(is, raw_data);
  // Everything should be fine
  if ((raw_data[0] == SPILL_NUMBER_MARKER || raw_data[0] == FIRST_MARKER) &&
      raw_data[2] != SPILL_HEADER_MARKER) {
    m_current_section.start = start_read;
    m_current_section.stop = stop_read;
    m_current_section.type = SpillNumber;
    return true;
  } else if (raw_data[0] == SPILL_NUMBER_MARKER) {
    // Else the spill number section is corrupted or missing. In that
    // case we just ignore it and go on with reading the spill header
    // section if we find it
    std::vector<std::bitset<BITS_PER_LINE>> raw_data_emergency(SPILL_NUMBER_LENGTH + 2);
    is.seekg(start_read);
    wg_utils::ReadChunk(is, raw_data_emergency);
    bool has_spill_header;
    std::size_t spill_header_pos;
    std::tie(has_spill_header, spill_header_pos) = wg_utils::FindInVector(raw_data, SPILL_HEADER_MARKER);
    if (has_spill_header) {
      // skip the spill number section
      is.seekg(start_read + std::streampos((spill_header_pos + 1) * BYTES_PER_LINE));
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

bool SectionSeeker::SeekSpillHeader(std::istream& is) {
  std::streampos start_read = is.tellg();
  std::vector<std::bitset<BITS_PER_LINE>> raw_data(SPILL_HEADER_LENGTH);
  std::streampos stop_read = wg_utils::ReadChunk(is, raw_data);

  // If everything is in place just return and call it a day
  if ((raw_data[0] == SPILL_HEADER_MARKER  || raw_data[0] == FIRST_MARKER) &&
      raw_data[3] == SP_MARKER &&
      raw_data[4] == IL_MARKER &&
      raw_data[5] == SPACE_MARKER) {
    m_current_section.start = start_read;
    m_current_section.stop = stop_read;
    m_current_section.type = SpillHeader;
    return true;
  }
  // we can still work with a partially corrupted header, if it has
  // the header marker and the SP marker and they are correctly
  // spaced.
  else {
    bool has_header_marker, has_SP_marker;
    std::size_t header_marker_pos, SP_marker_pos;
    std::tie(has_header_marker, header_marker_pos) = wg_utils::FindInVector(raw_data, SPILL_HEADER_MARKER);
    std::tie(has_SP_marker, SP_marker_pos) = wg_utils::FindInVector(raw_data, SP_MARKER);

    // Just make sure that the chip header is in the following lines.
    std::vector<std::bitset<BITS_PER_LINE>> raw_data_emergency(SPILL_HEADER_LENGTH + 2);
    is.seekg(start_read);
    wg_utils::ReadChunk(is, raw_data_emergency);
    
    bool has_chip_header;
    std::size_t chip_header_pos;
    std::tie(has_chip_header, chip_header_pos) = wg_utils::FindInVector(raw_data_emergency, CHIP_HEADER_MARKER);
    
    if (has_header_marker && has_SP_marker && has_chip_header &&
        (SP_marker_pos - header_marker_pos == 3)) {
      is.seekg(start_read + std::streampos((chip_header_pos + 1) * BYTES_PER_LINE));
      m_current_section.start = start_read;
      m_current_section.stop = is.tellg();
      m_current_section.type = SpillHeader;
      return true;
    }
    // else all is lost and we have to skip the spill header
    else if (has_header_marker && has_chip_header) {
      m_current_section.type = SpillHeader;
      is.seekg(start_read + std::streampos((chip_header_pos + 1) * BYTES_PER_LINE));
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

bool SectionSeeker::SeekChipHeader(std::istream& is) {
  std::streampos start_read = is.tellg();
  std::vector<std::bitset<BITS_PER_LINE>> raw_data(CHIP_HEADER_LENGTH);
  std::streampos stop_read = wg_utils::ReadChunk(is, raw_data);

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
    // spaced. The recovery procedure is a bit more complex for the
    // chip header because there is no header or trailer for the raw
    // data section, so we cannot rely on that to know where a
    // corrupted chip header ends
  } else {
    bool has_CH_marker, has_IP_marker, has_space_marker;
    std::size_t CH_marker_pos, IP_marker_pos, space_marker_pos;
    std::tie(has_CH_marker, CH_marker_pos) = wg_utils::FindInVector(raw_data, CH_MARKER);
    std::tie(has_IP_marker, IP_marker_pos) = wg_utils::FindInVector(raw_data, IP_MARKER);
    std::tie(has_space_marker, space_marker_pos) = wg_utils::FindInVector(raw_data, SPACE_MARKER);
  
    if ((raw_data[0] == CHIP_HEADER_MARKER && raw_data[2] == CH_MARKER) ||
        (raw_data[0] == CHIP_HEADER_MARKER && raw_data[1] != CH_MARKER && raw_data[2] == IP_MARKER)) {
      unsigned n_markers = (unsigned) has_CH_marker + (unsigned) has_IP_marker + (unsigned) has_space_marker;
      is.seekg(start_read + std::streampos((2 + n_markers) * BYTES_PER_LINE));
      m_current_section.start = start_read;
      m_current_section.stop = is.tellg();
      m_current_section.type = ChipHeader;
      return true;
      // as a last resort try to recover from an heavily corrupted chip
      // header by skipping it altogether. Try to guess the end of the
      // corrupted chip header by looking for the first and last
      // markers.
    } else if (raw_data[0] == CHIP_HEADER_MARKER && has_space_marker) {
      m_current_section.type = ChipHeader;
      is.seekg(start_read + std::streampos((space_marker_pos + 1) * BYTES_PER_LINE));
      return false;
    } else if (raw_data[0] == CHIP_HEADER_MARKER && has_IP_marker) {
      m_current_section.type = ChipHeader;
      is.seekg(start_read + std::streampos((IP_marker_pos + 1) * BYTES_PER_LINE));
      return false;
    } else if (raw_data[0] == CHIP_HEADER_MARKER && has_CH_marker) {
      m_current_section.type = ChipHeader;
      is.seekg(start_read + std::streampos((CH_marker_pos + 1) * BYTES_PER_LINE));
      return false;
    }
  }
  // In all other cases just rewind and try with another seeker  
  is.seekg(start_read);
  return false;
}

///////////////////////////////////////////////////////////////////////////////
//                              SeekChipTrailer                              //
///////////////////////////////////////////////////////////////////////////////

bool SectionSeeker::SeekChipTrailer(std::istream& is) {
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
    ++m_last_ichip;
    return true;
  }
  // If the chip trailer is corrupted we may as well skip it and go
  // straightly to the next section
  else if (raw_data[0] == CHIP_TRAILER_MARKER) {
    std::vector<std::bitset<BITS_PER_LINE>> raw_data_emergency(CHIP_TRAILER_LENGTH + 2);
    is.seekg(start_read);
    wg_utils::ReadChunk(is, raw_data_emergency);
    bool has_chip_header, has_spill_number, has_spill_trailer;
    std::size_t chip_header_pos, spill_number_pos, spill_trailer_pos, pos;
    std::tie(has_chip_header, chip_header_pos) = wg_utils::FindInVector(raw_data_emergency, CHIP_HEADER_MARKER);
    std::tie(has_spill_number, spill_number_pos) = wg_utils::FindInVector(raw_data_emergency, SPILL_NUMBER_MARKER);
    std::tie(has_spill_trailer, spill_trailer_pos) = wg_utils::FindInVector(raw_data_emergency, SPILL_TRAILER_MARKER);

    if (has_chip_header) pos = chip_header_pos;
    else if (m_config.has_spill_number && has_spill_number) pos = spill_number_pos;
    else if (has_spill_trailer) pos = spill_trailer_pos;
    else {
      //  try with another seeker 
      is.seekg(start_read);
      return false;
    }
    m_current_section.type = ChipTrailer;
    is.seekg(start_read + std::streampos((pos + 1) * BYTES_PER_LINE));
    ++m_last_ichip;
    return false;
  }
  // In all other cases just rewind and try with another seeker  
  is.seekg(start_read);
  return false;
}

///////////////////////////////////////////////////////////////////////////////
//                              SeekSpillTrailer                             //
///////////////////////////////////////////////////////////////////////////////

bool SectionSeeker::SeekSpillTrailer(std::istream& is) {
  std::streampos start_read = is.tellg();
  std::vector<std::bitset<BITS_PER_LINE>> raw_data(SPILL_TRAILER_LENGTH);
  std::streampos stop_read;
  try {
  stop_read = wg_utils::ReadChunk(is, raw_data);
  } catch (const wgEOF) {
    is.clear();
    is.seekg(0, is.end);
    stop_read = is.tellg();
  }
  // Everything is good
  if ((raw_data[0] == SPILL_TRAILER_MARKER) &&
      ((raw_data[3] & xFF00) == x0000) &&      
      (raw_data[6] == SPACE_MARKER)) {
    m_current_section.start = start_read;
    m_current_section.stop = stop_read;
    m_current_section.type = SpillTrailer;
    ++m_last_ispill;
    return true;
  } else {
    // Look for the space x2020 marker
    bool has_space_marker;
    std::size_t space_marker_pos;
    std::tie(has_space_marker, space_marker_pos) = wg_utils::FindInVector(raw_data, SPACE_MARKER);

    // Look for the spill number or spill header sections in the
    // immediate proximity of the spill trailer
    std::vector<std::bitset<BITS_PER_LINE>> raw_data_emergency(SPILL_TRAILER_LENGTH + 2);
    try {
      is.seekg(start_read);
      wg_utils::ReadChunk(is, raw_data_emergency);
    } catch (const wgEOF) {
      is.clear();
      is.seekg(0, is.end);
    }
    bool has_spill_header, has_spill_number;
    std::size_t spill_number_pos, spill_header_pos;
 
    std::tie(has_spill_header, spill_header_pos) = wg_utils::FindInVector(raw_data_emergency, SPILL_HEADER_MARKER);
    std::tie(has_spill_number, spill_number_pos) = wg_utils::FindInVector(raw_data_emergency, SPILL_NUMBER_MARKER);

    if (m_config.has_spill_number && has_spill_number) {
      is.seekg(start_read + std::streampos((spill_number_pos + 1) * BYTES_PER_LINE));
    } else if (has_spill_header) {
      is.seekg(start_read + std::streampos((spill_header_pos + 1) * BYTES_PER_LINE));
    }
    
    // If the trailer marker and the space x2020 marker are spaced at
    // least 3 positions there is still hope to extract something
    // meaningful from the spill trailer
    if ((raw_data[0] == SPILL_TRAILER_MARKER) && has_space_marker && space_marker_pos > 3) {
      m_current_section.start = start_read;
      m_current_section.stop = is.tellg();
      m_current_section.type = SpillTrailer;
      ++m_last_ispill;
      return true;
    } else if (raw_data[0] == SPILL_TRAILER_MARKER &&
               ((m_config.has_spill_number && has_spill_number) || has_spill_header)) {
      // The spill trailer is beyond any hope of recovery. Skip it.
      m_current_section.type = SpillTrailer;
      ++m_last_ispill;
      return false;
    }
  }
  // In all other cases just rewind and try with another seeker  
  is.seekg(start_read);
  return false;
}

///////////////////////////////////////////////////////////////////////////////
//                             SeekPhantomMenace                             //
///////////////////////////////////////////////////////////////////////////////

bool SectionSeeker::SeekPhantomMenace(std::istream& is) {
  std::streampos start_read = is.tellg();
  std::vector<std::bitset<BITS_PER_LINE>> raw_data(PHANTOM_MENACE_LENGTH);
  wg_utils::ReadChunk(is, raw_data);

  if (raw_data[1] == x0000 &&
      raw_data[2] == x0000) {
    return false;    
  }

  // In all other cases just rewind and try with another seeker  
  is.seekg(start_read);
  return false;
}

///////////////////////////////////////////////////////////////////////////////
//                                SeekRawData                                //
///////////////////////////////////////////////////////////////////////////////

bool SectionSeeker::SeekRawData(std::istream& is) {
  std::streampos start_read = is.tellg();
  std::bitset<BITS_PER_LINE> raw_data_line;

  if (start_read > BYTES_PER_LINE) {
    is.seekg(- BYTES_PER_LINE, std::ios::cur);
    wg_utils::ReadLine(is, raw_data_line);
    if (raw_data_line != SPACE_MARKER && raw_data_line != IP_MARKER) {
      return false;
    }
  }

  unsigned n_raw_data = 0;
  do {
    wg_utils::ReadLine(is, raw_data_line);
    ++n_raw_data;
  }
  while (raw_data_line != CHIP_TRAILER_MARKER);
  // rewind only the last line
  is.seekg(- BYTES_PER_LINE, std::ios::cur);
  --n_raw_data;

  if ((n_raw_data - m_config.n_chip_id) % ONE_COLUMN_LENGTH == 0) {
    m_current_section.start = start_read;
    m_current_section.stop = is.tellg();
    m_current_section.type = RawData;
    return true;
  }
  // In all other cases just rewind and try with another seeker  
  is.seekg(start_read);
  return false;
}

///////////////////////////////////////////////////////////////////////////////
//                               InitializeRing                              //
///////////////////////////////////////////////////////////////////////////////

void SectionSeeker::InitializeRing() {
  m_seekers_ring[SectionType::SpillHeader]  = [this](std::istream& is) { return this->SeekSpillHeader(is); };  
  m_seekers_ring[SectionType::ChipHeader]   = [this](std::istream& is) { return this->SeekChipHeader(is); };  
  m_seekers_ring[SectionType::RawData]      = [this](std::istream& is) { return this->SeekRawData(is); };  
  m_seekers_ring[SectionType::ChipTrailer]  = [this](std::istream& is) { return this->SeekChipTrailer(is); };  
  m_seekers_ring[SectionType::SpillTrailer] = [this](std::istream& is) { return this->SeekSpillTrailer(is); };
  if (m_config.has_phantom_menace)
    m_seekers_ring[SectionType::PhantomMenace] = [this](std::istream& is) { return this->SeekPhantomMenace(is); };
  if (m_config.has_spill_number)
    m_seekers_ring[SectionType::SpillNumber]   = [this](std::istream& is) { return this->SeekSpillNumber(is); };  
}

///////////////////////////////////////////////////////////////////////////////
//                          GetNumberOfLinesToRead                           //
///////////////////////////////////////////////////////////////////////////////

unsigned GetNumberOfLines(const SectionSeeker::Section & section) {
  return (section.stop - section.start) / BYTES_PER_LINE;
}
