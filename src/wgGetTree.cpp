// system includes
#include <iostream>

// ROOT includes
#include "TH1I.h"

// user includes
#include "wgGetTree.hpp"
#include "wgFileSystemTools.hpp"

#include "wgConst.hpp"
#include "wgLogger.hpp"

using namespace wagasci_tools;

//************************************************************************

wgGetTree::wgGetTree(const std::string& root_file_name, Raw_t& rd) :
    m_finputname(root_file_name), m_rd(rd) {
  this->Open();
  this->SetTreeFile();
}

//************************************************************************
void wgGetTree::Open() {
  if(!check_exist::RootFile(m_finputname))
    throw wgInvalidFile("[wgGetTree] TTree file not found : " +
                        m_finputname);
  
  m_finput = new TFile(m_finputname.c_str(), "read");
  if (m_finput == NULL || m_finput->IsOpen() == kFALSE)
    throw wgInvalidFile("[wgGetTree] Failed to open TTree file : " +
                        m_finputname);
}

//************************************************************************
void wgGetTree::Close() {
  if(m_finput != NULL) {
    try { m_finput->Close(); }
    catch (const std::exception &e) {
      Log.eWrite( "[wgGetTree] failed to close " + m_finputname +
                  " : " + std::string(e.what()));
    }
  }
}

//************************************************************************
wgGetTree::~wgGetTree() {
  this->Close();
  delete m_finput;
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
void wgGetTree::SetTreeFile() {
  tree = (TTree*) m_finput->Get("tree");
  try {
    if (BranchExists("spill_number"))
      tree->SetBranchAddress("spill_number",&m_rd.get().spill_number);
    if (BranchExists("spill_mode"))
      tree->SetBranchAddress("spill_mode",  &m_rd.get().spill_mode);
    if (BranchExists("spill_count"))
      tree->SetBranchAddress("spill_count", &m_rd.get().spill_count);
    if (BranchExists("bcid"))
      tree->SetBranchAddress("bcid",         m_rd.get().bcid.data());
    if (BranchExists("charge"))
      tree->SetBranchAddress("charge",       m_rd.get().charge.data());
    if (BranchExists("time"))
      tree->SetBranchAddress("time",         m_rd.get().time.data());
    if (BranchExists("gs"))
      tree->SetBranchAddress("gs",           m_rd.get().gs.data());
    if (BranchExists("hit"))
      tree->SetBranchAddress("hit",          m_rd.get().hit.data());
    if (BranchExists("chipid"))
      tree->SetBranchAddress("chipid",       m_rd.get().chipid.data());
    if (BranchExists("col"))
      tree->SetBranchAddress("col",          m_rd.get().col.data());
    if (BranchExists("chan"))
      tree->SetBranchAddress("chan",         m_rd.get().chan.data());
    if (BranchExists("chip"))
      tree->SetBranchAddress("chip",         m_rd.get().chip.data());
    if (BranchExists("debug_chip"))
      tree->SetBranchAddress("debug_chip",   m_rd.get().debug_chip.data());
    if (BranchExists("debug_spill"))
      tree->SetBranchAddress("debug_spill",  m_rd.get().debug_spill.data());
    if (BranchExists("view"))
      tree->SetBranchAddress("view",        &m_rd.get().view);
    if (BranchExists("pln"))
      tree->SetBranchAddress("pln",          m_rd.get().pln.data());
    if (BranchExists("ch"))
      tree->SetBranchAddress("ch",           m_rd.get().ch.data());
    if (BranchExists("grid"))
      tree->SetBranchAddress("grid",         m_rd.get().grid.data());
    if (BranchExists("x"))
      tree->SetBranchAddress("x",            m_rd.get().x.data());
    if (BranchExists("y"))
      tree->SetBranchAddress("y",            m_rd.get().y.data());
    if (BranchExists("z"))
      tree->SetBranchAddress("z",            m_rd.get().z.data());
    if (BranchExists("time_ns"))
      tree->SetBranchAddress("time_ns",      m_rd.get().time_ns.data());
    if (BranchExists("pe"))
      tree->SetBranchAddress("pe",           m_rd.get().pe.data());
    if (BranchExists("gain"))
      tree->SetBranchAddress("gain",         m_rd.get().gain.data());
    if (BranchExists("pedestal"))
      tree->SetBranchAddress("pedestal",     m_rd.get().pedestal.data());
    if (BranchExists("tdc_slope"))
      tree->SetBranchAddress("tdc_slope",    m_rd.get().tdc_slope.data());
    if (BranchExists("tdc_intcpt"))
      tree->SetBranchAddress("tdc_intcpt",   m_rd.get().tdc_intcpt.data());
  } catch (const std::exception &e) {
    throw wgElementNotFound( "[wgGetTree] failed to get the TTree from "
                             + m_finputname + " : " + std::string(e.what()));
  }
}

//************************************************************************
void wgGetTree::GetEntry(int event) {
  wgGetTree::tree->GetEntry(event);
}

//************************************************************************
double wgGetTree::GetStartTime() {
  TH1I* h = (TH1I*)m_finput->Get("start_time");
  double ret = h->GetXaxis()->GetBinCenter(h->GetMaximumBin());
  delete h;
  return ret;
}

//************************************************************************
double wgGetTree::GetStopTime() {
  TH1I* h = (TH1I*) m_finput->Get("stop_time");
  double ret = h->GetXaxis()->GetBinCenter(h->GetMaximumBin());
  delete h;
  return ret;
}

//************************************************************************
double wgGetTree::GetDataPacket() {
  TH1I* h = (TH1I*) m_finput->Get("nb_data_pkts");
  double ret = h->GetXaxis()->GetBinCenter(h->GetMaximumBin());
  delete h;
  return ret;
}

//************************************************************************
double wgGetTree::GetLostPacket() {
  TH1I* h = (TH1I*) m_finput->Get("nb_lost_pkts");
  double ret = h->GetXaxis()->GetBinCenter(h->GetMaximumBin());
  delete h;
  return ret;
}

//************************************************************************
TH1I* wgGetTree::GetHist_StartTime() {
  if (BranchExists("start_time"))
    return dynamic_cast<TH1I*>(m_finput->Get("start_time"));
  else return NULL;
}

//************************************************************************
TH1I* wgGetTree::GetHist_StopTime() {
  if (BranchExists("stop_time"))
    return dynamic_cast<TH1I*>(m_finput->Get("stop_time"));
  else return NULL;
}

//************************************************************************
TH1I* wgGetTree::GetHist_DataPacket() {
  if (BranchExists("nb_data_pkts"))
    return dynamic_cast<TH1I*>(m_finput->Get("nb_data_pkts"));
  else return NULL;
}

//************************************************************************
TH1I* wgGetTree::GetHist_LostPacket() {
  if (BranchExists("nb_lost_pkts"))
    return dynamic_cast<TH1I*>(m_finput->Get("nb_lost_pkts"));
  else return NULL;
}
