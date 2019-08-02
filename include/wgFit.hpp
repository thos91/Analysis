#ifndef wgFit_H_INCLUDE
#define wgFit_H_INCLUDE

// system includes
#include <string>

// user includes
#include "wgGetHist.hpp"

#define DEBUG_WGFIT

class wgFit
{
private:
  std::string m_output_img_dir;

public:
  wgGetHist histos;
  
  // Just call the GetHist constructor. Exceptions may be thrown.
  wgFit(const std::string& inputfile);
  wgFit(const std::string& inputfile, const std::string& outputIMGDir);

  // Calculate the dark noise rate for the chip "ichip" and channel "ichan". The
  // dark noise rate mean value is saved in x[0] and its variance in x[1].  If
  // the mode is PRINT_HIST_MODE, an image of the fitted histogram is saved in
  // the default WAGASCI_IMGDATADIR directory.
  void noise_rate(unsigned ichip, unsigned ichan, double (&x)[2], bool print_flag = false);

  // Calculate the charge (ADC count) peak value for chip "ichip",
  // channel "ichan" and column "icol" from the charge_hit histogram.
  // It is assumed that only one peak is present corresponding to low
  // p.e. events (mainly dark noise). The charge distribution is
  // fitted with a 3-parameter gaussian. The fit results are stored in
  // the x vector. The charge ADC count (the gaussian mean) is stored
  // in the x[0] element, the gaussian sigma is stored in the x[1]
  // element and the least interesting parameter, the gaussian peak
  // value is store in the x[3] element. If the mode is
  // PRINT_HIST_MODE, an image of the fitted histogram is saved in the
  // default WAGASCI_IMGDATADIR directory.
  void charge_hit(unsigned ichip, unsigned ichan, unsigned icol, double (&x)[3], bool print_flag = false);

  // Same as above but uses the charge_hit_HG histogram (only the hits from the
  // high gain preamp are considered)
  void charge_hit_HG(unsigned ichip, unsigned ichan, unsigned icol, double (&x)[3], bool print_flag = false);

  // Same as above but uses the charge_nohit histogram
  void charge_nohit(unsigned ichip, unsigned ichan, unsigned icol, double (&x)[3], bool print_flag = false);

  // Copy the passed string into the outputIMGDir private member 
  void set_output_img_dir(const std::string& output_image_dir);
};
#endif
