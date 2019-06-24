// system includes
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

// boost includes
#include <boost/filesystem.hpp>

// ROOT includes
#include <TFile.h>
#include <TH1D.h>
#include <TText.h>
#include <TF1.h>
#include <TTree.h>
#include <TMath.h>
#include <TGraph.h>
#include <TCanvas.h>
#include <stdio.h>
#include <TROOT.h>
#include <TStyle.h> 
#include <TSystem.h>
#include <TSpectrum.h>

// user includes
#include "wgConst.hpp"
#include "wgFileSystemTools.hpp"
#include "wgErrorCode.hpp"
#include "wgExceptions.hpp"
#include "wgGetHist.hpp"
#include "wgFit.hpp"
#include "wgFitConst.hpp"
#include "wgLogger.hpp"

//#define DEBUG_WGFIT

using namespace std;

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
wgFit::wgFit(const string& x_inputfile, const string& x_outputIMGDir) {
  wgFit::GetHist = new wgGetHist(x_inputfile);
  CheckExist Check;
  if( !Check.Dir(x_outputIMGDir) ) {
    boost::filesystem::path dir(x_outputIMGDir);
    if( !boost::filesystem::create_directories(dir) ) {
      throw wgInvalidFile("[wgFit][" + x_outputIMGDir + "] failed to create directory");
    }
  }
  wgFit::outputIMGDir = x_outputIMGDir;
}

//**********************************************************************
wgFit::wgFit(const string& x_inputfile) {
  wgFit::GetHist = new wgGetHist(x_inputfile);
  wgConst con;
  CheckExist Check;
  if( !Check.Dir(con.IMGDATA_DIRECTORY) ) {
    boost::filesystem::path dir(con.IMGDATA_DIRECTORY);
    if( !boost::filesystem::create_directories(dir) ) {
      throw wgInvalidFile("[wgFit][" + con.IMGDATA_DIRECTORY + "] failed to create directory");
    }
  }
  wgFit::outputIMGDir = con.IMGDATA_DIRECTORY;
}

//**********************************************************************
wgFit::~wgFit(){
  delete wgFit::GetHist;
}

//**********************************************************************
void wgFit::SetoutputIMGDir(const string& str){
  wgFit::outputIMGDir = str;
}

//**********************************************************************
void wgFit::swap(int Npeaks, double* px, double* py){
  double px_temp,py_temp;
  for(int i=0;i<Npeaks;i++){
    for(int j=i+1;j<Npeaks;j++){
      if(px[i] > px[j]){
        px_temp=px[i];
        px[i]=px[j];
        px[j]=px_temp;
        py_temp=py[i];
        py[i]=py[j];
        py[j]=py_temp;
      }
    }
  }
}

//**********************************************************************
void wgFit::noise_rate(unsigned ichip, unsigned ichan, double (&x)[2], bool print_flag) {
  if ( (! GetHist->Get_bcid_hit(ichip, ichan)) || (! GetHist->Get_spill()) ) {
    x[0] = NAN;
    x[1] = NAN;
    return;
  }

  // Number of recorded spills
  Int_t nEntries = GetHist->h_spill->GetEntries();
  if( nEntries == 0 ) {
    x[0]=0.;
    x[1]=1./sqrt(GetHist->h_spill->GetEntries());
    throw wgElementNotFound("BCID histogram has no entries");
  }

  // Create a step function to fit the histogram and precisely find the "non
  // zero range" for the BCID hits. We could use the value of the last non zero
  // bin as a measure of the histogram "non zero range" but, that way, a single
  // corrupted hit could spoil the whole measurement and we want to avoid
  // that. Better to make the code a little slower (more computational heavy)
  // than to make it a little more unreliable.
  TF1 * step_function = new TF1("step_function", "((x > 0) ? ((x < [0]) ? [1] : 0) : 0)");
  
  // Find the right-most bin that is non-zero. This value provides us with a
  // rough estimate of the [0] parameter in the step function.
  step_function->SetParameter(0,GetHist->h_bcid_hit->FindLastBinAbove(0,1));
  step_function->SetParameter(1,1);
  step_function->SetParLimits(0, 1, MAX_BCID_BIN);

  // Fit histogram with step function.
  // "Q": quiet mode (minimum printing)
  // "L": use log likelihood method (default is chi-square method). To be used
  // when the. histogram represents counts.
  // "same" : when a fit is executed, the image of the function is drawn on
  // the current pad.
  // (0, MAX_BCID_BIN) : range over which to apply the fit.
  GetHist->h_bcid_hit->Fit("step_function", "Q L", "same", 0, MAX_BCID_BIN);

  // Integral() is the sum of all the hits between BCID = 0 and the BCID given
  // by half of the step position in the step function fit. The principle here
  // is that we want to roughly select only half of the columns (8 columns out
  // of 16) to avoid finite memory artifacts.
  Double_t B = 0.5 * step_function->GetParameter(0);
  Double_t Sigma = GetHist->h_bcid_hit->Integral(0, B);
  GetHist->h_bcid_hit->GetXaxis()->SetRange(0, 2*B+10);

  x[0] = Sigma /((B * nEntries - Sigma) * time_bcid_bin);  // Hertz
  x[1] = ((B * nEntries - Sigma) * sqrt(Sigma)) / (pow(B,2) * pow(nEntries,2) * time_bcid_bin); // Hertz
  
  if ( print_flag ) {
    TString image;
    image.Form("%s/chip%u/chan%u/NoiseRate%u_%u.png", outputIMGDir.c_str(), ichip, ichan, ichip, ichan);
    GetHist->Print_bcid(image);
  }
  delete step_function;
}

