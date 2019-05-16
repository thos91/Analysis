// system C++ includes
#include <string>

// user includes
#include "Const.h"
#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgDecoder.hpp"

void print_help(const char * program_name) {
  cout << "this program decodes a .raw file into a .root file\n"
	"usage example: " << program_name << " -f inputfile.raw -r\n"
	"  -h         : help\n"
	"  -f (char*) : input .raw file that you want to read (mandatory)\n"
	"  -i (char*) : calibration card file\n"
	"  -p (char*) : pedestal card file\n"
	"  -t (char*) : tdc card file\n"
	"  -o (char*) : output directory (default = WAGASCI_DECODEDIR)\n"
	"  -r         : overwrite mode\n"
	"  -b         : batch (silent) mode\n";
}

int main(int argc, char** argv) {
  // object used to check if various files exist or not 
  CheckExist check;

  // Get environment variables
  wgConst con;
  con.GetENV();

  int opt;
  string inputFileName("");
  string calibFileName("");
  string pedFileName("");
  string tdcFileName("");
  string outputFile("");
  const string confDir(con.CONF_DIRECTORY);
  string outputDir(con.DECODE_DIRECTORY);
  bool overwrite = false;
  bool batch = false;

  while((opt = getopt(argc,argv, "hf:i:p:t:o:rb")) !=-1 ){
    switch(opt){
	case 'f':
	  inputFileName = optarg;
	  if( !check.RawFile(inputFileName) ) { 
		Log.eWrite("[" + inputFileName + "][Decoder] target is wrong");
		return 1;
	  }
	  Log.Write("[" + inputFileName + "][Decoder] start decoding");
	  break;
	case 'i':
	  calibFileName = optarg;
	  if( !check.XmlFile(calibFileName) ) { 
		Log.eWrite("[" + calibFileName + "][Decoder] calibration file is wrong");
		return 1;
	  }
	  break;
	case 'p':
	  pedFileName = optarg;
	  if( !check.XmlFile(pedFileName) ) { 
		Log.eWrite("[" + pedFileName + "][Decoder] pedestal file is wrong");
		return 1;
	  }
	  break;
	case 't':
	  tdcFileName = optarg;
	  if( !check.XmlFile(tdcFileName) ) { 
		Log.eWrite("[" + tdcFileName + "][Decoder] TDC calibration file is wrong");
		return 1;
	  }
	  break;
	case 'o':
	  outputDir = optarg; 
	  if( !check.Dir(outputDir) ) {
		Log.eWrite("[" + outputDir + "][Decoder]output directory is wrong");
		return 1;
	  }
	  break;
	case 'r':
	  overwrite = true;
	  Log.Write("[" + inputFileName + "][Decoder] overwrite mode");
	  break;
	case 'b':
	  batch = true;
	  Log.Write("[" + inputFileName + "][Decoder] batch (silent) mode");
	  break;
	case 'h':
	  print_help(argv[0]);
	  exit(0);
	  break;
	default:
	  print_help(argv[0]);
	  exit(0);
    }
  }

  if(inputFileName == ""){
    Log.eWrite("[Decoder] No input file");
    exit(1);
  }

  if(calibFileName == "") {
	calibFileName = confDir + "/cards/calibration_card.xml";
  }
  if(pedFileName == "") {
	pedFileName = confDir + "/cards/pedestal_card.xml";
  }
  if(tdcFileName == "") {
	tdcFileName = confDir + "/cards/tdc_coefficient_card.xml";
  }
  if ( (batch == true) && (Log.WhereToLog == COUT) ) {
    Log.eWrite("Batch mode is selected but cannot open the log file");
	exit(1);
  } else if (batch == true) Log.WhereToLog = LOGFILE;
  
  const unsigned int maxEvt = 99999999;
  Log.Write("Maximum number of events treated = " + to_string(maxEvt));

  int retcode;
  if ( (retcode = wgDecoder(inputFileName.c_str(), calibFileName.c_str(), pedFileName.c_str(), tdcFileName.c_str(), outputDir.c_str(), overwrite, maxEvt)) != 0 )
	Log.eWrite("Decoder failed with code " + to_string(retcode));
  return 0;
}

