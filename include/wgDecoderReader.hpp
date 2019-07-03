#ifndef WGDECODERREADER_HPP_
#define WGDECODERREADER_HPP_

// system includes
#include <bitset>
#include <istream>
#include <functional>

// user includes
#include "wgConst.hpp"
#include "wgDecoder.hpp"

///////////////////////////////////////////////////////////////////////////////
//                             EventReader class                             //
///////////////////////////////////////////////////////////////////////////////

class EventReader {

 private:

  Raw_t rd;

  void ReadSpillNumber (std::istream& is);
  void ReadSpillHeader (std::istream& is);
  void ReadChipHeader  (std::istream& is);
  void ReadChipTrailer (std::istream& is);
  void ReadSpillTrailer(std::istream& is);
};

#endif /* WGDECODERREADER_HPP_ */
