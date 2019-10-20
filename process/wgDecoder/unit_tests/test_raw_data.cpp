#include <string>

#include "wgDecoderUtils.hpp"
#include "wgRawEmulator.hpp"

void print_help(const char * program_name) {
  std::cout << program_name << " : print generic info about raw data\n"
      "  -o (char*) : output file path\n"
      "  -h         : print this help\n";
  exit(0);
}

int main (int argc, char** argv) {
  int opt;
  std::string raw_file;

  while ((opt = getopt(argc, argv, "f:h")) != -1) {
    switch(opt) {
      case 'f':
        raw_file = optarg;
        break;
      case 'h':
        print_help(argv[0]);
        break;
      default :
        print_help(argv[0]);
    }
  }

  unsigned n_chips = wagasci_decoder_utils::GetNumChips(raw_file);
  unsigned n_chip_id = wagasci_decoder_utils::GetNumChipID(raw_file);
  bool has_spill_number = wagasci_decoder_utils::HasSpillNumber(raw_file);
  bool has_phantom_menace = wagasci_decoder_utils::HasPhantomMenace(raw_file);
    
  std::cout << "Number of chips " << n_chips << "\n";
  std::cout << "Number of chip ID : " << n_chip_id << "\n";
  std::cout << "Has spill number : " << has_spill_number << "\n";
  std::cout << "Has phantom menace : " << has_phantom_menace << "\n";
  return 0;
}
