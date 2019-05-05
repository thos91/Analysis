#ifndef WG_GETHIST_H_INCLUDE
#define WG_GETHIST_H_INCLUDE

#include "TH1.h"
#include "TH2.h"
#include "TROOT.h"
#include "TFile.h"
#include "wgErrorCode.h"
#include "Const.h"
#include "wgTools.h"

using namespace std;

class wgGetHist
{
  friend class wgFit;

private:
  string finputname;
  TFile *freadhist;
  TCanvas *c1;
  TH1F* h_spill;
  TH1F* h_charge_hit;    // [n_chips][n_channels][MEMDEPTH];
  TH1F* h_charge_hit_HG; // [n_chips][n_channels][MEMDEPTH];
  TH1F* h_charge_hit_LG; // [n_chips][n_channels][MEMDEPTH];
  TH1F* h_pe_hit;        // [n_chips][n_channels][MEMDEPTH];
  TH1F* h_charge_nohit;  // [n_chips][n_channels][MEMDEPTH];
  TH1F* h_time_hit;      // [n_chips][n_channels][MEMDEPTH];
  TH1F* h_time_nohit;    // [n_chips][n_channels][MEMDEPTH];
  TH1F* h_bcid_hit;      // [n_chips][n_channels];

public:
  //wgGetHist::wgGetHist
  // Open the ROOT file created by the wgMakeHist program and assign the TFile
  // pointer to the freadhist member. Exceptions may be thrown.
  wgGetHist(const string&);
  // Calls the clear method and then closes the ROOT file freadhist
  ~wgGetHist();

  // wgGetHist::clear
  // free all the memory allocated for the histograms
  void clear();

  // wgGetHist::Get methods
  // They just read an histogram into the correspondent member
  void Get_charge_hit_HG(unsigned int i,unsigned int j,unsigned int k);
  void Get_charge_hit_LG(unsigned int i,unsigned int j,unsigned int k);
  void Get_charge_nohit(unsigned int i,unsigned int j,unsigned int k);
  void Get_time_hit(unsigned int i,unsigned int j,unsigned int k);
  void Get_time_nohit(unsigned int i,unsigned int j,unsigned int k);
  void Get_charge_hit(unsigned int i,unsigned int j);
  void Get_bcid_hit(unsigned int i,unsigned int j);
  void Get_pe_hit(unsigned int i,unsigned int j);
  void Get_spill();

  void Make_Canvas(int opt);
  void Print_charge(const char*, const char*,int);
  void Print_charge_hit_HG(const char*, const char*,int);
  void Print_charge_hit_LG(const char*, const char*,int);
  void Print_charge_nohit(const char*, const char*,int);
  void Print_time_hit(const char*, const char*,int);
  void Print_time_nohit(const char*, const char*,int);
  void Print_bcid(const char*, const char*,int);
  void Print_pe(const char*, const char*,int);
  void Print_spill(const char*, const char*,int);
  int  Get_start_time();
  int  Get_stop_time();
};

#endif
