// system includes
#include <vector>
#include <string>
#include <cmath>

// ROOT includes
#include <TF1.h>
#include <TString.h>
#include <TSpectrum.h>
#include <TMath.h>
#include "TVirtualFitter.h"
#include "Math/MinimizerOptions.h"

// user includes
#include "wgConst.hpp"
#include "wgFileSystemTools.hpp"
#include "wgExceptions.hpp"
#include "wgGetHist.hpp"
#include "wgFitConst.hpp"
#include "wgLogger.hpp"
#include "wgFit.hpp"

using namespace wagasci_tools;

const std::map<int, std::string> wgFit::fit_status_str_ = {
  {1, "Covariance was made pos defined"},
  {2, "Hesse is invalid"},
  {3, "Edm is above max"},
  {4, "Reached call limit"},
  {5, "Covariance is not positive defined"}};

void delete_pointers(std::vector<TH1I *> &hists) {
  for (auto &hist : hists)
    delete hist;
}

//**********************************************************************
wgFit::wgFit(const std::string& x_inputfile,
             const std::string& x_output_img_dir) :
    histos_(x_inputfile) {
  wgFit::SetOutputImgDir(x_output_img_dir);
}

//**********************************************************************
wgFit::wgFit(const std::string& x_inputfile) :
    histos_(x_inputfile) {
  wgEnvironment env;
  wgFit::SetOutputImgDir(env.IMGDATA_DIRECTORY);
}

//**********************************************************************
void wgFit::SetOutputImgDir(const std::string& x_output_img_dir) {
  if( !check_exist::directory(x_output_img_dir) ) {
    make::directory(x_output_img_dir);
  }
  output_img_dir_ = x_output_img_dir;
}

//**********************************************************************
void wgFit::NoiseRate(TH1I * bcid_hit, double (&x)[2], unsigned spill_count) {
  // Find the right-most bin that is non-zero.
  Int_t last_bin = bcid_hit->FindLastBinAbove(0,1);
  if (last_bin <= 0) {
    x[0] = x[1] = -1;
    return;
  }

  // Integral() is the sum of all the hits between BCID = 0 and a BCID
  // roughly in the middle of the histogram. The principle here is
  // that we want to roughly select only half of the columns (8
  // columns out of 16) to avoid finite memory artifacts.
  Double_t time_interval = 0.5 * last_bin;
  Double_t sum = bcid_hit->Integral(0, time_interval);
  bcid_hit->GetXaxis()->SetRange(0, 2 * time_interval + 10);

  // in Hertz
  x[0] = sum / ((time_interval * spill_count - sum) * TIME_BCID);
  x[1] = ((time_interval * spill_count - sum) * sqrt(sum)) /
         (pow(time_interval, 2) * pow(spill_count, 2) * TIME_BCID);
}

//**********************************************************************
void wgFit::NoiseRate(double (&x)[2], unsigned dif_id, unsigned ichip,
                      unsigned ichan, bool print_flag) {
  TH1I * bcid_hit = wgFit::histos_.Get_bcid_hit(dif_id,ichip, ichan);
  if (bcid_hit == NULL) {
    x[0] = x[1] = -1;
    throw wgElementNotFound("BCID histogram not found : chip = " +
                            std::to_string(ichip) + ", chan = " +
                            std::to_string(ichan));
  }
  bcid_hit->SetDirectory(0);

  // Number of recorded spills
  unsigned spill_count = wgFit::histos_.spill_count;
  if (spill_count <= 0) {
    x[0] = x[1] = -1;
    return;
  }

  wgFit::NoiseRate(bcid_hit, x, spill_count);

  if ( print_flag ) {
    TString image;
    image.Form("%s/chip%u/chan%u/NoiseRate%u_%u.png",
               output_img_dir_.c_str(), ichip, ichan, ichip, ichan);
    wgFit::histos_.Print_bcid(image, bcid_hit);
  }
  delete bcid_hit;
}

