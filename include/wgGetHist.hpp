#ifndef WG_GETHIST_H_INCLUDE
#define WG_GETHIST_H_INCLUDE

// system includes
#include <cstdbool>

// ROOT includes
#include "TH1I.h"
#include "TCanvas.h"
#include "TFile.h"

// user includes
#include "wgConst.hpp"
#include "wgFileSystemTools.hpp"

class wgGetHist
{
private:
  TFile *m_hist_file;

  // get the number of spills contained in the hist_file and assign it
  // to the spill_count member
  void Get_spill_count();

  // Create a ROOT canvas and return a pointer to it
  TCanvas * Make_Canvas(const char * name, bool y_logscale);
  
public:
  unsigned spill_count; // spill count
  
  // Open the ROOT file created by the wgMakeHist program and assign
  // the TFile pointer to the m_freadhist member. Get the spill count.
  // wgInvalidFile exception may be thrown.
  wgGetHist(const std::string& hist_file);

  // Closes the ROOT file m_freadhist
  ~wgGetHist();

  // wgGetHist::Get methods 
  // They just read an histogram into the correspondent member. If the histogram
  // could not be found they return false, otherwise they return true.
  TH1I * Get_charge_hit_HG(unsigned int i, unsigned int j, unsigned int k);
  TH1I * Get_charge_hit_LG(unsigned int i, unsigned int j, unsigned int k);
  TH1I * Get_charge_nohit (unsigned int i, unsigned int j, unsigned int k);
  TH1I * Get_time_hit     (unsigned int i, unsigned int j, unsigned int k);
  TH1I * Get_time_nohit   (unsigned int i, unsigned int j, unsigned int k);
  TH1I * Get_bcid_hit     (unsigned int i, unsigned int j);
  TH1I * Get_pe_hit       (unsigned int i, unsigned int j);

  // Return the value of the start_time or stop_time. These values are read from
  // the corresponding histograms. If the histograms could not be found they
  // return a negative value.
  int  Get_start_time();
  int  Get_stop_time();

  // Print the histograms
  void Print_charge_hit_HG(const TString& h_name, TH1I * h, const char* option="", bool y_logscale = false);
  void Print_charge_hit_LG(const TString& h_name, TH1I * h, const char* option="", bool y_logscale = false);
  void Print_charge_nohit (const TString& h_name, TH1I * h, const char* option="", bool y_logscale = false);
  void Print_time_hit     (const TString& h_name, TH1I * h, const char* option="", bool y_logscale = false);
  void Print_time_nohit   (const TString& h_name, TH1I * h, const char* option="", bool y_logscale = false);
  void Print_bcid         (const TString& h_name, TH1I * h, const char* option="", bool y_logscale = false);
};

#endif
