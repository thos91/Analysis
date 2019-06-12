// system C++ includes
#include <string>

// system C includes
#include <getopt.h>


// user includes
#include "Const.hpp"
#include "wgTools.hpp"
#include "wgErrorCode.hpp"
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
	"  -x (int)   : number of ASU chips per DIF (must be 1-20)\n"
	"  -y (int)   : number of channels per chip (must be 1-36)\n"
	"  -r         : overwrite mode\n"
	"  -b         : batch (silent) mode\n";
}

int main(int argc, char** argv) {
  // object used to check if various files exist or not 
  CheckExist check;
  OperateString OptStr;

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
  unsigned n_chips = NCHIPS;
  unsigned n_channels = NCHANNELS;
  unsigned dif = 0;

  while((opt = getopt(argc,argv, "f:i:p:t:o:n:x:y:rbh")) !=-1 ){
    switch(opt){
	case 'f':
	  inputFileName = optarg;
	  if( !check.RawFile(inputFileName) ) { 
		Log.eWrite("[" + OptStr.GetName(inputFileName) + "][wgDecoder] target is wrong");
		exit(1);
	  }
	  Log.Write("[" + OptStr.GetName(inputFileName) + "][wgDecoder] start decoding");
	  break;
	case 'i':
	  calibFileName = optarg;
	  if( !check.XmlFile(calibFileName) ) { 
		Log.eWrite("[" + OptStr.GetName(calibFileName) + "][wgDecoder] calibration file is wrong");
		exit(1);
	  }
	  break;
	case 'p':
	  pedFileName = optarg;
	  if( !check.XmlFile(pedFileName) ) { 
		Log.eWrite("[" + OptStr.GetName(pedFileName) + "][wgDecoder] pedestal file is wrong");
		exit(1);
	  }
	  break;
	case 't':
	  tdcFileName = optarg;
	  if( !check.XmlFile(tdcFileName) ) { 
		Log.eWrite("[" + OptStr.GetName(tdcFileName) + "][wgDecoder] TDC calibration file is wrong");
		exit(1);
	  }
	  break;
	case 'o':
	  outputDir = optarg; 
	  if( !check.Dir(outputDir) ) {
		Log.eWrite("[" + outputDir + "][wgDecoder]output directory is wrong");
		exit(1);
	  }
	  break;
	  	case 'n':
	  n_chips = atoi(optarg);
	  if( dif > NDIFS ) {
		Log.eWrite("[" + OptStr.GetName(inputFileName) + "][wgDecoder] The number of chips per DIF must be {1-" + to_string(NCHIPS) + "}");
		exit(1);
	  }
	  break;
	case 'x':
	  n_chips = atoi(optarg);
	  if( n_chips > NCHIPS ) {
		Log.eWrite("[" + OptStr.GetName(inputFileName) + "][wgDecoder] The number of chips per DIF must be {1-" + to_string(NCHIPS) + "}");
		exit(1);
	  }
	  break;
	case 'y':
	  n_channels = atoi(optarg);
	  if( n_channels > NCHANNELS ) {
		Log.eWrite("[" + OptStr.GetName(inputFileName) + "][wgDecoder] The number of channels per DIF must be {1-" + to_string(NCHANNELS) + "}");
		exit(1);
	  }
	  break;
	case 'r':
	  overwrite = true;
	  Log.Write("[" + OptStr.GetName(inputFileName) + "][wgDecoder] overwrite mode");
	  break;
	case 'b':
	  batch = true;
	  Log.Write("[" + OptStr.GetName(inputFileName) + "][wgDecoder] batch (silent) mode");
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
  
  const unsigned int maxEvt = MAX_EVENT;
  Log.Write("Maximum number of events treated = " + to_string(maxEvt));

  int retcode;
  if ( (retcode = wgDecoder(inputFileName.c_str(), calibFileName.c_str(), pedFileName.c_str(), tdcFileName.c_str(), outputDir.c_str(), overwrite, maxEvt, dif, n_chips, n_channels)) != 0 )
	Log.eWrite("Decoder failed with code " + to_string(retcode));
  exit(0);
}

