#include <iostream>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../../include/wgEditXML.h"
#include "../../include/wgEditConfig.h"
#include "../../include/wgTools.h"
#include "../../include/wgErrorCode.h"
#include "../../include/Const.h"

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
	      "  -s (char*) : spiroc2d configuration files folder\n"
          "  -d (int)   : number of DIFs (must be 1-8)\n"
	      "  -c (int)   : number of ASU chips per DIF (must be 1-20)\n"
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
  string wagasci_config_dif_dir("");
  string configDir("");
  string configName("");
  int mode     = 0;
  int inputDAC = 0;
  int pe       = 0;
  int n_difs   = 0;
  int n_chips   = 0;

  // If a log directory different from the standard one is needed add it to the
  // constructor
  Logger *Log = new Logger; 
  CheckExist *check = new CheckExist;

  Log->Write("start checking data quality...");

  while((opt = getopt(argc, argv, "hm:t:f:s:d:c:p:i:")) != -1 ){
    switch(opt){
	case 't':
	  threshold_card=optarg;
	  if ( !check->XmlFile(threshold_card) ) {
		cout << "!!Error!! " << threshold_card << "doesn't exist!!";
		Log->eWrite(Form("Error!!target:%s doesn't exist", threshold_card.c_str()));
		return 1;
	  }
	  Log->Write(Form("target:%s",threshold_card.c_str()));
	  break;
	case 'f':
	  calibration_card = optarg;
	  if(!check->XmlFile(calibration_card)) {
		cout << "!!Error!! " << calibration_card << "doesn't exist!!";
		Log->eWrite(Form("Error!!target:%s doesn't exist", calibration_card.c_str()));
		return 1;
	  }
	  Log->Write(Form("target:%s",calibration_card.c_str()));
	  break;
	case 's':
	  wagasci_config_dif_dir = optarg;
	  struct stat statbuf;
	  if ( (stat(wagasci_config_dif_dir.c_str(), &statbuf) != -1) || (! S_ISDIR(statbuf.st_mode)) ) {
		  cout << "!!Error!! " << wagasci_config_dif_dir << "doesn't exist!!";
		  Log->eWrite(Form("Error!!target:%s doesn't exist", wagasci_config_dif_dir.c_str()));
		  return 1;
		}
	  Log->Write(Form("target:%s", wagasci_config_dif_dir.c_str()));
	  break;
	case 'i':
	  inputDAC = atoi(optarg);
	  break;
	case 'd':
	  n_difs = atoi(optarg);
	  if( n_difs <= 0 && n_difs > 8 ) {
		cout << " number of DIFs is not correct!!" << endl;
		cout << " the number of DIFs must be {1-8}" << endl;
		exit(1);
	  }
	  break;
	case 'c':
	  n_chips = atoi(optarg);
	  if( n_chips <= 0 && n_chips > 20 ) {
		cout << " number of chips per DIF is not correct!!" << endl;
		cout << " the number of chips per DIF must be {1-20}" << endl;
		exit(1);
	  }
	  break;
	case 'm':
	  mode = atoi(optarg);
	  break;
	case 'p':
	  pe = atoi(optarg);
	  if( pe != 1 && pe != 2 && pe != 3 ) {
		cout << " p.e. value is not correct!!" << endl;
		cout << " p.e. must be {1,2,3}" << endl;
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

  if ( (mode == 1) && (calibration_card == "") ) {
    cout << "!!ERROR!! please input filename." <<endl;
    cout << "if you don't know how to input, please see help."<<endl;
    cout << "help : ./wgAnaHistSummary -h" <<endl;
    exit(1);
  }
  if ( (mode == 0) && ( ((inputDAC % 20) != 1) || (inputDAC < 1) || (inputDAC > 241) ) ) {
    cout << " inputDAC value is not correct!!" << endl;
    cout << " inputDAC must be in {1,21,41,61,81,101,121,141,161,181,201,221,241}!!" << endl;
    exit(1);
  }

  // double threshold[n_difs][n_chips]
  // double s_th[n_difs][n_chips]
  // double i_th[n_difs][n_chips]
  vector<vector<double>> threshold(n_difs, vector<double>(n_chips));
  vector<vector<double>> s_th(n_difs, vector<double>(n_chips));
  vector<vector<double>> i_th(n_difs, vector<double>(n_chips));
  wgEditXML *Edit = new wgEditXML();
  string name("");

  Edit->Open(threshold_card);
  // Read the threshold card
  for(int idif = 0; idif < n_difs; idif++) {
    for(int ichip = 0; ichip < n_chips; ichip++) {
	  // pre-calibration mode
      if (mode == 0) {
		// Get the optimal threshold for pe photo-electron equivalent
        name = Form("threshold_%d", pe);
        threshold[idif][ichip] = Edit->OPT_GetValue(name, idif + 1, ichip, inputDAC);
		// post-calibration mode
      }
	  else if (mode == 1) {
        if ( pe < 3 ) {
		  // s_th is the slope of the linear fit of the inputDAC (x) vs optimal
		  // threshold for the given p.e. equivalend (y)
		  // i_th is the intercept of the linear fit of the inputDAC (x) vs
		  // optimal threshold for the given p.e. equivalend (y)
          name=Form("s_th%d", pe);
          s_th[idif][ichip]=Edit->OPT_GetChipValue(name, idif + 1, ichip);
          name=Form("i_th%d", pe);
          i_th[idif][ichip]=Edit->OPT_GetChipValue(name, idif + 1, ichip);
        } else {
		  // Get the optimal threshold for 2.5 photo-electron equivalent
		  // for what value of the inputDAC??
          name=Form("threshold_3");
          threshold[idif][ichip]= Edit->OPT_GetChipValue(name, idif + 1, ichip);
        }
      }
    }
  }
  Edit->Close();

  // double s_Gain[n_difs][n_chips][NumChipCh]
  // double i_Gain[n_difs][n_chips][NumChipCh]
  vector<vector<array<double, NumChipCh>>> s_Gain(n_difs, vector<array<double, NumChipCh>>(n_chips));
  vector<vector<array<double, NumChipCh>>> i_Gain(n_difs, vector<array<double, NumChipCh>>(n_chips));
  // Get the slope and intercept of the inputDAC(x) vs Gain(y) graph from the calibration_card.xml file
  if(mode == 1) {
    Edit->Open(calibration_card);
    for(int idif = 0; idif < n_difs; idif++){
      for(int ichip = 0; ichip < n_chips; ichip++){
        for(int ich = 0; ich < NumChipCh; ich++){
          name=Form("s_Gain");
          s_Gain[idif][ichip][ich]= Edit->PreCalib_GetValue(name, idif + 1, ichip, ich);
          name=Form("i_Gain");
          i_Gain[idif][ichip][ich]= Edit->PreCalib_GetValue(name, idif + 1, ichip, ich);
        }
      }
    }
    Edit->Close();
  }

  delete Edit;

  // Edit the SPIROC2D configuration files for every DIF
  wgEditConfig *EditCon = new wgEditConfig();
  for(int idif = 0; idif < n_difs; idif++){
    for(int ichip = 0; ichip < n_chips; ichip++){
      configName = Form("%s/wagasci_config_dif%d_chip%d.txt", wagasci_config_dif_dir.c_str(), idif + 1, ichip + 1);
      if( !check->TxtFile(configName) ) { 
        cout << "!!Error!! " << configName.c_str() << " doesn't exist!!" << endl;
        return 1;
      }  
      EditCon->Open(configName);
	  
	  // In mode 0 edit the threshold of each chip to the optimal value taken from threshold_card.xml
      if( mode == 0 ) {
        EditCon->Change_trigth(threshold[idif][ichip]);
      }
	  
	  // In mode 1 edit the inputDAC of each channel of each chip to the optimal value calculated from calibration_card.xml
	  else if( mode == 1 ) {
        double mean_inputDAC = 0.;
        for(int ich = 0; ich < NumChipCh; ich++) {
          double inputDAC = 0.;
          if(s_Gain[idif][ichip][ich] == 0.) {
            inputDAC = 121.;
            mean_inputDAC += inputDAC;
          }
		  else {
			// This is the inputDAC value corresponding to a Gain of 40
            inputDAC = (40. - i_Gain[idif][ichip][ich]) / s_Gain[idif][ichip][ich];
            if (inputDAC < 1.)   inputDAC = 1.;
            if (inputDAC > 250.) inputDAC = 250.;
            mean_inputDAC += inputDAC;
          }
		  inputDAC=round(inputDAC);
		  // 
          EditCon->Change_inputDAC(ich, (int) inputDAC, 0);
        }//ich
		mean_inputDAC /= (double) NumChipCh;

		// In mode 1 (post configuration) set the chip-wise threshold to the value corresponding to mean_inputDAC
        double thresholdDAC;
        if ( pe < 3 ) {
		  // Use the slope and intercept of the inputDAC(x) vs threshold(y) graph
          thresholdDAC = s_th[idif][ichip] * mean_inputDAC + i_th[idif][ichip];
          thresholdDAC = round(thresholdDAC);
        }else if( pe == 3) {
		  // For 2.5. p.e. just read the threshold from the threshold card file
          thresholdDAC=threshold[idif][ichip];
          thresholdDAC=round(thresholdDAC);
        }
		// Set the global threshold of the chip
        EditCon->Change_trigth((int)thresholdDAC);
      }
      EditCon->Write(configName);
      EditCon->Clear();
    }//ichip
  }//idif
}

