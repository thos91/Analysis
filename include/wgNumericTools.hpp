#ifndef WGNUMERICTOOLS_H
#define WGNUMERICTOOLS_H

// system includes
#include <vector>
#include <array>
#include <numeric>
#include <cmath>
#include <algorithm>

// ROOT includes
#include "TMath.h"

namespace wagasci_tools {

namespace numeric {

double mean(std::vector<double> vec) {
    vec.erase(std::remove_if(std::begin(vec),  std::end(vec),
      [](const double& value) { return std::isnan(value); }),
            std::end(vec));
    if (vec.empty()) return std::nan("zero size");
  return std::accumulate(vec.begin(), vec.end(), 0) / vec.size();
}

template<std::size_t SIZE>
double mean(std::array<double, SIZE> arr)  {
  if (SIZE == 0) return std::nan("zero size");
  std::vector<double> vec(arr.begin(), arr.end());
  return mean(vec);
}

double standard_deviation(std::vector<double> vec) {
      vec.erase(std::remove_if(std::begin(vec),  std::end(vec),
      [](const double& value) { return std::isnan(value); }),
            std::end(vec));
      if (vec.empty()) return std::nan("zero size");
  return TMath::StdDev(vec.begin(), vec.end());
}

template<std::size_t SIZE>
double standard_deviation(std::array<double, SIZE> arr) {
  if (SIZE == 0) return std::nan("zero size");
  std::vector<double> vec(arr.begin(), arr.end());
   return standard_deviation(vec);
}

} // numeric

} // wagasci_tools

#endif /* WGNUMERICTOOLS_H */
