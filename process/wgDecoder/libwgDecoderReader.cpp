// system includes
#include <bitset>
#include <istream>
#include <functional>

// user includes
#include "wgConst.hpp"
#include "wgLogger.hpp"
#include "wgRawData.hpp"
#include "wgDecoder.hpp"
#include "wgDecoderReader.hpp"
#include "wgDecoderSeeker.hpp"
#include "wgDecoderUtils.hpp"

namespace wg_utils = wagasci_decoder_utils;

///////////////////////////////////////////////////////////////////////////////
//                             SectionReader class                           //
///////////////////////////////////////////////////////////////////////////////

SectionReader::SectionReader(const RawDataConfig& config, TTree* tree, Raw_t& rd):
    m_config(config), m_tree(tree), m_rd(rd) {
    if (m_config.has_spill_number) {
    m_num_marker_types = NUM_SECTION_TYPES - 1;
  } else {
    m_num_marker_types = NUM_SECTION_TYPES - 2;
  }
    if (m_tree == NULL)
      throw std::runtime_error("pointer to TTree is NULL");
    InitializeRing();
}

///////////////////////////////////////////////////////////////////////////////
//                              ReadSpillNumber                              //
///////////////////////////////////////////////////////////////////////////////

void SectionReader::ReadSpillNumber(std::istream& is, const SectionSeeker::Section& section) {
  is.seekg(section.start);
  std::vector<std::bitset<BITS_PER_LINE>> raw_data(section.lines);
  wg_utils::ReadChunk(is, raw_data);
  
  // raw_data[0] is the SPILL_NUMBER_MARKER
  m_rd.get().spill_number = raw_data[1].to_ulong();
  m_rd.get().spill_mode   = raw_data[2][15];
  if (m_rd.get().spill_mode != BEAM_SPILL && m_rd.get().spill_mode != NON_BEAM_SPILL) {
    m_rd.get().debug_spill[DEBUG_SPILL_MODE]++;
  } else {
    // If we are in NON_BEAM_SPILL mode basically the spill number has
    // no meaning so we can safely ignore it. In BEAM_SPILL mode we
    // want to check if the same spill number is used twice or if we
    // have a spill gap (non consecutive spill number)
    if (m_rd.get().spill_mode == BEAM_SPILL) {
      unsigned spill_number_gap = m_rd.get().spill_number - m_last_spill_number;    
      m_last_spill_number = m_rd.get().spill_number;
      if (spill_number_gap == 0) {
        m_rd.get().debug_spill[DEBUG_SAME_SPILL_NUMBER]++;
      } else if (spill_number_gap != 1) {
        m_rd.get().debug_spill[DEBUG_SPILL_NUMBER_GAP]++;
      }
    }
  }
}

void SectionReader::ReadSpillHeader(std::istream& is, const SectionSeeker::Section& section) {
  is.seekg(section.start);
  std::vector<std::bitset<BITS_PER_LINE>> raw_data(section.lines);
  wg_utils::ReadChunk(is, raw_data);
  // raw_data[0] is the SPILL_HEADER_MARKER
  // Spill count most significant byte
  std::bitset<2*BITS_PER_LINE> spill_count_msb = raw_data[1].to_ulong();
  spill_count_msb <<= BITS_PER_LINE;
  // Spill count least significant byte
  std::bitset<2*BITS_PER_LINE> spill_count_lsb = raw_data[2].to_ulong();
  m_rd.get().spill_count = (spill_count_msb | spill_count_lsb).to_ullong();

  if (!m_config.has_spill_number) {
    m_rd.get().spill_number = m_rd.get().spill_count;
    m_rd.get().spill_mode   = BEAM_SPILL;
  }

  unsigned spill_count_gap = m_rd.get().spill_count - m_last_spill_count;    
  m_last_spill_count = m_rd.get().spill_count;
  if (spill_count_gap == 0) {
    m_rd.get().debug_spill[DEBUG_SAME_SPILL_COUNT]++;
  } else if (spill_count_gap != 1) {
    m_rd.get().debug_spill[DEBUG_SPILL_COUNT_GAP]++;
  }
}

