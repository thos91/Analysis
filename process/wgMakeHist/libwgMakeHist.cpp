// system includes
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

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
#include "wgMakeHist.hpp"

using namespace wagasci_tools;

int wgMakeHist(const char * x_input_file_name,
               const char * x_output_dir,
               const bool overwrite,
               const unsigned dif,
               const unsigned n_chips) {

  /////////////////////////////////////////////////////////////////////////////
  //                          Check argument sanity                          //
  /////////////////////////////////////////////////////////////////////////////
  
  std::string input_file_name(x_input_file_name);
  std::string output_dir(x_output_dir);

  std::string output_file_name = GetNameBeforeLastUnderBar(input_file_name) + "_hist.root";
  std::string logfilename  = GetName(input_file_name);
  int pos = logfilename.rfind("_ecal_dif_") ;
  std::string logfile = GetPath(input_file_name) + logfilename.substr(0, pos ) + ".log";

  if (n_chips > NCHIPS) {
    Log.eWrite("[wgMakeHist] The number of chips per DIF must be {1-" + std::to_string(NCHIPS) + "}");
    return ERR_WRONG_CHIP_VALUE;
  }
  if(!check_exist::RootFile(input_file_name)) {
    Log.eWrite("[wgMakeHist] Input file " + input_file_name + " not found");
    return ERR_EMPTY_INPUT_FILE;
  }
  if (!wagasci_tools::check_exist::Dir(output_dir)) {
    wagasci_tools::MakeDir(output_dir);
  }

  Log.Write("[wgMakeHist] *****  READING FILE     : " + input_file_name  + "  *****");
  Log.Write("[wgMakeHist] *****  OUTPUT HIST FILE : " + output_file_name + "  *****");
  Log.Write("[wgMakeHist] *****  OUTPUT DIRECTORY : " + output_dir       + "  *****");
  Log.Write("[wgMakeHist] *****  LOG FILE         : " + logfilename      + "  *****");

  /////////////////////////////////////////////////////////////////////////////
  //                            Define Histograms                            //
  /////////////////////////////////////////////////////////////////////////////
  
  std::vector<std::array<std::array<TH1I*, MEMDEPTH>, NCHANNELS>> h_charge_hit   (n_chips);
  std::vector<std::array<std::array<TH1I*, MEMDEPTH>, NCHANNELS>> h_charge_hit_HG(n_chips);
  std::vector<std::array<std::array<TH1I*, MEMDEPTH>, NCHANNELS>> h_charge_hit_LG(n_chips);
  std::vector<std::array<std::array<TH1I*, MEMDEPTH>, NCHANNELS>> h_pe_hit       (n_chips);
  std::vector<std::array<std::array<TH1I*, MEMDEPTH>, NCHANNELS>> h_charge_nohit (n_chips);
  std::vector<std::array<std::array<TH1I*, MEMDEPTH>, NCHANNELS>> h_time_hit     (n_chips);
  std::vector<std::array<std::array<TH1I*, MEMDEPTH>, NCHANNELS>> h_time_nohit   (n_chips);
  // h_bcid_hit: For every channel fill it with the BCID of all the columns with a hit.
  std::vector<std::array<TH1I*, NCHANNELS>> h_bcid_hit(n_chips);

  int min_bin = 0;
  int max_bin = MAX_VALUE_12BITS;
  int bin     = MAX_VALUE_12BITS;
  TString h_name;
  wgColor wgColor;

  TH1::AddDirectory(kFALSE);
  
  for (unsigned ichip = 0; ichip < n_chips; ++ichip) {
    for (unsigned ichan = 0; ichan < NCHANNELS; ++ichan) {
      for (unsigned icol = 0; icol < MEMDEPTH; ++icol) {
        // ADC count when there is a hit (hit bit is one)
        h_name.Form("charge_hit_chip%u_ch%u_col%u", ichip, ichan, icol);
        h_charge_hit[ichip][ichan][icol] = new TH1I(h_name, h_name, bin, min_bin, max_bin);
        h_charge_hit[ichip][ichan][icol]->SetLineColor(wgColor::wgcolors[icol]);
        // ADC count when there is a hit (hit bit is one) and the high gain preamp is selected
        h_name.Form("charge_hit_HG_chip%u_ch%u_col%u", ichip, ichan, icol);
        h_charge_hit_HG[ichip][ichan][icol] = new TH1I(h_name, h_name, bin, min_bin, max_bin);
        h_charge_hit_HG[ichip][ichan][icol]->SetLineColor(wgColor::wgcolors[icol]);
        // ADC count when there is a hit (hit bit is one) and the low gain preamp is selected
        h_name.Form("charge_hit_LG_chip%u_ch%u_col%u", ichip, ichan, icol);
        h_charge_hit_LG[ichip][ichan][icol] = new TH1I(h_name, h_name, bin, min_bin, max_bin);
        h_charge_hit_LG[ichip][ichan][icol]->SetLineColor(wgColor::wgcolors[icol]);
        // ADC count when there is not hit (hit bit is zero)
        h_name.Form("charge_nohit_chip%u_ch%u_col%u", ichip, ichan, icol);
        h_charge_nohit[ichip][ichan][icol] = new TH1I(h_name, h_name, bin, min_bin, max_bin);
        h_charge_nohit[ichip][ichan][icol]->SetLineColor(wgColor::wgcolors[icol + MEMDEPTH * 2 + 2]);
        // Photo-electrons
        h_name.Form("pe_hit_chip%u_ch%u_col%u", ichip, ichan, icol);
        h_pe_hit[ichip][ichan][icol] = new TH1I(h_name, h_name, bin, min_bin, max_bin);
        h_pe_hit[ichip][ichan][icol]->SetLineColor(wgColor::wgcolors[icol]);
        // TDC count when there is a hit (hit bit is one)
        h_name.Form("time_hit_chip%u_ch%u_col%u", ichip, ichan, icol);
        h_time_hit[ichip][ichan][icol] = new TH1I(h_name, h_name, bin, min_bin, max_bin);
        h_time_hit[ichip][ichan][icol]->SetLineColor(wgColor::wgcolors[icol]);
        // TDC count when there is not hit (hit bit is zero)
        h_name.Form("time_nohit_chip%u_ch%u_col%u", ichip, ichan, icol);
        h_time_nohit[ichip][ichan][icol] = new TH1I(h_name, h_name, bin, min_bin, max_bin);   
        h_time_nohit[ichip][ichan][icol]->SetLineColor(wgColor::wgcolors[icol + MEMDEPTH * 2 + 2]);
      } //end col
      // BCID
      h_name.Form("bcid_hit_chip%u_ch%u", ichip, ichan);
      h_bcid_hit[ichip][ichan] = new TH1I(h_name, h_name, MAX_VALUE_16BITS, 0, MAX_VALUE_16BITS);
      h_bcid_hit[ichip][ichan]->SetLineColor(kBlack);
    } //end ch
  } //end chip
  
  TH1::AddDirectory(kTRUE);

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
        for(unsigned ichan = 0; ichan < NCHANNELS; ichan++) {
          // COLUMNS loop
          for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
            // HIT
            if ( rd.hit[ichip][ichan][icol] == HIT_BIT ) {
              h_bcid_hit  [ichipid][ichan]->      Fill(rd.bcid  [ichip]       [icol]);
              h_pe_hit    [ichipid][ichan][icol]->Fill(rd.pe    [ichip][ichan][icol]);
              h_time_hit  [ichipid][ichan][icol]->Fill(rd.time  [ichip][ichan][icol]);
              h_charge_hit[ichipid][ichan][icol]->Fill(rd.charge[ichip][ichan][icol]);
              // HIGH GAIN
              if(rd.gs[ichip][ichan][icol] == HIGH_GAIN_BIT) { 
                h_charge_hit_HG[ichipid][ichan][icol]->Fill(rd.charge[ichip][ichan][icol]);
              }
              // LOW GAIN
              else if(rd.gs[ichip][ichan][icol] == LOW_GAIN_BIT) {
                h_charge_hit_LG[ichipid][ichan][icol]->Fill(rd.charge[ichip][ichan][icol]);
              }	  
            }
            // NO HIT
            else if ( rd.hit[ichip][ichan][icol] == NO_HIT_BIT ) {
              h_charge_nohit[ichipid][ichan][icol]->Fill(rd.charge[ichip][ichan][icol]);
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
  
  /////////////////////////////////////////////////////////////////////////////
  //                      Write histograms to hist.root                      //
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

  output_hist_file->cd();

  // In some old runs the time and data packets info is not recorded
  output_hist_file->WriteObject(start_time,   "start_time");
  output_hist_file->WriteObject(stop_time,    "stop_time");
  output_hist_file->WriteObject(nb_data_pkts, "nb_data_pkts");
  output_hist_file->WriteObject(nb_lost_pkts, "nb_lost_pkts");
  output_hist_file->WriteObject(spill_count,  "spill_count");
  for (unsigned ichipid = 0; ichipid < n_chips; ichipid++) {
    for (unsigned ichan = 0; ichan < NCHANNELS; ichan++) {
      if (h_bcid_hit[ichipid][ichan] != NULL) h_bcid_hit[ichipid][ichan]->Write();
      for (unsigned icol = 0; icol < MEMDEPTH; icol++) {
        if (h_charge_hit   [ichipid][ichan][icol] != NULL) h_charge_hit   [ichipid][ichan][icol]->Write();
        if (h_charge_hit_HG[ichipid][ichan][icol] != NULL) h_charge_hit_HG[ichipid][ichan][icol]->Write();
        if (h_charge_hit_LG[ichipid][ichan][icol] != NULL) h_charge_hit_LG[ichipid][ichan][icol]->Write();
        if (h_pe_hit       [ichipid][ichan][icol] != NULL) h_pe_hit       [ichipid][ichan][icol]->Write();
        if (h_charge_nohit [ichipid][ichan][icol] != NULL) h_charge_nohit [ichipid][ichan][icol]->Write();
        if (h_time_hit     [ichipid][ichan][icol] != NULL) h_time_hit     [ichipid][ichan][icol]->Write();
        if (h_time_nohit   [ichipid][ichan][icol] != NULL) h_time_nohit   [ichipid][ichan][icol]->Write();
      }
    }
  }

  output_hist_file->Write();
  output_hist_file->Close();
  Log.Write("[wgMakeHist] finished");
  delete output_hist_file;
  
  return WG_SUCCESS;
}
