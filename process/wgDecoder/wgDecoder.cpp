// system C++ includes
#include <string>

// system C includes
#include <getopt.h>

// user includes
#include "wgConst.hpp"
#include "wgFileSystemTools.hpp"

#include "wgDecoder.hpp"
#include "wgLogger.hpp"

void print_help(const char * program_name) {
  cout << "this program decodes a .raw file into a .root file\n"
      "usage example: " << program_name << " -f inputfile.raw -r\n"
      "  -h         : help\n"
      "  -f (char*) : input .raw file that you want to read (mandatory)\n"
      "  -c (char*) : directory containing the calibration card files\n"
      "  -o (char*) : output directory (default = WAGASCI_DECODEDIR)\n"
      "  -n (int)   : DIF number (must be 1-8)\n"
      "  -x (int)   : number of ASU chips per DIF (must be 1-20)\n"
      "  -y (int)   : number of channels per chip (must be 1-36)\n"
      "  -e (int)   : maximum number of spills to record (default = UINT_MAX)"
      "  -r         : overwrite mode\n"
      "  -b         : batch (silent) mode\n";
  exit(0);
}

int main(int argc, char** argv) {
  string inputFile("");
  string calibDir("");
  string outputFile("");
  string outputDir("");

  int opt;
  bool overwrite = false;
  bool batch = false;
  unsigned n_chips = 0;
  unsigned n_channels = 0;
  unsigned dif = 0;
  unsigned max_n_events = 0;

  while((opt = getopt(argc,argv, "f:c:o:n:x:y:rbh")) !=-1) {
    switch (opt) {
      case 'f':
        inputFile = optarg;
        break;
      case 'c':
        calibDir = optarg;
        break;
      case 'o':
        outputDir = optarg; 
        break;
      case 'n':
        dif = atoi(optarg);
        break;
      case 'x':
        n_chips = atoi(optarg);
        break;
      case 'y':
        n_channels = atoi(optarg);
        break;
      case 'e':
        max_n_events = atoi(optarg);
        break;
      case 'r':
        overwrite = true;
        break;
      case 'b':
        batch = true;
        break;
      case 'h':
        print_help(argv[0]);
        break;
      default:
        print_help(argv[0]);
    }
  }

  if (batch == true) Log.WhereToLog = LOGFILE;
  
  // int retcode;
  // if ( (retcode = wgDecoder(inputFile.c_str(),
  //                           calibDir.c_str(),
  //                           outputDir.c_str(),
  //                           overwrite,
  //                           max_n_events,
  //                           dif,
  //                           n_chips,
  //                           n_channels)) != DE_SUCCESS ) {
  //   Log.eWrite("[wgDecoder] Decoder failed with code " + to_string(retcode));
  //   exit(1);
  // }
    
  exit(0);
}