//**********************************************************************
void wgFit::charge_hit(unsigned ichip, unsigned ichan, unsigned icol, double (&x)[3], bool print_flag) {

  if ( ! wgFit::GetHist->Get_charge_hit(ichip, ichan, icol) ) {
    x[0] = NAN;
    x[1] = NAN;
    x[2] = NAN;
    return;
  }

  if(wgFit::GetHist->h_charge_hit->Integral(begin_pe, end_pe) < 1 )
    {
#ifdef DEBUG_WGFIT
      Log.eWrite("[wgFit::charge_hit] no entry (chip:" + to_string(ichip) + ", ch:" + to_string(ichan) + ")");
#endif
      x[0] = x[1] = x[2] = 0;
      return;
    } 
  // begin_pe and end_pe are defined in wgFitConst.cpp
  wgFit::GetHist->h_charge_hit->GetXaxis()->SetRange(begin_pe,end_pe);

  double mean={(double)GetHist->h_charge_hit->GetMaximumBin()};
  double peak={(double)GetHist->h_charge_hit->GetMaximum()};

  TF1* gaussian = new TF1("gauss", gauss, mean - 3 * sigma, mean + 3 * sigma, 3);
  gaussian->SetLineColor(kGreen);
  gaussian->SetNpx(500);
  gaussian->SetParameters(peak, mean, sigma);
  gaussian->SetParNames("peak_fit", "mean_fit", "sigma_fit");
  gaussian->SetParLimits(0, 0.9 * peak, 1.1 * peak); // peak_fit
  gaussian->SetParLimits(1, mean - max_sigma, mean + max_sigma);   // mean_fit
  gaussian->SetParLimits(2, min_sigma, max_sigma);   // sigma_fit (min_sigma and max_sigma are defined in wgFitConst.cpp)

  // Fit histogram with gaussian function.
  // "Q": quiet mode (minimum printing)
  // "P" : drawing option
  // (mean - 3 * sigma, mean + 3 * sigma) : range over which to apply the fit.
  GetHist->h_charge_hit->Fit(gaussian, "Q", "P", mean - 3 * sigma, mean + 3 * sigma);
  x[0]=gaussian->GetParameter(1); // mean_fit
  x[1]=gaussian->GetParameter(2); // sigma_fit
  x[2]=gaussian->GetParameter(0); // peak_fit

  if( print_flag && (!outputIMGDir.empty()) ) {
    TString image;
    image.Form("%s/chip%u/chan%u/charge_hit%u_%u_%u.png", outputIMGDir.c_str(), ichip, ichan, ichip, ichan, icol);
    GetHist->Print_charge(image);
  }
  delete gaussian;
  return;  
}


