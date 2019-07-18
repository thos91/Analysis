// system includes
#include <string>

// ROOT includes
#include "TH1I.h"
#include "TCanvas.h"
#include "TFile.h"

// user includes
#include "wgConst.hpp"
#include "wgFileSystemTools.hpp"
#include "wgGetHist.hpp"
#include "wgLogger.hpp"

using namespace wagasci_tools;

//************************************************************************
wgGetHist::wgGetHist(const std::string& str) {
  
  if ( !check_exist::RootFile(str) ) throw wgInvalidFile("[" + str + "][SetHistFile] failed to set histogram file");
  try { wgGetHist::freadhist = new TFile(str.c_str(),"read"); }
  catch (const std::exception& e) {
    Log.eWrite("[" + str + "][SetHistFile] failed to set histogram file : " + std::string(e.what()));
    throw;
  }
}

//************************************************************************
wgGetHist::~wgGetHist(){
  freadhist->Close();
}

//************************************************************************
bool wgGetHist::Get_charge_hit_HG(unsigned i, unsigned j, unsigned k){
  TString charge_hit_HG;
  charge_hit_HG.Form("charge_hit_HG_chip%u_ch%u_col%u", i, j, k);
  if (freadhist->GetListOfKeys()->Contains(charge_hit_HG)) {
    h_charge_hit_HG = (TH1I*)freadhist->Get(charge_hit_HG);
    return true;
  }
  else return false;
}

//************************************************************************
bool wgGetHist::Get_charge_hit_LG(unsigned i, unsigned j, unsigned k){
  TString charge_hit_LG;
  charge_hit_LG.Form("charge_hit_LG_chip%u_ch%u_col%u", i, j, k);
  if (freadhist->GetListOfKeys()->Contains(charge_hit_LG)) {
    h_charge_hit_LG = (TH1I*)freadhist->Get(charge_hit_LG);
    return true;
  }
  else return false;
}

//************************************************************************
bool wgGetHist::Get_charge_nohit(unsigned i, unsigned j, unsigned k){
  TString charge_nohit;
  charge_nohit.Form("charge_nohit_chip%u_ch%u_col%u", i, j, k);
  if ( freadhist->GetListOfKeys()->Contains(charge_nohit) ) {
    h_charge_nohit = (TH1I*)freadhist->Get(charge_nohit);
    return true;
  }
  else return false;
}

//************************************************************************
bool wgGetHist::Get_time_hit(unsigned i, unsigned j, unsigned k){
  TString time_hit;
  time_hit.Form("time_hit_chip%u_ch%u_col%u", i, j, k);
  if ( freadhist->GetListOfKeys()->Contains(time_hit) ) {
    h_time_hit = (TH1I*)freadhist->Get(time_hit);
    return true;
  }
  else return false;
}

//************************************************************************
bool wgGetHist::Get_time_nohit(unsigned i, unsigned j, unsigned k){
  TString time_nohit;
  time_nohit.Form("time_nohit_chip%u_ch%u_col%u", i, j, k);
  if ( freadhist->GetListOfKeys()->Contains(time_nohit) ) {
    h_time_nohit = (TH1I*)freadhist->Get(time_nohit);
    return true;
  }
  else return false;
}

//************************************************************************
bool wgGetHist::Get_charge_hit(unsigned i, unsigned j, unsigned k){
  TString charge_hit;
  charge_hit.Form("charge_hit_chip%u_ch%u_col%u", i, j, k);
  if ( freadhist->GetListOfKeys()->Contains(charge_hit) ) {
    h_charge_hit = (TH1I*) freadhist->Get(charge_hit);
  }
  else return false;
  // If you want the cumulative hits for all the columns uncomment this
  // TString charge_hit;
  // for (unsigned k = 1; k < MEMDEPTH; k++) {
  //       charge_hit.Form("charge_hit_chip%u_ch%u_col%u", i, j, k);
  //       if ( freadhist->GetListOfKeys()->Contains(charge_hit) ) {
  //         h_charge_hit->Add((TH1I*) freadhist->Get(charge_hit));
  //       }
  //       else return false;
  // }
  return true;
}

//************************************************************************
bool wgGetHist::Get_bcid_hit(unsigned i, unsigned j) {
  TString bcid_hit;
  bcid_hit.Form("bcid_hit_chip%u_ch%u", i, j);
  if ( freadhist->GetListOfKeys()->Contains(bcid_hit) ) {
    h_bcid_hit = (TH1I*) freadhist->Get(bcid_hit);
    return true;
  }
  else return false;
}

