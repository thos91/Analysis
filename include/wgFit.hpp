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
  wgGetHist Histos;

public:
  wgGetHist histos;

  enum gain_select {
    low_gain,
    high_gain,
    pedestal
  };
  
  // Just call the GetHist constructor. Exceptions may be thrown.
  wgFit(const std::string& inputfile);
  wgFit(const std::string& inputfile, const std::string& outputIMGDir);

  // Calculate the dark noise rate for the chip "ichip" and channel "ichan". The
  // dark noise rate mean value is saved in x[0] and its variance in x[1].  If
  // the mode is PRINT_HIST_MODE, an image of the fitted histogram is saved in
  // the default WAGASCI_IMGDATADIR directory.
  static void noise_rate(TH1I * bcid_hit, double (&x)[2]);
  void noise_rate(unsigned ichip, unsigned ichan, double (&x)[2], bool print_flag = false);

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
  static void charge_hit(TH1I * charge_hit, double (&x)[3], gain_select gs = high_gain);
  void charge_hit_HG(unsigned ichip, unsigned ichan, unsigned icol, double (&x)[3], bool print_flag = false);

  // Same as above but uses the charge_hit_LG histogram (only the hits from the
  // low gain preamp are considered)
  void charge_hit_LG(unsigned ichip, unsigned ichan, unsigned icol, double (&x)[3], bool print_flag = false);

  // Same as above but uses the charge_nohit histogram
  void ChargeNohit(unsigned ichip, unsigned ichan, unsigned icol, double (&x)[3], bool print_flag = false);

  // Copy the passed string into the outputIMGDir private member 
  void SetOutputImgDir(const std::string& output_image_dir);
};
#endif
