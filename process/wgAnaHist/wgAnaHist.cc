#include <string>
#include <iostream>
#include <vector>
#include <bits/stdc++.h>

#include <THStack.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TH1.h>

#include "Const.h"
#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgFit.h"
#include "wgFitConst.h"
#include "wgEditXML.h"

using namespace std;

// Number of flags
#define M 7

#define SELECT_CONFIG          0
#define SELECT_DARK_NOISE      1
#define SELECT_CHARGE_LOW      2
#define SELECT_PEDESTAL        3
#define SELECT_CHARGE_HG_LOW   4
#define SELECT_CHARGE_HG_HIGH  5
#define SELECT_PRINT           6

// print_help
// prints an help message with all the arguments taken by the program
void print_help(const char * program_name) {
  cout << program_name << "is used to analyze the histograms created by the \n"
	"wgMakeHist program. The result of the analysis is stored in the outputdir.\n"
	"usage example: " << program_name << " -f inputfile.raw -r -m 10 -d 1\n"
	"  -h        : print this help\n"
	"  -f (char*): input histograms file (_hist.root file)\n"
	"  -i (char*): pyrame config file (.xml)\n"
	"  -o (char*): outputdir (default = XML_DIRECTORY)\n"
	"  -d (int)  : DIF number (integer starting from 1)\n"
	"  -x (int)  : number of chips per DIF (default is 20)\n"
	"  -y (int)  : number of channels per chip (default is 36)\n"
	"  -m (int)  : fit mode (mandatory)\n"
	"  -p        : print mode (default is true) \n"
	"  -r        : overwrite mode (default is false)\n\n"
	"   =========   fit modes   ========= \n\n"
	"   1  : only dark noise\n"
	"   2  : only pedestal\n"
	"   3  : only charge_hit (low range)\n"
	"   4  : only charge_HG  (low range)\n"
	"   5  : only charge_HG  (high range)\n"
	"   10 : dark noise + charge_hit (low range)\n"
	"   11 : dark noise + pedestal + charge_hit (low range)\n"
	"   12 : dark noise + pedestal + charge_HG  (low range)\n"
	"   13 : dark noise + pedestal + charge_HG  (high range)\n"
	"   20 : dark noise + pedestal + charge_HG  (low range) + charge_HG (high range)\n";
  exit(0);
}

void MakeDir(const string& outputDir, unsigned n_chips = NCHIPS);
void MakeXMLFILE(const string& outputDir, bool overwrite, unsigned n_chips = NCHIPS, unsigned n_channels = NCHANNELS);
void ModeSelect(int mode, bitset<M>& flag);
void AnaHist(const string& inputFileName,
			 const string& configFileName,
			 const string& outputDir,
			 const string& outputIMGDir,
			 int mode,
			 bitset<M>& flag,
			 unsigned idif,
			 unsigned ichip,
			 unsigned n_chans = NCHANNELS);

//***************************** MAIN *************************************

int main(int argc, char** argv){
  int opt;
  int mode = 0;
  unsigned idif = 1;
  unsigned n_chans = 0, n_chips = 0;
  bool overwrite=false;
  string inputFileName("");
  string configFileName("");
  bitset<M> flag;

  // Get the output directories from
  wgConst con;
  con.GetENV();
  string outputDir    = con.XMLDATA_DIRECTORY;
  string outputIMGDir = con.IMGDATA_DIRECTORY;

  OperateString OptStr;
  CheckExist Check;

  while((opt = getopt(argc,argv, "f:d:m:i:o:c:x:y:prh")) !=-1 ) {
    switch(opt){
	case 'f':
	  inputFileName=optarg;
	  if(!Check.RootFile(inputFileName)){
		Log.eWrite("[" + OptStr.GetName(inputFileName) + "][wgAnaHist] target doesn't exist");
		return 1;
	  }
	  Log.Write("[" + OptStr.GetName(inputFileName) + "][wgAnaHist] start wgAnaHist");
	  break;
	case 'd':
	  idif = atoi(optarg);
	  break;
	case 'm':
	  mode = atoi(optarg);
	  break;
	case 'i':
	  configFileName = optarg;
	  flag[SELECT_CONFIG] = true;
	  if(!Check.XmlFile(configFileName)){
		Log.eWrite("[" + OptStr.GetName(configFileName) + "][wgAnaHist] target doesn't exist");
		return 1;
	  }
	  Log.Write("[" + OptStr.GetName(inputFileName) + "][wgAnaHist] read config file: " + configFileName);
	  break;
	case 'o':
	  outputDir = optarg;
	  break;
	case 'c':
	  outputIMGDir = optarg;
	  break;
	case 'x':
	  n_chips = atoi(optarg);
	  break;
	case 'y':
	  n_chans = atoi(optarg);
	  break;
	case 'p':
	  flag[SELECT_PRINT] = true;
	  break;
	case 'r':
	  overwrite = true;
	  break;
	case 'h':
	  print_help(argv[0]);
	  break;
	default:
	  print_help(argv[0]);
	  break;
    }   
  }

  if(inputFileName == "") {
    Log.eWrite("[wgAnaHist] No input file");
    exit(1);
  }

  string DirName = OptStr.GetNameBeforeLastUnderBar(inputFileName);

  outputDir = outputDir + "/" + DirName;
  outputIMGDir = outputIMGDir + "/" + DirName;

  MakeDir(outputDir);
  if(flag[SELECT_PRINT]) MakeDir(outputIMGDir);
  
  MakeXMLFILE(outputDir, overwrite, n_chips, n_chans);

  // ============== LOOP over all the chips =============== //
  
  for(unsigned ichip = 0; ichip < n_chips; ichip++) {
    AnaHist(inputFileName,
			configFileName,
			outputDir,
			outputIMGDir,
			mode,
			flag,
			idif,
			ichip,
			n_chans);
  }

  Log.Write("[" + OptStr.GetName(inputFileName) + "][wgAnaHist] finish analyzing histgram");
}

