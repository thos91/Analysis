// system includes
#include <vector>
#include <string>

// boost includes
#include <boost/filesystem.hpp>

// ROOT includes
#include <TF1.h>
#include <TGraphErrors.h>
#include <TString.h>

// user includes
#include "wgConst.hpp"
#include "wgFileSystemTools.hpp"
#include "wgExceptions.hpp"
#include "wgGetHist.hpp"
#include "wgFitConst.hpp"
#include "wgLogger.hpp"
#include "wgFit.hpp"

using namespace wagasci_tools;

Double_t gauss(Double_t *x, Double_t *p){
  return p[0]*exp(-(((x[0]-p[1])/p[2]))*((x[0]-p[1])/p[2])*0.50);
}//pure gaussian

Double_t gauss_linear(Double_t *x, Double_t *p){
  return p[0]*exp(-(((x[0]-p[1])/p[2]))*((x[0]-p[1])/p[2])*0.50)+p[3]*x[0]+p[4];
}
//gaussian+background(ax+b)

Double_t double_gauss(Double_t *x, Double_t *p){
  return p[0]*exp(-(((x[0]-p[1])/p[2]))*((x[0]-p[1])/p[2])*0.50)+p[3]*exp(-(((x[0]-p[1]-p[4])/p[2]))*((x[0]-p[1]-p[4])/p[2])*0.50);
}//double gaussian(same distortion)

Double_t double_gauss_2(Double_t *x, Double_t *p){
  return p[0]*exp(-(((x[0]-p[1])/p[2]))*((x[0]-p[1])/p[2])*0.50)+p[3]*exp(-(((x[0]-p[1]-p[4])/p[5]))*((x[0]-p[1]-p[4])/p[5])*0.50);
}//double gaussian(different distortion)

//**********************************************************************
wgFit::wgFit(const std::string& x_inputfile, const std::string& x_output_img_dir) {
  wgFit::GetHist = new wgGetHist(x_inputfile);
  wgFit::set_output_img_dir(x_output_img_dir);
}

//**********************************************************************
wgFit::wgFit(const std::string& x_inputfile) {
  wgFit::GetHist = new wgGetHist(x_inputfile);
  wgEnvironment env;
  wgFit::set_output_img_dir(env.IMGDATA_DIRECTORY);
}

//**********************************************************************
wgFit::~wgFit(){
  delete wgFit::GetHist;
}

//**********************************************************************
void wgFit::set_output_img_dir(const std::string& x_output_img_dir) {
    if( !check_exist::Dir(x_output_img_dir) ) {
    boost::filesystem::path dir(x_output_img_dir);
    if( !boost::filesystem::create_directories(dir) ) {
      throw wgInvalidFile("failed to create directory : " + x_output_img_dir);
    }
  }
  m_output_img_dir = x_output_img_dir;
}

//**********************************************************************
void wgFit::noise_rate(unsigned ichip, unsigned ichan, double (&x)[2], bool print_flag) {
  TH1I * bcid_hit = GetHist->Get_bcid_hit(ichip, ichan);
  if (bcid_hit == NULL) {
    x[0] = x[1] = NAN;
    throw wgElementNotFound("BCID histogram not found : chip = " +
                            std::to_string(ichip) + ", chan = " +
                            std::to_string(ichan));
  }

  // Find the right-most bin that is non-zero.
  Int_t last_bin = bcid_hit->FindLastBinAbove(0,1);
  if (last_bin <= 0) {
    x[0] = x[1] = NAN;
    throw wgElementNotFound("BCID histogram has no entries : chip = " +
                            std::to_string(ichip) + ", chan = " +
                            std::to_string(ichan));
  }
  // Number of recorded spills
  Int_t spill_count = GetHist->spill_count;
  if (spill_count <= 0) {
    x[0] = x[1] = NAN;
    throw wgElementNotFound("spill count is zero or negative : chip = " +
                            std::to_string(ichip) + ", chan = " +
                            std::to_string(ichan));
  }

  // Integral() is the sum of all the hits between BCID = 0 and a BCID
  // roughly in the middle of the histogram. The principle here is
  // that we want to roughly select only half of the columns (8
  // columns out of 16) to avoid finite memory artifacts.
  Double_t time_interval = 0.5 * last_bin;
  Double_t sigma = bcid_hit->Integral(0, time_interval);
  bcid_hit->GetXaxis()->SetRange(0, 2 * time_interval + 10);

  // in Hertz
  x[0] = sigma / ((time_interval * spill_count - sigma) * time_bcid_bin);
  x[1] = ((time_interval * spill_count - sigma) * sqrt(sigma)) /
         (pow(time_interval, 2) * pow(spill_count, 2) * time_bcid_bin);
  
  if ( print_flag ) {
    TString image;
    image.Form("%s/chip%u/chan%u/NoiseRate%u_%u.png",
               m_output_img_dir.c_str(), ichip, ichan, ichip, ichan);
    GetHist->Print_bcid(image, bcid_hit);
  }
}

