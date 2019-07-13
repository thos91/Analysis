#ifndef WGDECODERREADER_HPP_
#define WGDECODERREADER_HPP_

// system includes
#include <bitset>
#include <istream>
#include <functional>

// user includes
#include "wgConst.hpp"
#include "wgRawData.hpp"
#include "wgDecoder.hpp"
#include "wgDecoderSeeker.hpp"

// Debug macros to fill the debug_spill and debug_chip histograms
// spill
const unsigned DEBUG_SPILL_MODE        = 0;
const unsigned DEBUG_SAME_SPILL_NUMBER = 1;
const unsigned DEBUG_SPILL_NUMBER_GAP  = 2;
const unsigned DEBUG_SAME_SPILL_COUNT  = 3;
const unsigned DEBUG_SPILL_COUNT_GAP   = 4;
const unsigned DEBUG_SPILL_TRAILER     = 5;
const unsigned DEBUG_WRONG_NCHIPS      = 6;

// chip
const unsigned DEBUG_WRONG_BCID        = 0;
const unsigned DEBUG_WRONG_HIT_BIT     = 1;
const unsigned DEBUG_WRONG_GAIN_BIT    = 2;
const unsigned DEBUG_WRONG_ADC         = 3;
const unsigned DEBUG_WRONG_TDC         = 4;
const unsigned DEBUG_WRONG_CHIPID      = 5;

///////////////////////////////////////////////////////////////////////////////
//                             SectionReader class                           //
///////////////////////////////////////////////////////////////////////////////

// Similarly to the SectionSeeker class, the SectionReader class is
// used to read a section into the Raw_t "object". Basically the
// SectionReader object reads the section into the rd object and then
// fill the TTree tree with it.
//
// Only one object is needed to read a single file. From a functional
// point of view, whether you create a new object every section or
// not, it doesn't matter. However, by frequently creating and
// destroying objects, you are just adding some unnecessary overhead
// and slowing down the decoder, so I would advise againt it.
//
// No assumption is made about the raw data: the data is always stored
// "as is" without any tampering or modification. If the data is
// corrupted it is left corrupted.
//
// The only assumption that is made about the passed section is for it
// to contain SOME data. This is always assured by the SectionSeeker
// class, because it returns only when the next section contains a
// readable amount of data (even if the data is corrupted). What I am
// trying to say is that, if the SectionSeeker class returns a
// section, you can be sure that it contains some data so you can
// safely pass it to the SectionReader class.

class SectionReader {
 public:

  // When creating a new object you have to pass a RawDataConfig
  // object, a pointer to an already initialized TTree and an empty
  // Raw_t object. If the TTree is not initialized a
  // std::runtime_error exception is thrown. The TTree must contain as
  // many branches as needed to store all the variables in the Raw_t
  // rd object. The branches must be initialized using the very same
  // "rd" object. If there is any mismatch between the TTree and the
  // Raw_t object, the behavior is undefined and most probably will
  // result in an exception being thrown.
  //
  // At the end of every ReadRawData method the TTree tree is filled
  // with the Raw_t rd object.
  
  SectionReader(const RawDataConfig& config, TTree* tree, Raw_t& rd);

  // Read the Section section from the stream is into the Raw_t m_rd object
  void ReadNextSection(std::istream& is, const SectionSeeker::Section section);
  
 private:

  RawDataConfig m_config;
  TTree * m_tree;
  std::reference_wrapper<Raw_t> m_rd;

  // To calculate the spill number gap and the spill count gap we need
  // to store the spill number and spill count of the last spill.
  unsigned m_last_spill_number = 0;
  unsigned m_last_spill_count = 0;

  // The concept of readers ring is the very same of the seekers ring
  // in the SectionSeeker class
  std::size_t m_num_marker_types;
  typedef std::function<void(std::istream& is, const SectionSeeker::Section& section)> reader;
  std::array<reader, NUM_SECTION_TYPES> m_readers_ring;

  // Read the section delimited by the "Section" struct from the "is"
  // file stream
  void ReadSpillNumber (std::istream& is, const SectionSeeker::Section& section);
  void ReadSpillHeader (std::istream& is, const SectionSeeker::Section& section);
  void ReadChipHeader  (std::istream& is, const SectionSeeker::Section& section);
  void ReadChipTrailer (std::istream& is, const SectionSeeker::Section& section);
  void ReadSpillTrailer(std::istream& is, const SectionSeeker::Section& section);
  void ReadRawData     (std::istream& is, const SectionSeeker::Section& section);

  void InitializeRing();

  //  Fill the m_tree TTree with the m_rd Raw_t object
  void FillTree();
};

#endif /* WGDECODERREADER_HPP_ */
