#ifndef WGDECODERUTILS_TPP_
#define WGDECODERUTILS_TPP_

///////////////////////////////////////////////////////////////////////////////
//                                  ReadLine                                 //
///////////////////////////////////////////////////////////////////////////////

namespace wagasci_decoder_utils {

std::streampos ReadLine(std::istream& is, std::bitset<M>& raw_data) {
  is.read((char*) &raw_data, M / 8);
  if (is) throw wgInvalidFile("EOF");
  return is.tellg();
}

///////////////////////////////////////////////////////////////////////////////
//                                 ReadChunk                                 //
///////////////////////////////////////////////////////////////////////////////

template<std::size_t SIZE>
std::streampos ReadChunk(std::istream& is, std::array<std::bitset<M>, SIZE>& raw_data) {
  for (unsigned i = 0; i < raw_data.size(); ++i) {
    is.read((char*) &raw_data[i], M / 8);
    if (is) throw wgInvalidFile("EOF");
  }
  return is.tellg();
}

///////////////////////////////////////////////////////////////////////////////
//                                FindInArray                                //
///////////////////////////////////////////////////////////////////////////////

template <typename T, std::size_t SIZE>
std::pair<bool, std::size_t> FindInArray(const std::array<T, SIZE>& array_of_elements, const T& element) {
  std::pair<bool, std::size_t > result;
 
  // Find given element in std::array
  auto it = std::find(array_of_elements.begin(), array_of_elements.end(), element);
 
  if (it != array_of_elements.end()) {
    result.second = distance(array_of_elements.begin(), it);
    result.first = true;
  }
  else {
    result.first = false;
    result.second = -1;
  }
  return result;
}

} // namespace wagasci_decoder_utils

#endif /* WGDECODERUTILS_TPP_ */
