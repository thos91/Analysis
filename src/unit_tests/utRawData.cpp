#include "wgRawData.hpp"
#include "wgConst.hpp"
#include <iostream>

#define N_CHIPS_TEST 2
#define N_CHANNELS_TEST 2

int main() {
  Raw_t rd(N_CHIPS_TEST, N_CHANNELS_TEST);

  std::cout << "spill_number\n";
  std::cout << rd.spill_number << "\n";
  std::cout << "spill_mode\n";
  std::cout << rd.spill_mode << "\n";
  std::cout << "spill_count\n";
  std::cout << rd.spill_count << "\n";
  std::cout << "chipid\n";
  for (unsigned ichip = 0; ichip < N_CHIPS_TEST; ++ichip) {
    std::cout << rd.chipid[ichip] << "\n";
  }
  std::cout << "difid\n";
  for (unsigned ichip = 0; ichip < N_CHIPS_TEST; ++ichip) {
    std::cout << rd.difid[ichip] << "\n";
  }
  std::cout << "charge\n";
  for (unsigned ichip = 0; ichip < N_CHIPS_TEST; ++ichip) {
    for (unsigned ichan = 0; ichan < N_CHANNELS_TEST; ++ichan) {
      for (unsigned icol = 0; icol < MEMDEPTH; ++icol) {
        std::cout << rd.charge[ichip][ichan][icol]  << "\n";       
      }
    }
  }
  std::cout << "time\n";
  for (unsigned ichip = 0; ichip < N_CHIPS_TEST; ++ichip) {
    for (unsigned ichan = 0; ichan < N_CHANNELS_TEST; ++ichan) {
      for (unsigned icol = 0; icol < MEMDEPTH; ++icol) {
        std::cout << rd.time[ichip][ichan][icol]  << "\n";       
      }
    }
  }
  
  std::cout << "hit\n";
  for (unsigned ichip = 0; ichip < N_CHIPS_TEST; ++ichip) {
    for (unsigned ichan = 0; ichan < N_CHANNELS_TEST; ++ichan) {
      for (unsigned icol = 0; icol < MEMDEPTH; ++icol) {
        std::cout << rd.hit[ichip][ichan][icol]  << "\n";       
      }
    }
  }
  std::cout << "gs\n";
  for (unsigned ichip = 0; ichip < N_CHIPS_TEST; ++ichip) {
    for (unsigned ichan = 0; ichan < N_CHANNELS_TEST; ++ichan) {
      for (unsigned icol = 0; icol < MEMDEPTH; ++icol) {
        std::cout << rd.gs[ichip][ichan][icol]  << "\n";       
      }
    }
  }
  std::cout << "gs\n";
  for (unsigned ichip = 0; ichip < N_CHIPS_TEST; ++ichip) {
    for (unsigned ichan = 0; ichan < N_CHANNELS_TEST; ++ichan) {
      for (unsigned icol = 0; icol < MEMDEPTH; ++icol) {
        std::cout << rd.gs[ichip][ichan][icol]  << "\n";       
      }
    }
  }
  std::cout << "bcid\n";
  for (unsigned ichip = 0; ichip < N_CHIPS_TEST; ++ichip) {
    for (unsigned icol = 0; icol < MEMDEPTH; ++icol) {
      std::cout << rd.bcid[ichip][icol]  << "\n";       
    }
  } 
  return 0;
}