//************************************************************************
bool wgGetHist::Get_pe_hit(unsigned i, unsigned j) {
  TString pe_hit_0;
  pe_hit_0.Form("pe_hit_chip%u_ch%u_col0", i, j);
  if ( freadhist->GetListOfKeys()->Contains(pe_hit_0) ) {
    h_pe_hit = (TH1I*) freadhist->Get(pe_hit_0);
  }
  else return false;
  TString pe_hit;
  for (unsigned k = 1; k < MEMDEPTH; k++) {
    pe_hit.Form("pe_hit_chip%u_ch%u_col%u", i, j, k);
    if ( freadhist->GetListOfKeys()->Contains(pe_hit) ) {
      h_pe_hit->Add((TH1I*) freadhist->Get(pe_hit));
    }
    else return false;	
  }
  return true;
}

//************************************************************************
bool wgGetHist::Get_spill_count(){
  if ( freadhist->GetListOfKeys()->Contains("spill_count") ) {
    h_spill_count = (TH1I*) freadhist->Get("spill_count");
    return true;
  }
  else return false; 
}

//************************************************************************
int wgGetHist::Get_start_time(){
  if ( freadhist->GetListOfKeys()->Contains("start_time") ) {
    TH1I * h = (TH1I*) freadhist->Get("start_time");
    return h->GetBinLowEdge(h->GetMinimumBin())-1;
  }
  else return -1;
}

//************************************************************************
int wgGetHist::Get_stop_time(){
  if ( freadhist->GetListOfKeys()->Contains("stop_time") ) {
    TH1I * h = (TH1I*) freadhist->Get("stop_time");
    return h->GetBinLowEdge(h->GetMaximumBin());
  }
  else return -1;
}

//************************************************************************
TCanvas * wgGetHist::Make_Canvas(const char * name, bool y_logscale) {
  TCanvas * canvas = new TCanvas(name, name);
  canvas->SetCanvasSize(1024,768);
  if(y_logscale) canvas->SetLogy();
  return canvas;
}

//************************************************************************
void wgGetHist::Print_charge(const TString& h_name, const char* option, bool y_logscale){
  TCanvas * canvas = this->Make_Canvas("canvas", y_logscale);
  h_charge_hit->Draw(option);
  canvas->Print(h_name);
  delete canvas;
}

//************************************************************************
void wgGetHist::Print_charge_hit_HG(const TString& h_name, const char* option, bool y_logscale){
  TCanvas * canvas = this->Make_Canvas("canvas", y_logscale);
  h_charge_hit_HG->Draw(option);
  canvas->Print(h_name);
  delete canvas; 
}

//************************************************************************
void wgGetHist::Print_charge_hit_LG(const TString& h_name, const char* option, bool y_logscale){
  TCanvas * canvas = this->Make_Canvas("canvas", y_logscale);
  h_charge_hit_LG->Draw(option);
  canvas->Print(h_name);
  delete canvas; 
}

//************************************************************************
void wgGetHist::Print_charge_nohit(const TString& h_name, const char* option, bool y_logscale){
  TCanvas * canvas = this->Make_Canvas("canvas", y_logscale);
  h_charge_nohit->Draw(option);
  canvas->Print(h_name);
  delete canvas; 
}

//************************************************************************
void wgGetHist::Print_time_hit(const TString& h_name, const char* option, bool y_logscale){
  TCanvas * canvas = this->Make_Canvas("canvas", y_logscale);
  h_time_hit->Draw(option);
  canvas->Print(h_name);
  delete canvas; 
}

//************************************************************************
void wgGetHist::Print_time_nohit(const TString& h_name, const char* option, bool y_logscale){
  TCanvas * canvas = this->Make_Canvas("canvas", y_logscale);
  h_time_nohit->Draw(option);
  canvas->Print(h_name);
  delete canvas; 
}

//************************************************************************
void wgGetHist::Print_bcid(const TString& h_name, const char* option, bool y_logscale) {
  TCanvas * canvas = this->Make_Canvas("canvas", y_logscale);
  h_bcid_hit->Draw(option);
  canvas->Print(h_name);
  delete canvas; 
}

//************************************************************************
void wgGetHist::Print_pe(const TString& h_name, const char* option, bool y_logscale){
  TCanvas * canvas = this->Make_Canvas("canvas", y_logscale);
  h_pe_hit->Draw(option);
  canvas->Print(h_name);
  delete canvas; 
}

//************************************************************************
void wgGetHist::Print_spill(const TString& h_name, const char* option, bool y_logscale){
  TCanvas * canvas = this->Make_Canvas("canvas", y_logscale);
  h_spill_count->Draw(option);
  canvas->Print(h_name);
  delete canvas; 
}