void wgFit::Charge(TH1I * charge, double (&x)[3], wgFit::GainSelect gs) {

  ROOT::Math::MinimizerOptions::SetDefaultStrategy(0);
  // If the fit fails too many times try to set a smaller tolerance
  // ROOT::Math::MinimizerOptions::SetDefaultTolerance(1.E-6);
  
  static int fail_counter = 0;
  
  if (charge->GetEntries() <= WG_MIN_ENTRIES_FOR_FIT) {
    x[0] = x[1] = x[2] = -1;
    return;
  }

  Int_t begin, end;
  switch (gs) {
    case wgFit::GainSelect::HighGain:
      begin = WG_BEGIN_CHARGE_HIT_HG;
      end = WG_END_CHARGE_HIT_HG;
      break;
    case wgFit::GainSelect::LowGain:
      begin = WG_BEGIN_CHARGE_HIT_LG;
      end = WG_END_CHARGE_HIT_LG;
      break;
    case wgFit::GainSelect::Pedestal:
      begin = WG_BEGIN_CHARGE_NOHIT;
      end = WG_END_CHARGE_NOHIT;
      break;
    default:
      begin = 0;
      end = MAX_VALUE_12BITS;
      break;
  }

  charge->GetXaxis()->SetRange(begin, end);
  Double_t par[3];
  par[0] = charge->GetBinContent(charge->GetMaximumBin());
  par[1] = charge->GetMaximumBin();
  par[2] = 5;

  std::unique_ptr<TF1> gaussian(new TF1("gaussian", "gaus(0)", begin, end));
  gaussian->SetParameters(par);
  gaussian->SetParNames("mean", "normalization", "sigma");
  gaussian->SetParLimits(0, 0.5 * par[0], 2 * par[0]);
  gaussian->SetParLimits(1, par[1] - 0.5 * WG_NOMINAL_GAIN,
                         par[1] + 0.5 * WG_NOMINAL_GAIN);
  gaussian->SetParLimits(2, 0, 0.5 * WG_NOMINAL_GAIN);
  
  // "B" : Use user defined boundaries
  // "Q": quiet mode (minimum printing)
  // "N" : Do not store the graphics function, do not draw
  // "0" : Do not plot the result of the fit.
  int fit_status = charge->Fit(gaussian.get(), "BQN0", "N0", begin, end);

  if (fit_status != 0) {
    x[0] = par[1];     // mean_fit
    x[1] = 2 * par[2]; // sigma_fit
    x[2] = -1;         // peak_fit
    std::stringstream ss;
    ss << "Fail counter " << ++fail_counter << " : " <<
        "charge fit failed : (" << fit_status << ") ";
    if (fit_status_str_.count(fit_status))
      ss << fit_status_str_.at(fit_status);
    throw wgFitFailed(ss.str());
  }
  
  x[0] = gaussian->GetParameter(1); // mean_fit
  x[1] = gaussian->GetParameter(2); // sigma_fit
  x[2] = gaussian->GetParameter(0); // peak_fit
}

//**********************************************************************
void wgFit::ChargeHitHG(double (&x)[3],unsigned dif_id, unsigned ichip,
                        unsigned ichan, int icol, bool print_flag) {
  std::vector<TH1I *> charge_hit_HG;
  
  if (icol >= 0)
    charge_hit_HG.push_back(wgFit::histos_.Get_charge_hit_HG(dif_id, ichip,
                                                             ichan, icol));
  else
    charge_hit_HG.push_back(wgFit::histos_.Get_charge_hit_HG(dif_id, ichip,
                                                             ichan, 0));

  if (charge_hit_HG.at(0) == nullptr) {
    x[0] = x[1] = x[2] = -1;
    std::stringstream ss;
    ss << "charge_hit_HG histogram not found : dif = " << dif_id <<
        "chip = " << ichip << ", chan = " << ichan << ", col = " << icol;
    throw wgElementNotFound(ss.str());
  }
  charge_hit_HG.at(0)->SetDirectory(0);
  
  if (icol < 0)
    for (unsigned i = 1; i < MEMDEPTH; ++i) {
      charge_hit_HG.push_back(wgFit::histos_.Get_charge_hit_HG(dif_id, ichip,
                                                               ichan, i));
      if (charge_hit_HG.at(i) != nullptr) {
        charge_hit_HG.at(i)->SetDirectory(0);
        charge_hit_HG.at(0)->Add(charge_hit_HG.at(i));
      }
    }

  try { wgFit::Charge(charge_hit_HG.at(0), x, wgFit::GainSelect::HighGain); }
  catch (const std::exception&) {
    delete_pointers(charge_hit_HG);
    throw;
  }

  if( print_flag && (!output_img_dir_.empty()) ) {
    TString image;
    image.Form("%s/chip%u/chan%u/charge_hit_HG%u_%u_%u.png",
               output_img_dir_.c_str(), ichip, ichan, ichip, ichan, icol);
    wgFit::histos_.Print_charge_hit_HG(image, charge_hit_HG.at(0));
  }

  delete_pointers(charge_hit_HG);
}

