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

wgGetTree::wgGetTree(const std::string& root_file_name, Raw_t& rd) :
    m_finputname(root_file_name) {
  this->Open();
  this->SetTreeFile(rd);
}

//************************************************************************
void wgGetTree::Open() {
  // Check if the ROOT file exists
  if(!check_exist::RootFile(m_finputname))
    throw wgInvalidFile( "[wgGetTree::Open] failed to open " + m_finputname );
  // Check if the ROOT file is already opened
  try {
    if ( wgGetTree::finput != NULL ) {
      if ( wgGetTree::finput->IsOpen() == kTRUE ) {
        Log.Write( "[wgGetTree::Open] " + m_finputname + "is already opened" );
        this->Close();
      }
    }
  } catch (...) {}
  // Open the ROOT file and get the TTree called "tree"
  // Store the TFile in wgGetTree::finput and TTree in wgGetTree::tree
  wgGetTree::finput = new TFile(m_finputname.c_str(), "read");
  wgGetTree::tree = (TTree*) wgGetTree::finput->Get("tree");
}

//************************************************************************
void wgGetTree::Close() {
  if(wgGetTree::finput != NULL) {
    try { wgGetTree::finput->Close(); }
    catch (const std::exception &e) {
      Log.eWrite( "[wgGetTree] failed to close " + wgGetTree::m_finputname +
                  " : " + std::string(e.what()));
    }
  }
}

//************************************************************************
wgGetTree::~wgGetTree() {
  this->Close();
  delete finput;
}

//************************************************************************
bool wgGetTree::BranchExists(const std::string& branch_name) {
  size_t n = tree->GetListOfBranches()->GetEntries();
  for (size_t i = 0; i < n; i++) {
    TBranch *br = dynamic_cast<TBranch*>(tree->GetListOfBranches()->At(i));
    if( br && br->GetName() == branch_name ) return true;
  }
  Log.Write("[SetTreeFile] Branch " + branch_name + " not found");
  return false;
}

//************************************************************************
void wgGetTree::SetTreeFile(Raw_t& rd){

  if(finput == NULL || finput->IsOpen() == kFALSE)
    throw wgInvalidFile("TFile " + m_finputname + " is not open!");
  try {
    if (BranchExists("spill_number"))
      tree->SetBranchAddress("spill_number",&rd.spill_number);
    if (BranchExists("spill_mode"))
      tree->SetBranchAddress("spill_mode",  &rd.spill_mode);
    if (BranchExists("spill_count"))
      tree->SetBranchAddress("spill_count", &rd.spill_count);
    if (BranchExists("bcid"))
      tree->SetBranchAddress("bcid",         rd.bcid.data());
    if (BranchExists("charge"))
      tree->SetBranchAddress("charge",       rd.charge.data());
    if (BranchExists("time"))
      tree->SetBranchAddress("time",         rd.time.data());
    if (BranchExists("gs"))
      tree->SetBranchAddress("gs",           rd.gs.data());
    if (BranchExists("hit"))
      tree->SetBranchAddress("hit",          rd.hit.data());
    if (BranchExists("chipid"))
      tree->SetBranchAddress("chipid",       rd.chipid.data());
    if (BranchExists("col"))
      tree->SetBranchAddress("col",          rd.col.data());
    if (BranchExists("chan"))
      tree->SetBranchAddress("chan",         rd.chan.data());
    if (BranchExists("chip"))
      tree->SetBranchAddress("chip",         rd.chip.data());
    if (BranchExists("debug_chip"))
      tree->SetBranchAddress("debug_chip",   rd.debug_chip.data());
    if (BranchExists("debug_spill"))
      tree->SetBranchAddress("debug_spill",  rd.debug_spill.data());
    if (BranchExists("view"))
      tree->SetBranchAddress("view",        &rd.view);
    if (BranchExists("pln"))
      tree->SetBranchAddress("pln",          rd.pln.data());
    if (BranchExists("ch"))
      tree->SetBranchAddress("ch",           rd.ch.data());
    if (BranchExists("grid"))
      tree->SetBranchAddress("grid",         rd.grid.data());
    if (BranchExists("x"))
      tree->SetBranchAddress("x",            rd.x.data());
    if (BranchExists("y"))
      tree->SetBranchAddress("y",            rd.y.data());
    if (BranchExists("z"))
      tree->SetBranchAddress("z",            rd.z.data());
    if (BranchExists("time_ns"))
      tree->SetBranchAddress("time_ns",      rd.time_ns.data());
    if (BranchExists("pe"))
      tree->SetBranchAddress("pe",           rd.pe.data());
    if (BranchExists("gain"))
      tree->SetBranchAddress("gain",         rd.gain.data());
    if (BranchExists("pedestal"))
      tree->SetBranchAddress("pedestal",     rd.pedestal.data());
    if (BranchExists("tdc_slope"))
      tree->SetBranchAddress("tdc_slope",    rd.tdc_slope.data());
    if (BranchExists("tdc_intcpt"))
      tree->SetBranchAddress("tdc_intcpt",   rd.tdc_intcpt.data());
  }	catch (const std::exception &e) {
    throw wgElementNotFound( "[wgGetTree::Open] failed to get the TTree from"
                             + wgGetTree::m_finputname + " : " + std::string(e.what()));
  }
}

//************************************************************************
void wgGetTree::GetEntry(int event) {
  wgGetTree::tree->GetEntry(event);
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
  if (BranchExists("nb_lost_pkts"))
    return dynamic_cast<TH1D*>(wgGetTree::finput->Get("nb_lost_pkts"));
  else return NULL;
}
