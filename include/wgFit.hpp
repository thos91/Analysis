#ifndef wgFit_H_INCLUDE
#define wgFit_H_INCLUDE

// system includes
#include <string>

// user includes
#include "wgGetHist.hpp"

class wgFit
{
private:
  std::string m_output_img_dir;
  static Double_t TwinPeaks(Double_t *x, Double_t *par);
  static bool SortPeaks(std::pair<Double_t, Double_t> peak1, std::pair<Double_t, Double_t> peak2);
  
public:
  wgGetHist histos_;

  enum GainSelect {
    LOW_GAIN,
    HIGH_GAIN,
    PEDESTAL
  };
  
  // Just call the GetHist constructor. Exceptions may be thrown.
  wgFit(const std::string& inputfile);
  wgFit(const std::string& inputfile, const std::string& outputIMGDir);

  // Calculate the dark noise rate for the chip "ichip" and channel "ichan". The
  // dark noise rate mean value is saved in x[0] and its variance in x[1].  If
  // the mode is PRINT_HIST_MODE, an image of the fitted histogram is saved in
  // the default WAGASCI_IMGDATADIR directory.
  static void NoiseRate(TH1I * bcid_hit, double (&x)[2], unsigned spill_count);
  void NoiseRate(unsigned ichip, unsigned ichan, double (&x)[2], bool print_flag = false);

  // Calculate the charge (ADC count) peak value for chip "ichip",
  // channel "ichan" and column "icol" from the charge_hit_HG histogram.
  // It is assumed that only one peak is present corresponding to low
  // p.e. events (mainly dark noise). The charge distribution is
  // fitted with a 3-parameter gaussian. The fit results are stored in
  // the x vector. The charge ADC count (the gaussian mean) is stored
  // in the x[0] element, the gaussian sigma is stored in the x[1]
  // element and the least interesting parameter, the gaussian peak
  // value is store in the x[3] element. If the mode is
  // PRINT_HIST_MODE, an image of the fitted histogram is saved in the
  // default WAGASCI_IMGDATADIR directory.
  static void ChargeHit(TH1I * charge_hit, double (&x)[3], GainSelect gs = GainSelect::HIGH_GAIN);
  void ChargeHitHG(unsigned ichip, unsigned ichan, unsigned icol, double (&x)[3], bool print_flag = false);

  // Fit a charge ADC fingers plot with two gaussians. One gaussian
  // fits the highest peak and the other one fits the peak just to the
  // right. A peak whose height is less than 5% the height of the
  // highest peak is ignored. The gain variable is an array whose
  // first element is the gain (calculated as the difference between
  // the peaks) and the sencod element is the error on it.  In case
  // the fitting is not successful the gain variable is set to NAN.
  static void Gain(TH1I * charge_hit, double (&gain)[2]);
  void Gain(unsigned ichip, unsigned ichan, unsigned icol, double (&x)[2], bool print_flag);
      
  // Same as above but uses the charge_hit_LG histogram (only the hits from the
  // low gain preamp are considered)
  void ChargeHitLG(unsigned ichip, unsigned ichan, unsigned icol, double (&x)[3], bool print_flag = false);

  // Same as above but uses the charge_nohit histogram
  void ChargeNohit(unsigned ichip, unsigned ichan, unsigned icol, double (&x)[3], bool print_flag = false);

  // Copy the passed string into the outputIMGDir private member 
  void SetOutputImgDir(const std::string& output_image_dir);
};
#endif
