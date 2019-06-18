// system includes
#include <string>
#include <iostream>
#include <vector>
// system C includes
#include <getopt.h>
// user includes
#include "wgConst.hpp"
#include "wgFileSystemTools.hpp"
#include "wgErrorCode.hpp"
#include "wgPreCalib.hpp"

void print_help(const char * program_name) {
  cout <<  program_name << " draws the inputDAC vs Gain graph and creates the calibration_card.xml file.\n"
	"  -h         : help\n"
	"  -f (char*) : input directory (mandatory)\n"
	"  -o (char*) : output directory (default: same as input directory)\n"
	"  -i (char*) : output image directory (default: image directory)\n"
	"  -n (int)   : number of DIFs (default is 2)\n"
	"  -x (int)   : number of chips per DIF (default is 20)\n"
	"  -y (int)   : number of channels per chip (default is 36)\n"
	"  -m (int)   : 0:use only 2pe peak. 1:use 1pe and 2pe peaks (default:0)\n";
  exit(0);
}

int main(int argc, char** argv){
  int opt;
  int mode = 0;
  int n_difs = NDIFS, n_chips = NCHIPS, n_chans = NCHANNELS;
  wgConst con;
  con.GetENV();
  string inputDirName("");
  string outputXMLDirName("");
  string outputIMGDirName = con.IMGDATA_DIRECTORY;
  string logoutputDir = con.LOG_DIRECTORY;

  OperateString OpStr;
  CheckExist check;

  while((opt = getopt(argc,argv, "f:o:i:n:x:y:m:h")) !=-1 ){
    switch(opt){
	case 'f':
	  inputDirName=optarg;
	  if(!check.Dir(inputDirName)){
		Log.eWrite("[" + OpStr.GetName(inputDirName) + "][wgPreCalib] target doesn't exist");
		return 1;
	  }   
	  break;
	case 'o':
	  outputXMLDirName = optarg; 
	  break;
	case 'i':
	  outputIMGDirName = optarg; 
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
	case 'm':
	  mode = atoi(optarg); 
	  break;
	case 'h':
	  print_help(argv[0]);
	  break;
	default:
	  print_help(argv[0]);
    }   
  }

  if(inputDirName == "") {
    Log.eWrite("[wgPreCalib] No input directory");
    exit(1);
  }

  Log.Write(" *****  READING DIRECTORY      :" + inputDirName     + "  *****");
  Log.Write(" *****  OUTPUT XML DIRECTORY   :" + outputXMLDirName + "  *****");
  Log.Write(" *****  OUTPUT IMAGE DIRECTORY :" + outputIMGDirName + "  *****");

  int result;
  if ( (result = wgPreCalib(inputDirName.c_str(),
							outputXMLDirName.c_str(),
							outputIMGDirName.c_str(),
							mode,
							n_difs,
							n_chips,
							n_chans)) != PC_SUCCESS ) {
	Log.eWrite("[" +OpStr.GetName( inputDirName) + "][wgPreCalib] wgPreCalib returned error " + to_string(result));
	exit(1);
  }

  Log.Write("[" + OpStr.GetName(inputDirName) + "][wgPreCalib] Finished");
}