///////////////////////////////////////////////////////////////////////////////
//                               ReadChipHeader                              //
///////////////////////////////////////////////////////////////////////////////

void SectionReader::ReadChipHeader(std::istream& is, const SectionSeeker::Section& section) {
  is.seekg(section.start);
  std::vector<std::bitset<BITS_PER_LINE>> raw_data(section.lines);
  wg_utils::ReadChunk(is, raw_data);

  unsigned chip_counter = (raw_data[1] & x00FF).to_ulong();
  if (chip_counter >= m_config.n_chips || chip_counter != section.ichip + 1) {
    m_rd.get().debug_chip[section.ichip][DEBUG_WRONG_CHIPID]++;
  }
}

///////////////////////////////////////////////////////////////////////////////
//                              ReadChipTrailer                              //
///////////////////////////////////////////////////////////////////////////////

void SectionReader::ReadChipTrailer(std::istream& is, const SectionSeeker::Section& section) {
  is.seekg(section.start);
  std::vector<std::bitset<BITS_PER_LINE>> raw_data(section.lines);
  wg_utils::ReadChunk(is, raw_data);

  unsigned chip_counter = (raw_data[1] & x00FF).to_ulong();
  if (chip_counter >= m_config.n_chips || chip_counter != section.ichip + 1) {
    m_rd.get().debug_chip[section.ichip][DEBUG_WRONG_CHIPID]++;
  }
}

///////////////////////////////////////////////////////////////////////////////
//                              ReadSpillTrailer                             //
///////////////////////////////////////////////////////////////////////////////

void SectionReader::ReadSpillTrailer(std::istream& is, const SectionSeeker::Section& section) {
  is.seekg(section.start);
  std::vector<std::bitset<BITS_PER_LINE>> raw_data(section.lines);
  wg_utils::ReadChunk(is, raw_data);

  // raw_data[0] is the SPILL_TRAILER_MARKER
  // Spill count most significant byte
  std::bitset<2*BITS_PER_LINE> spill_count_msb = raw_data[1].to_ulong();
  spill_count_msb <<= BITS_PER_LINE;
  // Spill count least significant byte
  std::bitset<2*BITS_PER_LINE> spill_count_lsb = raw_data[2].to_ulong();
  m_rd.get().spill_count = (spill_count_msb | spill_count_lsb).to_ullong();
  
  unsigned n_found_chips = ( raw_data[3] & x00FF ).to_ulong(); 
  if (n_found_chips != m_config.n_chips) {
    m_rd.get().debug_spill[DEBUG_WRONG_NCHIPS]++;
  }

  // It is not clear what this field is for. Even the SPIROC2D and
  // Pyrame developers (Stephan Callier and Frederic Magniette) do not
  // know the raison d'etre of this field, we are just ignoring this
  // for the time being
  // if (raw_data.size() == SPILL_TRAILER_LENGTH) {
  //   bitset<2*BITS_PER_LINE> unknown_field_msb = raw_data[4].to_ulong();
  //   unknown_field_msb <<= BITS_PER_LINE;
  //   // Spill count least significant byte
  //   bitset<2*BITS_PER_LINE> unknown_field_lsb = raw_data[5].to_ulong();
  //   unsigned unknown_field = (unknown_field_msb | unknown_field_lsb).to_ullong(); 
  // }

  // CHANID and COLID
  for (unsigned ichan = 0; ichan < NCHANNELS; ++ichan) {
    m_rd.get().chanid[ichan] = ichan;
  }
  for (unsigned icol = 0; icol < MEMDEPTH; ++icol) {
    m_rd.get().colid[icol] = icol;
  }
  
  FillTree();
}

///////////////////////////////////////////////////////////////////////////////
//                                ReadRawData                                //
///////////////////////////////////////////////////////////////////////////////

