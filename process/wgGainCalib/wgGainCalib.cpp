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
#include "wgGainCalib.hpp"

void print_help(const char * program_name) {
  std::cout <<  program_name << " draws the inputDAC vs Gain graph and creates the gain_card.xml file.\n"
      "  -h         : help\n"
      "  -f (char*) : input directory (mandatory)\n"
      "  -o (char*) : output directory (default: same as input directory)\n"
      "  -i (char*) : output image directory (default: image directory)\n"
      "  -w         : ignore the WallMRDs\n";
  exit(0);
}

int main(int argc, char** argv){
  int opt;
  std::string input_dir("");
  std::string output_xml_dir("");
  std::string output_img_dir("");
  bool ignore_wallmrd = false;
  
  while ((opt = getopt(argc,argv, "f:o:i:hw")) != -1 ) {
    switch (opt) {
      case 'f':
        input_dir = optarg;
        break;
      case 'o':
        output_xml_dir = optarg; 
        break;
      case 'i':
        output_img_dir = optarg; 
        break;
      case 'w':
        ignore_wallmrd = true; 
        break;
      case 'h':
        print_help(argv[0]);
        break;
      default:
        print_help(argv[0]);
    }
  }

  int result;
  if ((result = wgGainCalib(input_dir.c_str(),
                            output_xml_dir.c_str(),
                            output_img_dir.c_str(),
                            ignore_wallmrd)) != WG_SUCCESS ) {
    Log.eWrite("[wgGainCalib] error " + std::to_string(result));
    exit(1);
  }
}
