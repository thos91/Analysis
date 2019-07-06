#ifndef WGDECODERUTILS_TPP_
#define WGDECODERUTILS_TPP_

///////////////////////////////////////////////////////////////////////////////
//                                  ReadLine                                 //
///////////////////////////////////////////////////////////////////////////////

namespace wagasci_decoder_utils {

std::streampos ReadLine(std::istream& is, std::bitset<M>& raw_data) {
  is.read((char*) &raw_data, BytesPerLine);
  if (is) throw wgInvalidFile("EOF");
  return is.tellg();
}

///////////////////////////////////////////////////////////////////////////////
//                                 ReadChunk                                 //
///////////////////////////////////////////////////////////////////////////////

std::streampos ReadChunk(std::istream& is, std::vector<std::bitset<M>>& raw_data) {
  for (unsigned i = 0; i < raw_data.size(); ++i) {
    is.read((char*) &raw_data[i], BytesPerLine);
    if (is) throw wgInvalidFile("EOF");
  }
  return is.tellg();
}

///////////////////////////////////////////////////////////////////////////////
//                                FindInVector                               //
///////////////////////////////////////////////////////////////////////////////

template <typename T>
std::pair<bool, std::size_t> FindInVector(const std::vector<T>& vector_of_elements, const T& element) {
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
  
  bitset<M> raw_data;
  bitset<M> last_two_raw_data[2];
  bool found = false;
  unsigned iline = 0, max_lines = 10 * (16 * (1 + 2 * NCHANNELS) + 2) ;
  
  while ( ifs.read((char * ) &raw_data, M / 8) && !found && ++iline < max_lines ) {

    if (raw_data == CHIP_TRAILER_MARKER &&
        last_two_raw_data[0] == last_two_raw_data[1]) {
      found = true;
    }

    last_four_raw_data[1] = last_four_raw_data[0];
    last_four_raw_data[0] = raw_data;
  }

  ifs.close();
  if (found) return 2;
  else return 1;
}

///////////////////////////////////////////////////////////////////////////////
//                               GetInsertFlag                               //
///////////////////////////////////////////////////////////////////////////////

bool HasSpillNumber(string & input_raw_file) {
  
  ifstream ifs;
  ifs.open(input_raw_file.c_str(), ios_base::in | ios_base::binary);
  if (!ifs.is_open()) {
    Log.eWrite("[wgDecoder] Failed to open raw file: " + string(strerror(errno)));
    return ERR_FAILED_OPEN_RAW_FILE;
  }
  
  bitset<M> raw_data;
  bool found = false;
  unsigned iline = 0, max_lines = 10 * (16 * (1 + 2 * NCHANNELS) + 2) ;
  
  while ( ifs.read((char * ) &raw_data, M / 8) && !found && ++iline < max_lines ) {

    if (raw_data == SPILL_NUMBER_MARKER)
      found = true;
  }

  ifs.close();
  return found;
}

} // namespace wagasci_decoder_utils

#endif /* WGDECODERUTILS_TPP_ */
