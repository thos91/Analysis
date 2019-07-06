// system includes
#include <bitset>
#include <istream>
#include <functional>

// user includes
#include "wgConst.hpp"
#include "wgDecoder.hpp"
#include "wgDecoderReader.hpp"
#include "wgDecoderSeeker.hpp"

namespace wg_utils = wagasci_decoder_utils;

///////////////////////////////////////////////////////////////////////////////
//                             EventReader class                             //
///////////////////////////////////////////////////////////////////////////////

EventReader::EventReader(const RawDataConfig& config) :
    m_config(config.n_chips, config.n_channels, config.n_chip_id),
    m_rd(config.n_chips, config.n_channels) {}

void EventReader::ReadSpillNumber(std::istream& is, const MarkerSeeker::Section& section) {
  is.seekg(section.start);
  std::vector<std::bitset<M>> raw_data(GetNumberOfLinesToRead(section));
  wg_utils::ReadChunk(is, raw_data);
  
  // raw_data[0] is the SPILL_NUMBER_MARKER
  m_rd.spill_number = raw_data[1].to_ulong();
  m_rd.spill_mode   = raw_data[2][15];
  if (m_rd.spill_mode != BEAM_SPILL && m_rd.spill_mode != NON_BEAM_SPILL) {
    m_rd.debug_spill[DEBUG_SPILL_MODE]++;
  } else {
    // If we are in NON_BEAM_SPILL mode basically the spill number has
    // no meaning so we can safely ignore it. In BEAM_SPILL mode we
    // want to check if the same spill number is used twice or if we
    // have a spill gap (non consecutive spill number)
    if (m_rd.spill_mode == BEAM_SPILL) {
      unsigned spill_number_gap = m_rd.spill_number - m_config.last_spill_number;    
      m_config.last_spill_number = m_rd.spill_number;
      if (spill_number_gap == 0) {
        m_rd.debug_spill[DEBUG_SAME_SPILL_NUMBER]++;
      } else if (spill_number_gap != 1) {
        m_rd.debug_spill[DEBUG_SPILL_NUMBER_GAP]++;
      }
    }
  }
}

void EventReader::ReadSpillHeader(std::istream& is, const MarkerSeeker::Section& section) {
  is.seekg(section.start);
  std::vector<std::bitset<M>> raw_data(GetNumberOfLinesToRead(section));
  wg_utils::ReadChunk(is, raw_data);
  // raw_data[0] is the SPILL_HEADER_MARKER
  // Spill count most significant byte
  bitset<2*M> spill_count_msb = raw_data[1].to_ulong();
  spill_count_msb <<= M;
  // Spill count least significant byte
  bitset<2*M> spill_count_lsb = raw_data[2].to_ulong();
  m_rd.spill_count = (spill_count_msb | spill_count_lsb).to_ullong();

  if (m_config.spill_insert_flag) {
    m_rd.spill_number = m_rd.spill_count;
    m_rd.spill_mode   = BEAM_SPILL;
  }

  unsigned spill_count_gap = m_rd.spill_count - m_config.last_spill_count;    
  m_config.last_spill_count = m_rd.spill_number;
  if (spill_count_gap == 0) {
    m_rd.debug_spill[DEBUG_SAME_SPILL_COUNT]++;
  } else if (spill_count_gap != 1) {
    m_rd.debug_spill[DEBUG_SPILL_COUNT_GAP]++;
  }
}

void EventReader::ReadChipHeader(std::istream& is, const MarkerSeeker::Section& section) {
  is.seekg(section.start);
  std::vector<std::bitset<M>> raw_data(GetNumberOfLinesToRead(section));
  wg_utils::ReadChunk(is, raw_data);

  m_rd.chipid[section.ichip] = raw_data[1].to_ulong();
  if ((unsigned) m_rd.chipid[section.ichip] >= m_config.n_chips ||
      m_rd.chipid[section.ichip] < 0) {
    m_rd.debug_spill[DEBUG_WRONG_CHIPID]++;
  }
}

void EventReader::ReadChipTrailer(std::istream& is, const MarkerSeeker::Section& section) {
  is.seekg(section.start);
  std::vector<std::bitset<M>> raw_data(GetNumberOfLinesToRead(section));
  wg_utils::ReadChunk(is, raw_data);

  unsigned chipid = raw_data[1].to_ulong();
  if (chipid != (unsigned) m_rd.chipid[section.ichip] ||
      chipid >= m_config.n_chips || chipid < 0) {
    m_rd.debug_spill[DEBUG_WRONG_CHIPID]++;
  }
}

void EventReader::ReadSpillTrailer(std::istream& is, const MarkerSeeker::Section& section) {
  is.seekg(section.start);
  std::vector<std::bitset<M>> raw_data(GetNumberOfLinesToRead(section));
  wg_utils::ReadChunk(is, raw_data);

  // raw_data[0] is the SPILL_TRAILER_MARKER
  // Spill count most significant byte
  bitset<2*M> spill_count_msb1 = raw_data[1].to_ulong();
  spill_count_msb1 <<= M;
  // Spill count least significant byte
  bitset<2*M> spill_count_lsb1 = raw_data[2].to_ulong();
  m_rd.spill_count = (spill_count_msb1 | spill_count_lsb1).to_ullong();
  
  unsigned n_found_chips = ( raw_data[3] & x00FF ).to_ulong(); 
  if (n_found_chips != m_config.n_chips) {
    m_rd.debug_spill[DEBUG_WRONG_NCHIPS]++;
  }
  
  if (raw_data.size() == SPILL_TRAILER_LENGTH) {
    bitset<2*M> spill_count_msb2 = raw_data[4].to_ulong();
    spill_count_msb2 <<= M;
    // Spill count least significant byte
    bitset<2*M> spill_count_lsb2 = raw_data[5].to_ulong();
    unsigned spill_count2 = (spill_count_msb2 | spill_count_lsb2).to_ullong();
    
    if ((unsigned) m_rd.spill_count != spill_count2) {
      m_rd.debug_spill[DEBUG_SPILL_TRAILER]++;
    }
  }
}

