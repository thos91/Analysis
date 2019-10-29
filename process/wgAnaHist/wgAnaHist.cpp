// system includes
#include <iostream>
#include <string>
#include <bitset>

// system C includes
#include <getopt.h>

// user includes
#include "wgConst.hpp"
#include "wgErrorCodes.hpp"
#include "wgFileSystemTools.hpp"
#include "wgAnaHist.hpp"
#include "wgLogger.hpp"

using namespace wagasci_tools;

// print_help
// prints an help message with all the arguments taken by the program
void print_help(const char * program_name) {
  std::cout << program_name << "is used to analyze the histograms created by the \n"
      "wgAnaHist program. The result of the analysis is stored in the outputdir.\n"
      "usage example: " << program_name << " -f inputfile.root -r -m 10 -d 1\n"
      "  -h        : print this help\n"
      "  -f (char*): input histograms file (_hist.root file)\n"
      "  -p (char*): pyrame config file (.xml)\n"
      "  -o (char*): outputXMLdir (default = WAGASCI_XMLDATADIR)\n"
      "  -i (char*): outputIMGdir (default = WAGASCI_IMGDATADIR)\n"
      "  -n (int)  : DIF number (default = 0)\n"
      "  -m (int)  : fit mode (mandatory)\n"
      "  -q        : compatibility mode (default is false) \n"
      "  -s        : print mode (default is false) \n"
      "  -r        : overwrite mode (default is false)\n\n"
      "   =========   fit modes   ========= \n\n"
      "   1  : only dark noise\n"
      "   2  : only pedestal\n"
      "   3  : only charge_hit_LG\n"
      "   4  : only charge_hit_HG\n"
      "   10 : dark noise + pedestal + charge_hit_LG\n"
      "   11 : dark noise + pedestal + charge_hit_HG\n"
      "   20 : everything\n";
  exit(0);
}

//***************************** MAIN *************************************

int main(int argc, char** argv){
  int opt;
  int mode = 0;
  unsigned dif = 0;
  std::string inputFileName("");
  std::string configFileName("");
  std::bitset<ANAHIST_NFLAGS> flags;

  // Get the output directories from
  wgEnvironment env;
  std::string outputXMLDir = env.XMLDATA_DIRECTORY;
  std::string outputIMGDir = env.IMGDATA_DIRECTORY;

  while((opt = getopt(argc,argv, "f:n:m:p:o:i:sqrh")) !=-1 ) {
    switch(opt) {
      case 'f':
        inputFileName = optarg;
        break;
      case 'n':
        dif = atoi(optarg);
        break;
      case 'm':
        mode = atoi(optarg);
        break;
      case 'p':
        configFileName = optarg;
        flags[SELECT_CONFIG] = true;
        break;
      case 'o':
        outputXMLDir = optarg;
        break;
      case 'i':
        outputIMGDir = optarg;
        break;
      case 'q':
        flags[SELECT_COMPATIBILITY] = true;
        break;
      case 's':
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
                           dif)) != WG_SUCCESS ) {
    Log.eWrite("[wgAnaHist] wgAnaHist returned error " + std::to_string(result));
    exit(1);
  }

  exit(0);
}
