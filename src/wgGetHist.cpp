// system includes
#include <string>

// ROOT includes
#include "TH1I.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TParameter.h"

// user includes
#include "wgConst.hpp"
#include "wgFileSystemTools.hpp"
#include "wgLogger.hpp"
#include "wgGetHist.hpp"

using namespace wagasci_tools;

//************************************************************************
wgGetHist::wgGetHist(const std::string& hist_file) {
  if (!check_exist::RootFile(hist_file))
    throw wgInvalidFile("[wgGetHist] histogram file not found : " + hist_file);
  try { wgGetHist::m_hist_file = new TFile(hist_file.c_str(),"read"); }
  catch (const std::exception& e) {
    throw wgInvalidFile("[wgGetHist] failed to open histogram file : " +
               std::string(e.what()) + " : " + hist_file);
  }
  this->Get_spill_count();
}

//************************************************************************
wgGetHist::~wgGetHist(){
  m_hist_file->Close();
}

//************************************************************************
TH1I * wgGetHist::Get_charge_hit_HG(unsigned i, unsigned j, unsigned k){
  TString charge_hit_HG;
  charge_hit_HG.Form("charge_hit_HG_chip%u_ch%u_col%u", i, j, k);
  if (m_hist_file->GetListOfKeys()->Contains(charge_hit_HG))
    return (TH1I*) m_hist_file->Get(charge_hit_HG);
  else
    return NULL;
}

//************************************************************************
TH1I * wgGetHist::Get_charge_hit_LG(unsigned i, unsigned j, unsigned k) {
  TString charge_hit_LG;
  charge_hit_LG.Form("charge_hit_LG_chip%u_ch%u_col%u", i, j, k);
  if (m_hist_file->GetListOfKeys()->Contains(charge_hit_LG))
    return (TH1I*) m_hist_file->Get(charge_hit_LG);
  else
    return NULL;
}

//************************************************************************
TH1I * wgGetHist::Get_charge_nohit(unsigned i, unsigned j, unsigned k) {
  TString charge_nohit;
  charge_nohit.Form("charge_nohit_chip%u_ch%u_col%u", i, j, k);
  if (m_hist_file->GetListOfKeys()->Contains(charge_nohit))
    return (TH1I*) m_hist_file->Get(charge_nohit);
  else
    return NULL;
}

//************************************************************************
TH1I * wgGetHist::Get_time_hit(unsigned i, unsigned j, unsigned k) {
  TString time_hit;
  time_hit.Form("time_hit_chip%u_ch%u_col%u", i, j, k);
  if (m_hist_file->GetListOfKeys()->Contains(time_hit))
    return (TH1I*) m_hist_file->Get(time_hit);
  else
    return NULL;
}

//************************************************************************
TH1I * wgGetHist::Get_time_nohit(unsigned i, unsigned j, unsigned k) {
  TString time_nohit;
  time_nohit.Form("time_nohit_chip%u_ch%u_col%u", i, j, k);
  if (m_hist_file->GetListOfKeys()->Contains(time_nohit))
    return (TH1I*) m_hist_file->Get(time_nohit);
  else
    return NULL;
}

//************************************************************************
TH1I * wgGetHist::Get_charge_hit(unsigned i, unsigned j, unsigned k) {
  TString charge_hit;
  charge_hit.Form("charge_hit_chip%u_ch%u_col%u", i, j, k);
  if (m_hist_file->GetListOfKeys()->Contains(charge_hit))
    return (TH1I*) m_hist_file->Get(charge_hit);
  else
    return NULL;
}

//************************************************************************
TH1I * wgGetHist::Get_bcid_hit(unsigned i, unsigned j) {
  TString bcid_hit;
  bcid_hit.Form("bcid_hit_chip%u_ch%u", i, j);
  if (m_hist_file->GetListOfKeys()->Contains(bcid_hit))
    return (TH1I*) m_hist_file->Get(bcid_hit);
  else
    return NULL;
}

//************************************************************************
void wgGetHist::Get_spill_count() {
  if (m_hist_file->GetListOfKeys()->Contains("spill_count")) {
    TParameter<int> * p_spill_count;
    m_hist_file->GetObject("spill_count", p_spill_count);
    wgGetHist::spill_count = p_spill_count->GetVal();
  } else {
    wgGetHist::spill_count = -1;
  }
}

//************************************************************************
int wgGetHist::Get_start_time() {
  if (m_hist_file->GetListOfKeys()->Contains("start_time")) {
    TParameter<int> * p_start_time;
    m_hist_file->GetObject("start_time", p_start_time);
    return p_start_time->GetVal();
  }
  else return -1;
}

//************************************************************************
int wgGetHist::Get_stop_time() {
  if (m_hist_file->GetListOfKeys()->Contains("stop_time")) {
    TParameter<int> * p_stop_time;
    m_hist_file->GetObject("stop_time", p_stop_time);
    return p_stop_time->GetVal();
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
void wgGetHist::Print_charge(const TString& h_name, TH1I * h_charge_hit, const char* option, bool y_logscale) {
  TCanvas * canvas = this->Make_Canvas("canvas", y_logscale);
  h_charge_hit->Draw(option);
  canvas->Print(h_name);
  delete canvas;
}

//************************************************************************
void wgGetHist::Print_charge_hit_HG(const TString& h_name, TH1I * h_charge_hit_HG, const char* option, bool y_logscale){
  TCanvas * canvas = this->Make_Canvas("canvas", y_logscale);
  h_charge_hit_HG->Draw(option);
  canvas->Print(h_name);
  delete canvas; 
}

//************************************************************************
void wgGetHist::Print_charge_hit_LG(const TString& h_name, TH1I * h_charge_hit_LG, const char* option, bool y_logscale) {
  TCanvas * canvas = this->Make_Canvas("canvas", y_logscale);
  h_charge_hit_LG->Draw(option);
  canvas->Print(h_name);
  delete canvas; 
}

//************************************************************************
void wgGetHist::Print_charge_nohit(const TString& h_name, TH1I * h_charge_nohit, const char* option, bool y_logscale) {
  TCanvas * canvas = this->Make_Canvas("canvas", y_logscale);
  h_charge_nohit->Draw(option);
  canvas->Print(h_name);
  delete canvas; 
}

//************************************************************************
void wgGetHist::Print_time_hit(const TString& h_name, TH1I * h_time_hit, const char* option, bool y_logscale) {
  TCanvas * canvas = this->Make_Canvas("canvas", y_logscale);
  h_time_hit->Draw(option);
  canvas->Print(h_name);
  delete canvas; 
}

//************************************************************************
void wgGetHist::Print_time_nohit(const TString& h_name, TH1I * h_time_nohit, const char* option, bool y_logscale) {
  TCanvas * canvas = this->Make_Canvas("canvas", y_logscale);
  h_time_nohit->Draw(option);
  canvas->Print(h_name);
  delete canvas; 
}

//************************************************************************
void wgGetHist::Print_bcid(const TString& h_name, TH1I * h_bcid_hit, const char* option, bool y_logscale) {
  TCanvas * canvas = this->Make_Canvas("canvas", y_logscale);
  h_bcid_hit->Draw(option);
  canvas->Print(h_name);
  delete canvas; 
}
