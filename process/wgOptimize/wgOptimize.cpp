// system includes
#include <string>

// system C includes
#include <getopt.h>

// user includes
#include "wgFileSystemTools.hpp"
#include "wgErrorCode.hpp"
#include "wgConst.hpp"
#include "wgOptimize.hpp"

using namespace std;

void print_help(string program_name) {
  cout << "(mode 0) set the optimal threshold for each chip given the value of inputDAC.\n"
	"(mode 1) set the optimal inputDAC of each channel of each chip based on the calibration_card.xml\n"
	"         i.e. the inputDAC value so that the gain is as closest as possible to 40 ADC counts\n"
	"         If -p is 3 (2.5. p.e. equivalent) there is no \"optimization\" and the threshold is simply\n"
	"         read from the threshold card file (-t)\n"
	"Usage: " << program_name << " [OPTIONS]\n"
	"  -h         : print this help message\n"
	"  -m (int)   : mode selection (default 0)\n"
	"     0       :   pre calibration\n"
	"     1       :   post calibration\n"
	"  -t (char*) : threshold card you want to read (mandatory)\n"
	"  -f (char*) : calibration card you want to read (only mode 1)\n"
    "  -u (char*) : Pyrame configuration xml file (mandatory)\n"
	"  -s (char*) : spiroc2d configuration files folder\n"
	"  -p (int)   : photo electrons equivalent threshold (must be 1-3)\n"
	"     1       :   0.5 p.e. equivalent threshold\n"
	"     2       :   1.5 p.e. equivalent threshold\n"
	"     3       :   2.5 p.e. equivalent threshold\n"
	"  -i (int)   : inputDAC (high voltage adjustment DAC)\n"
	"     1+20*n  :   (only mode 0) where n is in (0,12)\n";
}

int main(int argc, char** argv){ 

  int opt;
  string calibration_card("");
  string threshold_card("");
  string config_xml_file("");
  string wagasci_config_dif_dir("");
  string configDir("");
  string configName("");
  int mode       = 0;
  int inputDAC   = 0;
  int pe         = 0;
  CheckExist check;

  while((opt = getopt(argc, argv, "hm:t:f:u:s:p:i:")) != -1 ){
    switch(opt){
	case 't':
	  threshold_card=optarg;
	  if ( !check.XmlFile(threshold_card) ) {
		Log.eWrite("[wgOptimize] threshold card doesn't exist : " + threshold_card);
		exit(1);
	  }
	  break;
	case 'f':
	  calibration_card = optarg;
	  if(!check.XmlFile(calibration_card)) {
		Log.eWrite("[wgOptimize] calibration card doesn't exist : " + calibration_card);
		calibration_card = "";
	  }
	  break;
	case 's':
	  wagasci_config_dif_dir = optarg;
	  if ( !check.Dir(wagasci_config_dif_dir)) {
		Log.eWrite("[wgOptimize] config directory doesn't exist : " + wagasci_config_dif_dir);
		exit(1);
	  }
	  break;
    case 'u':
	  config_xml_file = optarg;
	  if ( !check.XmlFile(config_xml_file)) {
		Log.eWrite("[wgOptimize] Pyrame config file doesn't exist : " + config_xml_file);
		exit(1);
	  }
	  break; 
	case 'i':
	  inputDAC = atoi(optarg);
	  break;
	case 'm':
	  mode = atoi(optarg);
	  break;
	case 'p':
	  pe = atoi(optarg);
	  if( pe != 1 && pe != 2 && pe != 3 ) {
		Log.eWrite("[wgOptimize] The number of p.e. must be {1,2,3}");
		exit(1);
	  }
	  break;
	case 'h':
	  print_help(argv[0]);
	  exit(0);
	default:
	  print_help(argv[0]);
	  exit(0);
    }   
  }

  int result;
  if ( (result = wgOptimize(threshold_card.c_str(),
                            calibration_card.c_str(),
                            config_xml_file.c_str(),
                            wagasci_config_dif_dir.c_str(),
							mode,
                            inputDAC,
                            pe)) != OP_SUCCESS ) {
	Log.eWrite("[wgOptimize] wgOptimize returned error " + to_string(result));
    exit(1);
  }
  exit(0);
}