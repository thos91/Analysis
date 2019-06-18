// systen include
#include <string>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
// systen C include
#include <getopt.h>
// use include
#include "wgConst.hpp"
#include "wgFileSystemTools.hpp"
#include "wgScurveSummary.hpp"
#include "wgLogger.hpp"

using namespace wagasci_tools;

void print_help(const char * program_name) {
  cout <<  program_name << " Calibrate the data of wgAnaHistSummary.\n"
  "  -h         : help\n"
  "  -f (char*) : input directory (mandatory)\n"
  "  -o (char*) : output directory (default: same as input directory)\n";
  exit(0);
}

//******************************************************************

int main(int argc, char** argv){

  int opt;
  wgConst con;
  string inputDir("");
  string outputXMLDir = con.CALIBDATA_DIRECTORY;
  string outputIMGDir = con.IMGDATA_DIRECTORY;

  while((opt = getopt(argc,argv, "f:o:h")) !=-1 ){
    switch(opt){
      case 'f':
        inputDir = optarg;
        break;

      case 'o':
        outputXMLDir = optarg; 
        break;

      case 'h':
        print_help(argv[0]);
        break;

      default:
        print_help(argv[0]);
        break;
    }   
  }
  
  int result;
  if ( (result =  wgScurveSummary(inputDir.c_str(),
                  outputXMLDir.c_str(),
                  outputIMGDir.c_str())) != SCURVESUMMARY_SUCCESS){
    Log.eWrite("[" + GetName(inputDir) + "][wgScurveSummary] wgScurveSummary returned error " + to_string(result));
    exit(1);
  }

  exit(0);
}
