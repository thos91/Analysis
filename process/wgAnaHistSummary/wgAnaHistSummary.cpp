// system includes
#include <iostream>
#include <string>

// system C includes
#include <getopt.h>

// user includes
#include "wgConst.hpp"
#include "wgAnaHistSummary.hpp"
#include "wgLogger.hpp"

void print_help(const char * program_name) {
  cout <<  program_name << " summarizes the wgAnaHist output into a TO-DO.\n"
    "  -h         : help\n"
    "  -f (char*) : input directory (mandatory)\n"
    "  -o (char*) : output directory (default: same as input directory)\n"
    "  -i (char*) : output directory for plots and images (default: WAGASCI_IMGDIR) "
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
  bool overwrite = false, print = false;
  wgConst con;
  string inputDirName("");
  string configFileName("");
  string outputXMLDirName("");
  string outputDirName("");
  string logoutputDir(con.LOG_DIRECTORY);
  string outputIMGDirName(con.IMGDATA_DIRECTORY);
  
  while((opt = getopt(argc, argv, "f:o:i:m:rph")) !=-1 ){
    switch(opt){
    case 'f':
      inputDirName = optarg;

      break;
    case 'o':
      outputXMLDirName = optarg; 
      break;
    case 'i':
      outputIMGDirName = optarg;
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

  int result;
  if ( (result = wgAnaHistSummary(inputDirName.c_str(),
                                  outputXMLDirName.c_str(),
                                  outputIMGDirName.c_str(),
                                  mode,
                                  overwrite,
                                  print)) != AHS_SUCCESS ) {
    Log.eWrite("[wgAnaHistSummary] wgAnaHistSummary returned error " + to_string(result));
    exit(1);
  }

  exit(0);
}
