#include <cmath>
#include <iostream>

#include <TError.h>

#include "wgFit.hpp"
#include "wgFileSystemTools.hpp"

int main() {
  gErrorIgnoreLevel = kError, kBreak, kSysError, kFatal;
  std::string histo_file("/home/wagasci-access/Desktop/test_hist.root");
  wgFit fit(histo_file, wagasci_tools::GetPath(histo_file));
  double fit_gain[2] = {0, 0};
  fit.gain(0, 0, 0, fit_gain, true);
  if (std::isnan(fit_gain[0]))
    std::cout << "Chip 0 channel 0 column 0 : fit failed\n";
  fit.gain(0, 1, 3, fit_gain, true);
  if (std::isnan(fit_gain[0]))
    std::cout << "Chip 0 channel 1 column 3 : fit failed\n";
  fit.gain(1, 0, 6, fit_gain, true);
  if (std::isnan(fit_gain[0]))
    std::cout << "Chip 1 channel 0 column 6 : fit failed\n";
  fit.gain(1, 1, 9, fit_gain, true);
  if (std::isnan(fit_gain[0]))
    std::cout << "Chip 1 channel 1 column 9 : fit failed\n";
  fit.gain(2, 0, 12, fit_gain, true);
  if (std::isnan(fit_gain[0]))
    std::cout << "Chip 2 channel 0 column 12 : fit failed\n";
  fit.gain(2, 1, 15, fit_gain, true);
  if (std::isnan(fit_gain[0]))
    std::cout << "Chip 2 channel 1 column 15 : fit failed\n";
}
