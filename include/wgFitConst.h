#ifndef WGFITCONST_H_INCLUDE
#define WGFITCONST_H_INCLUDE


extern const float sigma;         //initial sigma of gaussian value
extern const float max_sigma;     //max value
extern const float min_sigma;     //min value

extern const float threshold;     //initial threshold value
extern const float max_threshold;   
extern const float min_threshold;    

extern const int N_low_pe;     //maximum number of peak search for low pe
extern const int begin_low_pe; //begining value of fitting range for low pe
extern const int end_low_pe;   //ending value of fitting range for low pe
extern const int begin_low_pe_HG; //begining value of fitting range for low pe
extern const int end_low_pe_HG;   //ending value of fitting range for low pe

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

extern const float Gain ;     //estimate Gain

extern const float GSthreshold ;     //estimate Gain
extern const float GSthreshold_0 ;     //estimate Gain

extern const float time_bcid_bin ;   // bcid 1bin time[s]





#endif
