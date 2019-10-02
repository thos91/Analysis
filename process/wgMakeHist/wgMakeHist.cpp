// system includes
#include <iostream>
#include <string>
// system C includes
#include <getopt.h>

// user includes
#include "wgFileSystemTools.hpp"
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
      "  -r         : overwrite mode (default = false)\n";
  exit(0);
}

int main(int argc, char** argv) {
  int opt;
  std::string input_file("");
  std::string output_dir("");
  std::string pyrame_config_file("");
  bool overwrite = false;
  unsigned dif = 0;

  while((opt = getopt(argc,argv, "f:p:o:n:rh")) != -1 ){
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
      case 'r':
        overwrite = true;
        break;
      case 'h':
        print_help(argv[0]);
        break;
      default:
        print_help(argv[0]);
    }
  }

  int result;
  if ( (result = wgMakeHist(input_file.c_str(),
                            pyrame_config_file.c_str(),
                            output_dir.c_str(),
                            overwrite,
                            dif)) != WG_SUCCESS ) {
    Log.eWrite("[wgMakeHist] wgMakeHist returned error " + std::to_string(result));
    exit(1);
  }
  exit(0);
}