void SectionReader::ReadRawData(std::istream& is, const SectionSeeker::Section& section) {
  is.seekg(section.start);
  std::vector<std::bitset<BITS_PER_LINE>> raw_data(section.lines);
  wg_utils::ReadChunk(is, raw_data);

  if ((raw_data.size() - m_config.n_chip_id) % ONE_COLUMN_LENGTH != 0)
    throw std::out_of_range("SPIROC2D raw data is off range : " + std::to_string(raw_data.size()));
  unsigned n_columns = (raw_data.size() - m_config.n_chip_id) / ONE_COLUMN_LENGTH;
  if (n_columns > MEMDEPTH) {
    Log.eWrite("[wgDecoder] ichip = " + std::to_string(section.ichip) +
               " : number of columns (" + std::to_string(n_columns) +
               ") is greater than " + std::to_string(MEMDEPTH));
    m_rd.get().debug_chip[section.ichip][DEBUG_WRONG_NCOLUMNS]++;
    n_columns = MEMDEPTH;
  }

  std::vector<std::bitset<BITS_PER_LINE>>::reverse_iterator iraw_data = raw_data.rbegin(); 

  // CHIPID
  unsigned chipid = (*(iraw_data++) & x00FF).to_ulong();
  if (chipid > m_config.n_chips) {
    Log.eWrite("[wgDecoder] ichip = " + std::to_string(section.ichip) +
               " : Chip ID (" + std::to_string(chipid) +
               ") is greater than " + std::to_string(m_config.n_chips));
    m_rd.get().debug_chip[section.ichip][DEBUG_WRONG_CHIPID]++;
  }
  for (unsigned counter = 1; counter < m_config.n_chip_id; ++counter) {
    unsigned duplicate_chipid = (*(iraw_data++) & x00FF).to_ulong();
    if (duplicate_chipid != chipid)
      m_rd.get().debug_chip[section.ichip][DEBUG_WRONG_CHIPID]++;
  }
  m_rd.get().chipid[section.ichip] = chipid;
    
  // BCID
  for (unsigned icol = 0; icol < n_columns; ++icol) {
    int bcid = (*iraw_data & x0FFF).to_ulong();
    int loop_bcid = ((*(iraw_data++) & xF000) >> 12).to_ulong();
    int bcid_slope;
    int bcid_inter;
    switch (loop_bcid) {
      case 1:
        bcid_slope = -1; bcid_inter = 2 * 4096; break;
      case 3:
        bcid_slope = 1; bcid_inter = 2 * 4096; break;
      case 2:
        bcid_slope = -1; bcid_inter = 4 * 4096; break;
      default:
        bcid_slope = 1; bcid_inter = 0; break;
    }
    m_rd.get().bcid[section.ichip][icol] = bcid_inter + bcid * bcid_slope;
    if ((unsigned) m_rd.get().bcid[section.ichip][icol] > MAX_VALUE_16BITS)
      m_rd.get().debug_chip[section.ichip][DEBUG_WRONG_BCID]++;
  }

  for (unsigned icol = 0; icol < n_columns; ++icol) {

    for (unsigned ichan = 0; ichan < NCHANNELS; ++ichan) {
      // CHARGE
      m_rd.get().charge[section.ichip][ichan][icol] = (*iraw_data & x0FFF).to_ulong();
      if ((unsigned) m_rd.get().charge[section.ichip][ichan][icol] > MAX_VALUE_12BITS)
        m_rd.get().debug_chip[section.ichip][DEBUG_WRONG_ADC]++;
      // HIT (0: no hit, 1: hit)
      m_rd.get().hit[section.ichip][ichan][icol] = (*iraw_data)[12];
      // GAIN (0: low gain, 1: high gain)
      m_rd.get().gs[section.ichip][ichan][icol] = (*iraw_data)[13];
      // Only if the detector is already calibrated fithe histograms
      if (m_config.adc_is_calibrated) {
        // P.E.
        unsigned charge = m_rd.get().charge[section.ichip][ichan][icol];
        unsigned pedestal = m_rd.get().pedestal[m_rd.get().chipid[section.ichip]][ichan][icol];
        unsigned gain = m_rd.get().gain[m_rd.get().chipid[section.ichip]][ichan][icol];
        if( m_rd.get().gs[section.ichip][ichan][icol] == HIGH_GAIN_BIT ) { // High Gain
          m_rd.get().pe[section.ichip][ichan][icol] = HIGH_GAIN_NORM * ( charge - pedestal ) / gain;
        } else { // Low Gain
          m_rd.get().pe[section.ichip][ichan][icol] = LOW_GAIN_NORM * ( charge - pedestal ) / gain;
        }
      }
      ++iraw_data;
    }

    for (unsigned ichan = 0; ichan < NCHANNELS; ++ichan) {
      // TIME
      m_rd.get().time[section.ichip][ichan][icol] = (*iraw_data & x0FFF).to_ulong();
      if ((unsigned) m_rd.get().time[section.ichip][ichan][icol] > MAX_VALUE_12BITS)
        m_rd.get().debug_chip[section.ichip][DEBUG_WRONG_TDC]++;
      if (m_config.tdc_is_calibrated) {
        ;// TODO: TDC calibration
      }
      if (m_rd.get().hit[section.ichip][ichan][icol] != (*iraw_data)[12]) {
        m_rd.get().debug_chip[section.ichip][DEBUG_WRONG_HIT_BIT]++;
      }
      if (m_rd.get().gs[section.ichip][ichan][icol] != (*iraw_data)[13]) {
        m_rd.get().debug_chip[section.ichip][DEBUG_WRONG_GAIN_BIT]++;
      }
      ++iraw_data;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
//                              ReadNextSection                              //
///////////////////////////////////////////////////////////////////////////////

void SectionReader::ReadNextSection(std::istream& is, const SectionSeeker::Section& section) {
  if (section.ichip >= m_config.n_chips) {
    m_rd.get().debug_spill[DEBUG_WRONG_NCHIPS]++;
    Log.eWrite("[wgDecoder] Spill " + std::to_string(section.ispill) +
               " : number of chips overflown : " + "chip counter " +
               std::to_string(section.ichip) + " >= number of chips " +
               std::to_string(m_config.n_chips) );
    return;
  }
  m_readers_ring[section.type](is, section);
}

///////////////////////////////////////////////////////////////////////////////
//                                  FillTree                                 //
///////////////////////////////////////////////////////////////////////////////

void SectionReader::FillTree() {
  if (m_tree->Fill() < 0)
    throw std::runtime_error("Failed to fill the TTree");
  this->m_rd.get().clear();
}

///////////////////////////////////////////////////////////////////////////////
//                              InitializedRing                              //
///////////////////////////////////////////////////////////////////////////////

void SectionReader::InitializeRing() {
  m_readers_ring[SectionSeeker::SectionType::SpillHeader]  = [this](std::istream& is, const SectionSeeker::Section section) { return this->ReadSpillHeader(is, section); };
  m_readers_ring[SectionSeeker::SectionType::ChipHeader]   = [this](std::istream& is, const SectionSeeker::Section section) { return this->ReadChipHeader(is, section); };
  m_readers_ring[SectionSeeker::SectionType::RawData]      = [this](std::istream& is, const SectionSeeker::Section section) { return this->ReadRawData(is, section); };
  m_readers_ring[SectionSeeker::SectionType::ChipTrailer]  = [this](std::istream& is, const SectionSeeker::Section section) { return this->ReadChipTrailer(is, section); };
  m_readers_ring[SectionSeeker::SectionType::SpillTrailer] = [this](std::istream& is, const SectionSeeker::Section section) { return this->ReadSpillTrailer(is, section); };
  if (m_config.has_spill_number)
    m_readers_ring[SectionSeeker::SectionType::SpillNumber]  = [this](std::istream& is, const SectionSeeker::Section section) { return this->ReadSpillNumber(is, section); };
}