//**********************************************************************
void wgFit::ChargeHitLG(double (&x)[3], unsigned dif_id, unsigned ichip,
                        unsigned ichan, int icol, bool print_flag) {
  TH1I * charge_hit_LG = wgFit::histos_.Get_charge_hit_LG(dif_id, ichip,
                                                          ichan, icol);
  if (charge_hit_LG == NULL) {
    x[0] = x[1] = x[2] = -1;
    throw wgElementNotFound("charge_hit_LG histogram not found : chip = " +
                            std::to_string(ichip) + ", chan = " +
                            std::to_string(ichan));
  }
  charge_hit_LG->SetDirectory(0); 

  wgFit::Charge(charge_hit_LG, x, wgFit::GainSelect::LowGain);

  if( print_flag && (!output_img_dir_.empty()) ) {
    TString image;
    image.Form("%s/chip%u/chan%u/charge_hit_LG%u_%u_%u.png",
               output_img_dir_.c_str(), ichip, ichan, ichip, ichan, icol);
    wgFit::histos_.Print_charge_hit_LG(image, charge_hit_LG);
  }
  delete charge_hit_LG;
}

//**********************************************************************
void wgFit::ChargeNohit(double (&x)[3], unsigned dif_id, unsigned ichip,
                        unsigned ichan, int icol, bool print_flag) {
  TH1I * charge_nohit = wgFit::histos_.Get_charge_nohit(dif_id, ichip,
                                                        ichan, icol);
  if (charge_nohit == NULL) {
    x[0] = x[1] = x[2] = -1;
    throw wgElementNotFound("charge_nohit histogram not found : chip = " +
                            std::to_string(ichip) + ", chan = " +
                            std::to_string(ichan));
  }
  charge_nohit->SetDirectory(0);

  wgFit::Charge(charge_nohit, x, wgFit::GainSelect::Pedestal);

  if( print_flag && (!output_img_dir_.empty()) ) {
    TString image;
    image.Form("%s/chip%u/chan%u/charge_nohit%d_%d_%d.png",
               output_img_dir_.c_str(), ichip, ichan, ichip, ichan, icol);
    wgFit::histos_.Print_charge_nohit(image, charge_nohit);
  }
  delete charge_nohit;
}

//**********************************************************************
Double_t wgFit::TwinPeaks(Double_t *x, Double_t *par) {
   Double_t result = 0;
   for (Int_t p = 0; p < 2; ++p) {
      Double_t mean  = par[3 * p];
      Double_t norm  = par[3 * p + 1];
      Double_t sigma = par[3 * p + 2];
      result += norm * TMath::Gaus(x[0], mean, sigma, kFALSE);
   }
   return result;
}

bool wgFit::SortPeaks(std::pair<Double_t, Double_t> peak1,
                       std::pair<Double_t, Double_t> peak2) {
   return peak1.first < peak2.first;
}

