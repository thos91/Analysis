#include "TH1.h"
#include "TH2.h"
#include "TROOT.h"
#include "TFile.h"
#include "TCanvas.h"
#include "wgErrorCode.h"
#include "Const.h"
#include "wgTools.h"
#include "wgGetHist.h"

using namespace std;

//************************************************************************
wgGetHist::wgGetHist(const string& str) {
  CheckExist Check;
  if ( !Check.RootFile(str) ) throw wgInvalidFile("[" + str + "][SetHistFile] failed to set histogram file");
    try { wgGetHist::freadhist = new TFile(str.c_str(),"read"); }
  catch (const exception& e) {
	Log.eWrite("[" + str + "][SetHistFile] failed to set histogram file : " + string(e.what()));
	throw;
  }
}

//************************************************************************
wgGetHist::~wgGetHist(){
  this->clear();
  freadhist->Close();
}

//************************************************************************
void wgGetHist::clear(){  
  if(h_charge_hit) delete h_charge_hit; 
  if(h_charge_hit_HG) delete h_charge_hit_HG; 
  if(h_charge_hit_LG) delete h_charge_hit_LG;  
  if(h_charge_nohit) delete h_charge_nohit;  
  if(h_time_hit) delete h_time_hit; 
  if(h_time_nohit) delete h_time_nohit;  
  if(h_bcid_hit) delete h_bcid_hit;  
  if(h_pe_hit) delete h_pe_hit;  
  if(h_spill) delete h_spill;  
  if(c1) delete c1;
}

//************************************************************************
void wgGetHist::Get_charge_hit_HG(unsigned int i,unsigned int j,unsigned int k){
  if (h_charge_hit_HG != NULL) delete h_charge_hit_HG;
  h_charge_hit_HG=(TH1D*)freadhist->Get(Form("charge_hit_HG_chip%u_ch%u_col%u",i,j,k));
}

//************************************************************************
void wgGetHist::Get_charge_hit_LG(unsigned int i,unsigned int j,unsigned int k){
  if (h_charge_hit_LG != NULL ) delete h_charge_hit_LG; 
  h_charge_hit_LG=(TH1D*)freadhist->Get(Form("charge_hit_LG_chip%u_ch%u_col%u",i,j,k));
}

//************************************************************************
void wgGetHist::Get_charge_nohit(unsigned int i,unsigned int j,unsigned int k){
   if (h_charge_nohit != NULL ) delete h_charge_nohit;
  h_charge_nohit=(TH1D*)freadhist->Get(Form("charge_nohit_chip%u_ch%u_col%u",i,j,k));
}

//************************************************************************
void wgGetHist::Get_time_hit(unsigned int i,unsigned int j,unsigned int k){
   if (h_time_hit != NULL ) delete h_time_hit;
  h_time_hit=(TH1D*)freadhist->Get(Form("time_hit_chip%u_ch%u_col%u",i,j,k));
}

//************************************************************************
void wgGetHist::Get_time_nohit(unsigned int i,unsigned int j,unsigned int k){
   if (h_time_nohit != NULL ) delete h_time_nohit;
  h_time_nohit=(TH1D*)freadhist->Get(Form("time_nohit_chip%u_ch%u_col%u",i,j,k));
}

//************************************************************************
void wgGetHist::Get_charge_hit(unsigned int i,unsigned int j){
   if (h_charge_hit != NULL ) delete h_charge_hit;
  h_charge_hit = (TH1D*) freadhist->Get(Form("charge_hit_chip%u_ch%u_col0",i,j));
  for (unsigned k = 1; k < MEMDEPTH; k++) {
	  h_charge_hit->Add((TH1D*) freadhist->Get(Form("charge_hit_chip%u_ch%u_col%u",i,j,k)));
  }
}

//************************************************************************
void wgGetHist::Get_bcid_hit(unsigned int i,unsigned int j){
  if (h_bcid_hit != NULL) delete h_bcid_hit;
  h_bcid_hit=(TH1D*)freadhist->Get(Form("bcid_hit_chip%u_ch%u",i,j));
}

//************************************************************************
void wgGetHist::Get_pe_hit(unsigned int i,unsigned int j){
   if (h_pe_hit != NULL ) delete h_pe_hit;
  h_pe_hit = (TH1D*) freadhist->Get(Form("pe_hit_chip%u_ch%u_col0",i,j));
  for (unsigned k = 1; k < MEMDEPTH; k++) {
	h_pe_hit->Add((TH1D*) freadhist->Get(Form("pe_hit_chip%u_ch%u_col%u",i,j,k)));
  }
}

//************************************************************************
void wgGetHist::Get_spill(){
   if (h_spill != NULL ) delete h_spill;
  h_spill=(TH1D*)freadhist->Get(Form("spill"));
}

//************************************************************************
void wgGetHist::Make_Canvas(int opt=0){
  c1 = new TCanvas("c1","c1");
  if(opt!=0) c1->SetLogy();
}

//************************************************************************
void wgGetHist::Print_charge(const char* h_name,const char* option="", int opt=0){
  this->Make_Canvas(opt);
  h_charge_hit->Draw(option);
  c1->Print(h_name); 
}

//************************************************************************
void wgGetHist::Print_charge_hit_HG(const char* h_name,const char* option="", int opt=0){
  this->Make_Canvas(opt);
  h_charge_hit_HG->Draw(option);
  c1->Print(h_name); 
}

//************************************************************************
void wgGetHist::Print_charge_hit_LG(const char* h_name,const char* option="", int opt=0){
  this->Make_Canvas(opt);
  h_charge_hit_LG->Draw(option);
  c1->Print(h_name); 
}

//************************************************************************
void wgGetHist::Print_charge_nohit(const char* h_name,const char* option="", int opt=0){
  this->Make_Canvas(opt);
  h_charge_nohit->Draw(option);
  c1->Print(h_name); 
}

//************************************************************************
void wgGetHist::Print_time_hit(const char* h_name,const char* option="", int opt=0){
  this->Make_Canvas(opt);
  h_time_hit->Draw(option);
  c1->Print(h_name); 
}

//************************************************************************
void wgGetHist::Print_time_nohit(const char* h_name,const char* option="", int opt=0){
  this->Make_Canvas(opt);
  h_time_nohit->Draw(option);
  c1->Print(h_name); 
}

//************************************************************************
void wgGetHist::Print_bcid(const char* h_name,const char* option="", int opt=0){
  this->Make_Canvas(opt);
  h_bcid_hit->Draw(option);
  c1->Print(h_name); 
}

//************************************************************************
void wgGetHist::Print_pe(const char* h_name,const char* option="", int opt=0){
  this->Make_Canvas(opt);
  h_pe_hit->Draw(option);
  c1->Print(h_name); 
}

//************************************************************************
void wgGetHist::Print_spill(const char* h_name,const char* option="", int opt=0){
  this->Make_Canvas(opt);
  h_spill->Draw(option);
  c1->Print(h_name); 
}

//************************************************************************
int wgGetHist::Get_start_time(){
  TH1D *h;
  h=(TH1D*)freadhist->Get("start_time");
  return h->GetBinLowEdge(h->GetMinimumBin())-1; 
}

//************************************************************************
int wgGetHist::Get_stop_time(){
  TH1D *h;
  h=(TH1D*)freadhist->Get("stop_time");
  return h->GetBinLowEdge(h->GetMaximumBin()); 
}

