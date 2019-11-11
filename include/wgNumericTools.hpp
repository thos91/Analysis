#ifndef WGNUMERICTOOLS_H
#define WGNUMERICTOOLS_H

// system includes
#include <vector>
#include <array>
#include <cstddef>
#include <cmath>

namespace wagasci_tools {

namespace numeric {

// calculate the not weighted mean of a std::array or std::vector
// It is just a wrapper around std::accumulate
double mean(std::vector<double> vec);

template<std::size_t SIZE>
double mean(std::array<double, SIZE> arr)  {
  if (SIZE == 0) return std::nan("zero size");
  std::vector<double> vec(arr.begin(), arr.end());
  return mean(vec);
}

// calculate the non weighted standard deviation of a std::array or std::vector
// It is just a wrapper around ROOT TMath::StdDev
double standard_deviation(std::vector<double> vec);

template<std::size_t SIZE>
double standard_deviation(std::array<double, SIZE> arr) {
  if (SIZE == 0) return std::nan("zero size");
  std::vector<double> vec(arr.begin(), arr.end());
  return standard_deviation(vec);
}
  
// routines to check if the gain and its variance are physical or not
// If the mean argument is given the gain or sigma are assigned it.
// They return true if the value is not physical (> MAX_GAIN <
// MIN_GAIN) in the case of the gain or (> MAX_SIGMA < MIN_SIGMA) in
// the case of the variance
bool is_unphysical_gain(double gain);
bool is_unphysical_sigma(double sigma_1pe, double sigma_2pe);
bool is_unphysical_gain(double charge_1pe, double charge_2pe);
bool is_unphysical_sigma(double sigma);

} // numeric

} // wagasci_tools

#endif /* WGNUMERICTOOLS_H */
