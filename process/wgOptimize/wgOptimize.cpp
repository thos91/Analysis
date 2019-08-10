// system includes
#include <string>

// system C includes
#include <getopt.h>

// user includes
#include "wgFileSystemTools.hpp"

#include "wgConst.hpp"
#include "wgOptimize.hpp"
#include "wgLogger.hpp"

void print_help(std::string program_name) {
  std::cout <<
      "(mode 0) Set the optimal threshold given the inputDAC and PEU level\n"
      "         input values.The optimized threshold is defined as the\n"
      "         respective plateau region center in the Scurve plot.\n"
      "         The threshold_card.xml card file is needed.\n\n"
      "(mode 1) Set the optimal inputDAC and threshold given the PEU input\n"
      "         value. The inputDAC optimized value is defined as the value\n"
      "         of the the inputDAC so that the MPPC gain is as close as\n"
      "         possible to 40 ADC counts. The threshold_card.xml and\n"
      "         gain_card.xml card files are needed.\n\n"
      "Usage: " << program_name << " [OPTIONS]\n"
      "  -h         : print this help message\n"
      "  -m (int)   : mode selection (default 0)\n"
      "     0       :   optimized threshold\n"
      "     1       :   optimized threshold + inputDAC\n"
      "  -t (char*) : threshold card to read (mandatory)\n"
      "  -f (char*) : calibration card to read (mandatory only in mode 1)\n"
      "  -u (char*) : Pyrame configuration xml file\n"
      "  -s (char*) : spiroc2d bitstream files folder (default : \"/\")\n"
      "  -p (int)   : photo electrons equivalent threshold (mandatory)\n"
      "     1       :   0.5 p.e. equivalent threshold\n"
      "     2       :   1.5 p.e. equivalent threshold\n"
      "  -i (int)   : inputDAC (high voltage adjustment DAC) (only mode 0)\n"
      "     1+20*n  :   where n is in (0,12)\n";
}

int main(int argc, char** argv){ 

  int opt;
  std::string calibration_card("");
  std::string threshold_card("");
  std::string config_xml_file("");
  std::string wagasci_config_dif_dir("");
  std::string configDir("");
  std::string configName("");
  unsigned mode       = 0;
  unsigned inputDAC   = 121;
  unsigned peu        = 2;
  

  while((opt = getopt(argc, argv, "hm:t:f:u:s:p:i:")) != -1 ){
    switch(opt){
      case 't':
        threshold_card = optarg;
        break;
      case 'f':
        calibration_card = optarg;
        break;
      case 's':
        wagasci_config_dif_dir = optarg;
        break;
      case 'u':
        config_xml_file = optarg;
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
  int result;
  if ( (result = wgOptimize(threshold_card.c_str(),
                            calibration_card.c_str(),
                            config_xml_file.c_str(),
                            wagasci_config_dif_dir.c_str(),
                            mode,
                            peu,
                            inputDAC)) != WG_SUCCESS ) {
    Log.eWrite("[wgOptimize] wgOptimize returned error " + std::to_string(result));
    exit(1);
  }
  exit(0);
}
