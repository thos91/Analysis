#ifndef WG_GETHIST_H_INCLUDE
#define WG_GETHIST_H_INCLUDE

// system includes
#include <cstdbool>

// ROOT includes
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TFile.h"

// user includes
#include "wgErrorCode.hpp"
#include "wgConst.hpp"
#include "wgFileSystemTools.hpp"

using namespace std;

class wgGetHist
{
  friend class wgFit;

private:
  string finputname;
  TFile *freadhist;
  TCanvas *c1;
  TH1D* h_spill;
  TH1D* h_charge_hit;    // [n_chips][n_channels][MEMDEPTH];
  TH1D* h_charge_hit_HG; // [n_chips][n_channels][MEMDEPTH];
  TH1D* h_charge_hit_LG; // [n_chips][n_channels][MEMDEPTH];
  TH1D* h_pe_hit;        // [n_chips][n_channels][MEMDEPTH];
  TH1D* h_charge_nohit;  // [n_chips][n_channels][MEMDEPTH];
  TH1D* h_time_hit;      // [n_chips][n_channels][MEMDEPTH];
  TH1D* h_time_nohit;    // [n_chips][n_channels][MEMDEPTH];
  TH1D* h_bcid_hit;      // [n_chips][n_channels];

public:
  //wgGetHist::wgGetHist
  // Open the ROOT file created by the wgMakeHist program and assign the TFile
  // pointer to the freadhist member. Exceptions may be thrown.
  wgGetHist(const string&);
  // Calls the clear method and then closes the ROOT file freadhist
  ~wgGetHist();

  // wgGetHist::Get methods 
  // They just read an histogram into the correspondent member. If the histogram
  // could not be found they return false, otherwise they return true.
  bool Get_charge_hit_HG(unsigned int i, unsigned int j, unsigned int k);
  bool Get_charge_hit_LG(unsigned int i, unsigned int j, unsigned int k);
  bool Get_charge_nohit (unsigned int i, unsigned int j, unsigned int k);
  bool Get_charge_hit   (unsigned int i, unsigned int j, unsigned int k);
  bool Get_time_hit     (unsigned int i, unsigned int j, unsigned int k);
  bool Get_time_nohit   (unsigned int i, unsigned int j, unsigned int k);
  bool Get_bcid_hit     (unsigned int i, unsigned int j);
  bool Get_pe_hit       (unsigned int i, unsigned int j);
  bool Get_spill();
  // Return the value of the start_time or stop_time. These values are read from
  // the corresponding histograms. If the histograms could not be found they
  // return a negative value.
  int  Get_start_time();
  int  Get_stop_time();

  TCanvas * Make_Canvas(const char * name, bool y_logscale);
  void Print_charge       (const TString& h_name, const char* option="", bool y_logscale = false);
  void Print_charge_hit_HG(const TString& h_name, const char* option="", bool y_logscale = false);
  void Print_charge_hit_LG(const TString& h_name, const char* option="", bool y_logscale = false);
  void Print_charge_nohit (const TString& h_name, const char* option="", bool y_logscale = false);
  void Print_time_hit     (const TString& h_name, const char* option="", bool y_logscale = false);
  void Print_time_nohit   (const TString& h_name, const char* option="", bool y_logscale = false);
  void Print_bcid         (const TString& h_name, const char* option="", bool y_logscale = false);
  void Print_pe           (const TString& h_name, const char* option="", bool y_logscale = false);
  void Print_spill        (const TString& h_name, const char* option="", bool y_logscale = false);
};

#endif
