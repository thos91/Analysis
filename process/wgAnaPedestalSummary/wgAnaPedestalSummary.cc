// system includes
#include <string>

// user includes
#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgExceptions.h"
#include "Const.h"
#include "wgAnaPedestalSummary.hpp"

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
  con.GetENV();
  string inputDir("");
  string outputXMLDir("");
  string outputIMGDir = con.IMGDATA_DIRECTORY;

  OperateString OpStr;
  CheckExist check;

  while((opt = getopt(argc,argv, "f:o:i:n:x:y:h")) !=-1 ) {
    switch(opt){
	case 'f':
	  inputDir = optarg;
	  if( !check.Dir(inputDir) ) { 
		Log.eWrite("[" + OpStr.GetName(inputDir) + "][wgAnaPedestalSummary] target doesn't exist");
		return 1;
	  }   
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

  if (inputDir == "") {
    Log.eWrite("[wgAnaPedestal] No input directory");
    exit(1);
  }

  if (outputXMLDir == "") {
    outputXMLDir = con.CALIBDATA_DIRECTORY;
  } 

  Log.Write(" *****  READING DIRECTORY      :" + OpStr.GetName(inputDir)     + "  *****");
  Log.Write(" *****  OUTPUT XML DIRECTORY   :" + OpStr.GetName(outputXMLDir) + "  *****");
  Log.Write(" *****  OUTPUT IMAGE DIRECTORY :" + OpStr.GetName(outputIMGDir) + "  *****");


  AnaPedestalSummary(inputDir.c_str(),
					 outputXMLDir.c_str(),
					 outputIMGDir.c_str(),
					 n_difs,
					 n_chips,
					 n_chans);

  Log.Write("[" + OpStr.GetName(inputDir) + "][wgAnaPedestalSummary] Finished");
}

