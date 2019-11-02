// system includes
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <bitset>

// ROOT includes
#include "TFile.h"
#include "TH1I.h"
#include "TTree.h"
#include "TParameter.h"

// user includes
#include "wgConst.hpp"
#include "wgColor.hpp"
#include "wgFileSystemTools.hpp"
#include "wgGetTree.hpp"
#include "wgExceptions.hpp"
#include "wgLogger.hpp"
#include "wgTopology.hpp"
#include "wgMakeHist.hpp"

using namespace wagasci_tools;

//******************************************************************
void ModeSelect(const int mode, std::bitset<N_MODES>& flag){
  if ( mode == 1 || mode >= 10 )               flag[SELECT_DARK_NOISE] = true;
  if ( mode == 2 || mode >= 10 )               flag[SELECT_CHARGE]   = true;
  if ( mode == 3 || mode == 10 || mode >= 20 ) flag[SELECT_PEDESTAL]     = true;
  if ( mode == 4 || mode == 11 || mode >= 20 ) flag[SELECT_TIME]  = true;
  if ( mode < 0  || mode > 20 )
    throw std::invalid_argument("Mode " + std::to_string(mode) + " not recognized"); 
}

int wgMakeHist(const char * x_input_file_name,
               const char * x_pyrame_config_file,
               const char * x_output_dir,
               const int mode,
               const bool overwrite,
               const unsigned dif) {

  /////////////////////////////////////////////////////////////////////////////
  //                          Check argument sanity                          //
  /////////////////////////////////////////////////////////////////////////////
  
  std::string input_file_name(x_input_file_name);
  std::string pyrame_config_file(x_pyrame_config_file);
  std::string output_dir(x_output_dir);

  std::string output_file_name = GetNameBeforeLastUnderBar(input_file_name) + "_hist.root";
  std::string logfilename  = GetName(input_file_name);
  int pos = logfilename.rfind("_ecal_dif_") ;
  std::string logfile = GetPath(input_file_name) + logfilename.substr(0, pos ) + ".log";

  if(input_file_name.empty() || !check_exist::RootFile(input_file_name)) {
    Log.eWrite("[wgMakeHist] Input file not found : " + input_file_name);
    return ERR_EMPTY_INPUT_FILE;
  }
  if (pyrame_config_file.empty() || !check_exist::XmlFile(pyrame_config_file)) {
    Log.eWrite("[wgMakeHist] Pyrame xml configuration file not found : " + pyrame_config_file);
    return ERR_CONFIG_XML_FILE_NOT_FOUND;
  }
  if (!wagasci_tools::check_exist::Dir(output_dir)) {
    wagasci_tools::MakeDir(output_dir);
  }

  Log.Write("[wgMakeHist] *****  INPUT FILE         : " + input_file_name    + "  *****");
  Log.Write("[wgMakeHist] *****  PYRAME CONFIG FILE : " + pyrame_config_file + "  *****");
  Log.Write("[wgMakeHist] *****  OUTPUT HIST FILE   : " + output_file_name   + "  *****");
  Log.Write("[wgMakeHist] *****  OUTPUT DIRECTORY   : " + output_dir         + "  *****");
  Log.Write("[wgMakeHist] *****  LOG FILE           : " + logfilename        + "  *****");

  /////////////////////////////////////////////////////////////////////////////
  //                                   Mode                                  //
  /////////////////////////////////////////////////////////////////////////////

  // Set the correct flags according to the mode
  std::bitset<N_MODES> flags;
  try { ModeSelect(mode, flags); }
  catch (const std::exception& e) {
    Log.eWrite("[wgAnaHist] Failed to " + std::string(e.what()));
    exit(1);
  }
  
  /////////////////////////////////////////////////////////////////////////////
  //                                 Topology                                //
  /////////////////////////////////////////////////////////////////////////////

  Topology * topol;
  try {
    topol = new Topology(pyrame_config_file);
  }
  catch (const std::exception& e) {
    Log.eWrite("[wgMakeHist] " + std::string(e.what()));
    return ERR_TOPOLOGY;
  }
  unsigned n_chips = topol->dif_map[dif].size();

  if ( n_chips == 0 || n_chips > NCHIPS ) {
    Log.eWrite("[wgMakeHist] wrong number of chips : " + std::to_string(n_chips) );
    return ERR_WRONG_CHIP_VALUE;
  }
  
  /////////////////////////////////////////////////////////////////////////////
  //                            Define Histograms                            //
  /////////////////////////////////////////////////////////////////////////////

  std::vector<std::vector<std::array<TH1I*, MEMDEPTH>>> h_charge_hit   (n_chips);
  std::vector<std::vector<std::array<TH1I*, MEMDEPTH>>> h_charge_hit_HG(n_chips);
  std::vector<std::vector<std::array<TH1I*, MEMDEPTH>>> h_charge_hit_LG(n_chips);
  std::vector<std::vector<std::array<TH1I*, MEMDEPTH>>> h_pe_hit       (n_chips);
  std::vector<std::vector<std::array<TH1I*, MEMDEPTH>>> h_charge_nohit (n_chips);
  std::vector<std::vector<std::array<TH1I*, MEMDEPTH>>> h_time_hit     (n_chips);
  std::vector<std::vector<std::array<TH1I*, MEMDEPTH>>> h_time_nohit   (n_chips);
  // h_bcid_hit: For every channel fill it with the BCID of all the columns with a hit.
  std::vector<std::vector<TH1I*>> h_bcid_hit(n_chips);

  int min_bin = 0;
  int max_bin = MAX_VALUE_12BITS;
  int bin     = MAX_VALUE_12BITS;
  TString h_name;
  wgColor wgColor;

  /////////////////////////////////////////////////////////////////////////////
  //                            Create hist.root                             //
  /////////////////////////////////////////////////////////////////////////////

  TFile * output_hist_file;
  if (!overwrite)
    output_hist_file = new TFile((output_dir + "/" + output_file_name).c_str(), "create");
  else
    output_hist_file = new TFile((output_dir + "/" + output_file_name).c_str(), "recreate");
  if (!output_hist_file->IsOpen()) {
    Log.eWrite("[wgMakeHist] Failed to create output hist file : " +
               output_dir + "/" + output_file_name);
    return ERR_FAILED_OPEN_HIST_FILE;
  }

  for (unsigned ichip = 0; ichip < n_chips; ++ichip) {
    unsigned n_chans = topol->dif_map[dif][ichip];
    if (flags[SELECT_CHARGE]) {
      h_charge_hit   [ichip].resize(n_chans);
      h_charge_hit_HG[ichip].resize(n_chans);
      h_charge_hit_LG[ichip].resize(n_chans);
      h_pe_hit       [ichip].resize(n_chans);
    }
    if (flags[SELECT_PEDESTAL])
      h_charge_nohit [ichip].resize(n_chans);
    if (flags[SELECT_TIME]) { 
      h_time_hit     [ichip].resize(n_chans);
      h_time_nohit   [ichip].resize(n_chans);
    }
    if (flags[SELECT_DARK_NOISE] | flags[SELECT_TIME]) 
      h_bcid_hit     [ichip].resize(n_chans);
    for (unsigned ichan = 0; ichan < n_chans; ++ichan) {
      for (unsigned icol = 0; icol < MEMDEPTH; ++icol) {
        if (flags[SELECT_CHARGE]) {
          // ADC count when there is a hit (hit bit is one)
          h_name.Form("charge_hit_chip%u_ch%u_col%u", ichip, ichan, icol);
          h_charge_hit[ichip][ichan][icol] = new TH1I(h_name, h_name, bin, min_bin, max_bin);
          h_charge_hit[ichip][ichan][icol]->SetDirectory(output_hist_file);
          h_charge_hit[ichip][ichan][icol]->SetLineColor(wgColor::wgcolors[icol]);
          // ADC count when there is a hit (hit bit is one) and the high gain preamp is selected
          h_name.Form("charge_hit_HG_chip%u_ch%u_col%u", ichip, ichan, icol);
          h_charge_hit_HG[ichip][ichan][icol] = new TH1I(h_name, h_name, bin, min_bin, max_bin);
          h_charge_hit_HG[ichip][ichan][icol]->SetDirectory(output_hist_file);
          h_charge_hit_HG[ichip][ichan][icol]->SetLineColor(wgColor::wgcolors[icol]);
          // ADC count when there is a hit (hit bit is one) and the low gain preamp is selected
          h_name.Form("charge_hit_LG_chip%u_ch%u_col%u", ichip, ichan, icol);
          h_charge_hit_LG[ichip][ichan][icol] = new TH1I(h_name, h_name, bin, min_bin, max_bin);
          h_charge_hit_LG[ichip][ichan][icol]->SetDirectory(output_hist_file);
          h_charge_hit_LG[ichip][ichan][icol]->SetLineColor(wgColor::wgcolors[icol]);
          // Photo-electrons
          h_name.Form("pe_hit_chip%u_ch%u_col%u", ichip, ichan, icol);
          h_pe_hit[ichip][ichan][icol] = new TH1I(h_name, h_name, bin, min_bin, max_bin);
          h_pe_hit[ichip][ichan][icol]->SetDirectory(output_hist_file);
          h_pe_hit[ichip][ichan][icol]->SetLineColor(wgColor::wgcolors[icol]);
        }
        if (flags[SELECT_PEDESTAL]) {
          // ADC count when there is not hit (hit bit is zero)
          h_name.Form("charge_nohit_chip%u_ch%u_col%u", ichip, ichan, icol);
          h_charge_nohit[ichip][ichan][icol] = new TH1I(h_name, h_name, bin, min_bin, max_bin);
          h_charge_nohit[ichip][ichan][icol]->SetDirectory(output_hist_file);
          h_charge_nohit[ichip][ichan][icol]->SetLineColor(wgColor::wgcolors[icol + MEMDEPTH * 2 + 2]);
        }
        if (flags[SELECT_TIME]) { 
          // TDC count when there is a hit (hit bit is one)
          h_name.Form("time_hit_chip%u_ch%u_col%u", ichip, ichan, icol);
          h_time_hit[ichip][ichan][icol] = new TH1I(h_name, h_name, bin, min_bin, max_bin);
          h_time_hit[ichip][ichan][icol]->SetDirectory(output_hist_file);
          h_time_hit[ichip][ichan][icol]->SetLineColor(wgColor::wgcolors[icol]);
          // TDC count when there is not hit (hit bit is zero)
          h_name.Form("time_nohit_chip%u_ch%u_col%u", ichip, ichan, icol);
          h_time_nohit[ichip][ichan][icol] = new TH1I(h_name, h_name, bin, min_bin, max_bin);
          h_time_hit[ichip][ichan][icol]->SetDirectory(output_hist_file);
          h_time_nohit[ichip][ichan][icol]->SetLineColor(wgColor::wgcolors[icol + MEMDEPTH * 2 + 2]);
        }
      } //end col
      if (flags[SELECT_DARK_NOISE] | flags[SELECT_TIME]) {
        // BCID
        h_name.Form("bcid_hit_chip%u_ch%u", ichip, ichan);
        h_bcid_hit[ichip][ichan] = new TH1I(h_name, h_name, MAX_VALUE_16BITS, 0, MAX_VALUE_16BITS);
        h_bcid_hit[ichip][ichan]->SetLineColor(kBlack);
      }
    } //end ch
  } //end chip
  
  /////////////////////////////////////////////////////////////////////////////
  //                           Open tree.root file                           //
  /////////////////////////////////////////////////////////////////////////////
  
  Raw_t rd(n_chips);
  
  TParameter<int> * start_time; 
  TParameter<int> * stop_time;  
  TParameter<int> * nb_data_pkts;
  TParameter<int> * nb_lost_pkts;
  TParameter<int> * spill_count;

  try {
    wgGetTree wg_tree(input_file_name, rd, dif); 

    /////////////////////////////////////////////////////////////////////////////
    //                         Get acquisition run info                        //
    /////////////////////////////////////////////////////////////////////////////
  
    start_time   = new TParameter<int>("start_time",   wg_tree.GetStartTime());
    stop_time    = new TParameter<int>("stop_time",    wg_tree.GetStartTime());
    nb_data_pkts = new TParameter<int>("nb_data_pkts", wg_tree.GetStartTime());
    nb_lost_pkts = new TParameter<int>("nb_lost_pkts", wg_tree.GetStartTime());

    int max_spill = wg_tree.tree->GetMaximum("spill_count");
    int min_spill = wg_tree.tree->GetMinimum("spill_count");
    spill_count = new TParameter<int>("spill_count", std::abs(max_spill - min_spill));
    Int_t n_events = wg_tree.tree->GetEntries();
    if (n_events / n_chips != (unsigned) spill_count->GetVal()) {
      Log.eWrite("[wgMakeHist] some spills are missing : max_spill - min_spill = " +
                 std::to_string(max_spill) + " - " + std::to_string(min_spill) + " = " +
                 std::to_string(spill_count->GetVal()) + ", n_events/n_chips = " +
                 std::to_string(n_events / n_chips));
    }

    /////////////////////////////////////////////////////////////////////////////
    //                                Event loop                               //
    /////////////////////////////////////////////////////////////////////////////
  
    for (Int_t ievent = 0; ievent < n_events; ++ievent) {

      if ( ievent % 1000 == 0 )
        Log.Write("[wgMakeHist] Event number = " + std::to_string(ievent) +
                  " / " + std::to_string(n_events));
      // Read one event
      wg_tree.tree->GetEntry(ievent);

      // CHIPS loop
      for(unsigned ichip = 0; ichip < n_chips; ichip++) {
        // chipid: chip ID tag as it is recorded in the chip trailer
        unsigned ichipid = rd.chipid[ichip];
        if (ichipid >= n_chips) continue;
        // CHANNELS loop
        for(unsigned ichan = 0; ichan < topol->dif_map[dif][ichip]; ichan++) {
          // COLUMNS loop
          for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
            // HIT
            if ( rd.hit[ichip][ichan][icol] == HIT_BIT ) {
              if (flags[SELECT_DARK_NOISE] | flags[SELECT_TIME])
                h_bcid_hit  [ichipid][ichan]->      Fill(rd.bcid  [ichip]       [icol]);
              if (flags[SELECT_CHARGE]) {
                h_pe_hit    [ichipid][ichan][icol]->Fill(rd.pe    [ichip][ichan][icol]);
                h_charge_hit[ichipid][ichan][icol]->Fill(rd.charge[ichip][ichan][icol]);
              }
              if (flags[SELECT_TIME])
                h_time_hit  [ichipid][ichan][icol]->Fill(rd.time  [ichip][ichan][icol]);
              // HIGH GAIN
              if(rd.gs[ichip][ichan][icol] == HIGH_GAIN_BIT && flags[SELECT_CHARGE]) { 
                h_charge_hit_HG[ichipid][ichan][icol]->Fill(rd.charge[ichip][ichan][icol]);
              }
              // LOW GAIN
              else if(rd.gs[ichip][ichan][icol] == LOW_GAIN_BIT && flags[SELECT_CHARGE]) {
                h_charge_hit_LG[ichipid][ichan][icol]->Fill(rd.charge[ichip][ichan][icol]);
              }	  
            }
            // NO HIT
            else if ( rd.hit[ichip][ichan][icol] == NO_HIT_BIT ) {
              if (flags[SELECT_PEDESTAL])
                h_charge_nohit[ichipid][ichan][icol]->Fill(rd.charge[ichip][ichan][icol]);
              if (flags[SELECT_TIME])
                h_time_nohit  [ichipid][ichan][icol]->Fill(rd.time  [ichip][ichan][icol]);
            } // hit
          } // icol
        } // ichan
      } // ichipid
    } // ievent
  } // try
  catch (const std::exception& e) {
    Log.eWrite("[wgMakeHist] failed to get the TTree from file : " + std::string(e.what()));
    return ERR_FAILED_OPEN_TREE_FILE;
  }
  
  // In some old runs the time and data packets info is not recorded
  output_hist_file->WriteObject(start_time,   "start_time");
  output_hist_file->WriteObject(stop_time,    "stop_time");
  output_hist_file->WriteObject(nb_data_pkts, "nb_data_pkts");
  output_hist_file->WriteObject(nb_lost_pkts, "nb_lost_pkts");
  output_hist_file->WriteObject(spill_count,  "spill_count");

  output_hist_file->Write();
  output_hist_file->Close();
  Log.Write("[wgMakeHist] finished");
  delete output_hist_file;
  
  return WG_SUCCESS;
}
