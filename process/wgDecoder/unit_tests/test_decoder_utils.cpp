#include <string>

#include "wgDecoderUtils.hpp"
#include "wgRawEmulator.hpp"

int main () {

  RawEmulatorConfig raw_config;
  raw_config.n_spills = 10;
  raw_config.n_chip_id = 2;
  raw_config.has_spill_number = true;

  std::string raw_file;
  for (unsigned i = 1; i <= 20; ++i) {
    raw_file = std::to_string(i) + "_chips.raw";
    raw_config.n_chips = i;
    wgRawEmulator(raw_file, raw_config);
    unsigned n_chips = wagasci_decoder_utils::GetNumChips(raw_file);
    unsigned n_chip_id = wagasci_decoder_utils::GetNumChipID(raw_file);
    bool has_spill_number = wagasci_decoder_utils::HasSpillNumber(raw_file);
    if (n_chips != i) {
      std::cout << "[GetNumChips] " << i << " chips test failed\n";
      std::cout << "[GetNumChips] actual number of chips : " << i << " | number of chips detected : " << n_chips << "\n";
      break;
    }
    if (n_chip_id != 2) {
      std::cout << "[GetNumChipID] " << i << " chips test failed\n";
      std::cout << "[GetNumChipID] actual number of chip ID : " << 2 << " | number of chip ID detected : " << n_chip_id << "\n";
      break;
    }
    if (has_spill_number != true) {
      std::cout << "[HasSpillNumber] " << i << " chips test failed\n";
      std::cout << "[HasSpillNumber] should be true but is false\n";
      break;
    }
  }
  return 0;
}
