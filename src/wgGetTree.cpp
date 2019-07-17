// system includes
#include <iostream>

// ROOT includes
#include "TH1D.h"

// user includes
#include "wgGetTree.hpp"
#include "wgFileSystemTools.hpp"

#include "wgConst.hpp"
#include "wgLogger.hpp"

using namespace wagasci_tools;

//************************************************************************

wgGetTree::wgGetTree(const std::string& root_file_name, Raw_t& rd) {
  this->Open(root_file_name);
  this->SetTreeFile(rd);
}

//************************************************************************
void wgGetTree::Open(const std::string& root_file_name) {
  // Check if the ROOT file exists
  
  wgGetTree::finputname = root_file_name;
  if(!check_exist::RootFile(root_file_name))
    throw wgInvalidFile( "[wgGetTree::Open] failed to open " + root_file_name );
  // Check if the ROOT file is already opened
  try {
    if ( wgGetTree::finput != NULL ) {
      if ( wgGetTree::finput->IsOpen() == kTRUE ) {
        Log.Write( "[wgGetTree::Open] " + root_file_name + "is already opened" );
        this->Close();
      }
    }
  } catch (...) {}
  // Open the ROOT file and get the TTree called "tree"
  // Store the TFile in wgGetTree::finput and TTree in wgGetTree::tree
  wgGetTree::finput = new TFile(root_file_name.c_str(), "read");
  wgGetTree::tree_in = (TTree*) wgGetTree::finput->Get("tree");
}

//************************************************************************
void wgGetTree::Close() {
  if(wgGetTree::finput != NULL) {
    try {
      wgGetTree::finput->Close();
    }
    catch (const std::exception &e) {
      Log.eWrite( "[wgGetTree::Open] failed to close " + wgGetTree::finputname +
                  " : " + std::string(e.what()));
    }
    delete wgGetTree::finput;
  }
}

//************************************************************************
wgGetTree::~wgGetTree(){
  this->Close();
}

bool wgGetTree::BranchExists(const std::string& branch_name) {
  size_t n = tree_in->GetListOfBranches()->GetEntries();
  for (size_t i = 0; i < n; i++) {
    TBranch *br = dynamic_cast<TBranch*>(tree_in->GetListOfBranches()->At(i));
    if( br && br->GetName() == branch_name ) return true;
  }
  Log.Write("[SetTreeFile] Branch " + branch_name + " not found");
  return false;
}

