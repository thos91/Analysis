// system includes
#include <bitset>
#include <istream>
#include <functional>
#include <vector>

// system C includes
#include <csignal>

// user includes
#include "wgConst.hpp"
#include "wgExceptions.hpp"
#include "wgLogger.hpp"
#include "wgDecoder.hpp"
#include "wgDecoderUtils.hpp"

///////////////////////////////////////////////////////////////////////////////
//                                  ReadLine                                 //
///////////////////////////////////////////////////////////////////////////////

namespace wagasci_decoder_utils {

std::streampos ReadLine(std::istream& is, std::bitset<BITS_PER_LINE>& raw_data) {
  is.read((char*) &raw_data, BYTES_PER_LINE);
  if (is.eof()) throw wgEOF("EOF reached");
  return is.tellg();
}

///////////////////////////////////////////////////////////////////////////////
//                                 ReadChunk                                 //
///////////////////////////////////////////////////////////////////////////////

std::streampos ReadChunk(std::istream& is, std::vector<std::bitset<BITS_PER_LINE>>& raw_data) {
  for (unsigned i = 0; i < raw_data.size(); ++i) {
    is.read((char*) &raw_data[i], BYTES_PER_LINE);
    if (is.eof()) throw wgEOF("EOF reached");
  }
  return is.tellg();
}

///////////////////////////////////////////////////////////////////////////////
//                                FindInVector                               //
///////////////////////////////////////////////////////////////////////////////
std::pair<bool, std::size_t> FindInVector(const std::vector<std::bitset<BITS_PER_LINE>>& vector_of_elements,
                                          const std::bitset<BITS_PER_LINE>& element) {
  std::pair<bool, std::size_t > result;
 
  // Find given element in std::vector
  auto it = std::find(vector_of_elements.begin(), vector_of_elements.end(), element);
 
  if (it != vector_of_elements.end()) {
    result.second = distance(vector_of_elements.begin(), it);
    result.first = true;
  }
  else {
    result.first = false;
    result.second = -1;
  }
  return result;
}

///////////////////////////////////////////////////////////////////////////////
//                                GetNumChipID                               //
///////////////////////////////////////////////////////////////////////////////

unsigned GetNumChipID(string & input_raw_file) {
  
  ifstream ifs;
  ifs.open(input_raw_file.c_str(), ios_base::in | ios_base::binary);
  if (!ifs.is_open()) {
    Log.eWrite("[wgDecoder] Failed to open raw file: " + string(strerror(errno)));
    return ERR_FAILED_OPEN_RAW_FILE;
  }
  
  bitset<BITS_PER_LINE> raw_data;
  bitset<BITS_PER_LINE> last_two_raw_data[2];
  bool found = false;
  unsigned iline = 0, max_lines = 10 * (16 * (1 + 2 * NCHANNELS) + 2) ;
  
  while ( ifs.read((char * ) &raw_data, BYTES_PER_LINE) && !found && ++iline < max_lines ) {

    if (raw_data == CHIP_TRAILER_MARKER &&
        last_two_raw_data[0] == last_two_raw_data[1]) {
      found = true;
    }

    last_two_raw_data[1] = last_two_raw_data[0];
    last_two_raw_data[0] = raw_data;
  }

  ifs.close();
  if (found) return 2;
  else return 1;
}

///////////////////////////////////////////////////////////////////////////////
//                              HasSpillNumber                               //
///////////////////////////////////////////////////////////////////////////////

bool HasSpillNumber(string & input_raw_file) {
  
  ifstream ifs;
  ifs.open(input_raw_file.c_str(), ios_base::in | ios_base::binary);
  if (!ifs.is_open()) {
    Log.eWrite("[wgDecoder] Failed to open raw file: " + string(strerror(errno)));
    return false;
  }
  
  bitset<BITS_PER_LINE> raw_data;
  unsigned iline = 0;
  unsigned max_lines = 20 * NCHIPS * (16 * (1 + 2 * NCHANNELS) + 2);
  unsigned found_counter = 0;
  
  while (ifs.read((char * ) &raw_data, BYTES_PER_LINE) &&
         ++iline < max_lines &&
         found_counter < 3) {

    if (raw_data == SPILL_NUMBER_MARKER)
      found_counter++;
  }

  bool found = found_counter >= 3;

  ifs.close();
  return found;
}

///////////////////////////////////////////////////////////////////////////////
//                              HasPhantomMenace                             //
///////////////////////////////////////////////////////////////////////////////

bool HasPhantomMenace(string & input_raw_file) {
  
  ifstream ifs;
  ifs.open(input_raw_file.c_str(), ios_base::in | ios_base::binary);
  if (!ifs.is_open()) {
    Log.eWrite("[wgDecoder] Failed to open raw file: " + string(strerror(errno)));
    return false;
  }
  
  std::vector<bitset<BITS_PER_LINE>> raw_data(PHANTOM_MENACE_LENGTH + 2);
  unsigned iline = 0;
  unsigned max_lines = 20 * NCHIPS * (16 * (1 + 2 * NCHANNELS) + 2);
  unsigned found_counter = 0;

  try {
    while (++iline < max_lines && found_counter < 3) {
      ReadChunk(ifs, raw_data);
      if (raw_data[0] == SPACE_MARKER &&
          raw_data[2] == x0000 &&
          raw_data[3] == x0000 &&
          raw_data[4] == SPILL_HEADER_MARKER)
        ++found_counter;
      ifs.seekg(iline * BYTES_PER_LINE);
    }
  } catch (const wgEOF& e) {;}

  bool found = found_counter >= 3;
    
  ifs.close();
  return found;
}

///////////////////////////////////////////////////////////////////////////////
//                                 GetNumChips                               //
///////////////////////////////////////////////////////////////////////////////

unsigned GetNumChips(string & input_raw_file) {
  
  ifstream ifs;
  ifs.open(input_raw_file.c_str(), ios_base::in | ios_base::binary);
  if (!ifs.is_open()) {
    Log.eWrite("[wgDecoder] Failed to open raw file: " + string(strerror(errno)));
    return ERR_FAILED_OPEN_RAW_FILE;
  }
  
  std::bitset<BITS_PER_LINE> raw_data;
  std::vector<unsigned> n_chips_vec;

  unsigned counter = 0;
  
  while ( ifs.read((char * ) &raw_data, BYTES_PER_LINE) && counter < 10 ) {
    if (raw_data == SPILL_TRAILER_MARKER) {
      ++counter;
      ifs.seekg(2 * BYTES_PER_LINE, ios::cur);
      ifs.read((char * ) &raw_data, BYTES_PER_LINE);
      n_chips_vec.push_back(raw_data.to_ulong());
      }
    }
  ifs.close();
  
  if (n_chips_vec.size() == 0) return 0;

  unsigned max = 0;
  unsigned most_common_n_chips = -1;
  map<unsigned, unsigned> m;
  for (auto vi = n_chips_vec.begin(); vi != n_chips_vec.end(); vi++) {
    m[*vi]++;
    if (m[*vi] > max) {
      max = m[*vi]; 
      most_common_n_chips = *vi;
    }
  }
  return most_common_n_chips;
}

} // namespace wagasci_decoder_utils

///////////////////////////////////////////////////////////////////////////////
//                            RawDataConfig class                            //
///////////////////////////////////////////////////////////////////////////////

RawDataConfig::RawDataConfig() {}
RawDataConfig::RawDataConfig(unsigned n_chips, unsigned n_channels, unsigned n_chip_id, bool has_spill_number,
                             bool has_phantom_menace, bool adc_is_calibrated, bool tdc_is_calibrated)
    : n_chips(n_chips), n_channels(n_channels), n_chip_id(n_chip_id), has_spill_number(has_spill_number),
      has_phantom_menace(has_phantom_menace), adc_is_calibrated(adc_is_calibrated), tdc_is_calibrated(tdc_is_calibrated) {}
