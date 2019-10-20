#ifndef WGRAWEMULATOR_H
#define WGRAWEMULATOR_H

#include "wgDecoder.hpp"

class RawEmulatorConfig {

 public:
  unsigned n_spills = 1;
  unsigned n_chips = 1;
  unsigned n_columns = 1;
  unsigned n_chip_id = 1;
  unsigned spill_mode = BEAM_SPILL;
  bool has_spill_number = false;
  unsigned time = 1;
  unsigned charge = 1;
  unsigned bcid = 1;
  bool gain = true;
  bool hit = true;
  RawEmulatorConfig() {};
};

int wgRawEmulator(const std::string & output_file, RawEmulatorConfig & raw);

#endif /* WGRAWEMULATOR_H */
