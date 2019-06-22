// system includes
#include <iostream>
#include <string>

// system C includes
#include <bits/stdc++.h>
#include <getopt.h>
// user includes
#include "wgConst.hpp"
#include "wgFileSystemTools.hpp"
#include "wgErrorCode.hpp"
#include "wgAnaHist.hpp"
#include "wgLogger.hpp"

using namespace wagasci_tools;

// print_help
// prints an help message with all the arguments taken by the program
void print_help(const char * program_name) {
  cout << program_name << "is used to analyze the histograms created by the \n"
	"wgAnaHist program. The result of the analysis is stored in the outputdir.\n"
	"usage example: " << program_name << " -f inputfile.root -r -m 10 -d 1\n"
	"  -h        : print this help\n"
	"  -f (char*): input histograms file (_hist.root file)\n"
	"  -i (char*): pyrame config file (.xml)\n"
	"  -o (char*): outputXMLdir (default = WAGASCI_XMLDATADIR)\n"
	"  -q (char*): outputIMGdir (default = WAGASCI_IMGDATADIR)\n"
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
	"   14 : dark noise + pedestal + charge_HG  (low range) + charge_HG (high range)\n"
	"   20 : everything\n";
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
  string outputXMLDir = con.XMLDATA_DIRECTORY;
  string outputIMGDir = con.IMGDATA_DIRECTORY;

  CheckExist Check;

  while((opt = getopt(argc,argv, "f:d:m:i:o:q:x:y:prh")) !=-1 ) {
    switch(opt) {
	case 'f':
	  inputFileName = optarg;
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
	  break;
	case 'o':
	  outputXMLDir = optarg;
	  break;
	case 'q':
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
	  flags[SELECT_OVERWRITE] = true;
	  break;
	case 'h':
	  print_help(argv[0]);
	  break;
	default:
	  print_help(argv[0]);
	  break;
    }
  }

  int result;
  if ( (result = wgAnaHist(inputFileName.c_str(),
                           configFileName.c_str(),
                           outputXMLDir.c_str(),
                           outputIMGDir.c_str(),
                           mode,
                           flags.to_ulong(),
                           idif,
                           n_chips,
                           n_chans)) != AH_SUCCESS ) {
	Log.eWrite("[" + GetName( inputFileName) + "][wgAnaHist] wgAnaHist returned error " + to_string(result));
	exit(1);
  }

  exit(0);
}
