// system includes
#include <vector>
#include <array>
#include <numeric>
#include <cmath>
#include <algorithm>
#include <cstddef>

// ROOT includes
#include "TMath.h"

// user includes
#include "wgGainCalib.hpp"

namespace wagasci_tools {

namespace numeric {

double mean(std::vector<double> vec) {
    vec.erase(std::remove_if(std::begin(vec),  std::end(vec),
      [](const double& value) { return std::isnan(value); }),
            std::end(vec));
    if (vec.empty()) return std::nan("zero size");
  return std::accumulate(vec.begin(), vec.end(), 0) / vec.size();
}

double standard_deviation(std::vector<double> vec) {
      vec.erase(std::remove_if(std::begin(vec),  std::end(vec),
      [](const double& value) { return std::isnan(value); }),
            std::end(vec));
      if (vec.empty()) return std::nan("zero size");
  return TMath::StdDev(vec.begin(), vec.end());
}

bool is_unphysical_gain(double gain) {
  return (gain > gain_calib::MAX_GAIN ||
          gain < gain_calib::MIN_GAIN) ? true : false;
}

bool is_unphysical_sigma(double sigma) {
  return (sigma > gain_calib::MAX_SIGMA ||
          sigma < gain_calib::MIN_SIGMA) ? true : false;
}

bool is_unphysical_gain(double &gain, const double mean) {
  if (gain > gain_calib::MAX_GAIN ||
      gain < gain_calib::MIN_GAIN) {
    gain = mean;
    return true;
  }
  return false;
}

bool is_unphysical_sigma(double &sigma, const double mean) {
  if (sigma > gain_calib::MAX_SIGMA ||
      sigma < gain_calib::MIN_SIGMA) {
    sigma = mean;
    return true;
  }
  return false;
}

} // numeric

} // wagasci_tools
