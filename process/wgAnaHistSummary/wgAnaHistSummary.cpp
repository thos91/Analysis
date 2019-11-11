// system includes
#include <iostream>
#include <string>
#include <bitset>

// system C includes
#include <getopt.h>

// user includes
#include "wgConst.hpp"
#include "wgErrorCodes.hpp"
#include "wgAnaHist.hpp"
#include "wgAnaHistSummary.hpp"
#include "wgLogger.hpp"

void print_help(const char * program_name) {
  std::cout <<  program_name << " summarizes the output of wgAnaHist into \n"
      "a more readable form.\n"
      "  -h         : help\n"
      "  -f (char*) : input directory (mandatory)\n"
      "  -o (char*) : output directory (default: same as input directory)\n"
      "  -i (char*) : output directory for plots and images (default: WAGASCI_IMGDIR) "
      "  -p         : print plots and images\n"
      "  -r         : overwrite mode\n"
      "  -m (int)   : mode (default:10)\n"
      "   ===   mode  === \n"
      "   1  : Noise Rate\n"
      "   2  : Pedestal\n"
      "   3  : Raw Charge\n"
      "   10 : Noise Rate + Pedestal\n"
      "   11 : Noise Rate + Raw Charge\n"
      "   20 : everything\n";
  exit(0);
}

namespace anahistsummary {

void select_mode(int mode, std::bitset<anahist::NFLAGS>& flags) {
  if (mode == 1 || mode >= 10)
    flags[anahist::SELECT_DARK_NOISE] = true;
  if (mode == 2 || mode == 10 || mode >= 20)
    flags[anahist::SELECT_PEDESTAL] = true;
  if (mode == 3 || mode == 11 || mode >= 20)
    flags[anahist::SELECT_CHARGE_HG] = true;
  if (mode < 0  || mode > 20)
    throw std::invalid_argument("Mode " + std::to_string(mode) +
                                " not recognized"); 
}

} // anahistsummary

//******************************************************************
int main(int argc, char** argv){

  int opt;
  int mode = 10;

  wgEnvironment env;
  std::string input_dir_name("");
  std::string output_xml_dir_name("");
  std::string output_img_dir_name(env.IMGDATA_DIRECTORY);
  std::bitset<anahist::NFLAGS> flags;
    
  while((opt = getopt(argc, argv, "f:o:i:m:rph")) !=-1 ){
    switch(opt){
      case 'f':
        input_dir_name = optarg;
        break;
      case 'o':
        output_xml_dir_name = optarg; 
        break;
      case 'i':
        output_img_dir_name = optarg;
        break;	  
      case 'm':
        mode = atoi(optarg); 
        break;
      case 'p':
        flags[anahist::SELECT_PRINT] = true; 
        break;
      case 'r':
        flags[anahist::SELECT_OVERWRITE] = true; 
        break;
      case 'h':
        print_help(argv[0]);
        break;
      default:
        print_help(argv[0]);
    }   
  }

  // =========== FLAGS decoding =========== //
  
  // Set the correct flags according to the mode
  try { anahistsummary::select_mode(mode, flags); }
  catch (const std::exception& e) {
    Log.eWrite("[wgAnaHistSummary] Failed to " + std::string(e.what()));
    exit(ERR_WRONG_MODE);
  }

  int result;
  if ( (result = wgAnaHistSummary(input_dir_name.c_str(),
                                  output_xml_dir_name.c_str(),
                                  output_img_dir_name.c_str(),
                                  flags.to_ulong())) != WG_SUCCESS ) {
    Log.eWrite("[wgAnaHistSummary] wgAnaHistSummary returned error "
               + std::to_string(result));
  }

  exit(result);
}
