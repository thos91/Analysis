#include "wgFitConst.hpp"

const double sigma          = 8.;     // pre-gaussian-fit value of sigma
const double max_sigma      = 20.;    // max value (upper limit for sigma)
const double min_sigma      = 1.;     // min value (lower limit for sigma)

const double threshold      = 0.03;   //initial threshold value
const double max_threshold  = 0.05;   
const double min_threshold  = 0.01;    

const int begin_pe     = 350;   // begining value of fitting range for charge_hit histogram
const int end_pe       = 750;  // ending value of fitting range for charge_hit histogram

const int begin_pe_HG     = 350;   // begining value of fitting range for charge_hit_HG histogram
const int end_pe_HG       = 750;   // ending value of fitting range for charge_hit_HG histogram

const int begin_ped        = 300;   // begining value of fitting range for charge_nohit 
const int end_ped          = 700;   // ending value of fitting range for charge_nohit

const int N_DarkNoise      = 2;
const int N_Pedestal       = 2;
const int N_LED            = 2;

const double l_limit_3pe = 50;      // lower limit for dark noise with 2.5 p.e. threshold (50 Hz)     
const double u_limit_3pe = 400;     // upper limit for dark noise with 2.5 p.e. threshold (400 Hz)

const double l_limit_2pe = 1000;    // lower limit for dark noise with 1.5 p.e. threshold (1 kHz)
const double u_limit_2pe = 8000;    // upper limit for dark noise with 1.5 p.e. threshold (8 kHz)

const double l_limit_1pe =  20000;  // lower limit for dark noise with 0.5 p.e. threshold (20 kHz)
const double u_limit_1pe = 160000;  // upper limit for dark noise with 0.5 p.e. threshold (160 kHz)

const int begin_LED        = 400;    //begining value of fitting range for dark noise
const int end_LED          = 1000;    //ending value of fitting range for dark noise

const double est_Gain      = 40;     //estimate Gain

const double GSthreshold    = 2510;     //estimate Gain Select Threshold
const double GSthreshold_0  = 2730;     //estimate Gain Select Threshold for column 0.

const double time_bcid_bin = 580e-9;    // BCID (slow clock) period in seconds (580 ns)

const int ped_diff_max = 10;  // maximum (most right) value of the difference
							  // between the measured pedestal and the nominal
							  // pedestal
const int ped_diff_min = -50; // minimum (most left) value

const unsigned MAX_BCID_BIN  = 12288;

double NoiseToPe(const double noise) {
  if      (noise >  u_limit_1pe)                        return 0.5;
  else if (noise >= l_limit_1pe && noise < u_limit_1pe) return 1.0;
  else if (noise >= u_limit_2pe && noise < l_limit_1pe) return 1.5;
  else if (noise >= l_limit_2pe && noise < u_limit_2pe) return 2.0;
  else if (noise >= u_limit_3pe && noise < l_limit_2pe) return 2.5;
  else if (noise >= l_limit_3pe && noise < u_limit_3pe) return 3.0;
  else if (noise <  l_limit_3pe && noise > 0)           return 3.5;
  else                                                  return 0; 
}
