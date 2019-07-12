#ifndef wgFit_H_INCLUDE
#define wgFit_H_INCLUDE

// system includes
#include <string>
#include <vector>

// ROOT includes
#include "TROOT.h"
#include "TGraphErrors.h"

// user includes
#include "wgConst.hpp"
#include "wgFileSystemTools.hpp"
#include "wgGetHist.hpp"

#define DEBUG_WGFIT

using namespace std;

class wgFit
{
private:
  string outputIMGDir;

public:
  wgGetHist *GetHist;
  
  // wgFit::wgFit
  // Just call the GetHist constructor. Exceptions may be thrown.
  wgFit(const string& inputfile);
  wgFit(const string& inputfile, const string& outputIMGDir);
	wgFit(TGraphErrors* Scurve);
  // delete wgFit::GetHist;
  ~wgFit();
  
  void swap(int,double*,double*);
  // wgFit::NoiseRate
  // Calculate the dark noise rate for the chip "ichip" and channel "ichan". The
  // dark noise rate mean value is saved in x[0] and its variance in x[1].  If
  // the mode is PRINT_HIST_MODE, an image of the fitted histogram is saved in
  // the default WAGASCI_IMGDATADIR directory.
  void noise_rate(unsigned ichip, unsigned ichan, double (&x)[2], bool print_flag = false);

  // wgFit::low_pe_charge
  //
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
  // default WAGASCI_IMGDATADIR directory.  The lower limit of the
  // gaussian peak is 0.9 times the maximum bin height. The upper
  // limit is 1.1 times the maximum bin height. The mean value is
  // limited to the value of the maximum bin (its x) +/- three times
  // the max_sigma variable.
  void charge_hit(unsigned ichip, unsigned ichan, unsigned icol, double (&x)[3], bool print_flag = false);

  // wgFit::low_pe_charge
  // Same as above but uses the charge_hit_HG histogram (only the hits from the
  // high gain preamp are considered)
  void charge_hit_HG(unsigned ichip, unsigned ichan, unsigned icol, double (&x)[3], bool print_flag = false);

  // wgFit::charge_nohit
  // Calculate the pedestal value for chip "ichip", channel "ichan" and column
  // "icol". The pedestal distribution is fitted with a 3-parameter
  // gaussian. The fit results are stored in the x vector. The pedestal ADC
  // count (the gaussian mean) is stored in the x[0] element, the gaussian sigma
  // is stored in the x[1] element and the least interesting parameter, the
  // gaussian peak value is store in the x[3] element. If the mode is
  // PRINT_HIST_MODE, an image of the fitted histogram is saved in the default
  // WAGASCI_IMGDATADIR directory.
  void charge_nohit(unsigned ichip, unsigned ichan, unsigned icol, double (&x)[3], bool print_flag = false);

  // Copy the passed string into the outputIMGDir private member 
  void SetoutputIMGDir(const string&);

  // wgFit::scurve
  // Fit the noise rate s-curve for each inputDAC, chip "ichip" and channel "ichan".
	void scurve(TGraphErrors* Scurve, 
							double& pe1_t, 
							double& pe2_t, 
							unsigned idif_id, 
							unsigned ichip_id, 
							unsigned ichan_id, 
							unsigned inputDAC,
							string outputIMGDir, 
							bool print_flag = false);
};
#endif