//******************************************************************
void MakeDir(const string& outputDir, const unsigned n_chips) {
  system( Form("mkdir -p %s", outputDir.c_str()) );
  for(unsigned i = 0; i < n_chips; i++)
    system(Form("mkdir -p %s", Form("%s/chip%u", outputDir.c_str(), i)));
}

//******************************************************************
void MakeXMLFILE(const string& outputDir, bool overwrite, const unsigned n_chips, const unsigned n_channels) {
  wgEditXML Edit;
  CheckExist Check;
  for(unsigned ichip = 0; ichip < n_chips; ichip++) {
    for(unsigned ichan = 0; ichan < n_channels; ichan++) {
      string outputxmlfile(outputDir + "/chip" + to_string(ichip) + "/ch" + to_string(ichan) + ".xml");
      if( !Check.XmlFile(outputxmlfile) || overwrite )
        Edit.Make(outputxmlfile,ichip,ichan);
    }
  }
}

//******************************************************************
void ModeSelect(const int mode, bitset<M>& flag){
  if ( mode == 0 )
	print_help("wgAnaHist");
  if ( mode == 1 || mode >= 10 )               flag[SELECT_DARK_NOISE]    = true;
  if ( mode == 2 || mode >= 11 )               flag[SELECT_PEDESTAL]  = true;
  if ( mode == 3 || mode == 10 || mode == 11 ) flag[SELECT_CHARGE_LOW]    = true;
  if ( mode == 4 || mode == 12 || mode >= 20 ) flag[SELECT_CHARGE_HG_LOW] = true;
}

