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
void wgFit::noise_rate(TH1I * bcid_hit, double (&x)[2]) {
  // Find the right-most bin that is non-zero.
  Int_t last_bin = bcid_hit->FindLastBinAbove(0,1);
  if (last_bin <= 0) {
    x[0] = x[1] = NAN;
    return;
  }

  // Number of recorded spills
  Int_t spill_count = wgFit::histos.spill_count;
  if (spill_count <= 0) {
    x[0] = x[1] = NAN;
    return;
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
  bcid_hit->SetDirectory(0); 

  this->noise_rate(bcid_hit, x);

  if ( print_flag ) {
    TString image;
    image.Form("%s/chip%u/chan%u/NoiseRate%u_%u.png",
               m_output_img_dir.c_str(), ichip, ichan, ichip, ichan);
    wgFit::histos.Print_bcid(image, bcid_hit);
  }
  delete bcid_hit;
}

void wgFit::charge_hit(TH1I * charge_hit, double (&x)[3], wgFit::gain_select gs) {
  // Find the right-most bin that is non-zero.
  Int_t last_bin = charge_hit->FindLastBinAbove(0,1);
  if (last_bin <= 0) {
    x[0] = x[1] = x[2] = NAN;
    //throw wgElementNotFound("CHARGE histogram has no entries : chip = " +
    //                        std::to_string(ichip) + ", chan = " +
    //                        std::to_string(ichan));
    return;
  }

  unsigned begin, end;
  if (gs == wgFit::gain_select::high_gain) {
    begin = WG_BEGIN_CHARGE_HIT_HG;
    end = WG_END_CHARGE_HIT_HG;
  } else if (gs == wgFit::gain_select::low_gain) {
    begin = WG_BEGIN_CHARGE_HIT_LG;
    end = WG_END_CHARGE_HIT_LG;
  } else {
    begin = WG_BEGIN_CHARGE_NOHIT;
    end = WG_END_CHARGE_NOHIT;
  }
  
  charge_hit->GetXaxis()->SetRange(begin, end);
  TF1* gaussian = new TF1("gaussian", "gaus", begin, end);
  gaussian->SetLineColor(kGreen);

  // Fit histogram with gaussian function.
  // "Q": quiet mode (minimum printing)
  // "P" : drawing option
  charge_hit->Fit(gaussian, "Q", "P", begin, end);
  x[0] = gaussian->GetParameter(1); // mean_fit
  x[1] = gaussian->GetParameter(2); // sigma_fit
  x[2] = gaussian->GetParameter(0); // peak_fit
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

  this->charge_hit(charge_hit_HG, x, wgFit::gain_select::high_gain);

  if( print_flag && (!m_output_img_dir.empty()) ) {
    TString image;
    image.Form("%s/chip%u/chan%u/charge_hit_HG%u_%u_%u.png",
               m_output_img_dir.c_str(), ichip, ichan, ichip, ichan, icol);
    wgFit::histos.Print_charge_hit_HG(image, charge_hit_HG);
  }
  delete charge_hit_HG;
}

//**********************************************************************
void wgFit::charge_hit_LG(unsigned ichip, unsigned ichan, unsigned icol,
                       double (&x)[3], bool print_flag) {
  TH1I * charge_hit_LG = wgFit::histos.Get_charge_hit_LG(ichip, ichan, icol);
  if (charge_hit_LG == NULL) {
    x[0] = x[1] = x[2] = NAN;
    throw wgElementNotFound("charge_hit_LG histogram not found : chip = " +
                            std::to_string(ichip) + ", chan = " +
                            std::to_string(ichan));
  }
  charge_hit_LG->SetDirectory(0); 

  this->charge_hit(charge_hit_LG, x, wgFit::gain_select::low_gain);

  if( print_flag && (!m_output_img_dir.empty()) ) {
    TString image;
    image.Form("%s/chip%u/chan%u/charge_hit_LG%u_%u_%u.png",
               m_output_img_dir.c_str(), ichip, ichan, ichip, ichan, icol);
    wgFit::histos.Print_charge_hit_LG(image, charge_hit_LG);
  }
  delete charge_hit_LG;
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

  this->charge_hit(charge_nohit, x, wgFit::gain_select::pedestal);

  if( print_flag && (!m_output_img_dir.empty()) ) {
    TString image;
    image.Form("%s/chip%u/chan%u/charge_nohit%d_%d_%d.png",
               m_output_img_dir.c_str(), ichip, ichan, ichip, ichan, icol);
    wgFit::histos.Print_charge_nohit(image, charge_nohit);
  }
  delete charge_nohit;
}
