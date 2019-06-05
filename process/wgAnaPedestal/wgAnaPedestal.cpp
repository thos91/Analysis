// system C++ includes
#include <string>
#include <iostream>

// user includes
#include "wgTools.hpp"
#include "wgErrorCode.hpp"
#include "Const.hpp"
#include "wgAnaPedestal.hpp"

void print_help(const char * program_name) {
  cout <<  program_name << " is used to summarize the information contained in the\n"
	"xml files created by the wgAnaHist program.\n"
	"  -h         : help\n"
	"  -f (char*) : input directory (mandatory)\n"
	"  -o (char*) : output directory (default: same as input directory)\n"
	"  -i (char*) : output directory for plots and images (default: WAGASCI_IMGDIR)\n"
	"  -r         : overwrite mode\n"
	"  -x (int)   : number of chips per DIF (default is 20)\n"
	"  -y (int)   : number of channels per chip (default is 36)\n";
  exit(0);
}
  
//******************************************************************
int main(int argc, char** argv){

  int opt;
  bool overwrite = false;
  wgConst con;
  con.GetENV();
  string inputDir("");
  string configFileName("");
  string outputIMGDir=con.IMGDATA_DIRECTORY;
  string outputXMLDir("");
  string outputDir("");
  string logoutputDir=con.LOG_DIRECTORY;

  int n_chips = NCHIPS, n_chans = NCHANNELS;

  OperateString OpStr;
  CheckExist check;

  while((opt = getopt(argc,argv, "f:o:i:x:y:hr")) !=-1 ){
    switch(opt){
	case 'f':
	  inputDir=optarg;
	  if(!check.Dir(inputDir)){ 
		cout<<"!!Error!! "<< inputDir.c_str() << "doesn't exist!!";
		Log.eWrite("[" + OpStr.GetName(inputDir) + "][wgAnaHistSummary] target doesn't exist");
		return 1;
	  }   
	  break;

	case 'o':
	  outputXMLDir = optarg; 
	  break;

	case 'i':
	  outputIMGDir = optarg; 
	  break;
	  
	case 'x':
	  n_chips = atoi(optarg);
	  break;

	case 'y':
	  n_chans = atoi(optarg);
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

  if(inputDir == "") {
    Log.eWrite("[wgAnaPedestal] No input directory");
    exit(1);
  }
  
  if(outputXMLDir == "") outputXMLDir = inputDir;

  Log.Write(" *****  READING DIRECTORY      : " + OpStr.GetName(inputDir)     + "  *****");
  Log.Write(" *****  OUTPUT XML DIRECTORY   : " + OpStr.GetName(outputXMLDir) + "  *****");
  Log.Write(" *****  OUTPUT IMAGE DIRECTORY : " + OpStr.GetName(outputIMGDir) + "  *****");

  AnaPedestal(inputDir.c_str(),
			  outputXMLDir.c_str(),
			  outputIMGDir.c_str(),
			  overwrite,
			  n_chips,
			  n_chans);

  Log.Write("[" + OpStr.GetName(inputDir) + "][wgAnaPedestal] wgAnaPedestal finished");
}
