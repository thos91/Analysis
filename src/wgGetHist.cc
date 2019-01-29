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

TH1F* h_charge;
TH1F* h_charge_hit_HG;
TH1F* h_charge_hit_LG;
TH1F* h_charge_nohit;
TH1F* h_time_hit;
TH1F* h_time_nohit;
TH1F* h_bcid;
TH1F* h_pe;
TH1F* h_spill;

TFile* wgGetHist::freadhist;
string wgGetHist::finputname;

//************************************************************************
wgGetHist::wgGetHist()
{
}

//************************************************************************
wgGetHist::wgGetHist(string& str){
  this->SetHistFile(str);
}

//************************************************************************
wgGetHist::~wgGetHist(){
  freadhist->Close();
}

//************************************************************************
bool wgGetHist::SetHistFile(string& str){
  CheckExist *Check = new CheckExist;
  wgGetHist::finputname = str;
  if(!Check->RootFile(str)){
    cout << "ERROR!! FAIL TO SET HISTFILE!!" <<endl;
    return false;
  }
  wgGetHist::freadhist = new TFile(str.c_str(),"read");
  delete Check;
  return true;
}

void wgGetHist::clear(){  
  if(h_charge) delete h_charge; 
  if(h_charge_hit_HG) delete h_charge_hit_HG; 
  if(h_charge_hit_LG) delete h_charge_hit_LG;  
  if(h_charge_nohit) delete h_charge_nohit;  
  if(h_time_hit) delete h_time_hit; 
  if(h_time_nohit) delete h_time_nohit;  
  if(h_bcid) delete h_bcid;  
  if(h_pe) delete h_pe;  
  if(h_spill) delete h_spill;  
  if(c1) delete c1;

}

//************************************************************************
void wgGetHist::Get_charge_hit_HG(unsigned int i,unsigned int j,unsigned int k){
  h_charge_hit_HG=(TH1F*)freadhist->Get(Form("charge_hit_HG_chip%d_ch%d_col%d",i,j,k));
}

//************************************************************************
void wgGetHist::Get_charge_hit_LG(unsigned int i,unsigned int j,unsigned int k){
  h_charge_hit_LG=(TH1F*)freadhist->Get(Form("charge_hit_LG_chip%d_ch%d_col%d",i,j,k));
}

//************************************************************************
void wgGetHist::Get_charge_nohit(unsigned int i,unsigned int j,unsigned int k){
  h_charge_nohit=(TH1F*)freadhist->Get(Form("charge_nohit_chip%d_ch%d_col%d",i,j,k));
}

//************************************************************************
void wgGetHist::Get_time_hit(unsigned int i,unsigned int j,unsigned int k){
  h_time_hit=(TH1F*)freadhist->Get(Form("time_hit_chip%d_ch%d_col%d",i,j,k));
}

//************************************************************************
void wgGetHist::Get_time_nohit(unsigned int i,unsigned int j,unsigned int k){
  h_time_nohit=(TH1F*)freadhist->Get(Form("time_nohit_chip%d_ch%d_col%d",i,j,k));
}

//************************************************************************
void wgGetHist::Get_charge(unsigned int i,unsigned int j){
  h_charge=(TH1F*)freadhist->Get(Form("charge_chip%d_ch%d",i,j));
}

//************************************************************************
void wgGetHist::Get_bcid(unsigned int i,unsigned int j){
  h_bcid=(TH1F*)freadhist->Get(Form("bcid_chip%d_ch%d",i,j));
}

//************************************************************************
void wgGetHist::Get_pe(unsigned int i,unsigned int j){
  h_pe=(TH1F*)freadhist->Get(Form("pe_chip%d_ch%d",i,j));
}

//************************************************************************
void wgGetHist::Get_spill(){
  h_spill=(TH1F*)freadhist->Get(Form("spill"));
}

//************************************************************************
void wgGetHist::Make_Canvas(int opt=0){
  c1 = new TCanvas("c1","c1");
  if(opt!=0) c1->SetLogy();
}

//************************************************************************
void wgGetHist::Print_charge(const char* h_name,const char* option="", int opt=0){
  this->Make_Canvas(opt);
  h_charge->Draw(option);
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
  h_bcid->Draw(option);
  c1->Print(h_name); 
}

//************************************************************************
void wgGetHist::Print_pe(const char* h_name,const char* option="", int opt=0){
  this->Make_Canvas(opt);
  h_pe->Draw(option);
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
  TH1F *h;
  h=(TH1F*)freadhist->Get("start_time");
  return h->GetBinLowEdge(h->GetMinimumBin())-1; 
}

//************************************************************************
int wgGetHist::Get_stop_time(){
  TH1F *h;
  h=(TH1F*)freadhist->Get("stop_time");
  return h->GetBinLowEdge(h->GetMaximumBin()); 
}

