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
wgFit::wgFit(){
}

//**********************************************************************
wgFit::wgFit(string& inputfile){
  GetHist = new wgGetHist(inputfile);
}

//**********************************************************************
wgFit::~wgFit(){
  delete GetHist;
}

//**********************************************************************
void wgFit::clear(){
  GetHist->clear();
}

//**********************************************************************
void wgFit::SetoutputIMGDir(string& str){
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
void wgFit::NoiseRate(unsigned int i,unsigned int j,double* x, int mode=0){
  GetHist->Get_bcid(i,j);
  GetHist->Get_spill();
  if( h_bcid->GetEntries() == 0 ){
    x[0]=0.;
    x[1]=1./sqrt(h_spill->GetEntries());
    return;
  }
  
  double nEntry = h_spill -> GetEntries();
  double const1 = h_bcid ->GetBinContent(h_bcid->FindBin(0));
  double const2 = h_bcid ->GetBinContent(h_bcid->FindBin(1));
  if(const1 < 1.) const1 = h_bcid->GetMaximum();

  TF1 *f1 = new TF1("f1","pol0");
  f1->SetParameter(0,const1);
  f1->SetParLimits(0.,h_bcid->GetEntries(),0);
  
  double const_fit1;
  double const_fit1_e;
  double p1;

  if(h_bcid->Integral(begin_BCID,end_BCID)/nEntry > 1./32.0){
    if(const2 > const1 + 2*sqrt(const1)){
      h_bcid->Fit("f1","Q L","same",0,1);
      h_bcid->GetXaxis()->SetRange(begin_BCID,end_BCID+10);
      const_fit1 = f1->GetParameter(0);
      const_fit1_e = f1->GetParError(0);
    }else{
      h_bcid->Fit("f1","Q L","same",begin_BCID,end_BCID);
      h_bcid->GetXaxis()->SetRange(begin_BCID,end_BCID+10);
      const_fit1 = f1->GetParameter(0);
      const_fit1_e = f1->GetParError(0);
    }
  }else if(h_bcid->Integral(begin_BCID,end_BCID*16)/nEntry > 1./32.0){
    h_bcid->Fit("f1","Q L","same",begin_BCID,end_BCID*16);
    h_bcid->GetXaxis()->SetRange(begin_BCID,end_BCID*16+10);
    const_fit1 = f1->GetParameter(0);
    const_fit1_e = f1->GetParError(0);
  }else{
    const_fit1 = h_bcid->Integral(begin_BCID,limit_BCID)/(limit_BCID-begin_BCID);
    const_fit1_e = sqrt(h_bcid->Integral(begin_BCID,limit_BCID))/(limit_BCID-begin_BCID);

    h_bcid->GetXaxis()->SetRange(begin_BCID,limit_BCID+10);

  }
  
  p1  = const_fit1/nEntry;
  x[0] = p1/(1.+p1)/time_bcid_bin;
  x[1] = const_fit1_e*pow((1.+p1),-2)/nEntry/time_bcid_bin;
  
  if(mode==1){
    wgConst *con = new wgConst;
    con->GetENV();
    string output = Form("%s/chip%d/NoiseRate%d_%d.png",wgFit::outputIMGDir.c_str(),i,i,j);
    GetHist->Print_bcid(output.c_str(),"",1);
    delete con; 
  }
  delete f1;
  delete h_bcid;
  delete h_spill;
}

//**********************************************************************
void wgFit::low_pe_charge(unsigned int i,unsigned int j,double* x,int mode=0){

  GetHist->Get_charge(i,j);

  if(h_charge->Integral(begin_low_pe,end_low_pe) < 1 )
  {
#ifdef DEBUG_WGFIT
    cout<< "!Warning!!no entry (chip:"<< i <<", ch:"<< j <<", col:" << k <<")"<< endl;
#endif
    x[0]=x[1]=x[2]=0.;
    return;
  } 
  
  h_charge->GetXaxis()->SetRange(begin_low_pe,end_low_pe);

  double px[2]={(double)h_charge->GetMaximumBin(),0.0};
  double py[2]={(double)h_charge->GetMaximum(),0.0};
  
  TF1* prefit = new TF1("gauss",gauss,px[0]-sigma*3,px[0]+sigma*3,3);
  prefit->SetLineColor(kGreen);
  prefit->SetNpx(500);
  prefit->SetParameters(py[0],px[0],sigma);
  prefit->SetParNames("Event0","mean0","sigma0");
  prefit->SetParLimits(0,0.1*py[0],1.1*py[0]);
  prefit->SetParLimits(1,px[0]-20,px[0]+20);
  prefit->SetParLimits(2,min_sigma,max_sigma);
  h_charge->Fit(prefit,"Q","P",px[0]-sigma*2,px[0]+sigma*2.0);
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
    GetHist->Print_charge(output.c_str(),"",1);
    delete con;
  }

  //delete ts;
  //delete fit;
  delete prefit;
  delete h_charge;
  return;  
}


