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
          gain < gain_calib::MIN_GAIN ||
          std::isnan(gain)) ? true : false;
}

bool is_unphysical_sigma(double sigma) {
  return (sigma > gain_calib::MAX_SIGMA ||
          sigma < gain_calib::MIN_SIGMA ||
          std::isnan(sigma)) ? true : false;
}

bool is_unphysical_gain(double charge_1pe, double charge_2pe) {
  double gain = (charge_1pe <= 0 || charge_2pe <= 0) ? nan("") :
                charge_2pe - charge_1pe;
  return is_unphysical_gain(gain);
}

bool is_unphysical_sigma(double sigma_1pe, double sigma_2pe) {
    double sigma = (sigma_1pe <= 0 || sigma_2pe <= 0) ? nan("") :
                   std::sqrt(std::pow(sigma_1pe, 2) +
                             std::pow(sigma_2pe, 2));
    return is_unphysical_sigma(sigma);
}



} // numeric

} // wagasci_tools
