#ifndef wgFit_H_INCLUDE
#define wgFit_H_INCLUDE

// system includes
#include <string>
#include <array>
#include <unordered_map>

// user includes
#include "wgGetHist.hpp"

class wgFit
{
private:
  // This member object opens the inputfile (*hist.root file) and gets the
  // histograms from it as needed.
  wgGetHist histos_;

  // directory where the images are saved
  std::string output_img_dir_;

  // Function consisting of two gaussians
  static Double_t TwinPeaks(Double_t *x, Double_t *par);

  // Sort two peaks pair<mean, height> by height.
  static bool SortPeaks(std::pair<Double_t, Double_t> peak1,
                        std::pair<Double_t, Double_t> peak2);

  // String map containing the mapping between the ROOT fit status integer code
  // and their explanation in english.
  static const std::map<int, std::string> fit_status_str_;
  
public:
  // enum to select between the various ADC histograms saved in the inputfile
  enum GainSelect {
    LowGain,   // charge_hit_LG
    HighGain,  // charge_hit_HG
    Pedestal   // charge_nohit
  };
  
  // Just call the GetHist constructor. Exceptions may be thrown. The
  // outputIMGDir is set to the environment variable WAGASCI_IMGDATADIR.
  wgFit(const std::string& inputfile);
  
  // Same as above but let the user specify the outputIMGDir
  wgFit(const std::string& inputfile, const std::string& outputIMGDir);

  // Calculate the dark noise rate for the chip "ichip" and channel "ichan". The
  // dark noise rate mean value is saved in x[0] and its variance in x[1]. If
  // print_flag is true, an image of the fitted histogram is saved in the
  // directory set in the constructor or by the SetOutputImgDir method.

  // Method that does the actual fit.
  static void NoiseRate(TH1I * bcid_hit, double (&x)[2], unsigned spill_count);

  // Wrapper around the upper method
  void NoiseRate(double (&x)[2], unsigned dif_id, unsigned ichip,
                 unsigned ichan, bool print_flag = false);

  // Calculate the charge (ADC count) peak value for chip "ichip", channel
  // "ichan" and column "icol" from the charge_hit_HG histogram.  It is assumed
  // that only one peak is present corresponding to low p.e. events (mainly dark
  // noise). The charge distribution is fitted with a 3-parameter gaussian. The
  // fit results are stored in the x vector. The charge ADC count (the gaussian
  // mean) is stored in the x[0] element, the gaussian sigma is stored in the
  // x[1] element and the least interesting parameter, the gaussian peak value
  // is store in the x[3] element. If print_flag is true, an image of the fitted
  // histogram is saved in the directory set in the constructor or by the
  // SetOutputImgDir method.

  // Method that does the actual fit.
  static void Charge(TH1I * charge, double (&x)[3],
                     GainSelect gs = GainSelect::HighGain);

  // fit the charge_hit_HG histogram
  void ChargeHitHG(double (&x)[3], unsigned dif_id, unsigned ichip,
                   unsigned ichan, int icol = -1, bool print_flag = false);
     
  // Same as above but uses the charge_hit_LG histogram (only the hits from the
  // low gain preamp are considered)
  void ChargeHitLG(double (&x)[3], unsigned dif_id, unsigned ichip,
                   unsigned ichan, int icol = -1, bool print_flag = false);

  // Same as above but uses the charge_nohit histogram
  void ChargeNohit(double (&x)[3], unsigned dif_id, unsigned ichip,
                   unsigned ichan, int icol = -1, bool print_flag = false);

  // Fit a charge ADC fingers plot with two gaussians. One gaussian fits the
  // highest peak and the other one fits the peak just to the right. A peak
  // whose height is less than 5% the height of the highest peak is ignored. The
  // gain variable is an array whose first element is the gain (calculated as
  // the difference between the peaks) and the second element is the uncertainty
  // on it calculated as Sqrt(sigma1^2 + sigma2^2) where sigma1 and sigma2 are
  // the two gaussian standard deviations. In case the fitting is not successful
  // the gain variables are set to nan.

  // Method that does the actual fit.
  static void Gain(TH1I * charge_hit, std::array<double, 2>& gain,
                   unsigned n_peaks = 2, bool do_not_fit = false);

  // If print_flag is true, an image of the fitted histogram is saved in the
  // directory set in the constructor or by the SetOutputImgDir method. If the
  // do_not_fit flag is set to true, the fitting is not done (the peaks are
  // searched only through the TSpectrum class).
  void Gain(std::array<double, 2>& gain, unsigned dif_id, unsigned ichip,
            unsigned ichan, int icol = -1, unsigned n_peaks = 2,
            bool print_flag = false, bool do_not_fit = false);
  
  // Copy the passed string into the outputIMGDir private member 
  void SetOutputImgDir(const std::string& output_image_dir);


  int GetStartTime() {return histos_.GetStartTime();};
  int GetStopTime()  {return histos_.GetStopTime();};
};
#endif
