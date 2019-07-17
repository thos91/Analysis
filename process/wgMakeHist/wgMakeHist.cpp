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
      "  -o (char*) : output directory (default = WAGASCI_HISTDIR)\n"
      "  -x (int)   : number of ASU chips per DIF (must be 1-20)\n"
      "  -r         : overwrite mode\n";
  exit(0);
}

int main(int argc, char** argv) {
  int opt;
  std::string inputFileName("");
  std::string outputDir("");
  std::string outputFile("");
  bool overwrite = false;
  unsigned n_chips = NCHIPS;

  while((opt = getopt(argc,argv, "f:o:x:rh")) != -1 ){
    switch(opt){
      case 'f':
        inputFileName = optarg;
        break;
      case 'o':
        outputDir = optarg;
        break;
      case 'x':
        n_chips = std::stoi(optarg);
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
  if ( (result = wgMakeHist(inputFileName.c_str(),
                            outputDir.c_str(),
                            overwrite,
                            n_chips)) != WG_SUCCESS ) {
    Log.eWrite("[wgMakeHist] wgMakeHist returned error " + std::to_string(result));
    exit(1);
  }
  exit(0);
}
