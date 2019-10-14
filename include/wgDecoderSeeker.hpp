#ifndef WGDECODERNEW_H
#define WGDECODERNEW_H

// system includes
#include <bitset>
#include <istream>
#include <functional>

// user includes
#include "wgConst.hpp"
#include "wgDecoder.hpp"
#include "wgDecoderUtils.hpp"

///////////////////////////////////////////////////////////////////////////////
//                             SectionSeeker class                            //
///////////////////////////////////////////////////////////////////////////////

// The objects of this class are used to seek (locate) the start, end
// and type of each section of the raw data file.  For a description
// of all the sections refer to the sphinx documentation.
//
// No assumption is made about the data contained in each section, nor
// any attempt is made to read, interpret or store that data. The data
// will be read by another object of a different class.
//
// The only public method of this class (but the constructor
// obviously) is the "SeekNextSection" method. This is all you
// need. Even if the internal working of the SectionSeeker class is
// quite complex, the API couldn't be easier to use. Basically you
// just configure the object passing a RawDataConfig object to
// it. Then every time you call the "SeekNextSection" method, a
// "Section" struct is returned containing the start, stop and type of
// the next section found. If the found section refers to a particular
// chip or spill, that chip number or spill count is returned. More
// info about the "Section" struct is given below.
//
// Usually you need only one SectionSeeker object to parse a single
// file. The object needs to store some info about the previous
// sections so it is not recommended to destruct the object after
// having read every single section. It may be possible to destruct
// the object after having read a whole spill but, that way you would
// hinder the automatic recognition of the first section of the new
// spill because the new object would not have any knowledge about the
// previous spill.

class SectionSeeker {

 public:

  // The SectionType enum just enumerates all the possible types of
  // sections that you may found in a raw file. Beware that not every
  // raw data format is the same and some files may not contain one
  // section or another. For example the PhantoMenace section can be
  // found only in the most recent files. It was introduced after an
  // upgrade of the DIF firmware. Currently not even the LLR
  // developers know what this section is there for. Most probably is
  // just some garbage that the DIF is throwing there for no apparent
  // reason. That is why the spooky name.

  // Used for array indexes! // Don't change the numbers!
  enum SectionType {
    SpillHeader = 0,
    ChipHeader,
    RawData,
    ChipTrailer,
    SpillTrailer,
    PhantomMenace,
    SpillNumber
  };

  // The "Section" struct contains all the information needed to read
  // a certain section of the raw data file. The "start" and "stop"
  // elements point to the start and end of the section. They are the
  // the output of the tellg() method and are just the number of bytes
  // from the start of the file stream to the start of the section or
  // the end of the section respectively. The "type" element is the
  // type of the section as indexed by the "SectionType" enum. If the
  // section refers to a certain chip or spill the "ichip" and
  // "ispill" elements are set accordingly. If the section doesn't
  // refer to any chip or spills those elements are undefined (should
  // be ignored).
  //
  // ATTENTION!
  // The ichip and ispill elements have no connection with the chip ID
  // and spill number fields of the raw data. The ichip element starts
  // from zero and just count the number of chips found in a certain
  // spill. It is strictly consecutive : every time a new chip is
  // found the ichip element is just incremented by one. Same for the
  // ispill element. Every time a new spill is found the ispill
  // element is incremented by one, no matter what is the spill
  // number, the spill flag or the acquisition ID fields in the raw
  // data.

  struct Section {
    std::streampos start;
    std::streampos stop;
    unsigned ichip = 0;
    unsigned ispill = 0;
    unsigned type;
    unsigned lines;
  };

  // To construct the object you need to pass a RawDataConfig
  // object. This object just contains some parameters about the raw
  // data file structure. More info about the RawDataConfig class in
  // the wgDecoderUtils.hpp header file.

  SectionSeeker(const RawDataConfig &config);

  // The only argument is the input file stream descriptor "is". The
  // file must be opened as a binary file with at least read
  // access.
  //
  // If the file is not opened, or the program has not read
  // permission, or the stream is not clear (error bit are set) the
  // behavior of this method is not defined. Most probably in any of
  // those cases an exception will be thrown. This method doesn't
  // check the sanity of the "is" stream in any way to avoid the
  // overhead of having to check for all those condition each and
  // every section. The wgDecoder is already slow as it is. For this
  // reason, make sure to check if the file is correctly opened BEFORE
  // calling this method.
  //
  // If the end of file is reached an exception of type wgEOF is
  // thrown. Catch that exception if you want to gracefully close the
  // file and so on and so forth.
  //
  // This method returns a "Section" struct containing all the info
  // about the next section. The stream position after having called
  // the method is undefined so always use the Section.start element
  // to locate the start of the next section and don't rely on the
  // stream position.
  
  Section SeekNextSection(std::istream& is);

 private:

  // The user/developer shouldn't need to read the private members and
  // methods of this class in order to use it. Anyway, the variable
  // names are clear enough that no explanation should be needed.

  // nuber of section types for this raw file. It can be different
  // depending on the particular structure of the raw data file
  // (e.g. if the spill number section is present or not, etc..). It
  // is set when the object is constructed and never changed until the
  // object goes out of scope.
  std::size_t m_num_section_types;

  // The m_seeker_ring is a circular buffer containing all the "Seek*"
  // methods needed to decode the particular raw data file. It must be
  // a circular buffer because when the last section is reached we
  // want to go straight to the first section again.
  typedef std::function<bool(std::istream& is)> seeker;
  std::array<seeker, NUM_SECTION_TYPES> m_seekers_ring;

  // This object contains all the configuration parameters about the
  // raw data format. It is set when the object is constructed and
  // never changed until the object goes out of scope.
  RawDataConfig m_config;

  // The current section struct. It is returned by the SeekNextSection
  // method when the next section has been found.
  Section m_current_section;

  // The last spill count (not related to the raw data acquisition ID or spill number)
  unsigned m_last_ispill = 0;
  // The last chip count (not related to the chip ID)
  unsigned m_last_ichip = 0;
  unsigned m_current_ichip = 0;

  // The seekers return true if the section was found and in good
  // shape, false if the section was not found or was hopelessly
  // corrupted.
  bool SeekSpillNumber  (std::istream& is);
  bool SeekSpillHeader  (std::istream& is);
  bool SeekChipHeader   (std::istream& is);
  bool SeekChipTrailer  (std::istream& is);
  bool SeekSpillTrailer (std::istream& is);
  bool SeekPhantomMenace(std::istream& is);
  bool SeekRawData      (std::istream& is);

  // Given the last_section_type section type, tells what the next
  // section should be, depending if the last section was found or
  // not.
  unsigned NextSectionType(unsigned last_section_type, bool last_section_was_found);

  // During the object construction, the m_seekers_ring is initialized
  // with the needed seekers. Because all the seekers are not always
  // needed, the ring is initialized only with the necessary seekers
  // and no more.
  void InitializeRing();
};

// helper function that, given a certain section, returns the number
// of lines (16-bit) contained in that section.
unsigned GetNumberOfLines(const SectionSeeker::Section & section);

#endif /* WGDECODERNEW_H */
