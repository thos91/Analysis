#include "wgFitConst.h"
const float sigma          = 8.;         //initial sigma of gaussian value
const float max_sigma      = 20.;     //max value
const float min_sigma      = 1.;     //min value

const float threshold      = 0.03;     //initial threshold value
const float max_threshold  = 0.05;   
const float min_threshold  = 0.01;    

const int N_low_pe         = 5;     //maximum number of peak search for low_pe.
const int begin_low_pe     = 10;   //begining value of fitting range for low_pe.
const int end_low_pe       = 300;   //ending value of fitting range for low_pe.

const int begin_low_pe_HG     = 350;   //begining value of fitting range for low_pe.
const int end_low_pe_HG       = 750;   //ending value of fitting range for low_pe.

const int N_pde            = 5;     //maximum number of peak search fot charge_nohit
const int begin_ped        = 350;    //begining value of fitting range for dark noise
const int end_ped          = 700;    //ending value of fitting range for dark noise

const int N_DarkNoise      = 2;     //maximum number of peak search fot dark noise
const int N_Pedestal       = 2;     //maximum number of peak search fot dark noise
const int N_LED            = 2;     //maximum number of peak search fot dark noise

const double l_limit_3pe = 50;
const double u_limit_3pe = 400;

const double l_limit_2pe = 1000;
const double u_limit_2pe = 8000;

const double l_limit_1pe =  20000;
const double u_limit_1pe = 160000;

const int begin_LED        = 400;    //begining value of fitting range for dark noise
const int end_LED          = 1000;    //ending value of fitting range for dark noise

const int begin_BCID       = 0;    //begining value of fitting range for dark noise
const int end_BCID         = 16;    //ending value of fitting range for dark noise
const int limit_BCID       = 3000;    //limit value of fitting range for dark noise

const float Gain           = 25;     //estimate Gain

const float GSthreshold    = 2510;     //estimate Gain Select Threshold
const float GSthreshold_0  = 2730;     //estimate Gain Select Threshold for column 0.

const float time_bcid_bin = 580e-9;
