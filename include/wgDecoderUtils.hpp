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

// Read a line from "is" and store it in raw_data. Return the stream
// position of the line.
std::streampos ReadLine(std::istream& is, std::bitset<BITS_PER_LINE>& raw_data);

// Read raw_data.size() lines form "is" and store them in
// raw_data. Return the stream position of the last line.
std::streampos ReadChunk(std::istream& is, std::vector<std::bitset<BITS_PER_LINE>>& raw_data);

// Throw one byte away from the istream *is*. Sometimes the byte offset may
// become wrong and we need it to restore balance in the force.
void ThrowOneByte(std::istream& is);

// Find if element is present in vector_of_elements. If at least one
// occurrence is found return true and the position of the first
// occurence as a std::pair.
std::pair<bool, std::size_t> FindInVector(const std::vector<std::bitset<BITS_PER_LINE>>& vector_of_elements,
                                          const std::bitset<BITS_PER_LINE>& element);

// Parse the input_raw_file file and guess how many CHIP ID fields are
// present at the end of the SPIROC2D raw data format.
unsigned GetNumChipID(std::string & input_raw_file);

// Parse the input_raw_file file and return true if the PhantoMenace
// section is present
bool HasPhantomMenace(std::string & input_raw_file);

// Parse the input_raw_file file and return true if the SpillNumber
// section is present
bool HasSpillNumber  (std::string & input_raw_file);

// Parse the input_raw_file file and guess how many chips are present
// in each spill
unsigned GetNumChips (std::string & input_raw_file);

// increase the stack size to 64MB
void increase_stack_size();
}

///////////////////////////////////////////////////////////////////////////////
//                            RawDataConfig class                            //
///////////////////////////////////////////////////////////////////////////////

// class containing only some constant parameters describing the
// structure of the raw data file. The member names are so clear that
// hopefully no explanation should be needed.

class RawDataConfig {

 public:
  const unsigned n_chips        = NCHIPS;
  const unsigned n_channels     = NCHANNELS;
  const unsigned n_chip_id      = 1;
  const bool has_spill_number   = false;
  const bool has_phantom_menace = false;
  const bool adc_is_calibrated  = false;
  const bool tdc_is_calibrated  = false;
  
  RawDataConfig();
  RawDataConfig(unsigned n_chips, unsigned n_channels, unsigned n_chip_id, bool has_spill_number,
                bool has_phantom_menace, bool adc_is_calibrated, bool tdc_is_calibrated);
};

#endif /* WGDECODERUTILS_H */
