#ifndef WGDECODER_HPP_
#define WGDECODER_HPP_

// user includes
#include "wgConst.hpp"

#define NUM_MARKER_TYPES 6

#define BITS_PER_LINE    16
#define BYTES_PER_LINE   2

// Error codes
#define DE_SUCCESS                       0
#define ERR_CANNOT_CREATE_DIRECTORY      1
#define ERR_CANNOT_OVERWRITE_OUTPUT_FILE 2
#define ERR_WRONG_DIF_VALUE              3
#define ERR_FAILED_OPEN_RAW_FILE         4
#define ERR_INPUT_FILE_NOT_FOUND         5
#define ERR_READING_RAW_FILE             6

// markers
#define FIRST_SPILL_MARKER   std::bitset<BITS_PER_LINE>(0xFFFA)
#define SPILL_NUMBER_MARKER  std::bitset<BITS_PER_LINE>(0xFFFB)
#define SPILL_HEADER_MARKER  std::bitset<BITS_PER_LINE>(0xFFFC)
#define CHIP_HEADER_MARKER   std::bitset<BITS_PER_LINE>(0xFFFD)
#define CHIP_TRAILER_MARKER  std::bitset<BITS_PER_LINE>(0xFFFE)
#define SPILL_TRAILER_MARKER std::bitset<BITS_PER_LINE>(0xFFFF)
#define SPACE_MARKER         std::bitset<BITS_PER_LINE>(0x2020)
#define SP_MARKER            std::bitset<BITS_PER_LINE>(0x5053)
#define IL_MARKER            std::bitset<BITS_PER_LINE>(0x4C49)
#define CH_MARKER            std::bitset<BITS_PER_LINE>(0x4843)
#define IP_MARKER            std::bitset<BITS_PER_LINE>(0x5049)

// masks
#define x000F                std::bitset<BITS_PER_LINE>(0x00FF)
#define x00FF                std::bitset<BITS_PER_LINE>(0x00FF)
#define x0FFF                std::bitset<BITS_PER_LINE>(0x0FFF)
#define xFFF0                std::bitset<BITS_PER_LINE>(0x0FFF)
#define xFF00                std::bitset<BITS_PER_LINE>(0xFF00)
#define xF000                std::bitset<BITS_PER_LINE>(0xF000)
#define x0000                std::bitset<BITS_PER_LINE>(0x0000)

// This is needed to call the following functions from Python using ctypes
#ifdef __cplusplus
extern "C" {
#endif

int wgDecoder(const char * x_input_raw_file,
              const char * x_calibration_dir,
              const char * x_output_dir,
              bool overwrite = false,
              bool compatibility_mode = false,
              unsigned dif = 1,
              unsigned n_chips = 0);
  
#ifdef __cplusplus
}
#endif

#endif /* WGDECODER_HPP_ */