//**********************************************************************
void wgFit::Gain(TH1I * charge_hit, std::array<double, 2>& gain,
                 unsigned max_nb_peaks, bool do_not_fit) {

  static int fail_counter = 0;
  
  if (max_nb_peaks > 2)
    throw wgNotImplemented("Fitting more than two peaks is not implemented");
  
  std::unique_ptr<TSpectrum> fingers_plot(new TSpectrum(max_nb_peaks));
  Int_t n_peaks = fingers_plot->Search(charge_hit, 2,
                                       "nobackground,nodraw,goff", 0.05);
  if (n_peaks < 2) {
    gain[0] = gain[1] = -1;
    throw wgFitFailed("Less than 2 peaks found (" +
                      std::to_string(n_peaks) + ")");
  }

  Double_t *xpeaks = fingers_plot->GetPositionX();

  std::vector<std::pair<Double_t, Double_t>> peaks;
  for (Int_t peak = 0; peak < n_peaks; ++peak) {
    Double_t xp = xpeaks[peak];
    Int_t bin = charge_hit->GetXaxis()->FindBin(xp);
    Double_t yp = charge_hit->GetBinContent(bin);
    peaks.push_back(std::pair<Double_t, Double_t>(xp, yp));
  }
  std::sort(peaks.begin(), peaks.end(), wgFit::SortPeaks);

  if (peaks[0].first < WG_BEGIN_CHARGE_NOHIT) {
    gain[0] = gain[1] = -1;
    std::stringstream ss;
    ss << "left peak (" << peaks[0].first <<
        ") is less than " << WG_BEGIN_CHARGE_NOHIT;
    throw wgFitFailed(ss.str());
  }
  if (peaks[1].first > WG_END_CHARGE_HIT_HG) {
    gain[0] = gain[1] = -1;
    std::stringstream ss;
    ss << "right peak (" << peaks[1].first <<
        ") is greater than " << WG_END_CHARGE_HIT_HG;
    throw wgFitFailed(ss.str());
  }

  Double_t par[6];
  bool tspectrum_failed = false;
  par[0] = peaks[0].first;   // first peak x
  par[1] = peaks[0].second;  // first peak y
  par[2] = 10;               // first peak sigma
  par[3] = peaks[1].first;   // second peak x
  par[4] = peaks[1].second;  // second peak y
  par[5] = 10;               // second peak sigma
  if (TMath::Abs(peaks[0].first - peaks[1].first) < 20) {
    tspectrum_failed = true;
    par[3] = WG_PEAK_CHARGE_1PE; // second peak x
  }

  if (do_not_fit) {
    gain[0] = TMath::Abs(par[0] - par[3]);
    gain[1] = 10;
  } else {
    ROOT::Math::MinimizerOptions::SetDefaultStrategy(0);
    // If the fit fails too many times try to set a smaller tolerance
    // ROOT::Math::MinimizerOptions::SetDefaultTolerance(1.E-6);
    
    std::unique_ptr<TF1> fit(new TF1("twin_peaks", wgFit::TwinPeaks,
                                     WG_BEGIN_CHARGE_NOHIT, WG_END_CHARGE_HIT_HG,
                                     3 * n_peaks));
    fit->SetParameters(par);
    fit->SetParLimits(0, WG_BEGIN_CHARGE_NOHIT, par[3]);
    fit->SetParLimits(3, par[0], WG_END_CHARGE_HIT_HG);
    if (!tspectrum_failed) {
      fit->SetParLimits(1, par[1] / 2., par[1] * 2.);
      fit->SetParLimits(4, par[4] / 2., par[4] * 2.);
    }

    // "B" : Use user defined boundaries
    // "Q": quiet mode (minimum printing)
    // "N" : Do not store the graphics function, do not draw
    // "0" : Do not plot the result of the fit.
    int fit_status = charge_hit->Fit("twin_peaks", "BQN0", "N0",
                                     WG_BEGIN_CHARGE_NOHIT, WG_END_CHARGE_HIT_HG);
    if (fit_status != 0) {
      gain[0] = gain[1] = -1;
      std::stringstream ss;
      ss << "fail counter " << ++fail_counter << " : (" << fit_status << ") ";
      if (fit_status_str_.count(fit_status))
        ss << fit_status_str_.at(fit_status);
      throw wgFitFailed(ss.str());
    }
    fit->GetParameters(par);

    gain[0] = TMath::Abs(par[0] - par[3]);
    // error on gain
    gain[1] = TMath::Sqrt(TMath::Power(par[2], 2) + TMath::Power(par[5], 2));
  }
}

//**********************************************************************
void wgFit::Gain(std::array<double, 2>& gain, unsigned dif_id, unsigned ichip,
                 unsigned ichan, int icol, unsigned n_peaks,
                 bool print_flag, bool do_not_fit) {
  
  std::vector<TH1I *> charge_hit_HG;
  
  if (icol >= 0)
    charge_hit_HG.push_back(wgFit::histos_.Get_charge_hit_HG(dif_id, ichip,
                                                             ichan, icol));
  else
    charge_hit_HG.push_back(wgFit::histos_.Get_charge_hit_HG(dif_id, ichip,
                                                             ichan, 0));

  if (charge_hit_HG.at(0) == nullptr) {
    gain[0] = gain[1] = -1;
    std::stringstream ss;
    ss << "charge_hit_HG histogram not found : dif = " << dif_id <<
        "chip = " << ichip << ", chan = " << ichan << ", col = " << icol;
    throw wgElementNotFound(ss.str());
  }
  charge_hit_HG.at(0)->SetDirectory(0);
  
  if (icol < 0)
    for (unsigned i = 1; i < MEMDEPTH; ++i) {
      charge_hit_HG.push_back(wgFit::histos_.Get_charge_hit_HG(dif_id, ichip,
                                                               ichan, i));
      if (charge_hit_HG.at(i) != nullptr) {
        charge_hit_HG.at(i)->SetDirectory(0);
        charge_hit_HG.at(0)->Add(charge_hit_HG.at(i));
      }
    }

  try { wgFit::Gain(charge_hit_HG.at(0), gain, n_peaks, do_not_fit); }
  catch (const std::exception&) {
    delete_pointers(charge_hit_HG);
    throw;
  }

  if (print_flag && (!output_img_dir_.empty())) {
    icol = (icol < 0) ? 0 : icol;
    TString image;
    image.Form("%s/gain_%d_%d_%d.png",
               output_img_dir_.c_str(), ichip, ichan, icol);
    charge_hit_HG.at(0)->GetXaxis()->SetRange(WG_BEGIN_CHARGE_NOHIT,
                                              WG_END_CHARGE_HIT_HG);
    wgFit::histos_.Print_charge_hit_HG(image, charge_hit_HG.at(0));
  }
  
  delete_pointers(charge_hit_HG);
}
