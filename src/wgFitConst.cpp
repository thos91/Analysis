#include "wgFitConst.hpp"

// minimum number of entries to fit a histogram
const int WG_MIN_ENTRIES_FOR_FIT = 100;

// fit range for the charge_nohit histogram
const int WG_BEGIN_CHARGE_NOHIT = 300;
const int WG_END_CHARGE_NOHIT = 700;

// fit range for the charge_hit_* histogram
const int WG_BEGIN_CHARGE_HIT_HG = 350;
const int WG_END_CHARGE_HIT_HG = 750;
const int WG_BEGIN_CHARGE_HIT_LG = 0; // TODO: 
const int WG_END_CHARGE_HIT_LG = 0; // TODO: 

// average values of the peak for each peu
const int WG_PEAK_CHARGE_0PE = 500;
const int WG_PEAK_CHARGE_1PE = 600;
const int WG_PEAK_CHARGE_2PE = 700;

// limits for the dark noise rate
const double WG_LOWER_LIMIT_1PE = 20000;  // 20 kHz
const double WG_UPPER_LIMIT_1PE = 160000; // 160 kHz
const double WG_LOWER_LIMIT_2PE = 1000;   // 1 kHz
const double WG_UPPER_LIMIT_2PE = 8000;   // 8 kHz
const double WG_LOWER_LIMIT_3PE = 50;     // 50 Hz
const double WG_UPPER_LIMIT_3PE = 400;    // 400 Hz

// nominal value for the MPPC gain
const double WG_NOMINAL_GAIN = 60;

// time interval in seconds of 1 BCID count
const double TIME_BCID = 580e-9;

// Maximum value of the BCID
const unsigned MAX_BCID_BIN = 12288;

// maximum (most right) and minimum (most left) values of the
// difference between the measured pedestal and the nominal pedestal
const int WG_PED_DIFF_MAX = 10;  
const int WG_PED_DIFF_MIN = -50;

double noise_to_pe(const double noise) {
  if      (noise >  WG_UPPER_LIMIT_1PE)                               return 0.5;
  else if (noise >= WG_LOWER_LIMIT_1PE && noise < WG_UPPER_LIMIT_1PE) return 1.0;
  else if (noise >= WG_UPPER_LIMIT_2PE && noise < WG_LOWER_LIMIT_1PE) return 1.5;
  else if (noise >= WG_LOWER_LIMIT_2PE && noise < WG_UPPER_LIMIT_2PE) return 2.0;
  else if (noise >= WG_UPPER_LIMIT_3PE && noise < WG_LOWER_LIMIT_2PE) return 2.5;
  else if (noise >= WG_LOWER_LIMIT_3PE && noise < WG_UPPER_LIMIT_3PE) return 3.0;
  else if (noise <  WG_LOWER_LIMIT_3PE && noise > 0)                  return 3.5;
  else                                                                return 0; 
}