//**********************************************************************
void wgFit::charge_hit(unsigned ichip, unsigned ichan, unsigned icol,
                       double (&x)[3], bool print_flag) {
  TH1I * charge_hit = GetHist->Get_charge_hit(ichip, ichan, icol);
  if (charge_hit == NULL) {
    x[0] = x[1] = x[2] = NAN;
    throw wgElementNotFound("charge_hit histogram not found : chip = " +
                            std::to_string(ichip) + ", chan = " +
                            std::to_string(ichan));
  }

  // Find the right-most bin that is non-zero.
  Int_t last_bin = charge_hit->FindLastBinAbove(0,1);
  if (last_bin <= 0) {
    x[0] = x[1] = x[2] = NAN;
    throw wgElementNotFound("CHARGE histogram has no entries : chip = " +
                            std::to_string(ichip) + ", chan = " +
                            std::to_string(ichan));
  }
 
  // begin_pe and end_pe are defined in wgFitConst.cpp
  charge_hit->GetXaxis()->SetRange(begin_pe,end_pe);

  double mean={(double)charge_hit->GetMaximumBin()};
  double peak={(double)charge_hit->GetMaximum()};

  TF1* gaussian = new TF1("gauss", gauss, mean-3*sigma, mean+3*sigma, 3);
  gaussian->SetLineColor(kGreen);
  gaussian->SetNpx(500);
  gaussian->SetParameters(peak, mean, sigma);
  gaussian->SetParNames("peak_fit", "mean_fit", "sigma_fit");
  gaussian->SetParLimits(0, 0.9 * peak, 1.1 * peak); // peak_fit
  gaussian->SetParLimits(1, mean - max_sigma, mean + max_sigma); // mean_fit
  gaussian->SetParLimits(2, min_sigma, max_sigma); // sigma_fit 

  // Fit histogram with gaussian function.
  // "Q": quiet mode (minimum printing)
  // "P" : drawing option
  // (mean - 3 * sigma, mean + 3 * sigma) : range over which to apply the fit.
  charge_hit->Fit(gaussian, "Q", "P", mean - 3 * sigma, mean + 3 * sigma);
  x[0] = gaussian->GetParameter(1); // mean_fit
  x[1] = gaussian->GetParameter(2); // sigma_fit
  x[2] = gaussian->GetParameter(0); // peak_fit

  if( print_flag && (!m_output_img_dir.empty()) ) {
    TString image;
    image.Form("%s/chip%u/chan%u/charge_hit%u_%u_%u.png",
               m_output_img_dir.c_str(), ichip, ichan, ichip, ichan, icol);
    GetHist->Print_charge(image, charge_hit);
  }
  delete gaussian;
}


