#ifndef WGFITCONST_H_INCLUDE
#define WGFITCONST_H_INCLUDE


extern const double sigma;         //initial sigma of gaussian value
extern const double max_sigma;     //max value
extern const double min_sigma;     //min value

extern const double threshold;     //initial threshold value
extern const double max_threshold;   
extern const double min_threshold;    

extern const int begin_pe; //begining value of fitting range for low pe
extern const int end_pe;   //ending value of fitting range for low pe

extern const int begin_pe_HG; //begining value of fitting range for low pe
extern const int end_pe_HG;   //ending value of fitting range for low pe

extern const int N_ped ;       //maximum number of peak search for pedestal
extern const int begin_ped;    //begining value of fitting range for pedestal
extern const int end_ped;    //ending value of fitting range for pedestal

extern const int N_DarkNoise ;     //maximum number of peak search for dark noise
extern const int N_Pedestal ;     //maximum number of peak search for pedestal
extern const int N_LED ;     //maximum number of peak search for LED

extern const double l_limit_1pe;
extern const double u_limit_1pe;
extern const double l_limit_2pe;
extern const double u_limit_2pe;
extern const double l_limit_3pe;
extern const double u_limit_3pe;

extern const int begin_LED;    //begining value of fitting range for LED
extern const int end_LED;    //ending value of fitting range for LED

extern const int begin_BCID;    //begining value of fitting range for LED
extern const int end_BCID;    //ending value of fitting range for LED
extern const int limit_BCID;    //ending value of fitting range for LED

extern const double est_Gain ;     //estimate Gain

extern const double GSthreshold ;     //estimate Gain
extern const double GSthreshold_0 ;     //estimate Gain

extern const double time_bcid_bin ;   // bcid 1bin time[s]

extern const int ped_diff_max;  // maximum (most right) value of the difference
							    // between the measured pedestal and the nominal
							    // pedestal
extern const int ped_diff_min;  // minimum (most left) value

// Guess the threshold value (in p.e.) given the dark noise rate
// This little function tries to guess the threshold level (0.5, 1.5 or 2.5)
// given the value of the noise rate.
double NoiseToPe(const double noise);

#endif
