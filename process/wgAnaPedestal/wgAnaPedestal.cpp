// system C++ includes
#include <string>
#include <iostream>
// system C includes
#include <getopt.h>
// user includes
#include "wgFileSystemTools.hpp"
#include "wgErrorCode.hpp"
#include "wgConst.hpp"
#include "wgAnaPedestal.hpp"
#include "wgLogger.hpp"

using namespace wagasci_tools;

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
  string inputDir("");
  string configFileName("");
  string outputIMGDir = con.IMGDATA_DIRECTORY;
  string outputXMLDir = con.XMLDATA_DIRECTORY;

  int n_chips = NCHIPS, n_chans = NCHANNELS;

  while((opt = getopt(argc,argv, "f:o:i:x:y:hr")) !=-1 ){
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

  int result;
  if ( (result = wgAnaPedestal(inputDir.c_str(),
                outputXMLDir.c_str(),
                outputIMGDir.c_str(),
                overwrite,
                n_chips,
                               n_chans)) != AP_SUCCESS ) {
    Log.Write("[wgAnaPedestalSummary] returned error code " + to_string(result));
    exit(1);
  }
  exit(0);
}
