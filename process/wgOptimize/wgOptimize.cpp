// system includes
#include <string>

// system C includes
#include <getopt.h>

// user includes
#include "wgFileSystemTools.hpp"
#include "wgErrorCodes.hpp"
#include "wgConst.hpp"
#include "wgLogger.hpp"
#include "wgOptimize.hpp"

void print_help(std::string program_name) {
  std::cout <<
      "(mode 0) Set the optimal threshold given the inputDAC and PEU level\n"
      "         input values. The optimized threshold is defined as the\n"
      "         center of the plateau region in the Scurve plot.\n"
      "         The threshold_card.xml card file is needed.\n\n"
      "(mode 1) Set the optimal inputDAC and threshold given the PEU input\n"
      "         value. The inputDAC optimized value is defined as the value\n"
      "         of the the inputDAC so that the MPPC gain is as close as\n"
      "         possible to 40 ADC counts. The threshold_card.xml AND\n"
      "         gain_card.xml card files are needed.\n"
      "(mode 2 and 3) Set only the default values for WallMRDs (threshold = "
            <<  WALL_MRD_THRESHOLD << ", input DAC = " << WALL_MRD_INPUTDAC
            << ")\n"
      "Usage: " << program_name << " [OPTIONS]\n"
      "  -h         : print this help message\n"
      "  -m (int)   : mode selection (default 0)\n"
      "     0       :   optimized threshold\n"
      "     1       :   optimized threshold + inputDAC\n"
      "     2       :   optimized threshold + default values for WallMRD\n"
      "     3       :   optimized threshold + inputDAC + default values for WallMRD\n"
      "  -t (char*) : threshold card to read (mandatory)\n"
      "             :   default : \"/opt/calicoes/config/threshold_card.xml\"\n"
      "  -f (char*) : gain calibration card to read (mandatory only in mode 1)\n"
      "             :   default : \"/opt/calicoes/config/gain_card.xml\"\n"
      "  -u (char*) : Pyrame configuration xml file or topology string\n"
      "  -s (char*) : spiroc2d bitstream files folder (default : \"/tmp/calicoes\")\n"
      "  -p (int)   : photo electrons equivalent threshold (default 2)\n"
      "     1       :   0.5 p.e. equivalent threshold\n"
      "     2       :   1.5 p.e. equivalent threshold\n"
      "  -i (int)   : inputDAC (high voltage adjustment DAC) (used only in mode 0)\n"
      "     1+20*n  :   where n is in (0,12)\n"
      "             :   default : 241\n";
}

int main(int argc, char** argv) { 

  int opt;
  std::string threshold_card("");
  std::string gain_card("");
  std::string topology_source("");
  std::string wagasci_bitstream_dir("");
  unsigned mode       = 0;
  unsigned inputDAC   = 241;
  unsigned peu        = 2;

  while((opt = getopt(argc, argv, "hm:t:f:u:s:p:i:")) != -1 ){
    switch(opt){
      case 't':
        threshold_card = optarg;
        break;
      case 'f':
        gain_card = optarg;
        break;
      case 's':
        wagasci_bitstream_dir = optarg;
        break;
      case 'u':
        topology_source = optarg;
        break; 
      case 'i':
        inputDAC = atoi(optarg);
        break;
      case 'm':
        mode = atoi(optarg);
        break;
      case 'p':
        peu = atoi(optarg);
        break;
      case 'h':
        print_help(argv[0]);
        exit(0);
      default:
        print_help(argv[0]);
        exit(0);
    }   
  }

  std::bitset<optimize::NUM_OPTIMIZE_MODES> flags;

  switch (mode) {
    case 0:
      flags[optimize::OP_THRESHOLD_MODE] = true;
      break;
    case 1:
      flags[optimize::OP_INPUTDAC_MODE] = true;
      break;
    case 2:
      flags[optimize::OP_THRESHOLD_MODE] = true;
      flags[optimize::OP_WALL_MRD] = true;
      break;
    case 3:
      flags[optimize::OP_THRESHOLD_MODE] = true;
      flags[optimize::OP_WALL_MRD] = true;
      break;
    default:
      Log.eWrite("[wgOptimize] wrong mode.");
      exit(ERR_WRONG_MODE);
      break;                                           
  }
  
  int result;
  if ((result = wgOptimize(threshold_card.c_str(),
                           gain_card.c_str(),
                           topology_source.c_str(),
                           wagasci_bitstream_dir.c_str(),
                           flags.to_ulong(),
                           peu,
                           inputDAC)) != WG_SUCCESS) {
    Log.eWrite("[wgOptimize] wgOptimize returned error " +
               std::to_string(result));
  }
  exit(result);
}
