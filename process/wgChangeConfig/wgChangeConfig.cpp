// system C++ includes
#include <string>
#include <iostream>
#include <bitset>

// system C includes
#include <getopt.h>

// user includes
#include "wgFileSystemTools.hpp"
#include "wgChangeConfig.hpp"
#include "wgLogger.hpp"

using namespace wagasci_tools;

void print_help(const char * program_name) {
  std::cout << program_name << " checks or changes a SPIROC bitstream file.\n"
      "  -h        : help\n"
      "  -f (char*): input file  (.txt file) (mandatory)\n"
      "  -o (char*): output file (default: test.txt)\n"
      "  -r        : over write mode (default: unset)\n"
      "  -e        : edit mode (default: unset (check mode))\n"
      "  -m        : parameter to modify\n"
      "              0 : trigger threshold    (10bit) 0-1023\n"
      "              1 : gainselect threshold (10bit) 0-1023\n"
      "              2 : inputDAC             (8bit)  0-255\n"
      "              3 : HG & LG amplifier    (6bit)  0-63\n"
      "              4 : threshold adjutment  (4bit)  0-15\n"
      "              5 : Chip ID              (8bit)  0-255\n"
      "              6 : inputDAC reference   (1bit)  0-1\n"
      "  -b        : channel (default :36 = all channels)\n"
      "  -v        : new value\n"
      "\n";
  exit(0);
}

//******************************************************************
int main(int argc, char** argv){
  int opt;
  unsigned mode    = 0;
  unsigned channel = 0;
  int value        = 0;
  bitset<WG_CHANGE_CONFIG_FLAGS> flags;
  string inputFile("");
  string outputFile("");
  string outputPath("");

  while ((opt = getopt(argc,argv, "f:o:m:b:v:her")) !=-1 ) {
    switch (opt) {
      case 'f':
        inputFile = optarg;
        break;

      case 'o':
        outputFile = optarg;
        break;

      case 'r':
        flags[OVERWRITE_FLAG] = true;
        break;

      case 'e':
        flags[EDIT_FLAG] = true;
        break;
      
      case 'm':
        mode = atoi(optarg);
        break;

      case 'b':
        channel = atoi(optarg);
        break;

      case 'v':
        value = atoi(optarg);
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
  if ( (result = wgChangeConfig(inputFile.c_str(),
                                outputFile.c_str(),
                                flags.to_ulong(),
                                value,
                                mode,
                                channel)) != 0) {
    Log.eWrite(std::string(argv[0]) + " returned error code " + std::to_string(result));
    exit(1);
  }

  exit(0);
}