//************************************************************************
void wgGetTree::SetTreeFile(Raw_t& rdin){

  if(finput == NULL || finput->IsOpen() == kFALSE)
    throw wgInvalidFile("TFile " + finputname + " is not open!");
  try {
    if (BranchExists("spill_number"))
      tree_in->SetBranchAddress("spill_number",&rdin.spill_number);
    if (BranchExists("spill_mode"))
      tree_in->SetBranchAddress("spill_mode",  &rdin.spill_mode);
    if (BranchExists("spill_count"))
      tree_in->SetBranchAddress("spill_count", &rdin.spill_count);
    if (BranchExists("bcid"))
      tree_in->SetBranchAddress("bcid",         rdin.bcid.data());
    if (BranchExists("charge"))
      tree_in->SetBranchAddress("charge",       rdin.charge.data());
    if (BranchExists("time"))
      tree_in->SetBranchAddress("time",         rdin.time.data());
    if (BranchExists("gs"))
      tree_in->SetBranchAddress("gs",           rdin.gs.data());
    if (BranchExists("hit"))
      tree_in->SetBranchAddress("hit",          rdin.hit.data());
    if (BranchExists("chipid"))
      tree_in->SetBranchAddress("chipid",       rdin.chipid.data());
    if (BranchExists("col"))
      tree_in->SetBranchAddress("col",          rdin.col.data());
    if (BranchExists("chan"))
      tree_in->SetBranchAddress("chan",         rdin.chan.data());
    if (BranchExists("chip"))
      tree_in->SetBranchAddress("chip",         rdin.chip.data());
    if (BranchExists("debug_chip"))
      tree_in->SetBranchAddress("debug_chip",   rdin.debug_chip.data());
    if (BranchExists("debug_spill"))
      tree_in->SetBranchAddress("debug_spill",  rdin.debug_spill.data());
    if (BranchExists("view"))
      tree_in->SetBranchAddress("view",        &rdin.view);
    if (BranchExists("pln"))
      tree_in->SetBranchAddress("pln",          rdin.pln.data());
    if (BranchExists("ch"))
      tree_in->SetBranchAddress("ch",           rdin.ch.data());
    if (BranchExists("grid"))
      tree_in->SetBranchAddress("grid",         rdin.grid.data());
    if (BranchExists("x"))
      tree_in->SetBranchAddress("x",            rdin.x.data());
    if (BranchExists("y"))
      tree_in->SetBranchAddress("y",            rdin.y.data());
    if (BranchExists("z"))
      tree_in->SetBranchAddress("z",            rdin.z.data());
    if (BranchExists("time_ns"))
      tree_in->SetBranchAddress("time_ns",      rdin.time_ns.data());
    if (BranchExists("pe"))
      tree_in->SetBranchAddress("pe",           rdin.pe.data());
    if (BranchExists("gain"))
      tree_in->SetBranchAddress("gain",         rdin.gain.data());
    if (BranchExists("pedestal"))
      tree_in->SetBranchAddress("pedestal",     rdin.pedestal.data());
    if (BranchExists("tdc_slope"))
      tree_in->SetBranchAddress("tdc_slope",    rdin.tdc_slope.data());
    if (BranchExists("tdc_intcpt"))
      tree_in->SetBranchAddress("tdc_intcpt",   rdin.tdc_intcpt.data());
  }	catch (const std::exception &e) {
    throw wgElementNotFound( "[wgGetTree::Open] failed to get the TTree from"
                             + wgGetTree::finputname + " : " + std::string(e.what()));
  }
}

//************************************************************************
void wgGetTree::GetEntry(int i){
  wgGetTree::tree_in->GetEntry(i);
}

//************************************************************************
double wgGetTree::GetStartTime(){
  TH1D* h = (TH1D*)wgGetTree::finput->Get("start_time");
  double ret = h->GetXaxis()->GetBinCenter(h->GetMaximumBin());
  delete h;
  return ret;
}

//************************************************************************
double wgGetTree::GetStopTime(){
  TH1D* h = (TH1D*) wgGetTree::finput->Get("stop_time");
  double ret = h->GetXaxis()->GetBinCenter(h->GetMaximumBin());
  delete h;
  return ret;
}

//************************************************************************
double wgGetTree::GetDataPacket(){
  TH1D* h = (TH1D*) wgGetTree::finput->Get("nb_data_pkts");
  double ret = h->GetXaxis()->GetBinCenter(h->GetMaximumBin());
  delete h;
  return ret;
}

//************************************************************************
double wgGetTree::GetLostPacket(){
  TH1D* h = (TH1D*) wgGetTree::finput->Get("nb_lost_pkts");
  double ret = h->GetXaxis()->GetBinCenter(h->GetMaximumBin());
  delete h;
  return ret;
}

//************************************************************************
TH1D* wgGetTree::GetHist_StartTime(){
  if (BranchExists("start_time"))
    return dynamic_cast<TH1D*>(wgGetTree::finput->Get("start_time"));
  else return NULL;
}

//************************************************************************
TH1D* wgGetTree::GetHist_StopTime(){
  if (BranchExists("stop_time"))
    return dynamic_cast<TH1D*>(wgGetTree::finput->Get("stop_time"));
  else return NULL;
}

//************************************************************************
TH1D* wgGetTree::GetHist_DataPacket(){
  if (BranchExists("nb_data_pkts"))
    return dynamic_cast<TH1D*>(wgGetTree::finput->Get("nb_data_pkts"));
  else return NULL;
}

//************************************************************************
TH1D* wgGetTree::GetHist_LostPacket(){
  if (BranchExists("nb__pktlosts"))
    return dynamic_cast<TH1D*>(wgGetTree::finput->Get("nb_lost_pkts"));
  else return NULL;
}
