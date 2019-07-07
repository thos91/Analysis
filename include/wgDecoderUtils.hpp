#ifndef WGDECODERUTILS_H
#define WGDECODERUTILS_H

// system includes
#include <bitset>
#include <istream>
#include <vector>

// user includes
#include "wgDecoder.hpp"

///////////////////////////////////////////////////////////////////////////////
//                           wagasci_decoder_utils                           //
///////////////////////////////////////////////////////////////////////////////

namespace wagasci_decoder_utils {

std::streampos ReadLine(std::istream& is, std::bitset<BITS_PER_LINE>& raw_data);
std::streampos ReadChunk(std::istream& is, std::vector<std::bitset<BITS_PER_LINE>>& raw_data);
std::pair<bool, std::size_t> FindInVector(const std::vector<std::bitset<BITS_PER_LINE>>& vector_of_elements,
                                          const std::bitset<BITS_PER_LINE>& element);
unsigned GetNumChipID(string & input_raw_file);
bool HasSpillNumber(string & input_raw_file);

}

///////////////////////////////////////////////////////////////////////////////
//                            RawDataConfig class                            //
///////////////////////////////////////////////////////////////////////////////

class RawDataConfig {

 public:
  const unsigned n_chips       = NCHIPS;
  const unsigned n_channels    = NCHANNELS;
  const unsigned n_chip_id     = 1;
  const bool has_spill_number  = false;
  const bool adc_is_calibrated = false;
  const bool tdc_is_calibrated = false;
  
  RawDataConfig();
  RawDataConfig(unsigned n_chips, unsigned n_channels, unsigned n_chip_id, bool has_spill_number,
                bool adc_is_calibrated, bool tdc_is_calibrated);
};

#endif /* WGDECODERUTILS_H */
