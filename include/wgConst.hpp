#ifndef WG_CONST_HPP_INCLUDE
#define WG_CONST_HPP_INCLUDE

// system includes
#include <vector>
#include <climits>
#include <map>
#include <mutex>

// user includes
#include "wgContiguousVectors.hpp"
#include "wgEnvironment.hpp"

// Only the ROOT Minuit2 minimizer is thread-safe. All the others are
// not. So if ROOT has not support for the Minuit2 minimizer than we
// have to restrict access to the fitting sections only to one thread
// at a time
#ifdef ROOT_HAS_NOT_MINUIT2
extern std::mutex MUTEX;
#endif

///////////////////////////////////////////////////////////////////////////////
//                            SPIROC2D and DECODER                           //
///////////////////////////////////////////////////////////////////////////////

const unsigned NDIFS      = 8;
const unsigned NCHIPS     = 20;
const unsigned NCHANNELS  = 36;
const unsigned MEMDEPTH   = 16;

const unsigned CHIP_HEADER_LENGTH    = 5;
const unsigned CHIP_TRAILER_LENGTH   = 4;
const unsigned SPILL_HEADER_LENGTH   = 6;
const unsigned SPILL_TRAILER_LENGTH  = 7;
const unsigned PHANTOM_MENACE_LENGTH = 3;
const unsigned SPILL_NUMBER_LENGTH   = 3;
const unsigned HEADERS_TRAILERS_TOTAL_LENGTH = CHIP_HEADER_LENGTH +
                                               CHIP_TRAILER_LENGTH +
                                               SPILL_HEADER_LENGTH +
                                               SPILL_TRAILER_LENGTH +
                                               PHANTOM_MENACE_LENGTH +
                                               SPILL_NUMBER_LENGTH;

// One column length in 16 bits lines (but the BCID)
// 1 BCID              +
// n_channels times    +
// n_channels charges
const unsigned ONE_COLUMN_LENGTH = 1 + 2 * NCHANNELS;
const unsigned MAX_EVENT = UINT_MAX;
const unsigned MAX_RAWDATA_LENGTH = 2 * (NCHIPS * MEMDEPTH * ONE_COLUMN_LENGTH +
                                         HEADERS_TRAILERS_TOTAL_LENGTH);

// Debug macros to fill the debug_spill and debug_chip histograms
// spill
enum SPILL_DEBUG_CODES {
  DEBUG_SPILL_MODE        = 0,
  DEBUG_SAME_SPILL_NUMBER = 1,
  DEBUG_SPILL_NUMBER_GAP  = 2,
  DEBUG_SAME_SPILL_COUNT  = 3,
  DEBUG_SPILL_COUNT_GAP   = 4,
  DEBUG_SPILL_TRAILER     = 5,
  DEBUG_WRONG_NCHIPS      = 6,
  N_DEBUG_SPILL           = 7
};

// chip
enum CHIP_DEBUG_CODES {
  DEBUG_WRONG_BCID        = 0,
  DEBUG_WRONG_HIT_BIT     = 1,
  DEBUG_WRONG_GAIN_BIT    = 2,
  DEBUG_WRONG_ADC         = 3,
  DEBUG_WRONG_TDC         = 4,
  DEBUG_WRONG_CHIPID      = 5,
  DEBUG_WRONG_NCOLUMNS    = 6,
  N_DEBUG_CHIP            = 7
};

enum SPILL_TYPE {
  NON_BEAM_SPILL = 0,  // non beam spill bit (spill_flag)
  BEAM_SPILL     = 1   // beam spill bit (spill_flag)
};

const unsigned HIT_BIT        = 1;    // there was a hit (over threshold)
const unsigned NO_HIT_BIT     = 0;    // there was no hit (under threshold)
const unsigned HIGH_GAIN_BIT  = 1;    // high gain bit (gs)
const unsigned LOW_GAIN_BIT   = 0;    // low gain bit (gs)
const double   HIGH_GAIN_NORM   = 1.08; // Normalization for the high gain
const double   LOW_GAIN_NORM    = 10.8; // Normalization for the low gain

const unsigned MAX_VALUE_16BITS = 65535;
const unsigned MAX_VALUE_12BITS = 4095;
const unsigned MAX_VALUE_10BITS = 1023;
const unsigned MAX_VALUE_8BITS  = 255;
const unsigned MAX_VALUE_6BITS  = 63;
const unsigned MAX_VALUE_4BITS  = 15;

// The SPIROC2D bitstream is 1192 bits long. It contains all the parameters that
// can be set on the SPIROC2D chip. These parameters are listed in the
// spiroc2d.csv file.
//  - The _INDEX macros are just for sorting the parameters
//  - The _START macros are the start bit of the parameter(s) they refer to
//  - the _LENGTH macros are the length in bits of the parameter
//  - the _OFFSET macros are the length in bits of a single channel parameter
//    (when the parameter can be set for each channel individually)

