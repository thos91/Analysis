// system includes
#include <string>
#include <bitset>
#include <vector>
#include <fstream>

// user includes
#include "wgDecoder.hpp"
#include "wgRawEmulator.hpp"

std::vector<std::bitset<BITS_PER_LINE>> SpillNumber(unsigned spill_id, unsigned spill_mode) {
  std::vector<std::bitset<BITS_PER_LINE>> spill_number;

  spill_number.push_back(SPILL_NUMBER_MARKER); // Marker
  spill_number.push_back(std::bitset<BITS_PER_LINE>(spill_id)); // spill ID
  spill_number.push_back(std::bitset<BITS_PER_LINE>(spill_mode)); // spill flag

  return spill_number;
}

std::vector<std::bitset<BITS_PER_LINE>> SpillHeader(unsigned spill_id) {
  std::vector<std::bitset<BITS_PER_LINE>> spill_header;

  spill_header.push_back(SPILL_HEADER_MARKER); // Marker
  spill_header.push_back(std::bitset<BITS_PER_LINE>(0)); // spill ID
  spill_header.push_back(std::bitset<BITS_PER_LINE>(spill_id)); // spill ID
  spill_header.push_back(SP_MARKER);
  spill_header.push_back(IL_MARKER);
  spill_header.push_back(SPACE_MARKER);

  return spill_header;
}

std::vector<std::bitset<BITS_PER_LINE>> ChipHeader(unsigned chip_id) {
  std::vector<std::bitset<BITS_PER_LINE>> chip_header;

  if (chip_id > 255)
    throw std::invalid_argument("chip ID is greater than 255 : " + std::to_string(chip_id));
  
  chip_header.push_back(CHIP_HEADER_MARKER);
  chip_header.push_back(std::bitset<BITS_PER_LINE>(chip_id) | xFF00); // chip ID
  chip_header.push_back(CH_MARKER);
  chip_header.push_back(IP_MARKER);
  chip_header.push_back(SPACE_MARKER);

  return chip_header;
}

std::vector<std::bitset<BITS_PER_LINE>> RawTDC(unsigned time, bool hit, bool gain) {

  if (time > 4095)
    throw std::invalid_argument("time is greater than 4095 : " + std::to_string(time));

  std::bitset<BITS_PER_LINE> bitset(time);
  bitset[BITS_PER_LINE-3] = gain;
  bitset[BITS_PER_LINE-4] = hit;
  std::vector<std::bitset<BITS_PER_LINE>> raw_tdc(NCHANNELS, bitset);

  return raw_tdc;
}

std::vector<std::bitset<BITS_PER_LINE>> RawADC(unsigned charge, bool hit, bool gain) {

  if (charge > 4095)
    throw std::invalid_argument("charge is greater than 4095 : " + std::to_string(charge));

  std::bitset<BITS_PER_LINE> bitset(charge);
  bitset[BITS_PER_LINE-3] = gain;
  bitset[BITS_PER_LINE-4] = hit;
  std::vector<std::bitset<BITS_PER_LINE>> raw_adc(NCHANNELS, bitset);
  return raw_adc;
}

std::vector<std::bitset<BITS_PER_LINE>> RawBCID(unsigned bcid, unsigned n_columns) {
  std::vector<std::bitset<BITS_PER_LINE>> raw_bcid(n_columns, std::bitset<BITS_PER_LINE>(bcid));
  return raw_bcid;
}

std::vector<std::bitset<BITS_PER_LINE>> ChipID(unsigned chip_id, unsigned n_chip_id) {
  std::vector<std::bitset<BITS_PER_LINE>> raw_chip_id(n_chip_id, chip_id);
  return raw_chip_id;
}


std::vector<std::bitset<BITS_PER_LINE>> ChipTrailer(unsigned chip_id) {
  std::vector<std::bitset<BITS_PER_LINE>> chip_trailer;

  if (chip_id > 255)
    throw std::invalid_argument("chip ID is greater than 255 : " + std::to_string(chip_id));
  
  chip_trailer.push_back(CHIP_TRAILER_MARKER);
  chip_trailer.push_back(std::bitset<BITS_PER_LINE>(chip_id) | xFF00 ); // chip ID
  chip_trailer.push_back(SPACE_MARKER);
  chip_trailer.push_back(SPACE_MARKER);

  return chip_trailer;
}

std::vector<std::bitset<BITS_PER_LINE>> SpillTrailer(unsigned spill_id, unsigned n_chips) {
  std::vector<std::bitset<BITS_PER_LINE>> spill_trailer;

  if (n_chips > 255)
    throw std::invalid_argument("chip ID is greater than 255 : " + std::to_string(n_chips));

  spill_trailer.push_back(SPILL_TRAILER_MARKER);
  spill_trailer.push_back(std::bitset<BITS_PER_LINE>(0)); // spill ID
  spill_trailer.push_back(std::bitset<BITS_PER_LINE>(spill_id)); // spill ID
  spill_trailer.push_back(std::bitset<BITS_PER_LINE>(n_chips) & x00FF); // nb_chip
  spill_trailer.push_back(std::bitset<BITS_PER_LINE>(0)); // spill ID
  spill_trailer.push_back(std::bitset<BITS_PER_LINE>(spill_id)); // spill ID
  spill_trailer.push_back(SPACE_MARKER);

  return spill_trailer;
}

std::ofstream& operator<<(std::ofstream& ofs, uint16_t data) {
  ofs.write((char*) &data, sizeof(uint16_t));
  return ofs;
}

void WriteToBinary(std::ofstream & os, std::vector<std::bitset<BITS_PER_LINE>> raw_vector) {
  for (auto const & raw_line : raw_vector) {
    uint16_t bit_line = raw_line.to_ulong();
    os << bit_line;
  }
}

int wgRawEmulator(const std::string & output_file, RawEmulatorConfig & raw) {

  std::ofstream os;
  
  try {
    os.open(output_file, std::ios::out | std::ios::binary);
    
    for (unsigned ispill = 1; ispill <= raw.n_spills; ++ispill) {

      if (raw.has_spill_number) WriteToBinary(os, SpillNumber(ispill, raw.spill_mode));
      WriteToBinary(os, SpillHeader(ispill));

      for (unsigned ichip = 1; ichip <= raw.n_chips; ++ichip) {
        WriteToBinary(os, ChipHeader(ichip));
  
        for (unsigned icol = 1; icol <= raw.n_columns; ++icol) {
          WriteToBinary(os, RawTDC(raw.time, raw.hit, raw.gain));
          WriteToBinary(os, RawADC(raw.charge, raw.hit, raw.gain));
        }
        WriteToBinary(os, RawBCID(raw.bcid, raw.n_columns));
        WriteToBinary(os, ChipID(ichip, raw.n_chip_id));
  
        WriteToBinary(os, ChipTrailer(ichip));
      }

      WriteToBinary(os, SpillTrailer(ispill, raw.n_chips));
    }
    os.close();
  }
  catch (const std::exception & e) {
    std::cout << e.what() << "\n";
    if (os.is_open()) os.close();
    return 1;
  }
  return 0;
}
