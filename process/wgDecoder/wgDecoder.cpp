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
      "  -f (char*) : input .raw file to read (mandatory)\n"
      "  -c (char*) : directory containing the calibration card files (default = WAGASCI_CONFDIR)\n"
      "  -o (char*) : output directory (default = WAGASCI_DECODEDIR)\n"
      "  -n (int)   : DIF number 1-8 (default = 1)\n"
      "  -x (int)   : number of ASU chips per DIF 1-20 (default = autodetected)\n"
      "  -r         : overwrite mode (default = false)\n"
      "  -q         : compatibility mode for old data (default = false)\n"
      "  -b         : silent mode (default = false)\n";
  exit(0);
}

int main(int argc, char** argv) {
  string inputFile("");
  string calibDir("");
  string outputDir("");

  int opt;
  bool overwrite = false;
  bool batch = false;
  bool compatibility_mode = false;
  unsigned n_chips = 0;
  unsigned dif = 0;

  while((opt = getopt(argc,argv, "f:c:o:n:x:y:rbqh")) !=-1) {
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
      case 'r':
        overwrite = true;
        break;
      case 'q':
        compatibility_mode = true;
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
  
  int retcode;
  if ( (retcode = wgDecoder(inputFile.c_str(),
                            calibDir.c_str(),
                            outputDir.c_str(),
                            overwrite,
                            compatibility_mode,
                            dif,
                            n_chips)) != WG_SUCCESS ) {
    Log.eWrite("[wgDecoder] Decoder failed with code " + to_string(retcode));
    exit(1);
  }
    
  exit(0);
}

