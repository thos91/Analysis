// system includes
#include <bitset>
#include <istream>
#include <functional>

// user includes
#include "wgConst.hpp"
#include "wgDecoder.hpp"
#include "wgDecoderReader.hpp"
#include "wgDecoderSeeker.hpp"
#include "wgDecoderUtils.hpp"

namespace wg_utils = wagasci_decoder_utils;

///////////////////////////////////////////////////////////////////////////////
//                             EventReader class                             //
///////////////////////////////////////////////////////////////////////////////

EventReader::EventReader(const RawDataConfig& config, TTree* tree, Raw_t& rd) :
    m_config(config), m_tree(tree), m_rd(rd) {
    if (m_config.has_spill_number) {
    m_num_marker_types = NUM_MARKER_TYPES;
  } else {
    m_num_marker_types = NUM_MARKER_TYPES - 1;
  }
    if (m_tree == NULL)
      throw std::runtime_error("pointer to TTree is NULL");
    InitializeRing();
}

void EventReader::ReadSpillNumber(std::istream& is, const MarkerSeeker::Section& section) {
  is.seekg(section.start);
  std::vector<std::bitset<BITS_PER_LINE>> raw_data(GetNumberOfLinesToRead(section));
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

void EventReader::ReadSpillHeader(std::istream& is, const MarkerSeeker::Section& section) {
  is.seekg(section.start);
  std::vector<std::bitset<BITS_PER_LINE>> raw_data(GetNumberOfLinesToRead(section));
  wg_utils::ReadChunk(is, raw_data);
  // raw_data[0] is the SPILL_HEADER_MARKER
  // Spill count most significant byte
  bitset<2*BITS_PER_LINE> spill_count_msb = raw_data[1].to_ulong();
  spill_count_msb <<= BITS_PER_LINE;
  // Spill count least significant byte
  bitset<2*BITS_PER_LINE> spill_count_lsb = raw_data[2].to_ulong();
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

void EventReader::ReadChipHeader(std::istream& is, const MarkerSeeker::Section& section) {
  is.seekg(section.start);
  std::vector<std::bitset<BITS_PER_LINE>> raw_data(GetNumberOfLinesToRead(section));
  wg_utils::ReadChunk(is, raw_data);

  m_rd.get().chipid[section.ichip] = (raw_data[1] & x00FF).to_ulong();
  if ((unsigned) m_rd.get().chipid[section.ichip] > m_config.n_chips ||
      m_rd.get().chipid[section.ichip] == 0) {
    m_rd.get().debug_chip[section.ichip][DEBUG_WRONG_CHIPID]++;
  }
}

void EventReader::ReadChipTrailer(std::istream& is, const MarkerSeeker::Section& section) {
  is.seekg(section.start);
  std::vector<std::bitset<BITS_PER_LINE>> raw_data(GetNumberOfLinesToRead(section));
  wg_utils::ReadChunk(is, raw_data);

  unsigned chipid = (raw_data[1] & x00FF).to_ulong();
  if (chipid != (unsigned) m_rd.get().chipid[section.ichip] ||
      chipid > m_config.n_chips || chipid == 0) {
    m_rd.get().debug_chip[section.ichip][DEBUG_WRONG_CHIPID]++;
  }
}

void EventReader::ReadSpillTrailer(std::istream& is, const MarkerSeeker::Section& section) {
  is.seekg(section.start);
  std::vector<std::bitset<BITS_PER_LINE>> raw_data(GetNumberOfLinesToRead(section));
  wg_utils::ReadChunk(is, raw_data);

  // raw_data[0] is the SPILL_TRAILER_MARKER
  // Spill count most significant byte
  bitset<2*BITS_PER_LINE> spill_count_msb1 = raw_data[1].to_ulong();
  spill_count_msb1 <<= BITS_PER_LINE;
  // Spill count least significant byte
  bitset<2*BITS_PER_LINE> spill_count_lsb1 = raw_data[2].to_ulong();
  m_rd.get().spill_count = (spill_count_msb1 | spill_count_lsb1).to_ullong();
  
  unsigned n_found_chips = ( raw_data[3] & x00FF ).to_ulong(); 
  if (n_found_chips != m_config.n_chips) {
    m_rd.get().debug_spill[DEBUG_WRONG_NCHIPS]++;
  }
  
  if (raw_data.size() == SPILL_TRAILER_LENGTH) {
    bitset<2*BITS_PER_LINE> spill_count_msb2 = raw_data[4].to_ulong();
    spill_count_msb2 <<= BITS_PER_LINE;
    // Spill count least significant byte
    bitset<2*BITS_PER_LINE> spill_count_lsb2 = raw_data[5].to_ulong();
    unsigned spill_count2 = (spill_count_msb2 | spill_count_lsb2).to_ullong();
    
    if ((unsigned) m_rd.get().spill_count != spill_count2) {
      m_rd.get().debug_spill[DEBUG_SPILL_TRAILER]++;
    }
  }
}

void EventReader::ReadRawData(std::istream& is, const MarkerSeeker::Section& section) {
  is.seekg(section.start);
  std::vector<std::bitset<BITS_PER_LINE>> raw_data(GetNumberOfLinesToRead(section));
  wg_utils::ReadChunk(is, raw_data);

  if ((raw_data.size() - m_config.n_chip_id) % ONE_COLUMN_LENGTH != 0)
    throw std::out_of_range("SPIROC2D raw data is off range : " + to_string(raw_data.size()));
  unsigned n_columns = (raw_data.size() - m_config.n_chip_id) / ONE_COLUMN_LENGTH;

  std::vector<std::bitset<BITS_PER_LINE>>::reverse_iterator iraw_data = raw_data.rbegin(); 

  // CHIPID
  std::vector<unsigned> chipid(m_config.n_chip_id);
  for (auto ichipid : chipid) {
    ichipid = (*(iraw_data++) & x00FF).to_ulong();
    if (ichipid > m_config.n_chips) {
      m_rd.get().debug_chip[section.ichip][DEBUG_WRONG_CHIPID]++;
    } else if ((unsigned) m_rd.get().chipid[section.ichip] != ichipid) {
      m_rd.get().debug_chip[section.ichip][DEBUG_WRONG_CHIPID]++;
      m_rd.get().chipid[section.ichip] = ichipid;
    } else  {
      m_rd.get().chipid[section.ichip] = ichipid;
    }
  }
    
  // BCID
  for (unsigned icol = 0; icol < n_columns; ++icol) {
    m_rd.get().bcid[section.ichip][icol] = (*(iraw_data++)).to_ulong();
    if (m_rd.get().bcid[section.ichip][icol] > MAX_VALUE_16BITS)
      m_rd.get().debug_chip[section.ichip][DEBUG_WRONG_BCID]++;
  }

  for (unsigned icol = 0; icol < n_columns; ++icol) {

    for (unsigned ichan = 0; ichan < NCHANNELS; ++ichan) {
      // CHARGE
      m_rd.get().charge [section.ichip][ichan][icol] = (*iraw_data & x0FFF).to_ulong();
      if (m_rd.get().charge[section.ichip][ichan][icol] > MAX_VALUE_12BITS)
        m_rd.get().debug_chip[section.ichip][DEBUG_WRONG_ADC]++;
      // HIT (0: no hit, 1: hit)
      m_rd.get().hit    [section.ichip][ichan][icol] = (*iraw_data)[12];
      // GAIN (0: low gain, 1: high gain)
      m_rd.get().gs     [section.ichip][ichan][icol] = (*(iraw_data++))[13];
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
    }

    for (unsigned ichan = 0; ichan < NCHANNELS; ++ichan) {
      // TIME
      m_rd.get().time[section.ichip][ichan][icol] = (*iraw_data & x0FFF).to_ulong();
      if (m_rd.get().time[section.ichip][ichan][icol] > MAX_VALUE_12BITS)
        m_rd.get().debug_chip[section.ichip][DEBUG_WRONG_TDC]++;
      if (m_config.tdc_is_calibrated) {
        ;// TODO: TDC calibration
      }
      if (m_rd.get().hit[section.ichip][ichan][icol] != (*iraw_data)[12]) {
        m_rd.get().debug_chip[section.ichip][DEBUG_WRONG_HIT_BIT]++;
      }
      if (m_rd.get().gs [section.ichip][ichan][icol] != (*(iraw_data++))[13]) {
        m_rd.get().debug_chip[section.ichip][DEBUG_WRONG_GAIN_BIT]++;
      }
    }
  }
  FillTree();
}

void EventReader::ReadNextSection(std::istream& is, const MarkerSeeker::Section section) {
  m_readers_ring[section.type](is, section);
}

void EventReader::FillTree() {
  if (m_tree->Fill() < 0)
    throw std::runtime_error("Failed to fill the TTree");
}

void EventReader::InitializeRing() {
  m_readers_ring[MarkerSeeker::MarkerType::SpillHeader]  = [this](std::istream& is, const MarkerSeeker::Section section) { return this->ReadSpillHeader(is, section); };
  m_readers_ring[MarkerSeeker::MarkerType::ChipHeader]   = [this](std::istream& is, const MarkerSeeker::Section section) { return this->ReadChipHeader(is, section); };
  m_readers_ring[MarkerSeeker::MarkerType::RawData]      = [this](std::istream& is, const MarkerSeeker::Section section) { return this->ReadRawData(is, section); };
  m_readers_ring[MarkerSeeker::MarkerType::ChipTrailer]  = [this](std::istream& is, const MarkerSeeker::Section section) { return this->ReadChipTrailer(is, section); };
  m_readers_ring[MarkerSeeker::MarkerType::SpillTrailer] = [this](std::istream& is, const MarkerSeeker::Section section) { return this->ReadSpillTrailer(is, section); };
  if (m_config.has_spill_number)
    m_readers_ring[MarkerSeeker::MarkerType::SpillNumber]  = [this](std::istream& is, const MarkerSeeker::Section section) { return this->ReadSpillNumber(is, section); };
}
