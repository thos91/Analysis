// system includes
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

// ROOT includes
#include "TDirectory.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2D.h"
#include "TTree.h"

// user includes
#include "wgConst.hpp"
#include "wgColor.hpp"
#include "wgFileSystemTools.hpp"
#include "wgGetTree.hpp"
#include "wgExceptions.hpp"
#include "wgErrorCode.hpp"
#include "wgMakeHist.hpp"
#include "wgLogger.hpp"

using namespace wagasci_tools;

int wgMakeHist(const char * x_inputFileName,
               const char * x_outputDir,
               const bool overwrite,
               const unsigned n_chips,
               const unsigned n_channels) {

  string inputFileName(x_inputFileName);
  string outputDir(x_outputDir);
  CheckExist check;
  if(!check.RootFile(inputFileName)) {
    Log.eWrite("[wgMakeHist] Input file " + inputFileName + " not found");
    return ERR_EMPTY_INPUT_FILE;
  }
  
  string outputHistFileName = GetNameBeforeLastUnderBar(inputFileName) + "_hist.root";
  string logfilename  = GetName(inputFileName);
  int pos             = logfilename.rfind("_ecal_dif_") ;
  string logfile      = GetPath(inputFileName) + logfilename.substr(0, pos ) + ".log";

  Log.Write("[wgMakeHist] *****  READING FILE     : " + GetName(inputFileName)      + "  *****");
  Log.Write("[wgMakeHist] *****  OUTPUT HIST FILE : " + GetName(outputHistFileName) + "  *****");
  Log.Write("[wgMakeHist] *****  OUTPUT DIRECTORY : " + outputDir                          + "  *****");
  Log.Write("[wgMakeHist] *****  LOG FILE         : " + GetName(logfilename)        + "  *****");
  
  TFile * outputHistFile;

  if (!overwrite){
    outputHistFile = new TFile((outputDir + "/" + outputHistFileName).c_str(), "create");
    if ( !outputHistFile->IsOpen() ) {
      Log.eWrite("[" + logfilename + "][wgMakeHist] Error:" + outputDir + "/" + outputHistFileName + " already exists!");
      return ERR_CANNOT_OVERWRITE_OUTPUT_FILE;
    }
  }
  else outputHistFile = new TFile((outputDir + "/" + outputHistFileName).c_str(), "recreate");

  // === start make histgram ===

  Raw_t rd(n_chips, n_channels);
  wgColor wgColor;

  TH1D* h_spill;

  vector<vector<array<TH1D*, MEMDEPTH>>> h_charge_hit   (n_chips, vector<array<TH1D*, MEMDEPTH>>(n_channels));
  vector<vector<array<TH1D*, MEMDEPTH>>> h_charge_hit_HG(n_chips, vector<array<TH1D*, MEMDEPTH>>(n_channels));
  vector<vector<array<TH1D*, MEMDEPTH>>> h_charge_hit_LG(n_chips, vector<array<TH1D*, MEMDEPTH>>(n_channels));
  vector<vector<array<TH1D*, MEMDEPTH>>> h_pe_hit       (n_chips, vector<array<TH1D*, MEMDEPTH>>(n_channels));
  vector<vector<array<TH1D*, MEMDEPTH>>> h_charge_nohit (n_chips, vector<array<TH1D*, MEMDEPTH>>(n_channels));
  vector<vector<array<TH1D*, MEMDEPTH>>> h_time_hit     (n_chips, vector<array<TH1D*, MEMDEPTH>>(n_channels));
  vector<vector<array<TH1D*, MEMDEPTH>>> h_time_nohit   (n_chips, vector<array<TH1D*, MEMDEPTH>>(n_channels));
  
  // h_bcid_hit: For every channel fill it with the BCID of all the columns with
  // a hit.
  vector<vector<TH1D*>> h_bcid_hit(n_chips, vector<TH1D*>(n_channels));

  int min_bin = 0;
  int max_bin = MAX_12BIT_BIN;
  int bin     = MAX_12BIT_BIN;

  for (unsigned i = 0; i < n_chips; i++) {
    for (unsigned j = 0; j < n_channels; j++) {
      for (unsigned k = 0; k < MEMDEPTH; k++) {
		// ADC count when there is a hit (hit bit is one)
		h_charge_hit[i][j][k] = new TH1D(Form("charge_chip%u_ch%u_col%u",i,j,k), Form("charge_chip%u_ch%u_col%u",i,j,k), bin, min_bin, max_bin);
		h_charge_hit[i][j][k]->SetLineColor(wgColor::wgcolors[k]);
        // ADC count when there is a hit (hit bit is one) and the high gain preamp is selected
        h_charge_hit_HG[i][j][k] = new TH1D(Form("charge_hit_HG_chip%u_ch%u_col%u",i,j,k), Form("charge_hit_HG_chip%u_ch%u_col%u",i,j,k), bin, min_bin, max_bin);
        h_charge_hit_HG[i][j][k]->SetLineColor(wgColor::wgcolors[k]);
		// ADC count when there is a hit (hit bit is one) and the low gain preamp is selected
        h_charge_hit_LG[i][j][k] = new TH1D(Form("charge_hit_LG_chip%u_ch%u_col%u",i,j,k), Form("charge_hit_LG_chip%u_ch%u_col%u",i,j,k), bin, min_bin, max_bin);
        h_charge_hit_LG[i][j][k]->SetLineColor(wgColor::wgcolors[k]);
		// ADC count when there is not hit (hit bit is zero)
        h_charge_nohit[i][j][k] = new TH1D(Form("charge_nohit_chip%u_ch%u_col%u",i,j,k), Form("charge_nohit_chip%u_ch%u_col%u",i,j,k), bin, min_bin, max_bin);
        h_charge_nohit[i][j][k]->SetLineColor(wgColor::wgcolors[k+MEMDEPTH*2+2]);
		// Photo-electrons
		h_pe_hit[i][j][k] = new TH1D(Form("pe_hit_chip%u_ch%u_col%u",i,j,k), Form("pe_hit_chip%u_ch%u_col%u",i,j,k), bin, min_bin, max_bin);
		h_pe_hit[i][j][k]->SetLineColor(wgColor::wgcolors[k]);
		// TDC count when there is a hit (hit bit is one)
		h_time_hit[i][j][k] = new TH1D(Form("time_hit_chip%u_ch%u_col%u",i,j,k), Form("time_hit_chip%u_ch%u_col%u",i,j,k), bin, min_bin, max_bin);
		h_time_hit[i][j][k]->SetLineColor(wgColor::wgcolors[k]);
		// TDC count when there is not hit (hit bit is zero)
		h_time_nohit[i][j][k] = new TH1D(Form("time_nohit_chip%u_ch%u_col%u",i,j,k), Form("time_nohit_chip%u_ch%u_col%u",i,j,k), bin, min_bin, max_bin);   
		h_time_nohit[i][j][k]->SetLineColor(wgColor::wgcolors[k + MEMDEPTH * 2 + 2]);
      } //end col
      // BCID
      h_bcid_hit[i][j] = new TH1D(Form("bcid_hit_chip%u_ch%u",i,j), Form("bcid_hit_chip%u_ch%u",i,j), MAX_BCID_BIN, 0, MAX_BCID_BIN);
      h_bcid_hit[i][j]->SetLineColor(kBlack);
    } //end ch
  }   //end chip
  // === end make histgram ===

  wgGetTree * GetTree;
  try {
	GetTree = new wgGetTree( inputFileName, rd ); 
  }
  catch (const exception& e) {
	Log.eWrite("[wgMakeHist] failed to get the TTree : " + string(e.what()));
	exit(1);
  }
  TH1D * start_time;
  TH1D * stop_time;
  TH1D * nb_data_pkts;
  TH1D * nb_lost_pkts;
  start_time   = GetTree->GetHist_StartTime();
  stop_time    = GetTree->GetHist_StopTime();
  nb_data_pkts = GetTree->GetHist_DataPacket();
  nb_lost_pkts = GetTree->GetHist_LostPacket();

  TTree * tree = GetTree->tree_in;
  double max_spill = tree->GetMaximum("spill");
  double min_spill = tree->GetMinimum("spill");
  if(min_spill < 0) {
    Log.eWrite("[wgMakeHist] some spill value is missed : min_spill = " + to_string(min_spill));
  }

  outputHistFile->cd();
  // Pad the histogram with 100 empty bins on the left and on the right
  h_spill = new TH1D("spill", "Spill number weighted by the number of chips read", (int) max_spill - min_spill + 200, (int) min_spill - 100, (int) max_spill + 100);

  GetTree->finput->cd();
  int n_events = tree->GetEntries();

  //=================================================================//
  //                        EVENT LOOP                               //
  //=================================================================//
  
  for (int ievent = 0; ievent < n_events; ievent++) {

    if ( ievent % 1000 == 0 )
	  Log.Write("[wgMakeHist] Event number = " + to_string(ievent) + " / " + to_string(n_events));
	// Read one event
    tree->GetEntry(ievent);
	// Fill the spill histogram with using the spill_flag as a weight.
	// This means that the more chips are missing in a spill, the more that
	// spill number will be suppressed.
	// spill_flag: for each event counts the number of chips that were correctly
	// read
    h_spill->Fill(rd.spill, rd.spill_flag);

	// CHIPS loop
    for(unsigned i = 0; i < n_chips; i++) {
	  if( rd.chipid[i] < 0 || rd.chipid[i] >= (int) n_chips ) {
#ifdef DEBUG_MAKEHIST
		Log.Write("[wgMakeHist] event " + to_string(ievent) + " : chipid[" + to_string(i) + "] = " + to_string(rd.chipid[i])); 
#endif
		continue;
	  }
	  // chipid: chip ID tag as it is recorded in the chip trailer
      unsigned ichip = rd.chipid[i];
	  // CHANNELS loop
	  for(unsigned ichan = 0; ichan < n_channels; ichan++) {
		// COLUMNS loop
		for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
		  // HIT
          if ( rd.hit[i][ichan][icol] == HIT_BIT ) {
            h_bcid_hit  [ichip][ichan]->       Fill(rd.bcid[i][icol]);
            h_pe_hit    [ichip][ichan][icol]-> Fill(rd.pe[i][ichan][icol]);
            h_time_hit  [ichip][ichan][icol]-> Fill(rd.time[i][ichan][icol]);
			h_charge_hit[ichip][ichan][icol]-> Fill(rd.charge[i][ichan][icol] - rd.pedestal[ichip][ichan][icol]);
			// HIGH GAIN
            if(rd.gs[i][ichan][icol] == HIGH_GAIN_BIT) { 
              h_charge_hit_HG[ichip][ichan][icol]-> Fill(rd.charge[i][ichan][icol]);
            }
			// LOW GAIN
			else if(rd.gs[i][ichan][icol] == LOW_GAIN_BIT) {
              h_charge_hit_LG[ichip][ichan][icol]->Fill(rd.charge[i][ichan][icol]);
            }
#ifdef DEBUG_MAKEHIST	
			else Log.Write("[wgMakeHist] event " + to_string(ievent) + " : bad gain bit = " + to_string(rd.gs[i][ichan][icol] ));
#endif	  
          }
		  // NO HIT
		  else if ( rd.hit[i][ichan][icol] == NO_HIT_BIT ) {
            h_charge_nohit[ichip][ichan][icol]->Fill(rd.charge[i][ichan][icol]);
            h_time_nohit[ichip][ichan][icol]->Fill(rd.time[i][ichan][icol]);
          }//end hit
        }//end icol
      }//end ich
    }//end V_chipid
  }//end ievent
			
  Log.Write("     *****  Finished reading file  *****     ");
			
  outputHistFile->cd();

  // In some old runs the time and data packets info is not recorded
  if (start_time != NULL) start_time-> Write();
  if (stop_time != NULL) stop_time-> Write();
  if (nb_data_pkts != NULL) nb_data_pkts-> Write();
  if (nb_lost_pkts != NULL) nb_lost_pkts-> Write();
  if (h_spill != NULL) h_spill->Write();
  for (unsigned ichip = 0; ichip < n_chips; ichip++) {
    for (unsigned ichan = 0; ichan < n_channels; ichan++) {
      if (h_bcid_hit[ichip][ichan] != NULL) h_bcid_hit[ichip][ichan]->Write();
      for (unsigned icol = 0; icol < MEMDEPTH; icol++) {
		if (h_charge_hit[ichip][ichan][icol]    != NULL) h_charge_hit   [ichip][ichan][icol]->Write();
        if (h_charge_hit_HG[ichip][ichan][icol] != NULL) h_charge_hit_HG[ichip][ichan][icol]->Write();
		if (h_charge_hit_LG[ichip][ichan][icol] != NULL) h_charge_hit_LG[ichip][ichan][icol]->Write();
		if (h_pe_hit[ichip][ichan][icol]        != NULL) h_pe_hit       [ichip][ichan][icol]->Write();
		if (h_charge_nohit[ichip][ichan][icol]  != NULL) h_charge_nohit [ichip][ichan][icol]->Write();
        if (h_time_hit[ichip][ichan][icol]      != NULL) h_time_hit     [ichip][ichan][icol]->Write();
        if (h_time_nohit[ichip][ichan][icol]    != NULL) h_time_nohit   [ichip][ichan][icol]->Write();
      }
    }
  }

  outputHistFile->Close();
  Log.Write("[" + inputFileName + "][wgMakeHist] finished");
  delete GetTree;
  delete outputHistFile;
  return MH_SUCCESS;
}