//**********************************************************************
void wgFit::charge_hit_HG(unsigned ichip, unsigned ichan, unsigned icol,
                          double (&x)[3], bool print_flag) {
  TH1I * charge_hit_HG = GetHist->Get_charge_hit_HG(ichip, ichan, icol);
  if (charge_hit_HG == NULL) {
    x[0] = x[1] = x[2] = NAN;
    throw wgElementNotFound("charge_hit_HG histogram not found : chip = " +
                            std::to_string(ichip) + ", chan = " +
                            std::to_string(ichan));
  }

  // Find the right-most bin that is non-zero.
  Int_t last_bin = charge_hit_HG->FindLastBinAbove(0,1);
  if (last_bin <= 0) {
    x[0] = x[1] = x[2] = NAN;
    throw wgElementNotFound("CHARGE histogram has no entries : chip = " +
                            std::to_string(ichip) + ", chan = " +
                            std::to_string(ichan));
  }

  // begin_pe_HG and end_pe_HG are defined in wgFitConst.cpp
  charge_hit_HG->GetXaxis()->SetRange(begin_pe_HG,end_pe_HG);
  double mean = (double) charge_hit_HG->GetMaximumBin();
  double peak = (double) charge_hit_HG->GetMaximum();

  TF1* gaussian = new TF1("gauss", gauss, mean-3*sigma, mean+3*sigma, 3);
  gaussian->SetLineColor(kGreen);
  gaussian->SetNpx(500);
  gaussian->SetParameters(peak, mean, sigma);
  gaussian->SetParNames("peak_fit", "mean_fit", "sigma_fit");
  gaussian->SetParLimits(0, 0.9 * peak, 1.1 * peak); // peak_fit
  gaussian->SetParLimits(1, mean - max_sigma, mean + max_sigma); // mean_fit
  gaussian->SetParLimits(2, min_sigma, max_sigma); // sigma_fit 

  // Fit histogram with gaussian function.
  // "Q": quiet mode (minimum printing)
  // "P" : drawing option
  // (mean - 3 * sigma, mean + 3 * sigma) : range over which to apply the fit.
  charge_hit_HG->Fit(gaussian, "Q", "P", mean - 3 * sigma, mean + 3 * sigma);
  x[0] = gaussian->GetParameter(1); // mean_fit
  x[1] = gaussian->GetParameter(2); // sigma_fit
  x[2] = gaussian->GetParameter(0); // peak_fit

  if( print_flag && (!m_output_img_dir.empty()) ) {
    TString image;
    image.Form("%s/chip%u/chan%u/HG%u_%u_%u.png",
               m_output_img_dir.c_str(), ichip, ichan, ichip, ichan, icol);
    GetHist->Print_charge_hit_HG(image, charge_hit_HG);
  }
  delete gaussian;
}

//**********************************************************************
void wgFit::charge_nohit(unsigned ichip, unsigned ichan, unsigned icol,
                         double (&x)[3], bool print_flag) {
  TH1I * charge_nohit = GetHist->Get_charge_nohit(ichip, ichan, icol);
  if (charge_nohit == NULL) {
    x[0] = x[1] = x[2] = NAN;
    throw wgElementNotFound("charge_nohit histogram not found : chip = " +
                            std::to_string(ichip) + ", chan = " +
                            std::to_string(ichan));
  }

  // Find the right-most bin that is non-zero.
  Int_t last_bin = charge_nohit->FindLastBinAbove(0,1);
  if (last_bin <= 0) {
    x[0] = x[1] = x[2] = NAN;
    throw wgElementNotFound("CHARGE histogram has no entries : chip = " +
                            std::to_string(ichip) + ", chan = " +
                            std::to_string(ichan));
  }

  // Set the histogram x axis range to 350 --- 700
  // (the pedestal lies usually in that range)
  charge_nohit->GetXaxis()->SetRange(begin_ped, end_ped);
  
  double mean = (double) charge_nohit->GetMaximumBin();
  double peak = (double) charge_nohit->GetMaximum();
  // Gaussian function to use when fitting
  // Arguments:
  // (function name, function pointer, xmin, xmax, number of parameters)
  // sigma is defined in wgFitConst.cpp
  TF1* gaussian = new TF1("gauss", gauss, mean-3*sigma, mean+3*sigma, 3);
  gaussian->SetLineColor(kViolet);
  gaussian->SetNpx(500);
  gaussian->SetParameters(peak, mean, sigma);
  gaussian->SetParNames("peak_fit", "mean_fit", "sigma_fit");
  gaussian->SetParLimits(0, 0.9 * peak, 1.1 * peak); // peak_fit
  gaussian->SetParLimits(1, mean - max_sigma, mean + max_sigma); // mean_fit
  gaussian->SetParLimits(2, min_sigma, max_sigma); // sigma_fit

  // Fit histogram with gaussian function.
  // "Q": quiet mode (minimum printing)
  // "same" : when a fit is executed, the image of the function is drawn on
  // the current pad.
  // (mean - 3 * sigma, mean + 3 * sigma) : range over which to apply the fit.
  charge_nohit->Fit(gaussian, "Q", "same", mean-3*sigma, mean+3*sigma);
  x[0]=gaussian->GetParameter(1); // mean_fit
  x[1]=gaussian->GetParameter(2); // sigma_fit
  x[2]=gaussian->GetParameter(0); // peak_fit
    
  if( print_flag && (!m_output_img_dir.empty()) ) {
    TString image;
    image.Form("%s/chip%u/chan%u/nohit%d_%d_%d.png",
               m_output_img_dir.c_str(), ichip, ichan, ichip, ichan, icol);
    GetHist->Print_charge_nohit(image, charge_nohit);
  }
  
  delete gaussian;
}