void EventReader::ReadRawData(std::istream& is, const MarkerSeeker::Section& section) {
  is.seekg(section.start);
  std::vector<std::bitset<M>> raw_data(GetNumberOfLinesToRead(section));
  wg_utils::ReadChunk(is, raw_data);

  unsigned n_columns = raw_data.size() - CHIP_ID_LENGTH / ONE_COLUMN_LENGTH;

  for (std::vector<std::bitset<M>>::reverse_iterator iraw_data = raw_data.rbegin(); 
       iraw_data != raw_data.rend(); ++iraw_data) {

    // CHIPID
    unsigned chipid = (*iraw_data & x00FF).to_ulong();
    if (chipid > m_config.n_chips) {
      m_rd.debug_chip[section.ichip][DEBUG_WRONG_CHIPID]++;
    } else if ((unsigned) m_rd.chipid[section.ichip] != chipid) {
      m_rd.debug_chip[section.ichip][DEBUG_WRONG_CHIPID]++;
      m_rd.chipid[section.ichip] = chipid;
    } else  {
      m_rd.chipid[section.ichip] = chipid;
    }
    
    // BCID
    for (unsigned icol = 0; icol < n_columns; ++icol) {
      m_rd.bcid[section.ichip][icol] = iraw_data->to_ulong();
      if (m_rd.bcid[section.ichip][icol] > MAX_VALUE_16BITS)
        m_rd.debug_chip[section.ichip][DEBUG_WRONG_BCID]++;
    }

    for (unsigned icol = 0; icol < n_columns; ++icol) {

      for (unsigned ichan = 0; ichan < NCHANNELS; ++ichan) {
        // CHARGE
        m_rd.charge [section.ichip][ichan][icol] = (*iraw_data & x0FFF).to_ulong();
        if (m_rd.charge[section.ichip][ichan][icol] > MAX_VALUE_12BITS)
          m_rd.debug_chip[section.ichip][DEBUG_WRONG_ADC]++;
        // HIT (0: no hit, 1: hit)
        m_rd.hit    [section.ichip][ichan][icol] = (*iraw_data)[12];
        // GAIN (0: low gain, 1: high gain)
        m_rd.gs     [section.ichip][ichan][icol] = (*iraw_data)[13];
        // Only if the detector is already calibrated fill the histograms
        if (m_config.adc_is_calibrated) {
          // P.E.
          unsigned charge = m_rd.charge[section.ichip][ichan][icol];
          unsigned pedestal = m_rd.pedestal[m_rd.chipid[section.ichip]][ichan][icol];
          unsigned gain = m_rd.gain[m_rd.chipid[section.ichip]][ichan][icol];
          if( m_rd.gs[section.ichip][ichan][icol] == HIGH_GAIN_BIT ) { // High Gain
            m_rd.pe[section.ichip][ichan][icol] = HIGH_GAIN_NORM * ( charge - pedestal ) / gain;
          } else { // Low Gain
            m_rd.pe[section.ichip][ichan][icol] = LOW_GAIN_NORM * ( charge - pedestal ) / gain;
          }
        }
      }

      for (unsigned ichan = 0; ichan < NCHANNELS; ++ichan) {
        // TIME
        m_rd.time[section.ichip][ichan][icol] = (*iraw_data & x0FFF).to_ulong();
        if (m_rd.time[section.ichip][ichan][icol] > MAX_VALUE_12BITS)
          m_rd.debug_chip[section.ichip][DEBUG_WRONG_TDC]++;
        if (m_config.tdc_is_calibrated) {
          ;// TODO: TDC calibration
        }
        if (m_rd.hit[section.ichip][ichan][icol] != (*iraw_data)[12]) {
          m_rd.debug_chip[section.ichip][DEBUG_WRONG_HIT_BIT]++;
        }
        if (m_rd.gs [section.ichip][ichan][icol] != (*iraw_data)[13]) {
          m_rd.debug_chip[section.ichip][DEBUG_WRONG_GAIN_BIT]++;
        }
      }
    }
  }
}

void EventReader::ReadNextSection(std::istream& is, MarkerSeeker::Section section) {
  m_readers_ring[section.type](is, section);
}

void EventReader::FillTree(TTree * tree) {
  tree->Fill();
  m_rd.clear();
}

void EventReader::InitializeRing() {
  m_readers_ring[MarkerSeeker::MarkerType::SpillNumber]  = std::bind(&EventReader::ReadSpillNumber,  this, std::placeholders::_1, std::placeholders::_2);
  m_readers_ring[MarkerSeeker::MarkerType::SpillHeader]  = std::bind(&EventReader::ReadSpillHeader,  this, std::placeholders::_1, std::placeholders::_2);
  m_readers_ring[MarkerSeeker::MarkerType::ChipHeader]   = std::bind(&EventReader::ReadChipHeader,   this, std::placeholders::_1, std::placeholders::_2);
  m_readers_ring[MarkerSeeker::MarkerType::RawData]      = std::bind(&EventReader::ReadRawData,      this, std::placeholders::_1, std::placeholders::_2);
  m_readers_ring[MarkerSeeker::MarkerType::ChipTrailer]  = std::bind(&EventReader::ReadChipTrailer,  this, std::placeholders::_1, std::placeholders::_2);
  m_readers_ring[MarkerSeeker::MarkerType::SpillTrailer] = std::bind(&EventReader::ReadSpillTrailer, this, std::placeholders::_1, std::placeholders::_2);
}
