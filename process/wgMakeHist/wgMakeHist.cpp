// system includes
#include <iostream>
#include <string>
#include <bitset>

// system C includes
#include <getopt.h>

// user includes
#include "wgFileSystemTools.hpp"
#include "wgErrorCodes.hpp"
#include "wgMakeHist.hpp"
#include "wgLogger.hpp"

using namespace wagasci_tools;

// print_help
// prints an help message with all the arguments taken by the program
void print_help(const char * program_name) {
  std::cout << "this program creates histograms from _tree.root file to _hist.root file\n"
      "usage example: " << program_name << " -f inputfile.raw -r\n"
      "  -h         : help\n"
      "  -f (char*) : input ROOT file (mandatory)\n"
      "  -p (char*) : input Pyrame config file (mandatory)\n"
      "  -o (char*) : output directory (default = WAGASCI_HISTDIR)\n"
      "  -n (int)   : DIF number (must be 0-7) (default = 0)\n"
      "  -r         : overwrite mode (default = false)\n"
      "  -m (int)   : mode (mandatory)\n\n"
      "   =========   modes   ========= \n\n"
      "   1  : only dark noise\n"
      "   2  : only charge\n"
      "   3  : only pedestal\n"
      "   4  : only time\n"
      "   10 : dark noise + pedestal + charge\n"
      "   11 : dark noise + charge + time\n"
      "   20 : everything\n";
  exit(0);
}

//******************************************************************
void ModeSelect(const unsigned long mode, std::bitset<makehist::NFLAGS>& flag){
  if ( mode == 1 || mode >= 10 )
    flag[makehist::SELECT_DARK_NOISE] = true;
  if ( mode == 2 || mode >= 10 ) {
    flag[makehist::SELECT_CHARGE_HG]  = true;
    flag[makehist::SELECT_CHARGE_LG]  = true;
    flag[makehist::SELECT_PEU]        = true;
  }
  if ( mode == 3 || mode == 10 || mode >= 20 )
    flag[makehist::SELECT_PEDESTAL]   = true;
  if ( mode == 4 || mode == 11 || mode >= 20 )
    flag[makehist::SELECT_TIME]       = true;
  if ( mode < 0  || mode > 20 )
    throw std::invalid_argument("Mode " + std::to_string(mode) +
                                " not recognized"); 
}

int main(int argc, char** argv) {
  int opt;
  int mode = 0;
  std::string input_file("");
  std::string output_dir("");
  std::string pyrame_config_file("");
  unsigned dif = 0;
  std::bitset<makehist::NFLAGS> flags;

  while((opt = getopt(argc,argv, "f:p:o:n:m:rh")) != -1 ){
    switch(opt){
      case 'f':
        input_file = optarg;
        break;
      case 'p':
        pyrame_config_file = optarg;
        break;
      case 'o':
        output_dir = optarg;
        break;
      case 'n':
        dif= std::stoi(optarg);
        break;
      case 'm':
        mode = atoi(optarg);
        break;
      case 'r':
        flags[makehist::OVERWRITE] = true;
        break;
      case 'h':
        print_help(argv[0]);
        break;
      default:
        print_help(argv[0]);
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  //                                   Mode                                  //
  /////////////////////////////////////////////////////////////////////////////

  // Set the correct flags according to the mode
  try { ModeSelect(mode, flags); }
  catch (const std::exception& e) {
    Log.eWrite("[wgMakeHist] Failed to select mode : " + std::string(e.what()));
    exit(1);
  }

  int result;
  if ((result = wgMakeHist(input_file.c_str(),
                           pyrame_config_file.c_str(),
                           output_dir.c_str(),
                           flags.to_ulong(),
                           dif)) != WG_SUCCESS ) {
    Log.eWrite("[wgMakeHist] returned error " +  std::to_string(result));
    exit(result);
  }
  exit(0);
}
