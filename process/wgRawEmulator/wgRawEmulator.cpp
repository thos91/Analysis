// system includes
#include <string>

// system C includes
#include <getopt.h>

// user includes
#include "wgDecoder.hpp"
#include "wgRawEmulator.hpp"


void print_help(const char * program_name) {
  std::cout << program_name << " : emulates the SPIROC2D raw data\n"
      "  -s         : number of spills (default 10)\n"
      "  -c         : number of chips (default 20)\n"
      "  -m         : old raw data format mode (default no)\n"
      "  -n         : has spill number (default no)\n"
      "  -o (char*) : output file path\n"
      "  -h         : print this help\n";
  exit(0);
}

int main(int argc, char** argv) {
  int opt;
  std::string output_file("");

  RawEmulatorConfig raw_config;
  raw_config.n_spills = 10;
  raw_config.n_chips = 20;
  raw_config.n_chip_id = 2;
  raw_config.has_spill_number = false;

  while ((opt = getopt(argc, argv, "s:c:o:hmn")) != -1) {
    switch(opt) {
      case 's':
        raw_config.n_spills = std::stoi(optarg);
        break;
      case 'c':
        raw_config.n_chips = std::stoi(optarg);
        break;
      case 'm':
        raw_config.n_chip_id = 1;
        break;
      case 'n':
         raw_config.has_spill_number = true;
        break;
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

  wgRawEmulator(output_file, raw_config);
}
