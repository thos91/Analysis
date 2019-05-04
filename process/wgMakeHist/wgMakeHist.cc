#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <stdio.h>

#include "TDirectory.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2F.h"
#include "TTree.h"

#include "Const.h"
#include "wgColor.h"
#include "wgTools.h"
#include "wgGetTree.h"
#include "wgExceptions.h"
#include "wgErrorCode.h"

#define DEBUG_MAKEHIST

using namespace std;

// print_help
// prints an help message with all the arguments taken by the program
void print_help(const char * program_name) {
  cout << "this program creates histogram from _tree.root file to _hist.root file\n"
	"usage example: " << program_name << " -f inputfile.raw -r\n"
	"  -h         : help\n"
	"  -f (char*) : input ROOT file (mandatory)\n"
	"  -o (char*) : output directory (default = WAGSASCI_HISTDIR)\n"
	"  -r         : overwrite mode\n";
  exit(0);
}

int MakeHist(const string& inputFileName, const string& outputDir, const bool overwrite, const size_t n_chips = NCHIPS, const size_t n_channels = NCHANNELS);

int main(int argc, char** argv) {

  CheckExist check;

  // Get environment variables
  wgConst con;
  con.GetENV();
  
  int opt;
  string inputFileName("");
  string outputDir = con.HIST_DIRECTORY;
  string outputFile("");
  bool overwrite = false;

  while((opt = getopt(argc,argv, "f:o:rh")) !=-1 ){
    switch(opt){
	case 'f':
	  inputFileName=optarg;
	  if(!check.RootFile(inputFileName)){ 
		cout<<"!!Error!! "<<inputFileName.c_str()<<"is wrong!!";
		Log.eWrite(Form("Error!!target:%s is wrong",inputFileName.c_str()));
		return 1;
	  }
	  cout << "== readfile :" << inputFileName.c_str() << " ==" << endl;
	  Log.Write(Form("[%s][wgMakeHist]start wgMakeHist",inputFileName.c_str()));
	  break;
	case 'o':
	  outputDir = optarg;
	  break;
	case 'r':
	  overwrite = true;
	  cout << "== mode : overwrite  ==" << endl;
	  Log.Write(Form("[%s][wgMakeHist] overwrite mode",inputFileName.c_str()));
	  break;
	case 'h':
	  print_help(argv[0]);
	  break;
	default:
	  print_help(argv[0]);
    }
  }

  if(inputFileName == ""){
    Log.eWrite("[Decoder] No input file");
    exit(1);
  }

  MakeHist(inputFileName, outputDir, overwrite);
  return 0;
}

