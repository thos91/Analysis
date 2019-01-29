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

extern TH1F* h_charge;
extern TH1F* h_charge_hit_HG;
extern TH1F* h_charge_hit_LG;
extern TH1F* h_charge_nohit;
extern TH1F* h_time_hit;
extern TH1F* h_time_nohit;
extern TH1F* h_bcid;
extern TH1F* h_pe;
extern TH1F* h_spill;

class wgGetHist
{
private:
  static string finputname;
  static TFile *freadhist;
  TCanvas *c1;
public:
  wgGetHist();
  wgGetHist(string&);
  ~wgGetHist();
  bool SetHistFile(string&);
  void clear();
  void Get_charge_hit_HG(unsigned int i,unsigned int j,unsigned int k);
  void Get_charge_hit_LG(unsigned int i,unsigned int j,unsigned int k);
  void Get_charge_nohit(unsigned int i,unsigned int j,unsigned int k);
  void Get_time_hit(unsigned int i,unsigned int j,unsigned int k);
  void Get_time_nohit(unsigned int i,unsigned int j,unsigned int k);
  void Get_charge(unsigned int i,unsigned int j);
  void Get_bcid(unsigned int i,unsigned int j);
  void Get_pe(unsigned int i,unsigned int j);
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
