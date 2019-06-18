// system C++ includes
#include <string>

// system C includes
#include <getopt.h>

// user includes
#include "wgConst.hpp"
#include "wgFileSystemTools.hpp"
#include "wgErrorCode.hpp"
#include "wgDecoder.hpp"
#include "wgLogger.hpp"

void print_help(const char * program_name) {
  cout << "this program decodes a .raw file into a .root file\n"
	"usage example: " << program_name << " -f inputfile.raw -r\n"
	"  -h         : help\n"
	"  -f (char*) : input .raw file that you want to read (mandatory)\n"
	"  -i (char*) : calibration card file\n"
	"  -p (char*) : pedestal card file\n"
	"  -t (char*) : tdc card file\n"
	"  -o (char*) : output directory (default = WAGASCI_DECODEDIR)\n"
	"  -x (int)   : number of ASU chips per DIF (must be 1-20)\n"
	"  -y (int)   : number of channels per chip (must be 1-36)\n"
	"  -r         : overwrite mode\n"
	"  -b         : batch (silent) mode\n";
  exit(0);
}

int main(int argc, char** argv) {
  // object used to check if various files exist or not 
  CheckExist check;

  // Get environment variables
  wgConst con;

  int opt;
  string inputFileName("");
  string calibFileName("");
  string pedFileName("");
  string tdcFileName("");
  string outputFile("");
  const string confDir(con.CONF_DIRECTORY);
  string outputDir(con.DECODE_DIRECTORY);
  bool overwrite = false;
  bool batch = false;
  unsigned n_chips = NCHIPS;
  unsigned n_channels = NCHANNELS;
  unsigned dif = 0;

  while((opt = getopt(argc,argv, "f:i:p:t:o:n:x:y:rbh")) !=-1 ){
    switch(opt){
	case 'f':
	  inputFileName = optarg;
	  break;
	case 'i':
	  calibFileName = optarg;
	  break;
	case 'p':
	  pedFileName = optarg;
	  break;
	case 't':
	  tdcFileName = optarg;
	  break;
	case 'o':
	  outputDir = optarg; 
	  break;
    case 'n':
	  n_chips = atoi(optarg);
	  break;
	case 'x':
	  n_chips = atoi(optarg);
	  break;
	case 'y':
	  n_channels = atoi(optarg);
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
  
  int retcode;
  if ( (retcode = wgDecoder(inputFileName.c_str(),
                            calibFileName.c_str(),
                            pedFileName.c_str(),
                            tdcFileName.c_str(),
                            outputDir.c_str(),
                            overwrite,
                            MAX_EVENT,
                            dif,
                            n_chips,
                            n_channels)) != DE_SUCCESS ) {
	Log.eWrite("Decoder failed with code " + to_string(retcode));
    exit(1);
  }
    
  exit(0);
}