//******************************************************************************
int MakeHist(const string& inputFileName, const string& outputDir, const bool overwrite, const size_t n_chips, const size_t n_channels) {

  OperateString OptStr;
  string outputHistFileName = OptStr.GetNameBeforeLastUnderBar(inputFileName)+"_hist.root";
  string logfilename  = OptStr.GetName(inputFileName);
  int pos             = logfilename.rfind("_ecal_dif_") ;
  string logfile      = OptStr.GetPath(inputFileName) + logfilename.substr(0, pos ) + ".log";

#ifndef DEBUG_MAKEHIST
  Log.Write("[" + logfilename + "][wgMakeHist] *****  READING FILE     :" + inputFileName      + "  *****");
  Log.Write("[" + logfilename + "][wgMakeHist] *****  OUTPUT HIST FILE :" + outputHistFileName + "  *****");
  Log.Write("[" + logfilename + "][wgMakeHist] *****  OUTPUT DIRECTORY :" + outputDir          + "  *****");
  Log.Write("[" + logfilename + "][wgMakeHist] *****  LOG FILE         :" + logfilename        + "  *****");
#endif
  
  TFile * outputHistFile;

  if (!overwrite){
    outputHistFile = new TFile((outputDir + "/" + outputHistFileName).c_str(), "create");
    if ( !outputHistFile->IsOpen() ) {
      Log.eWrite("[" + logfilename + "][wgMakeHist] Error:" + outputDir + "/" + outputHistFileName + " already exists!");
      return 1;
    }
  }
  else outputHistFile = new TFile((outputDir + "/" + outputHistFileName).c_str(), "recreate");

  // === start make histgram ===

  Raw_t rd(n_chips, n_channels);
  wgColor wgColor;

  TH1F* h_spill;
  TH1F* h_charge_hit    [n_chips][n_channels][MEMDEPTH];
  TH1F* h_charge_hit_HG [n_chips][n_channels][MEMDEPTH];
  TH1F* h_charge_hit_LG [n_chips][n_channels][MEMDEPTH];
  TH1F* h_pe_hit        [n_chips][n_channels][MEMDEPTH];
  TH1F* h_charge_nohit  [n_chips][n_channels][MEMDEPTH];
  TH1F* h_time_hit      [n_chips][n_channels][MEMDEPTH]; 
  TH1F* h_time_nohit    [n_chips][n_channels][MEMDEPTH]; 

  // h_bcid_hit: For every channel fill it with the BCID of all the columns with
  // a hit. This way all the channels in a chip will have a copy of the same
  // histogram
  TH1F* h_bcid_hit      [n_chips][n_channels]; 

  int min_bin = 0;
  int max_bin = 4096;
  int bin     = 4096;

  for (size_t i = 0; i < n_chips; i++) {
    for (size_t j = 0; j < n_channels; j++) {
      for (size_t k = 0; k < MEMDEPTH; k++) {
		// ADC count when there is a hit (hit bit is one)
		h_charge_hit[i][j][k] = new TH1F(Form("charge_chip%lu_ch%lu_col%lu",i,j,k), Form("charge_chip%lu_ch%lu_col%lu",i,j,k), bin, min_bin, max_bin);
		h_charge_hit[i][j][k]->SetLineColor(wgColor::wgcolors[k]);
        // ADC count when there is a hit (hit bit is one) and the high gain preamp is selected
        h_charge_hit_HG[i][j][k] = new TH1F(Form("charge_hit_HG_chip%lu_ch%lu_col%lu",i,j,k), Form("charge_hit_HG_chip%lu_ch%lu_col%lu",i,j,k), bin, min_bin, max_bin);
        h_charge_hit_HG[i][j][k]->SetLineColor(wgColor::wgcolors[k]);
		// ADC count when there is a hit (hit bit is one) and the low gain preamp is selected
        h_charge_hit_LG[i][j][k] = new TH1F(Form("charge_hit_LG_chip%lu_ch%lu_col%lu",i,j,k), Form("charge_hit_LG_chip%lu_ch%lu_col%lu",i,j,k), bin, min_bin, max_bin);
        h_charge_hit_LG[i][j][k]->SetLineColor(wgColor::wgcolors[k]);
		// ADC count when there is not hit (hit bit is zero)
        h_charge_nohit[i][j][k] = new TH1F(Form("charge_nohit_chip%lu_ch%lu_col%lu",i,j,k), Form("charge_nohit_chip%lu_ch%lu_col%lu",i,j,k), bin, min_bin, max_bin);
        h_charge_nohit[i][j][k]->SetLineColor(wgColor::wgcolors[k+MEMDEPTH*2+2]);
		// Photo-electrons
		h_pe_hit[i][j][k] = new TH1F(Form("pe_hit_chip%lu_ch%lu_col%lu",i,j,k), Form("pe_hit_chip%lu_ch%lu_col%lu",i,j,k), bin, min_bin, max_bin);
		h_pe_hit[i][j][k]->SetLineColor(wgColor::wgcolors[k]);
		// TDC count when there is a hit (hit bit is one)
		h_time_hit[i][j][k] = new TH1F(Form("time_hit_chip%lu_ch%lu_col%lu",i,j,k), Form("time_hit_chip%lu_ch%lu_col%lu",i,j,k), bin, min_bin, max_bin);
		h_time_hit[i][j][k]->SetLineColor(wgColor::wgcolors[k]);
		// TDC count when there is not hit (hit bit is zero)
		h_time_nohit[i][j][k] = new TH1F(Form("time_nohit_chip%lu_ch%lu_col%lu",i,j,k), Form("time_nohit_chip%lu_ch%lu_col%lu",i,j,k), bin, min_bin, max_bin);   
		h_time_nohit[i][j][k]->SetLineColor(wgColor::wgcolors[k + MEMDEPTH * 2 + 2]);
      } //end col
      // BCID
      h_bcid_hit[i][j] = new TH1F(Form("bcid_hit_chip%lu_ch%lu",i,j), Form("bcid_hit_chip%lu_ch%lu",i,j), bin, min_bin, max_bin);
      h_bcid_hit[i][j]->SetLineColor(kBlack);
    } //end ch
  }   //end chip
  // === end make histgram ===

  wgGetTree * GetTree;
  try {
	GetTree = new wgGetTree( inputFileName, rd ); 
  }
  catch (const exception& e) {
	Log.eWrite("[" + inputFileName + "][wgMakeHist] failed to get the TTree : " + string(e.what()));
	exit(1);
  }
  TH1F * start_time;
  TH1F * stop_time;
  TH1F * nb_data_pkts;
  TH1F * nb_lost_pkts;
  start_time   = GetTree->GetHist_StartTime();
  stop_time    = GetTree->GetHist_StopTime();
  nb_data_pkts = GetTree->GetHist_DataPacket();
  nb_lost_pkts = GetTree->GetHist_LostPacket();

  TTree * tree = GetTree->tree_in;
  float max_spill = tree->GetMaximum("spill");
  float min_spill = tree->GetMinimum("spill");
  if(min_spill < 0) {
    Log.eWrite("[" + inputFileName + "][wgMakeHist] some spill value is missed : min_spill = " + to_string(min_spill));
  }

  outputHistFile->cd();
  // Pad the histogram with 100 empty bins on the left and on the right
  h_spill = new TH1F("spill", "Spill number weighted by the number of chips read", (int) max_spill - min_spill + 200, (int) min_spill - 100, (int) max_spill + 100);

  GetTree->finput->cd();
  int ievent   = 0;
  int n_events = tree->GetEntries();

  for(ievent=0; ievent < n_events; ievent++){
    if ( ievent % 1000 == 0 )
	  Log.Write("Event number = " + to_string(ievent) + " / " + to_string(n_events));
	// Read one event
    tree->GetEntry(ievent);
	// Fill the spill histogram with using the spill_flag as a weight.
	// This means that if the more chips are missing in a spill, the more that
	// spill number will be suppressed.
	// spill_flag: for each event counts the number of chips that were correctly
	// read
    h_spill->Fill(rd.spill, rd.spill_flag);

	// CHIPS loop
    for(size_t i = 0; i < n_chips; i++) {
	  if( rd.chipid[i] < 0 || rd.chipid[i] >= (int) n_chips ) continue;
	  // chipid: chip ID tag as it is recorded in the chip trailer
      int ichip = rd.chipid[i];
#ifdef DEBUG_MAKEHIST
	  if (ichip != rd.chipid[i])
		Log.Write("[" + inputFileName + "][wgMakeHist] event " + to_string(ievent) + " : chipid[" + to_string(i) + "] = " + to_string(rd.chipid[i])); 
#endif
	  // COLUMNS loop
	  for(size_t icol = 0; icol < MEMDEPTH; icol++) {
		// CHANNELS loop
		for(size_t ich = 0; ich < n_channels; ich++) {
		  // HIT
          if ( rd.hit[i][ich][icol] == HIT_BIT ) {
            h_bcid_hit  [ichip][ich]->       Fill(rd.bcid[i][icol]);
            h_pe_hit    [ichip][ich][icol]-> Fill(rd.pe[i][ich][icol]);
            h_time_hit  [ichip][ich][icol]-> Fill(rd.time[i][ich][icol]);
			h_charge_hit[ichip][ich][icol]-> Fill(rd.charge[i][ich][icol] - rd.pedestal[ichip][ich][icol]);
			// HIGH GAIN
            if(rd.gs[i][ich][icol] == HIGH_GAIN_BIT) { 
              h_charge_hit_HG[ichip][ich][icol]-> Fill(rd.charge[i][ich][icol]);
            }
			// LOW GAIN
			else if(rd.gs[i][ich][icol] == LOW_GAIN_BIT) {
              //h_charge_hit_LG[ichip][ich][icol]->Fill(rd.charge[i][ich][icol]);
            }
#ifdef DEBUG_MAKEHIST	
			else Log.Write("[" + inputFileName + "][wgMakeHist] event " + to_string(ievent) + " : bad gain bit = " + to_string(rd.gs[i][ich][icol] ));
#endif	  
          }
		  // NO HIT
		  else if ( rd.hit[i][ich][icol] == NO_HIT_BIT ) {
            h_charge_nohit[ichip][ich][icol]->Fill(rd.charge[i][ich][icol]);
            h_time_nohit[ichip][ich][icol]->Fill(rd.time[i][ich][icol]);
          }//end hit
        }//end icol
      }//end ich
    }//end V_chipid
  }//end ievent
			
  Log.Write("     *****  Finished reading file  *****     ");
			
  outputHistFile->cd();

  start_time   -> Write();
  stop_time    -> Write();
  nb_data_pkts -> Write();
  nb_lost_pkts -> Write();
  h_spill->Write();
  for (size_t ichip = 0; ichip < n_chips; ichip++) {
    for (size_t ichan = 0; ichan < n_channels; ichan++) {
      h_bcid_hit[ichip][ichan]->Write();
      for (size_t icol = 0; icol < MEMDEPTH; icol++) {
		h_charge_hit   [ichip][ichan][icol]->Write();
        h_charge_hit_HG[ichip][ichan][icol]->Write();
		h_charge_hit_LG[ichip][ichan][icol]->Write();
		h_pe_hit       [ichip][ichan][icol]->Write();
		h_charge_nohit [ichip][ichan][icol]->Write();
        h_time_hit     [ichip][ichan][icol]->Write();
        h_time_nohit   [ichip][ichan][icol]->Write();
      }
    }
  }

  outputHistFile->Close();
  Log.Write("[" + inputFileName + "][wgMakeHist] finished");
  delete GetTree;
  return 0;
}

