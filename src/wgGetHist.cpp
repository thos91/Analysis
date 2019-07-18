// system includes
#include <string>

// ROOT includes
#include "TH1I.h"
#include "TCanvas.h"
#include "TFile.h"

// user includes
#include "wgConst.hpp"
#include "wgFileSystemTools.hpp"
#include "wgLogger.hpp"
#include "wgGetHist.hpp"

using namespace wagasci_tools;

class RootSideEffectGuard {
public:
  RootSideEffectGuard(): m_directory(gDirectory) { gDirectory = 0; }
  ~RootSideEffectGuard() { gDirectory = m_directory; }
private:
  TDirectory* m_directory;
};

//************************************************************************
wgGetHist::wgGetHist(const std::string& hist_file) {
  if (!check_exist::RootFile(hist_file))
    throw wgInvalidFile("[wgGetHist] histogram file not found : " + hist_file);
  try { wgGetHist::m_hist_file = new TFile(hist_file.c_str(),"read"); }
  catch (const std::exception& e) {
    throw wgInvalidFile("[wgGetHist] failed to open histogram file : " +
               std::string(e.what()) + " : " + hist_file);
    this->Get_spill_count();
  }
}

//************************************************************************
wgGetHist::~wgGetHist(){
  m_hist_file->Close();
}

//************************************************************************
TH1I * wgGetHist::Get_charge_hit_HG(unsigned i, unsigned j, unsigned k){
  TH1I * h_charge_hit_HG = NULL;
  TString charge_hit_HG;
  charge_hit_HG.Form("charge_hit_HG_chip%u_ch%u_col%u", i, j, k);
  if (m_hist_file->GetListOfKeys()->Contains(charge_hit_HG)) {
    RootSideEffectGuard root_guard;
    h_charge_hit_HG = (TH1I*) m_hist_file->Get(charge_hit_HG);
  }
  return h_charge_hit_HG;
}

//************************************************************************
TH1I * wgGetHist::Get_charge_hit_LG(unsigned i, unsigned j, unsigned k) {
  TH1I * h_charge_hit_LG = NULL;
  TString charge_hit_LG;
  charge_hit_LG.Form("charge_hit_LG_chip%u_ch%u_col%u", i, j, k);
  if (m_hist_file->GetListOfKeys()->Contains(charge_hit_LG)) {
    RootSideEffectGuard root_guard;
    h_charge_hit_LG = (TH1I*) m_hist_file->Get(charge_hit_LG);
  }
  return h_charge_hit_LG;
}

//************************************************************************
TH1I * wgGetHist::Get_charge_nohit(unsigned i, unsigned j, unsigned k) {
  TH1I * h_charge_nohit = NULL;
  TString charge_nohit;
  charge_nohit.Form("charge_nohit_chip%u_ch%u_col%u", i, j, k);
  if (m_hist_file->GetListOfKeys()->Contains(charge_nohit)) {
    RootSideEffectGuard root_guard;
    h_charge_nohit = (TH1I*) m_hist_file->Get(charge_nohit);
  }
  return h_charge_nohit;
}

//************************************************************************
TH1I * wgGetHist::Get_time_hit(unsigned i, unsigned j, unsigned k) {
  TH1I * h_time_hit = NULL;
  TString time_hit;
  time_hit.Form("time_hit_chip%u_ch%u_col%u", i, j, k);
  if (m_hist_file->GetListOfKeys()->Contains(time_hit)) {
    RootSideEffectGuard root_guard;
    h_time_hit = (TH1I*) m_hist_file->Get(time_hit);
  }
  return h_time_hit;
}

//************************************************************************
TH1I * wgGetHist::Get_time_nohit(unsigned i, unsigned j, unsigned k) {
  TH1I * h_time_nohit = NULL;
  TString time_nohit;
  time_nohit.Form("time_nohit_chip%u_ch%u_col%u", i, j, k);
  if (m_hist_file->GetListOfKeys()->Contains(time_nohit)) {
    RootSideEffectGuard root_guard;
    h_time_nohit = (TH1I*) m_hist_file->Get(time_nohit);
  }
  return h_time_nohit;
}

//************************************************************************
TH1I * wgGetHist::Get_charge_hit(unsigned i, unsigned j, unsigned k) {
  TH1I * h_charge_hit = NULL;
  TString charge_hit;
  charge_hit.Form("charge_hit_chip%u_ch%u_col%u", i, j, k);
  if (m_hist_file->GetListOfKeys()->Contains(charge_hit)) {
    RootSideEffectGuard root_guard;
    h_charge_hit = (TH1I*) m_hist_file->Get(charge_hit);
  }
  return h_charge_hit;
}

//************************************************************************
TH1I * wgGetHist::Get_bcid_hit(unsigned i, unsigned j) {
  TH1I * h_bcid_hit = NULL;
  TString bcid_hit;
  bcid_hit.Form("bcid_hit_chip%u_ch%u", i, j);
  if (m_hist_file->GetListOfKeys()->Contains(bcid_hit)) {
    RootSideEffectGuard root_guard;
    h_bcid_hit = (TH1I*) m_hist_file->Get(bcid_hit);
  }
  return h_bcid_hit;
}

//************************************************************************
void wgGetHist::Get_spill_count() {
  if (m_hist_file->GetListOfKeys()->Contains("spill_count")) {
    TArrayI * a_spill_count;
    m_hist_file->GetObject("spill_count", a_spill_count);
    wgGetHist::spill_count = a_spill_count->At(0);
  }
}

//************************************************************************
int wgGetHist::Get_start_time(){
  if (m_hist_file->GetListOfKeys()->Contains("start_time")) {
    TH1I * h = (TH1I*) m_hist_file->Get("start_time");
    return h->GetBinLowEdge(h->GetMinimumBin()) - 1;
  }
  else return -1;
}

//************************************************************************
int wgGetHist::Get_stop_time(){
  if ( m_hist_file->GetListOfKeys()->Contains("stop_time") ) {
    TH1I * h = (TH1I*) m_hist_file->Get("stop_time");
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