const unsigned BITSTREAM_HEX_STRING_LENGTH = 300;  // length of the bitstream hex string
const unsigned BITSTREAM_BIN_STRING_LENGTH = 1192; // length of the bitstream bin string
const unsigned VALUE_OFFSET_IN_BITS = 6;           // offset in bits before a parameter start

// global 10-bit threshold
const unsigned GLOBAL_THRESHOLD_INDEX      = 0;
const unsigned GLOBAL_THRESHOLD_START      = 931;
const unsigned GLOBAL_THRESHOLD_LENGTH     = 10; // 4 bits (most significant bits) +
                                                 // 4 bits (least significant bits) +
                                                 // 2 bits (??)

// global 10-bit gain selection threshold
const unsigned GLOBAL_GS_INDEX             = 1;
const unsigned GLOBAL_GS_THRESHOLD_START   = 941;
const unsigned GLOBAL_GS_THRESHOLD_LENGTH  = 10; // 4 bits (most significant bits) +
                                                 // 4 bits (least significant bits) +
                                                 // 2 bits (??)

// Input 8-bit DAC Data from channel 0 to 35 
const unsigned ADJ_INPUTDAC_INDEX          = 2;
const unsigned ADJ_INPUTDAC_START          = 37;
const unsigned ADJ_INPUTDAC_LENGTH         = 8;
const unsigned ADJ_INPUTDAC_OFFSET         = 9; // 8 bits (DAC7...DAC0) + 1 bit (DAC ON)

// adjustable 6-bit high gain (HG) preamp
const unsigned ADJ_AMPDAC_INDEX            = 3;
const unsigned ADJ_AMPDAC_START            = 367;  
const unsigned ADJ_AMPDAC_LENGTH           = 6;
const unsigned ADJ_AMPDAC_OFFSET           = 15; // 6 bits (HG) + 6 bits (LG) + 3 bits '000'

// adjustable 4-bit threshold
const unsigned ADJ_THRESHOLD_INDEX         = 4;
const unsigned ADJ_THRESHOLD_START         = 1006;
const unsigned ADJ_THRESHOLD_LENGTH        = 4;
const unsigned ADJ_THRESHOLD_OFFSET        = 4; // 4 bits

// CHIP ID 8-bit
const unsigned CHIPID_INDEX                 = 5;
const unsigned CHIPID_START                 = 18;
const unsigned CHIPID_LENGTH                = 8; // 8 bits

// 1-bit input DAC Voltage Reference (1 = internal 4.5V   0 = internal 2.5V)
const unsigned GLOBAL_INPUT_DAC_REF_INDEX   = 6;
const unsigned GLOBAL_INPUT_DAC_REF_START   = 36;

///////////////////////////////////////////////////////////////////////////////
//                                 wgEditXML                                 //
///////////////////////////////////////////////////////////////////////////////

const bool NO_CREATE_NEW_MODE = false;
const bool CREATE_NEW_MODE    = true;

///////////////////////////////////////////////////////////////////////////////
//                      wgPedestalCalib and wgGainCalib                      //
///////////////////////////////////////////////////////////////////////////////

enum PEU_LEVEL {
  ONE_PE = 0, // threshold at 0.5 p.e.
  TWO_PE = 1, // threshold at 1.5 p.e.
  NUM_PE = 2
};

///////////////////////////////////////////////////////////////////////////////
//                                  TYPEDEF                                  //
///////////////////////////////////////////////////////////////////////////////

typedef std::vector<std::vector<std::vector<std::vector<std::vector<double>>>>> d5vector;
typedef std::vector<std::vector<std::vector<std::vector<double>>>> d4vector;
typedef std::vector<std::vector<std::vector<double>>> d3vector;
typedef std::vector<std::vector<double>> d2vector;
typedef std::vector<double> d1vector;

typedef Contiguous3Vector<double> d3CCvector;
typedef Contiguous2Vector<double> d2CCvector;

typedef std::vector<std::vector<std::vector<std::vector<int>>>> i4vector;
typedef std::vector<std::vector<std::vector<int>>> i3vector;
typedef std::vector<std::vector<int>> i2vector;
typedef std::vector<int> i1vector;

typedef Contiguous3Vector<int> i3CCvector;
typedef Contiguous2Vector<int> i2CCvector;

typedef std::vector<std::vector<std::vector<std::vector<unsigned>>>>u4vector;
typedef std::vector<std::vector<std::vector<unsigned>>> u3vector;
typedef std::vector<std::vector<unsigned>> u2vector;
typedef std::vector<unsigned> u1vector;

typedef Contiguous3Vector<unsigned> u3CCvector;
typedef Contiguous2Vector<unsigned> u2CCvector;

typedef std::map<unsigned, std::vector<std::vector<std::vector<std::array<int, NUM_PE>>>>>
ChargeVector;
typedef std::vector<std::vector<std::vector<std::vector<size_t>>>>s4vector;

typedef std::map<unsigned, std::vector<std::vector<std::array<int, MEMDEPTH>>>>
GainVector;

#endif // WG_CONST_HPP_INCLUDE
