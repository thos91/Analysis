// system includes
#include <vector>
#include <string>

// ROOT includes
#include <TF1.h>
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

//**********************************************************************
wgFit::wgFit(const std::string& x_inputfile, const std::string& x_output_img_dir) :
    histos(x_inputfile) {
  wgFit::set_output_img_dir(x_output_img_dir);
}

//**********************************************************************
wgFit::wgFit(const std::string& x_inputfile) :
    histos(x_inputfile) {
  wgEnvironment env;
  wgFit::set_output_img_dir(env.IMGDATA_DIRECTORY);
}

//**********************************************************************
void wgFit::set_output_img_dir(const std::string& x_output_img_dir) {
  if( !check_exist::Dir(x_output_img_dir) ) {
    MakeDir(x_output_img_dir);
  }
  m_output_img_dir = x_output_img_dir;
}

//**********************************************************************
void wgFit::noise_rate(unsigned ichip, unsigned ichan, double (&x)[2], bool print_flag) {
  TH1I * bcid_hit = wgFit::histos.Get_bcid_hit(ichip, ichan);
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
    //throw wgElementNotFound("BCID histogram has no entries : chip = " +
    //                        std::to_string(ichip) + ", chan = " +
    //                        std::to_string(ichan));
  	return;
	}
  // Number of recorded spills
  Int_t spill_count = wgFit::histos.spill_count;
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
  x[0] = sigma / ((time_interval * spill_count - sigma) * TIME_BCID);
  x[1] = ((time_interval * spill_count - sigma) * sqrt(sigma)) /
         (pow(time_interval, 2) * pow(spill_count, 2) * TIME_BCID);
  
  if ( print_flag ) {
    TString image;
    image.Form("%s/chip%u/chan%u/NoiseRate%u_%u.png",
               m_output_img_dir.c_str(), ichip, ichan, ichip, ichan);
    wgFit::histos.Print_bcid(image, bcid_hit);
  }
}

//**********************************************************************
void wgFit::charge_hit(unsigned ichip, unsigned ichan, unsigned icol,
                       double (&x)[3], bool print_flag) {
  TH1I * charge_hit = wgFit::histos.Get_charge_hit(ichip, ichan, icol);
  if (charge_hit == NULL) {
    x[0] = x[1] = x[2] = NAN;
    throw wgElementNotFound("charge_hit histogram not found : chip = " +
                            std::to_string(ichip) + ", chan = " +
                            std::to_string(ichan));
  }
  charge_hit->SetDirectory(0); 

  // Find the right-most bin that is non-zero.
  Int_t last_bin = charge_hit->FindLastBinAbove(0,1);
  if (last_bin <= 0) {
    x[0] = x[1] = x[2] = NAN;
    //throw wgElementNotFound("CHARGE histogram has no entries : chip = " +
    //                        std::to_string(ichip) + ", chan = " +
    //                        std::to_string(ichan));
  	return;
  }
 
  charge_hit->GetXaxis()->SetRange(WG_BEGIN_CHARGE_HIT, WG_END_CHARGE_HIT);
  TF1* gaussian = new TF1("gaussian", "gaus", WG_BEGIN_CHARGE_HIT, WG_END_CHARGE_HIT);
  gaussian->SetLineColor(kGreen);

  // Fit histogram with gaussian function.
  // "Q": quiet mode (minimum printing)
  // "P" : drawing option
  charge_hit->Fit(gaussian, "Q", "P", WG_BEGIN_CHARGE_HIT, WG_END_CHARGE_HIT);
  x[0] = gaussian->GetParameter(1); // mean_fit
  x[1] = gaussian->GetParameter(2); // sigma_fit
  x[2] = gaussian->GetParameter(0); // peak_fit

  if( print_flag && (!m_output_img_dir.empty()) ) {
    TString image;
    image.Form("%s/chip%u/chan%u/charge_hit%u_%u_%u.png",
               m_output_img_dir.c_str(), ichip, ichan, ichip, ichan, icol);
    wgFit::histos.Print_charge(image, charge_hit);
  }
  delete charge_hit;
  delete gaussian;
}


