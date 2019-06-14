// system includes
#include <iostream>
#include <string>
// system C includes
#include <getopt.h>

// user includes
#include "wgErrorCode.hpp"
#include "wgTools.hpp"
#include "wgMakeHist.hpp"

// print_help
// prints an help message with all the arguments taken by the program
void print_help(const char * program_name) {
  cout << "this program creates histograms from _tree.root file to _hist.root file\n"
	"usage example: " << program_name << " -f inputfile.raw -r\n"
	"  -h         : help\n"
	"  -f (char*) : input ROOT file (mandatory)\n"
	"  -o (char*) : output directory (default = WAGASCI_HISTDIR)\n"
	"  -x (int)   : number of ASU chips per DIF (must be 1-20)\n"
	"  -y (int)   : number of channels per chip (must be 1-36)\n"
	"  -r         : overwrite mode\n";
  exit(0);
}

int main(int argc, char** argv) {

  CheckExist check;
  OperateString OptStr;

  // Get environment variables
  wgConst con;
  con.GetENV();
  
  int opt;
  string inputFileName("");
  string outputDir = con.HIST_DIRECTORY;
  string outputFile("");
  bool overwrite = false;
  unsigned n_chips = NCHIPS;
  unsigned n_channels = NCHANNELS;

  while((opt = getopt(argc,argv, "f:o:x:y:rh")) !=-1 ){
    switch(opt){
	case 'f':
	  inputFileName=optarg;
	  if(!check.RootFile(inputFileName)){ 
		Log.eWrite("[wgMakeHist] " + inputFileName + " not found");
		exit(1);
	  }
	  break;
	case 'o':
	  outputDir = optarg;
	  break;
	case 'x':
	  n_chips = atoi(optarg);
	  if( n_chips <= 0 && n_chips > NCHIPS ) {
		Log.eWrite("[" + OptStr.GetName(inputFileName) + "][wgMakeHist] The number of chips per DIF must be {1-" + to_string(NCHIPS) + "}");
		exit(1);
	  }
	  break;
	case 'y':
	  n_channels = atoi(optarg);
	  if( n_channels <= 0 && n_channels > NCHANNELS ) {
		Log.eWrite("[" + OptStr.GetName(inputFileName) + "][wgMakeHist] The number of channels per DIF must be {1-" + to_string(NCHANNELS) + "}");
		exit(1);
	  }
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

  if(inputFileName == ""){
    Log.eWrite("[wgMakeHist] No input file");
    exit(1);
  }
  
  int result;
  if ( (result = wgMakeHist(inputFileName.c_str(),
                            outputDir.c_str(),
                            overwrite,
                            n_chips,
                            n_channels)) != MH_SUCCESS ) {
	Log.eWrite("[" + OptStr.GetName(inputFileName) + "][wgMakeHist] wgMakeHist returned error " + to_string(result));
    exit(1);
  }
	exit(0);
}