//**********************************************************************
void wgFit::low_pe_charge_HG(unsigned int i,unsigned int j,unsigned int k,double* x,int mode=0){

  GetHist->Get_charge_hit_HG(i,j,k);

  if(h_charge_hit_HG->Integral(begin_low_pe_HG,end_low_pe_HG) < 1 )
  {
#ifdef DEBUG_WGFIT
    cout<< "!Warning!!no entry (chip:"<< i <<", ch:"<< j <<", col:" << k <<")"<< endl;
#endif
    x[0]=x[1]=x[2]=0.;
    return;
  } 
  
  h_charge_hit_HG->GetXaxis()->SetRange(begin_low_pe_HG,end_low_pe_HG);

  double px[2]={(double)h_charge_hit_HG->GetMaximumBin(),0.0};
  double py[2]={(double)h_charge_hit_HG->GetMaximum(),0.0};
  
  TF1* prefit = new TF1("gauss",gauss,px[0]-sigma*3,px[0]+sigma*3,3);
  prefit->SetLineColor(kGreen);
  prefit->SetNpx(500);
  prefit->SetParameters(py[0],px[0],sigma);
  prefit->SetParNames("Event0","mean0","sigma0");
  prefit->SetParLimits(0,0.1*py[0],1.1*py[0]);
  prefit->SetParLimits(1,px[0]-20,px[0]+20);
  prefit->SetParLimits(2,min_sigma,max_sigma);
  h_charge_hit_HG->Fit(prefit,"Q","P",px[0]-sigma*2,px[0]+sigma*2.0);
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
  delete h_charge_hit_HG;
  return;  
}

//**********************************************************************
void wgFit::charge_nohit(unsigned int i,unsigned int j,unsigned int k,double* x,int mode=0){
  GetHist->Get_charge_nohit(i,j,k);

  if(h_charge_nohit->Integral(begin_ped,end_ped) < 1 )
  {
#ifdef DEBUG_WGFIT 
    cout<< "!Warning!!no entry (chip:"<< i <<", ch:"<< j <<", col:" << k <<")"<< endl;
#endif
    x[0]=x[1]=x[2]=0.;
    return;
  } 
  
  h_charge_nohit->GetXaxis()->SetRange(begin_ped,end_ped);
  
  double px[1]={(double)h_charge_nohit->GetMaximumBin()};
  double py[1]={(double)h_charge_nohit->GetMaximum()};

  TF1* prefit = new TF1("gauss",gauss,px[0]-sigma*3,px[0]+sigma*3,3);
  prefit->SetLineColor(kViolet);
  prefit->SetNpx(500);
  prefit->SetParameters(py[0],px[0],sigma);
  prefit->SetParNames("Event0","mean0","sigma0");
  prefit->SetParLimits(0,0.9*py[0],1.1*py[0]);
  prefit->SetParLimits(1,px[0]-20,px[0]+20);
  prefit->SetParLimits(2,min_sigma,max_sigma);
  h_charge_nohit->Fit(prefit,"Q","",px[0]-sigma*2,px[0]+sigma*2.0);
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
  delete h_charge_nohit;
  return;  
}

//**********************************************************************
void wgFit::GainSelect(unsigned int i,unsigned int j,unsigned int k,double* x, int mode=0){
}

