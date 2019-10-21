// system includes
#include <vector>
#include <string>
#include <cmath>

// ROOT includes
#include <TF1.h>
#include <TString.h>
#include <TSpectrum.h>
#include <TMath.h>

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
void wgFit::noise_rate(TH1I * bcid_hit, double (&x)[2], unsigned spill_count) {
  // Find the right-most bin that is non-zero.
  Int_t last_bin = bcid_hit->FindLastBinAbove(0,1);
  if (last_bin <= 0) {
    x[0] = x[1] = std::nan("noise");
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
    x[0] = x[1] = std::nan("");
    throw wgElementNotFound("BCID histogram not found : chip = " +
                            std::to_string(ichip) + ", chan = " +
                            std::to_string(ichan));
  }
  bcid_hit->SetDirectory(0);

  // Number of recorded spills
  unsigned spill_count = wgFit::histos.spill_count;
  if (spill_count <= 0) {
    x[0] = x[1] = std::nan("noise");
    return;
  }

  this->noise_rate(bcid_hit, x, spill_count);

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
    x[0] = x[1] = x[2] = std::nan("charge");
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
    x[0] = x[1] = x[2] = std::nan("charge");
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
    x[0] = x[1] = x[2] = std::nan("charge");
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
    x[0] = x[1] = x[2] = std::nan("charge");
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

//**********************************************************************
Double_t wgFit::twin_peaks(Double_t *x, Double_t *par) {
   Double_t result = 0;
   for (Int_t p = 0; p < 2; ++p) {
      Double_t norm  = par[3 * p];
      Double_t mean  = par[3 * p + 1];
      Double_t sigma = par[3 * p + 2];
      result += norm * TMath::Gaus(x[0], mean, sigma);
   }
   return result;
}

bool wgFit::sort_peaks(std::pair<Double_t, Double_t> peak1,
                       std::pair<Double_t, Double_t> peak2) {
   return peak1.first > peak2.first;
}

//**********************************************************************
void wgFit::gain(TH1I * charge_hit, double (&gain)[2]) {
  TSpectrum *fingers_plot = new TSpectrum(4); // Maximum number of peaks is 4
  Int_t n_peaks = fingers_plot->Search(charge_hit, 2, "nobackground", 0.05);
  if (n_peaks < 2) {
    gain[0] = gain[1] = std::nan("gain");
    return;
  }
  Double_t par[6];
  Double_t *xpeaks = fingers_plot->GetPositionX();

  std::vector<std::pair<Double_t, Double_t>> peaks(n_peaks);
  for (Int_t peak = 0; peak < n_peaks; ++peak) {
    Double_t xp = xpeaks[peak];
    Int_t bin = charge_hit->GetXaxis()->FindBin(xp);
    Double_t yp = charge_hit->GetBinContent(bin);
    peaks.push_back(std::pair<Double_t, Double_t>(xp, yp));
  }
  std::sort(peaks.begin(), peaks.end(), sort_peaks);

  // Select the highest and second highest peaks (if it is not to
  // close to the highest one)
  par[0] = peaks[0].first;
  par[1] = peaks[0].second;
  par[2] = 10;
  par[3] = peaks[1].first;
  par[4] = peaks[1].second;
  par[5] = 10;
  if (TMath::Abs((par[1] - par[4])/par[1]) > 0.75 &&
      TMath::Abs(par[0] - par[0]) < 10 &&
      n_peaks > 2) {
    par[3] = peaks[2].first;
    par[4] = peaks[2].second;
    par[5] = 10;
  }

  if (par[0] < WG_BEGIN_CHARGE_NOHIT ||
      par[3] > WG_END_CHARGE_HIT_HG) {
    gain[0] = gain[1] = std::nan("charge");
    return;
  }

  // Fit
  TF1 *fit = new TF1("twin_peaks", twin_peaks, WG_BEGIN_CHARGE_NOHIT,
                     WG_END_CHARGE_HIT_HG, 6);
  fit->SetParameters(par);
  charge_hit->Fit("twin_peaks", "QLN0");

  gain[0] = TMath::Abs(par[0] - par[3]);
  // error on gain
  gain[1] = TMath::Sqrt(TMath::Power(par[2], 2) + TMath::Power(par[5], 2));

  delete fit;
  delete fingers_plot;
}

//**********************************************************************
void wgFit::gain(unsigned ichip, unsigned ichan, unsigned icol,
                 double (&gain)[2], bool print_flag) {
  TH1I * charge_hit_HG = wgFit::histos.Get_charge_hit_HG(ichip, ichan, icol);
  if (charge_hit_HG == NULL) {
    gain[0] = gain[1] = std::nan("gain");
    throw wgElementNotFound("charge_hit_HG histogram not found : chip = " +
                            std::to_string(ichip) + ", chan = " +
                            std::to_string(ichan));
  }
  charge_hit_HG->SetDirectory(0);

  wgFit::gain(charge_hit_HG, gain);
  if (std::isnan(gain[0])) return;
  
  if( print_flag && (!m_output_img_dir.empty()) ) {
    TString image;
    image.Form("%s/gain%d_%d_%d.png",
               m_output_img_dir.c_str(), ichip, ichan, icol);
    charge_hit_HG->GetXaxis()->SetRange(WG_BEGIN_CHARGE_NOHIT,
                                        WG_END_CHARGE_HIT_HG);
    wgFit::histos.Print_charge_hit_HG(image, charge_hit_HG);
  }
  delete charge_hit_HG;
}
