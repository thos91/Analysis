// system includes
#include <iostream>
#include <string>

// user includes
#include "Const.hpp"
#include "wgTools.hpp"
#include "wgErrorCode.hpp"
#include "wgAnaHistSummary.hpp"

void print_help(const char * program_name) {
  cout <<  program_name << " summarizes the wgAnaHist output into a TO-DO.\n"
	"  -h         : help\n"
	"  -f (char*) : input directory (mandatory)\n"
	"  -o (char*) : output directory (default: same as input directory)\n"
	"  -i (char*) : output directory for plots and images (default: WAGASCI_IMGDIR) "
	"  -x (int)   : number of chips per DIF (default is 20)\n"
	"  -y (int)   : number of channels per chip (default is 36)\n"
	"  -p         : print plots and images\n"
	"  -r         : overwrite mode\n"
	"  -m (int)   : mode (default:10)\n"
	"   ===   mode  === \n"
	"   1  : Noise Rate\n"
    "   2  : Gain\n"
	"   3  : Pedestal\n"
	"   4  : Raw Charge\n"
	"   10 : Noise Rate + Gain\n"
	"   11 : Noise Rate + Gain + Pedestal\n"
	"   12 : Noise Rate + Gain + Pedestal + Raw Charge\n";
  exit(0);
}


//******************************************************************
int main(int argc, char** argv){

  int opt;
  int mode = 10;
  int n_chips = NCHIPS, n_chans = NCHANNELS;
  bool overwrite = false, print = false;
  wgConst con;
  con.GetENV();
  string inputDirName("");
  string configFileName("");
  string outputXMLDirName("");
  string outputDirName("");
  string logoutputDir(con.LOG_DIRECTORY);
  string outputIMGDirName(con.IMGDATA_DIRECTORY);
  
  OperateString OpStr;
  CheckExist check;

  while((opt = getopt(argc,argv, "f:o:i:x:y:m:rph")) !=-1 ){
    switch(opt){
	case 'f':
	  inputDirName=optarg;
	  if(!check.Dir(inputDirName)){ 
		Log.eWrite("[" + OpStr.GetName(inputDirName) + "][wgAnaHistSummary] target doesn't exist");
		return 1;
	  }   
	  break;
	case 'o':
	  outputXMLDirName = optarg; 
	  break;
	case 'i':
	  outputIMGDirName = optarg;
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
	case 'p':
	  print = true; 
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

  if(inputDirName == "") {
    Log.eWrite("[wgAnaPedestal] No input directory");
    exit(1);
  }

  Log.Write(" *****  READING DIRECTORY      :" + OpStr.GetName(inputDirName)     + "  *****");
  Log.Write(" *****  OUTPUT XML DIRECTORY   :" + OpStr.GetName(outputXMLDirName) + "  *****");
  Log.Write(" *****  OUTPUT IMAGE DIRECTORY :" + OpStr.GetName(outputIMGDirName) + "  *****");
  
  int result;
  if ( (result = wgAnaHistSummary(inputDirName.c_str(),
								  outputXMLDirName.c_str(),
								  outputIMGDirName.c_str(),
								  mode,
								  overwrite,
								  print,
								  n_chips,
								  n_chans)) != AHS_SUCCESS ) {
	Log.eWrite("[" + OpStr.GetName(inputDirName) + "][wgAnaHistSummary] wgAnaHistSummary returned error " + to_string(result));
	exit(1);
  }

  Log.Write("[" + OpStr.GetName(inputDirName) + "][wgAnaHistSummary] finished");
  exit(0);
}
