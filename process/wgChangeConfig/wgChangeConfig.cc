// system C++ includes
#include <string>
#include <iostream>

// system C includes
#include <bits/stdc++.h>

// user includes
#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgChangeConfig.hpp"

using namespace std;

void print_help(const char * program_name) {
  cout << program_name << " checks or changes a SPIROC bitstream file.\n"
	"  -h        : help\n"
	"  -f (char*): input file  (.txt file) (mandatory)\n"
	"  -o (char*): output file (default: test.txt)\n"
	"  -r        : over write mode\n"
	"  -e        : edit mode (default: unset (check mode))\n"
	"  -m        : parameter to modify\n"
	"              0 : trigger threshold    (10bit) 0-1023\n"
	"              1 : gainselect threshold (10bit) 0-1023\n"
	"              2 : inputDAC             (8bit)  0-255\n"
	"              3 : HG & LG amplifier    (6bit)  0-63\n"
	"              4 : threshold adjutment  (4bit)  0-15\n"
	"              5 : inputDAC reference   (1bit)  0-1\n"
	"  -b        : channel (default :36 = all channels)\n"
	"  -v        : new value\n"
	"  -t        : chip number (fine tuning mode*)\n"
	"\n"
	"  * fine tuning mode: in this mode the individual MPPC breakdown voltage\n"
	"    is taken into account. It is only relevant for mode 2 where :\n"
	"      inputDAC = (value) + (tuning)\n";
  exit(0);
}

//******************************************************************
int main(int argc, char** argv){
  int opt;
  int mode   = -1;
  int channel = -1;
  int value  = -1;
  int ichip  = -1;
  bitset<4> flags;
  string inputFile("");
  string outputFile("");
  string outputPath("");

  CheckExist Check;
  OperateString OpStr;

  while((opt = getopt(argc,argv, "f:o:m:b:v:t:her")) !=-1 ){
    switch(opt){
	case 'f':
	  inputFile=optarg;
	  Log.Write("[" + OpStr.GetName(inputFile) + "][wgChangeConfig] input directory: " + OpStr.GetPath(inputFile));
	  break;

	case 'o':
	  outputFile=optarg;
	  outputPath=OpStr.GetPath(outputFile); 
	  if(!Check.Dir(outputPath)){
		Log.eWrite("[" + OpStr.GetName(inputFile) + "][wgChangeConfig] " + outputPath + " doesn't exist");
		return 1;
	  }
	  Log.Write("[" + OpStr.GetName(inputFile) + "][wgChangeConfig] Output directory: " + outputPath);
	  break;

	case 'r':
	  flags[OVERWRITE_FLAG] = true;
	  Log.Write("[" + OpStr.GetName(inputFile) + "][wgChangeConfig] overwrite mode");
	  break;

	case 'e':
	  flags[EDIT_FLAG] = true;
	  Log.Write("[" + OpStr.GetName(inputFile) + "][wgChangeConfig] edit mode");
	  break;
      
	case 't':
	  flags[MPPC_DATA_FLAG] = true;
	  ichip = atoi(optarg);
	  Log.Write("[" + OpStr.GetName(inputFile) + "][wgChangeConfig] fine tuning mode");
	  break;

	case 'm':
	  mode = atoi(optarg);
	  break;

	case 'b':
	  channel = atoi(optarg);
	  break;

	case 'v':
	  value = atoi(optarg);
	  break;

	case 'h':
	  print_help(argv[0]);
	  break;
	  
	default:
	  print_help(argv[0]);
	  break;
	}
  }
  if(flags[OVERWRITE_FLAG] && outputFile == "") outputFile = inputFile;

  int result;
  if ( (result = wgChangeConfig(inputFile.c_str(), outputFile.c_str(), flags.to_ulong(), value, mode, ichip, channel)) != 0) {
	Log.eWrite(string(argv[0]) + " returned error code " + to_string(result));
  }
}
