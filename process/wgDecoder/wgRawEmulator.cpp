// system includes
#include <string>
#include <bitset>
#include <vector>

// system C includes
#include <getopt.h>

// user includes
#include "wgDecoder.hpp"

std::vector<std::bitset<M>> SpillHeader(unsigned spill_id) {
  std::vector<std::bitset<M>> spill_header;

  spill_header.push_back(xFFFC); // Marker
  spill_header.push_back(bitset<M>(0)); // spill ID
  spill_header.push_back(bitset<M>(spill_id)); // spill ID
  spill_header.push_back(x5053); // "SP"
  spill_header.push_back(x4C49); // "IL"
  spill_header.push_back(x2020); // "  "

  return spill_header;
}

std::vector<std::bitset<M>> ChipHeader(unsigned chip_id) {
  std::vector<std::bitset<M>> chip_header;

  if (chip_id > 255)
    throw std::invalid_argument("chip ID is greater than 255 : " + to_string(chip_id));
  
  chip_header.push_back(xFFFD); // Marker
  chip_header.push_back(bitset<M>(chip_id) | xFF00); // chip ID
  chip_header.push_back(x4843); // "CH"
  chip_header.push_back(x5049); // "IP"
  chip_header.push_back(x2020); // "  "

  return chip_header;
}

std::vector<std::bitset<M>> RawTDC(unsigned time, bool hit, bool gain) {

  if (time > 4095)
    throw std::invalid_argument("time is greater than 4095 : " + to_string(time));

  std::bitset<M> bitset(time);
  bitset[M-3] = gain;
  bitset[M-4] = hit;
  std::vector<std::bitset<M>> raw_tdc(NCHANNELS, bitset);

  return raw_tdc;
}

std::vector<std::bitset<M>> RawADC(unsigned charge, bool hit, bool gain) {

  if (charge > 4095)
    throw std::invalid_argument("charge is greater than 4095 : " + to_string(charge));

  std::bitset<M> bitset(charge);
  bitset[M-3] = gain;
  bitset[M-4] = hit;
  std::vector<std::bitset<M>> raw_adc(NCHANNELS, bitset);
  return raw_adc;
}

std::vector<std::bitset<M>> RawBCID(unsigned bcid, unsigned n_columns) {
  std::vector<std::bitset<M>> raw_bcid(n_columns, bitset<M>(bcid));
  return raw_bcid;
}

std::vector<std::bitset<M>> ChipID(unsigned chip_id, unsigned n_chip_id) {
  std::vector<std::bitset<M>> raw_chip_id(n_chip_id, chip_id);
  return raw_chip_id;
}


std::vector<std::bitset<M>> ChipTrailer(unsigned chip_id) {
  std::vector<std::bitset<M>> chip_trailer;

  if (chip_id > 255)
    throw std::invalid_argument("chip ID is greater than 255 : " + to_string(chip_id));
  
  chip_trailer.push_back(xFFFE); // Marker
  chip_trailer.push_back(bitset<M>(chip_id) | xFF00 ); // chip ID
  chip_trailer.push_back(x2020); // "  "
  chip_trailer.push_back(x2020); // "  "

  return chip_trailer;
}

std::vector<std::bitset<M>> SpillTrailer(unsigned spill_id, unsigned n_chips) {
  std::vector<std::bitset<M>> spill_trailer;

  if (n_chips > 255)
    throw std::invalid_argument("chip ID is greater than 255 : " + to_string(n_chips));

  spill_trailer.push_back(xFFFF); // Marker
  spill_trailer.push_back(bitset<M>(0)); // spill ID
  spill_trailer.push_back(bitset<M>(spill_id)); // spill ID
  spill_trailer.push_back(bitset<M>(n_chips) & x00FF); // nb_chip
  spill_trailer.push_back(bitset<M>(0)); // spill ID
  spill_trailer.push_back(bitset<M>(spill_id)); // spill ID
  spill_trailer.push_back(x2020); // "  "

  return spill_trailer;
}

std::ofstream& operator<<(std::ofstream& ofs, uint16_t d) {
  ofs.write((char*) &d, sizeof(uint16_t));
  return ofs;
}

void WriteToBinary(std::ofstream & os, std::vector<std::bitset<M>> raw_vector) {
  for (auto const & raw_line : raw_vector) {
    uint16_t bit_line = raw_line.to_ulong();
    os << bit_line;
  }
}

class RawEmulator {

 public:
  unsigned n_spills = 1;
  unsigned n_chips = 1;
  unsigned n_channels = 1;
  unsigned n_columns = 1;
  unsigned n_chip_id = 1;
  unsigned time = 1;
  unsigned charge = 1;
  unsigned bcid = 1;
  bool gain = true;
  bool hit = true;
  RawEmulator() {};
  RawEmulator(unsigned n_spills) : n_spills(n_spills) {}
  RawEmulator(unsigned n_spills, unsigned n_chips, unsigned n_channels, unsigned n_columns, unsigned n_chip_id)
      : n_spills(n_spills), n_chips(n_chips), n_channels(n_channels), n_columns(n_columns), n_chip_id(n_chip_id) {}
  RawEmulator(unsigned n_spills, unsigned n_chips, unsigned n_channels, unsigned n_columns, unsigned n_chip_id,
              unsigned time, unsigned charge, unsigned bcid)
      : n_spills(n_spills), n_chips(n_chips), n_channels(n_channels), n_columns(n_columns), n_chip_id(n_chip_id),
        time(time), charge(charge), bcid(bcid) {}
};

int wgRawEmulator(const string & output_file, RawEmulator & raw) {

  std::ofstream os;
  
  try {
    os.open(output_file, std::ios::out | std::ios::binary);
    
    for (unsigned ispill = 1; ispill <= raw.n_spills; ++ispill) {

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
  catch (const exception & e) {
    std::cout << e.what() << "\n";
    if (os.is_open()) os.close();
    return 1;
  }
  return 0;
}

void print_help(const char * program_name) {
  cout << program_name << " : emulates the SPIROC2D raw data\n"
      "  -o (char*) : output file path\n"
      "  -x         : print this help\n";
  exit(0);
}

int main(int argc, char** argv) {
  int opt;
  string output_file("");
  while ((opt = getopt(argc, argv, "o:h")) != -1) {
    switch(opt) {
      case 'o':
        output_file = optarg;
        break;
      case 'h':
        print_help(argv[0]);
        break;
      default :
        print_help(argv[0]);
    }
  }
  
  if( output_file.empty() ) { 
    std::cout << "Output file is empty" << std::endl;
    return 1;
  }
  
  RawEmulator raw;
  wgRawEmulator(output_file, raw);
}
