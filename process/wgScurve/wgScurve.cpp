// system includes
#include <iostream>
#include <string>

// system C includes
#include <getopt.h>

// user includes
#include "wgConst.hpp"
#include "wgLogger.hpp"
#include "wgScurve.hpp"

void print_help(const char * program_name) {
  std::cout <<  program_name << " summarizes the wgAnaHist output into a TO-DO.\n"
      "  -h         : help\n"
      "  -f (char*) : input directory (mandatory)\n"
      "  -o (char*) : output xml directory (default: same as input directory)\n"
      "  -i (char*) : output image directory (default: same as input directory)\n";
  exit(0);
}

//******************************************************************
int main(int argc, char** argv){

  int opt;
  wgEnvironment env;
  std::string inputDir("");
  std::string outputXMLDir = env.CALIBDATA_DIRECTORY ;
  std::string outputIMGDir = env.IMGDATA_DIRECTORY;

  while((opt = getopt(argc,argv, "f:o:i:h")) != -1 ){
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

      case 'h':
        print_help(argv[0]);
        break;

      default:
        print_help(argv[0]);
    }   
  }

  int result;
  if ( (result = wgScurve(inputDir.c_str(), 
                          outputXMLDir.c_str(), 
                          outputIMGDir.c_str())) != WG_SUCCESS){
    Log.eWrite("[wgScurve] wgScurve returned error " + std::to_string(result));
    exit(1);
  }

  exit(0);
}

