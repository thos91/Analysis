// system includes
#include <iostream>
#include <string>

// system C includes
#include <bits/stdc++.h>

// user includes
#include "Const.h"
#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgAnaHist.hpp"

// print_help
// prints an help message with all the arguments taken by the program
void print_help(const char * program_name) {
  cout << program_name << "is used to analyze the histograms created by the \n"
	"wgAnaHist program. The result of the analysis is stored in the outputdir.\n"
	"usage example: " << program_name << " -f inputfile.root -r -m 10 -d 1\n"
	"  -h        : print this help\n"
	"  -f (char*): input histograms file (_hist.root file)\n"
	"  -i (char*): pyrame config file (.xml)\n"
	"  -o (char*): outputdir (default = XML_DIRECTORY)\n"
	"  -d (int)  : DIF number (integer starting from 1)\n"
	"  -x (int)  : number of chips per DIF (default is 20)\n"
	"  -y (int)  : number of channels per chip (default is 36)\n"
	"  -m (int)  : fit mode (mandatory)\n"
	"  -p        : print mode (default is false) \n"
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

//***************************** MAIN *************************************

int main(int argc, char** argv){
  int opt;
  int mode = 0;
  unsigned idif = 1;
  unsigned n_chans = 0, n_chips = 0;
  string inputFileName("");
  string configFileName("");
  bitset<M> flags;

  // Get the output directories from
  wgConst con;
  con.GetENV();
  string outputDir    = con.XMLDATA_DIRECTORY;
  string outputIMGDir = con.IMGDATA_DIRECTORY;

  OperateString OptStr;
  CheckExist Check;

  while((opt = getopt(argc,argv, "f:d:m:i:o:c:x:y:prh")) !=-1 ) {
    switch(opt) {
	case 'f':
	  inputFileName=optarg;
	  if(!Check.RootFile(inputFileName)){
		Log.eWrite("[" + OptStr.GetName(inputFileName) + "][wgAnaHist] target doesn't exist");
		exit(1);
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
	  flags[SELECT_CONFIG] = true;
	  if(!Check.XmlFile(configFileName)) {
		Log.eWrite("[" + OptStr.GetName(configFileName) + "][wgAnaHist] target doesn't exist");
		exit(1);
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
	  flags[SELECT_PRINT] = true;
	  break;
	case 'r':
	  flags[OVERWRITE] = true;
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

  Log.Write(" *****  READING FILE     : " + inputFileName + "  *****");
  Log.Write("start analyzing ...");
  
  AnaHist(inputFileName.c_str(),
		  configFileName.c_str(),
		  outputDir.c_str(),
		  outputIMGDir.c_str(),
		  mode,
		  flags.to_ulong(),
		  idif,
		  n_chips,
		  n_chans);

  Log.Write("[" + OptStr.GetName(inputFileName) + "][wgAnaHist] finished analyzing histograms");
}
