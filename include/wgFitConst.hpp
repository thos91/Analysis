#ifndef WGFITCONST_H_INCLUDE
#define WGFITCONST_H_INCLUDE

// minimum number of entries to fit a histogram
extern const int WG_MIN_ENTRIES_FOR_FIT;

// fit range for the charge_nohit histogram
extern const int WG_BEGIN_CHARGE_NOHIT;
extern const int WG_END_CHARGE_NOHIT;

// fit range for the charge_hit_HG histogram
extern const int WG_BEGIN_CHARGE_HIT_HG;
extern const int WG_END_CHARGE_HIT_HG;
extern const int WG_BEGIN_CHARGE_HIT_LG;
extern const int WG_END_CHARGE_HIT_LG;

// average values of the peak for each peu
extern const int WG_PEAK_CHARGE_0PE;
extern const int WG_PEAK_CHARGE_1PE;
extern const int WG_PEAK_CHARGE_2PE;

// limits for the dark noise rate
extern const double WG_LOWER_LIMIT_1PE;
extern const double WG_UPPER_LIMIT_1PE;
extern const double WG_LOWER_LIMIT_2PE;
extern const double WG_UPPER_LIMIT_2PE;
extern const double WG_LOWER_LIMIT_3PE;
extern const double WG_UPPER_LIMIT_3PE;

// nominal value for the MPPC gain
extern const double WG_NOMINAL_GAIN;

// time interval in seconds of 1 BCID count
extern const double TIME_BCID;

// Maximum value of the BCID
extern const unsigned MAX_BCID_BIN;

// maximum (most right) and minimum (most left) values of the
// difference between the measured pedestal and the nominal pedestal
extern const int WG_PED_DIFF_MAX;  
extern const int WG_PED_DIFF_MIN;

// Guess the threshold value (in p.e.) given the dark noise rate
// This little function tries to guess the threshold level (0.5, 1.5 or 2.5)
// given the value of the noise rate.
double noise_to_pe(const double noise);

#endif
