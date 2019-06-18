// system includes
#include <string>

// system C includes
#include <getopt.h>
// user includes
#include "wgFileSystemTools.hpp"
#include "wgErrorCode.hpp"
#include "wgExceptions.hpp"
#include "wgConst.hpp"
#include "wgAnaPedestalSummary.hpp"
#include "wgLogger.hpp"

using namespace wagasci_tools;

void print_help(const char * program_name) {
  cout <<  program_name << " TO DO.\n"
	"  -h         : help\n"
	"  -f (char*) : input directory (mandatory)\n"
	"  -o (char*) : output directory (default: same as input directory)\n"
	"  -i (char*) : output directory for plots and images (default: WAGASCI_IMGDIR)\n"
	"  -n (int)   : number of DIFs (default is 2)\n"
	"  -x (int)   : number of chips per DIF (default is 20)\n"
	"  -y (int)   : number of channels per chip (default is 36)\n";
  exit(0);
}

int main(int argc, char** argv){
  int opt, n_difs = NDIFS, n_chips = NCHIPS, n_chans = NCHANNELS;
  wgConst con;
  string inputDir("");
  string outputXMLDir = con.XMLDATA_DIRECTORY;
  string outputIMGDir = con.IMGDATA_DIRECTORY;


  while((opt = getopt(argc,argv, "f:o:i:n:x:y:h")) !=-1 ) {
    switch(opt){
	case 'f':
	  inputDir = optarg;
	  break;
	case 'o':
	  outputXMLDir = optarg; 
	  break;
	case 'i':
	  outputIMGDir = optarg; 
	  break;
	case 'n':
	  n_difs = atoi(optarg);
	  break;
	case 'x':
	  n_chips = atoi(optarg);
	  break;
	case 'y':
	  n_chans = atoi(optarg);
	  break;
	case 'h':
	  print_help(argv[0]);
	  break;
	default:
	  print_help(argv[0]);
    }   
  }

  int result;
  if ( (result = wgAnaPedestalSummary(inputDir.c_str(),
                                      outputXMLDir.c_str(),
                                      outputIMGDir.c_str(),
                                      n_difs,
                                      n_chips,
                                      n_chans)) != APS_SUCCESS ) {
    Log.Write("[wgAnaPedestalSummary] Returned error code " + to_string(result));
  }

}