//******************************************************************
void AnaHist(const string& inputFileName,
			 const string& configFileName,
			 const string& outputDir,
			 const string& outputIMGDir,
			 const int mode,
			 bitset<M>& flag,
			 const unsigned idif,
			 const unsigned ichip,
			 const unsigned n_chans) {

  Log.Write(" *****  READING FILE     : " + inputFileName + "  *****");
  Log.Write("start analyzing ...");

  ModeSelect(mode, flag);

  wgEditXML Edit;
  wgFit Fit(inputFileName);
  Fit.SetoutputIMGDir(outputIMGDir);
  string outputChipDir(outputDir + "/chip" + to_string(ichip));
  string outputxmlfile("");

  // v[channel][0] = global 10-bit discriminator threshold
  // v[channel][1] = global 10-bit gain selection discriminator threshold
  // v[channel][2] = adjustable input 8-bit DAC
  // v[channel][3] = adjustable 6-bit high gain (HG) preamp feedback capacitance
  // v[channel][4] = adjustable 4-bit discriminator threshold
  vector<vector<int>> config; // n_chans * 5 parameters

  Log.Write("start chip " + ichip);
  // Read the SPIROC2D configuration parameters from the configFileName (the xml
  // configuration file used during acquisition) into the "config" vector.
  if( flag[SELECT_CONFIG] ) Edit.GetConfig(configFileName, idif, ichip + 1, n_chans, config);

  // For the idif DIF and ichip chip, loop over all the channels
  for(unsigned ichan = 0; ichan < n_chans; ichan++){
	// Open the outputxmlfile as an XML file
	string outputxmlfile(outputChipDir + "/ch" + to_string(ichan));
    Edit.Open(outputxmlfile);

    // ******************** FILL THE XML FILES **********************//
	int start_time;
	int stop_time;
    if(ichip == 0) {
      start_time = Fit.GetHist->Get_start_time();
      stop_time  = Fit.GetHist->Get_stop_time();
    }
    Edit.SetConfigValue(string("start_time"), start_time);
    Edit.SetConfigValue(string("stop_time"), stop_time);

	//************ SELECT_CONFIG ************//

    if ( flag[SELECT_CONFIG] ) {
	  // Write the parameters values contained in the config vector into the
	  // outputxmlfile
      Edit.SetConfigValue(string("trigth"),   config[ichan][GLOBAL_THRESHOLD_INDEX], CREATE_NEW_MODE);
      Edit.SetConfigValue(string("gainth"),   config[ichan][GLOBAL_GS_INDEX], CREATE_NEW_MODE);
      Edit.SetConfigValue(string("inputDAC"), config[ichan][ADJ_INPUTDAC_INDEX], CREATE_NEW_MODE);
      Edit.SetConfigValue(string("HG"),       config[ichan][ADJ_AMPDAC_INDEX], CREATE_NEW_MODE);
	  Edit.SetConfigValue(string("trig_adj"), config[ichan][ADJ_THRESHOLD_INDEX], CREATE_NEW_MODE);
    }

	//************* SELECT_DARK_NOISE *************//

    if ( flag[SELECT_DARK_NOISE] ) {  //for bcid
      double x_bcid[2] = {0, 0};
	  // calculate the dark noise rate for chip "ichip" and channel "ichan" and
	  // save the mean and standard deviation in x_bcid[0] and x_bcid[1]
	  // respectively.
      Fit.NoiseRate(ichip, ichan, x_bcid, (int) flag[SELECT_PRINT]);
	  // Save the noise rate and its standard deviation in the outputxmlfile xml
	  // file
      Edit.SetChValue(string("NoiseRate"),   x_bcid[0], CREATE_NEW_MODE); // mean
      Edit.SetChValue(string("NoiseRate_e"), x_bcid[1], CREATE_NEW_MODE); // standard deviation
    }

	//************* SELECT_PEDESTAL *************//

    if ( flag[SELECT_PEDESTAL] ) {
      double x_nohit[3] = {0, 0, 0};
      for(int icol = 0; icol < MEMDEPTH; icol++) {
		// Calculate the pedestal value and its sigma
        Fit.charge_nohit(ichip, ichan, icol, x_nohit, (int) flag[SELECT_PRINT]);
        Edit.SetColValue(string("charge_nohit"), icol, x_nohit[0], CREATE_NEW_MODE);
		Edit.SetColValue(string("sigma_nohit"),  icol, x_nohit[1], CREATE_NEW_MODE);
      } 
    }

	//************* SELECT_CHARGE_LOW *************//

    if ( flag[SELECT_CHARGE_LOW] ) {
      double x_low[3] = {0, 0, 0};
      Fit.low_pe_charge(ichip, ichan, x_low, (int) flag[SELECT_PRINT]);
      Edit.SetChValue(string("charge_low"),x_low[0], CREATE_NEW_MODE);
      Edit.SetChValue(string("sigma_low") ,x_low[1], CREATE_NEW_MODE);
    }

	//************* SELECT_CHARGE_HG_LOW *************//

    if ( flag[SELECT_CHARGE_HG_LOW] ) {
      double x_low_HG[3] = {0, 0, 0};
      for(int icol = 0; icol < MEMDEPTH; icol++) {
        Fit.low_pe_charge_HG(ichip, ichan, icol, x_low_HG, (int) flag[SELECT_PRINT]);
        Edit.SetColValue(string("charge_lowHG"), icol, x_low_HG[0], CREATE_NEW_MODE);
        Edit.SetColValue(string("sigma_lowHG"),  icol, x_low_HG[1], CREATE_NEW_MODE);
      }
    }

	//************* SELECT_CHARGE_HG_HIGH *************//

    if ( flag[SELECT_CHARGE_HG_HIGH] ) {
      for(int icol = 0; icol < MEMDEPTH; icol++) {
        double x_high_HG[3] = {0, 0};
        Fit.GainSelect(ichip, ichan, icol, x_high_HG, (int) flag[SELECT_PRINT]);
		Edit.SetColValue(string("GS_eff_m"), icol, x_high_HG[0], CREATE_NEW_MODE);
        Edit.SetColValue(string("GS_eff_e"), icol, x_high_HG[1], CREATE_NEW_MODE);
      }
    }
    Edit.Write();
    Edit.Close();
  } //ichip
}