//**********************************************************************
void wgFit::charge_hit_HG(unsigned ichip, unsigned ichan, unsigned icol,
                          double (&x)[3], bool print_flag) {
  TH1I * charge_hit_HG = wgFit::histos.Get_charge_hit_HG(ichip, ichan, icol);
  if (charge_hit_HG == NULL) {
    x[0] = x[1] = x[2] = NAN;
    throw wgElementNotFound("charge_hit_HG histogram not found : chip = " +
                            std::to_string(ichip) + ", chan = " +
                            std::to_string(ichan));
  }
  charge_hit_HG->SetDirectory(0);
  
  // Find the right-most bin that is non-zero.
  Int_t last_bin = charge_hit_HG->FindLastBinAbove(0,1);
  if (last_bin <= 0) {
    x[0] = x[1] = x[2] = NAN;
    //throw wgElementNotFound("CHARGE histogram has no entries : chip = " +
    //                        std::to_string(ichip) + ", chan = " +
    //                        std::to_string(ichan));
  	return;
  }

  charge_hit_HG->GetXaxis()->SetRange(WG_BEGIN_CHARGE_HIT_HG, WG_END_CHARGE_HIT_HG);
  TF1* gaussian = new TF1("gaussian", "gaus", WG_BEGIN_CHARGE_HIT_HG, WG_END_CHARGE_HIT_HG);
  gaussian->SetLineColor(kRed);

  // Fit histogram with gaussian function.
  // "Q": quiet mode (minimum printing)
  // "P" : drawing option
  charge_hit_HG->Fit(gaussian, "Q", "P", WG_BEGIN_CHARGE_HIT_HG, WG_END_CHARGE_HIT_HG);
  x[0] = gaussian->GetParameter(1); // mean_fit
  x[1] = gaussian->GetParameter(2); // sigma_fit
  x[2] = gaussian->GetParameter(0); // peak_fit

  if( print_flag && (!m_output_img_dir.empty()) ) {
    TString image;
    image.Form("%s/chip%u/chan%u/HG%u_%u_%u.png",
               m_output_img_dir.c_str(), ichip, ichan, ichip, ichan, icol);
    wgFit::histos.Print_charge_hit_HG(image, charge_hit_HG);
  }
  delete charge_hit_HG;
  delete gaussian;
}

//**********************************************************************
void wgFit::charge_nohit(unsigned ichip, unsigned ichan, unsigned icol,
                         double (&x)[3], bool print_flag) {
  TH1I * charge_nohit = wgFit::histos.Get_charge_nohit(ichip, ichan, icol);
  if (charge_nohit == NULL) {
    x[0] = x[1] = x[2] = NAN;
    throw wgElementNotFound("charge_nohit histogram not found : chip = " +
                            std::to_string(ichip) + ", chan = " +
                            std::to_string(ichan));
  }
  charge_nohit->SetDirectory(0);
  
  // Find the right-most bin that is non-zero.
  Int_t last_bin = charge_nohit->FindLastBinAbove(0,1);
  if (last_bin <= 0) {
    x[0] = x[1] = x[2] = NAN;
    //throw wgElementNotFound("CHARGE histogram has no entries : chip = " +
    //                        std::to_string(ichip) + ", chan = " +
    //                        std::to_string(ichan));
  	return;
  }

  charge_nohit->GetXaxis()->SetRange(WG_BEGIN_CHARGE_NOHIT, WG_END_CHARGE_NOHIT);
  TF1* gaussian = new TF1("gauss", "gaus", WG_BEGIN_CHARGE_NOHIT, WG_END_CHARGE_NOHIT);
  gaussian->SetLineColor(kViolet);

  // Fit histogram with gaussian function.
  // "Q": quiet mode (minimum printing)
  // "P" : drawing option
  charge_nohit->Fit(gaussian, "Q", "P", WG_BEGIN_CHARGE_NOHIT, WG_END_CHARGE_NOHIT);
  x[0] = gaussian->GetParameter(1); // mean_fit
  x[1] = gaussian->GetParameter(2); // sigma_fit
  x[2] = gaussian->GetParameter(0); // peak_fit
    
  if( print_flag && (!m_output_img_dir.empty()) ) {
    TString image;
    image.Form("%s/chip%u/chan%u/nohit%d_%d_%d.png",
               m_output_img_dir.c_str(), ichip, ichan, ichip, ichan, icol);
    wgFit::histos.Print_charge_nohit(image, charge_nohit);
  }
  delete charge_nohit;
  delete gaussian;
}
