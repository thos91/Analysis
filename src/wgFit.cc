#include <TFile.h>
#include <TH1F.h>
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

#include <iostream>
#include <fstream>
#include <math.h>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <string>

#include "Const.h"
#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgGetHist.h"
#include "wgFit.h"
#include "wgFitConst.h"

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
wgFit::wgFit(const string& inputfile) {
  wgFit::GetHist = new wgGetHist(inputfile);
}

//**********************************************************************
wgFit::~wgFit(){
  delete wgFit::GetHist;
}

//**********************************************************************
void wgFit::clear(){
  wgFit::GetHist->clear();
}

//**********************************************************************
void wgFit::SetoutputIMGDir(const string& str){
  wgFit::outputIMGDir=str;
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
void wgFit::NoiseRate(unsigned ichip, unsigned ichan, double x[2], int mode) {
  GetHist->Get_bcid_hit(ichip, ichan);
  GetHist->Get_spill();
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
  TF1 * step_function = new TF1("step_function", "((x > 0) ? ((x < [0]) ? 1 : 0) : 0)");
  
  // Find the right-most bin that is non-zero. This value provides us with a
  // rough estimate of the [0] parameter in the step function.
  step_function->SetParameter(0,GetHist->h_bcid_hit->FindLastBinAbove(0,1));
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

  x[0] = Sigma /((B * nEntries - Sigma) * time_bcid_bin);
  x[1] = ((B * nEntries - Sigma) * sqrt(Sigma)) / (pow(B,2) * pow(nEntries,2) * time_bcid_bin);
  
  if ( mode == 1 ) {
    wgConst con;
    con.GetENV();
    GetHist->Print_bcid(Form("%s/chip%d/NoiseRate%d_%d.png", outputIMGDir.c_str(), ichip, ichip, ichan), "", 1);
  }
  delete step_function;
  delete GetHist->h_bcid_hit;
  delete GetHist->h_spill;
}

//**********************************************************************
void wgFit::low_pe_charge(unsigned int i,unsigned int j,double* x,int mode=0){

  wgFit::GetHist->Get_charge_hit(i,j);

  if(wgFit::GetHist->h_charge_hit->Integral(begin_low_pe,end_low_pe) < 1 )
  {
#ifdef DEBUG_WGFIT
    cout<< "!Warning!!no entry (chip:"<< i <<", ch:"<< j <<", col:" << k <<")"<< endl;
#endif
    x[0]=x[1]=x[2]=0.;
    return;
  } 
  
  wgFit::GetHist->h_charge_hit->GetXaxis()->SetRange(begin_low_pe,end_low_pe);

  double px[2]={(double)wgFit::GetHist->h_charge_hit->GetMaximumBin(),0.0};
  double py[2]={(double)wgFit::GetHist->h_charge_hit->GetMaximum(),0.0};
  
  TF1* prefit = new TF1("gauss",gauss,px[0]-sigma*3,px[0]+sigma*3,3);
  prefit->SetLineColor(kGreen);
  prefit->SetNpx(500);
  prefit->SetParameters(py[0],px[0],sigma);
  prefit->SetParNames("Event0","mean0","sigma0");
  prefit->SetParLimits(0,0.1*py[0],1.1*py[0]);
  prefit->SetParLimits(1,px[0]-20,px[0]+20);
  prefit->SetParLimits(2,min_sigma,max_sigma);
  wgFit::GetHist->h_charge_hit->Fit(prefit,"Q","P",px[0]-sigma*2,px[0]+sigma*2.0);
  double Event0=prefit->GetParameter(0);
  double mean0=prefit->GetParameter(1);
  double sigma0=prefit->GetParameter(2);
 
  x[0]=mean0;
  x[1]=Event0;
  x[2]=sigma0;

  if(mode==1){ 
    wgConst *con = new wgConst;
    con->GetENV();
    string output = Form("%s/chip%d/charge%d_%d.png",wgFit::outputIMGDir.c_str(),i,i,j);
    wgFit::GetHist->Print_charge(output.c_str(),"",1);
    delete con;
  }

  //delete ts;
  //delete fit;
  delete prefit;
  delete wgFit::GetHist->h_charge_hit;
  return;  
}


//**********************************************************************
void wgFit::low_pe_charge_HG(unsigned int i,unsigned int j,unsigned int k,double* x,int mode=0){

  wgFit::GetHist->Get_charge_hit_HG(i,j,k);

  if(wgFit::GetHist->h_charge_hit_HG->Integral(begin_low_pe_HG,end_low_pe_HG) < 1 )
  {
#ifdef DEBUG_WGFIT
    cout<< "!Warning!!no entry (chip:"<< i <<", ch:"<< j <<", col:" << k <<")"<< endl;
#endif
    x[0]=x[1]=x[2]=0.;
    return;
  } 
  
  wgFit::GetHist->h_charge_hit_HG->GetXaxis()->SetRange(begin_low_pe_HG,end_low_pe_HG);

  double px[2]={ (double) wgFit::GetHist->h_charge_hit_HG->GetMaximumBin(),0.0};
  double py[2]={ (double) wgFit::GetHist->h_charge_hit_HG->GetMaximum(),0.0};
  
  TF1* prefit = new TF1("gauss",gauss,px[0]-sigma*3,px[0]+sigma*3,3);
  prefit->SetLineColor(kGreen);
  prefit->SetNpx(500);
  prefit->SetParameters(py[0],px[0],sigma);
  prefit->SetParNames("Event0","mean0","sigma0");
  prefit->SetParLimits(0,0.1*py[0],1.1*py[0]);
  prefit->SetParLimits(1,px[0]-20,px[0]+20);
  prefit->SetParLimits(2,min_sigma,max_sigma);
  GetHist->h_charge_hit_HG->Fit(prefit,"Q","P",px[0]-sigma*2,px[0]+sigma*2.0);
  double Event0=prefit->GetParameter(0);
  double mean0=prefit->GetParameter(1);
  double sigma0=prefit->GetParameter(2);
 
  x[0]=mean0;
  x[1]=Event0;
  x[2]=sigma0;

  if(mode==1){ 
    wgConst *con = new wgConst;
    con->GetENV();
    string output = Form("%s/chip%d/HG%d_%d_%d.png",wgFit::outputIMGDir.c_str(),i,i,j,k);
    GetHist->Print_charge_hit_HG(output.c_str(),"",1);
    delete con;
  }

  //delete ts;
  //delete fit;
  delete prefit;
  delete GetHist->h_charge_hit_HG;
  return;  
}

//**********************************************************************
void wgFit::charge_nohit(unsigned int i,unsigned int j,unsigned int k,double* x,int mode=0){
  GetHist->Get_charge_nohit(i,j,k);

  if(GetHist->h_charge_nohit->Integral(begin_ped,end_ped) < 1 )
  {
#ifdef DEBUG_WGFIT 
    cout<< "!Warning!!no entry (chip:"<< i <<", ch:"<< j <<", col:" << k <<")"<< endl;
#endif
    x[0]=x[1]=x[2]=0.;
    return;
  } 
  
  GetHist->h_charge_nohit->GetXaxis()->SetRange(begin_ped,end_ped);
  
  double px[1]={(double)GetHist->h_charge_nohit->GetMaximumBin()};
  double py[1]={(double)GetHist->h_charge_nohit->GetMaximum()};

  TF1* prefit = new TF1("gauss",gauss,px[0]-sigma*3,px[0]+sigma*3,3);
  prefit->SetLineColor(kViolet);
  prefit->SetNpx(500);
  prefit->SetParameters(py[0],px[0],sigma);
  prefit->SetParNames("Event0","mean0","sigma0");
  prefit->SetParLimits(0,0.9*py[0],1.1*py[0]);
  prefit->SetParLimits(1,px[0]-20,px[0]+20);
  prefit->SetParLimits(2,min_sigma,max_sigma);
  GetHist->h_charge_nohit->Fit(prefit,"Q","",px[0]-sigma*2,px[0]+sigma*2.0);
  double Event0=prefit->GetParameter(0);
  double mean0=prefit->GetParameter(1);
  double sigma0=prefit->GetParameter(2);

  x[0]=mean0;
  x[1]=Event0;
  x[2]=sigma0;
    
  if(mode==1){ 
    string output = Form("%s/chip%d/nohit%d_%d_%d.png",wgFit::outputIMGDir.c_str(),i,i,j,k);
    GetHist->Print_charge_nohit(output.c_str(),"",1);
  }
  delete prefit;
  delete GetHist->h_charge_nohit;
  return;  
}

//**********************************************************************
void wgFit::GainSelect(unsigned int i,unsigned int j,unsigned int k,double* x, int mode=0){
}

