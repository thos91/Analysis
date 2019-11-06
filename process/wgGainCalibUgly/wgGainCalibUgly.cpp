// system includes
#include <string>
#include <iostream>

// system C includes
#include <getopt.h>

// user includes
#include "wgConst.hpp"
#include "wgErrorCodes.hpp"
#include "wgFileSystemTools.hpp"
#include "wgLogger.hpp"
#include "wgGainCalibUgly.hpp"

void print_help(const char * program_name) {
  std::cout <<  program_name << " draws the inputDAC vs Gain graph and\n"
      "creates the gain_card.xml file. Ugly because it does not take into\n"
      "account the pedestal shift of the SPIROC2D.\n"
      "  -h         : help\n"
      "  -f (char*) : input run directory (mandatory)\n"
      "  -x (char*) : input Pyrame XML configuration file (mandatory)\n"
      "  -o (char*) : output directory (default: same as input directory)\n"
      "  -i (char*) : output image directory (default: image directory)\n"
      "  -w         : if set only the WallMRD DIF numbers will be analyzed\n";
  exit(0);
}

int main(int argc, char** argv){
  int opt;
  std::string input_dir("");
  std::string xml_config_file("");
  std::string output_xml_dir("");
  std::string output_img_dir("");
  bool ignore_wagasci = false;

  while ((opt = getopt(argc,argv, "f:x:o:i:wh")) != -1 ) {
    switch (opt) {
      case 'f':
        input_dir = optarg;
        break;
      case 'x':
        xml_config_file = optarg;
        break;
      case 'o':
        output_xml_dir = optarg; 
        break;
      case 'i':
        output_img_dir = optarg; 
        break;
      case 'w':
        ignore_wagasci = true;
        break;
      case 'h':
        print_help(argv[0]);
        break;
      default:
        print_help(argv[0]);
    }
  }

  int result;
  if ((result = wgGainCalibUgly(input_dir.c_str(),
                                xml_config_file.c_str(),
                                output_xml_dir.c_str(),
                                output_img_dir.c_str(),
                                ignore_wagasci)) != WG_SUCCESS ) {
    Log.eWrite("[wgGainCalibUgly] error " + std::to_string(result));
    exit(1);
  }
}