//**********************************************************************
void wgFit::charge_hit_HG(unsigned ichip, unsigned ichan, unsigned icol, double (&x)[3], bool print_flag) {

  if ( ! wgFit::GetHist->Get_charge_hit_HG(ichip,ichan,icol) ) {
    x[0] = NAN;
    x[1] = NAN;
    x[2] = NAN;
    return;
  }
  
  if(wgFit::GetHist->h_charge_hit_HG->Integral(begin_pe_HG,end_pe_HG) < 1 )
    {
#ifdef DEBUG_WGFIT
      Log.eWrite("[wgFit::charge_nohit] no entry (chip:" + to_string(ichip) + ", ch:" + to_string(ichan) + ", col:" + to_string(icol) + ")");
#endif
      x[0]=x[1]=x[2]=0.;
      return;
    } 
  // begin_pe_HG and end_pe_HG are defined in wgFitConst.cpp
  wgFit::GetHist->h_charge_hit_HG->GetXaxis()->SetRange(begin_pe_HG,end_pe_HG);
  double mean=(double)GetHist->h_charge_hit_HG->GetMaximumBin();
  double peak=(double)GetHist->h_charge_hit_HG->GetMaximum();

  TF1* gaussian = new TF1("gauss", gauss, mean - 3 * sigma, mean + 3 * sigma, 3);
  gaussian->SetLineColor(kGreen);
  gaussian->SetNpx(500);
  gaussian->SetParameters(peak, mean, sigma);
  gaussian->SetParNames("peak_fit", "mean_fit", "sigma_fit");
  gaussian->SetParLimits(0, 0.9 * peak, 1.1 * peak); // peak_fit
  gaussian->SetParLimits(1, mean - max_sigma, mean + max_sigma);   // mean_fit
  gaussian->SetParLimits(2, min_sigma, max_sigma);   // sigma_fit (min_sigma and max_sigma are defined in wgFitConst.cpp)

  // Fit histogram with gaussian function.
  // "Q": quiet mode (minimum printing)
  // "P" : drawing option
  // (mean - 3 * sigma, mean + 3 * sigma) : range over which to apply the fit.
  GetHist->h_charge_hit_HG->Fit(gaussian, "Q", "P", mean - 3 * sigma, mean + 3 * sigma);
  x[0]=gaussian->GetParameter(1); // mean_fit
  x[1]=gaussian->GetParameter(2); // sigma_fit
  x[2]=gaussian->GetParameter(0); // peak_fit

  if( print_flag && (!outputIMGDir.empty()) ) {
    TString image;
    image.Form("%s/chip%u/chan%u/HG%u_%u_%u.png", outputIMGDir.c_str(), ichip, ichan, ichip, ichan, icol);
    GetHist->Print_charge_hit_HG(image);
  }
  delete gaussian;
  return;
}

//**********************************************************************
void wgFit::charge_nohit(const unsigned ichip, const unsigned ichan, const unsigned icol, double (&x)[3], const bool print_flag) {

  // Read the "charge_nohit" histogram for the _hist.root file
  if ( ! GetHist->Get_charge_nohit(ichip, ichan, icol) ) {
    x[0] = NAN;
    x[1] = NAN;
    x[2] = NAN;
    return;
  }

  // If the histogram is empty return a 0 vector
  if(GetHist->h_charge_nohit->Integral(begin_ped, end_ped) < 1 )
    {
#ifdef DEBUG_WGFIT 
      Log.eWrite("[wgFit::charge_nohit] no entry (chip:" + to_string(ichip) + ", ch:" + to_string(ichan) + ", col:" + to_string(icol) + ")");
#endif
      x[0]=x[1]=x[2]=0.;
      return;
    }
  
  // Set the histogram x axis range to 350 --- 700 (the pedestal lies usually in
  // that range)
  GetHist->h_charge_nohit->GetXaxis()->SetRange(begin_ped, end_ped);
  
  double mean=(double)GetHist->h_charge_nohit->GetMaximumBin();
  double peak=(double)GetHist->h_charge_nohit->GetMaximum();
  // Gaussian function to use when fitting
  // Arguments:
  // (function name, function pointer, xmin, xmax, number of parameters)
  // Sigma is defined in wgFitConst.cpp
  TF1* gaussian = new TF1("gauss", gauss, mean - 3 * sigma, mean + 3 * sigma, 3);
  gaussian->SetLineColor(kViolet);
  gaussian->SetNpx(500);
  gaussian->SetParameters(peak, mean, sigma);
  gaussian->SetParNames("peak_fit", "mean_fit", "sigma_fit");
  gaussian->SetParLimits(0, 0.9 * peak, 1.1 * peak); // peak_fit
  gaussian->SetParLimits(1, mean - max_sigma, mean + max_sigma);   // mean_fit
  gaussian->SetParLimits(2, min_sigma, max_sigma);   // sigma_fit (min_sigma and max_sigma are defined in wgFitConst.cpp)

  // Fit histogram with gaussian function.
  // "Q": quiet mode (minimum printing)
  // "same" : when a fit is executed, the image of the function is drawn on
  // the current pad.
  // (mean - 3 * sigma, mean + 3 * sigma) : range over which to apply the fit.
  GetHist->h_charge_nohit->Fit(gaussian, "Q", "same", mean - 3 * sigma, mean + 3 * sigma);
  x[0]=gaussian->GetParameter(1); // mean_fit
  x[1]=gaussian->GetParameter(2); // sigma_fit
  x[2]=gaussian->GetParameter(0); // peak_fit
    
  if( print_flag && (!outputIMGDir.empty()) ) {
    TString image;
    image.Form("%s/chip%u/chan%u/nohit%d_%d_%d.png", outputIMGDir.c_str(), ichip, ichan, ichip, ichan, icol);
    GetHist->Print_charge_nohit(image);
  }
  
  delete gaussian;
  return;  
}
