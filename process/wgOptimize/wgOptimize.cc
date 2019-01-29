#include <iostream>
#include <string>

#include "wgEditXML.h"
#include "wgEditConfig.h"
#include "wgTools.h"
#include "wgErrorCode.h"
#include "Const.h"

using namespace std;

int main(int argc, char** argv){ 

  int opt;
  wgConst *con = new wgConst;
  con->GetENV();

  string inputFileName("");
  string configDir("");
  string configName("");
  int mode=0;
  int inputDAC=0;
  int pe=0;
  string threshold_card="/home/data/calibration/threshold_card.xml";

  Logger *Log = new Logger;
  CheckExist *check = new CheckExist;

  Log->Initialize();
  Log->Write("start checking data quality...");

  while((opt = getopt(argc,argv, "f:m:i:hp:")) !=-1 ){
    switch(opt){
      case 'f':
        inputFileName=optarg;
        if(!check->XmlFile(inputFileName)){ 
          cout<<"!!Error!! "<<inputFileName.c_str()<<"doesn't exist!!";
          Log->eWrite(Form("Error!!target:%s doesn't exist",inputFileName.c_str()));
          return 1;
        }   
        Log->Write(Form("target:%s",inputFileName.c_str()));
        break;
      case 'i':
        inputDAC = atoi(optarg);
        break;
      case 'm':
        mode = atoi(optarg);
        break;
      case 'p':
        pe = atoi(optarg);
        break;
      case 'h':
        cout <<"this program is to optimize threshold to input inputDAC. "<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -m (int)   : mode" <<endl;
        cout <<"     0 : pre calib" <<endl; 
        cout <<"     1 : calib" <<endl; 
        cout <<"  -f (char*) : choose calibration card you wanna read(mode 1)"<<endl;
        cout <<"  -p (int)   : photo electron(must)" << endl;
        cout <<"  -i (int)   : inputDAC(mode 0)" << endl;
        exit(0);
      default:
        cout <<"this program is to optimize threshold to input inputDAC. "<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -m (int)   : mode" <<endl;
        cout <<"     0 : pre calib" <<endl; 
        cout <<"     1 : calib" <<endl; 
        cout <<"  -f (char*) : choose calibration card you wanna read(mode 1)"<<endl;
        cout <<"  -p (int)   : photo electron(must)" << endl;
        cout <<"  -i (int)   : inputDAC(mode 0)" << endl;
        exit(0);
    }   
  }

  if(mode==1 && inputFileName==""){
    cout << "!!ERROR!! please input filename." <<endl;
    cout << "if you don't know how to input, please see help."<<endl;
    cout << "help : ./wgAnaHistSummary -h" <<endl;
    return 1;
  }

  if(mode==0 && inputDAC%20!=1){
    cout << " inputDAC value is not correct!!" << endl;
    cout << " inputDAC musr be {1,21,41,61,81,101,121,141,161,181,201,221,241!!" << endl;
    return 1;
  }

  if(pe!=1 && pe!=2 && pe!=3 ){
    cout << " p.e. value is not correct!!" << endl;
    cout << " p.e. must be {1,2}" << endl;
    return 1;
  }

  double threshold[2][NCHIPS];
  double s_th[2][NCHIPS];
  double i_th[2][NCHIPS];
  wgEditXML *Edit = new wgEditXML();
  string name("");

  Edit->Open(threshold_card);
  for(int idif=0;idif<2;idif++){
    for(unsigned int i=0;i<NCHIPS;i++){
      int ichip = i;
      if(mode==0){
        name=Form("threshold_%d",pe);
        threshold[idif][ichip]= Edit->OPT_GetValue(name,idif+1,ichip,inputDAC);
      }else if(mode==1){
        if(pe<3){
          name=Form("s_th%d",pe);
          s_th[idif][ichip]=Edit->OPT_GetChipValue(name,idif+1,ichip);
          name=Form("i_th%d",pe);
          i_th[idif][ichip]=Edit->OPT_GetChipValue(name,idif+1,ichip);
        }else{
          name=Form("threshold_3");
          threshold[idif][ichip]= Edit->OPT_GetChipValue(name,idif+1,ichip);
        }
      }
    }
  }
  Edit->Close();

  double s_Gain[2][NCHIPS][32];
  double i_Gain[2][NCHIPS][32];
  if(mode==1){
    Edit->Open(inputFileName);
    for(int idif=0;idif<2;idif++){
      for(unsigned int i=0;i<NCHIPS;i++){
        int ichip = i;
        for(unsigned int j=0;j<32;j++){
          int ich = j;
          name=Form("s_Gain");
          s_Gain[idif][ichip][ich]= Edit->PreCalib_GetValue(name,idif+1,ichip,ich);
          name=Form("i_Gain");
          i_Gain[idif][ichip][ich]= Edit->PreCalib_GetValue(name,idif+1,ichip,ich);
        }
      }
    }
    Edit->Close();
  }

  delete Edit;

  wgEditConfig *EditCon = new wgEditConfig();
  for(int idif=0;idif<2;idif++){
    for(unsigned int i=0;i<NCHIPS;i++){
      int ichip = i;
      configName=Form("/opt/calicoes/config/spiroc2d/wagasci_config_dif%d_chip%d.txt",idif+1,ichip+1);
      if(!check->TxtFile(configName)){ 
        cout<<"!!Error!! "<<configName.c_str()<<" doesn't exist!!"<<endl;
        return 1;
      }  
      EditCon->Open(configName);

      if(mode==0){
        EditCon->Change_trigth(threshold[idif][ichip]);
     
      }else if(mode==1){
        double mean_inputDAC=0.;
        for(unsigned int j=0;j<32;j++){
          int ich=j;
          double inputDAC=0.;
          if(s_Gain[idif][ichip][ich]==0.){
            inputDAC=121.;
            mean_inputDAC += inputDAC/32.0;
          }else{
            inputDAC=(40.-i_Gain[idif][ichip][ich])/s_Gain[idif][ichip][ich];
            if(inputDAC<1.) inputDAC=1.;
            if(inputDAC>250.) inputDAC=250.;
            mean_inputDAC += inputDAC/32.0;
            inputDAC=round(inputDAC);
          }
          EditCon->Change_inputDAC(ich,(int)inputDAC,0);
        }//ich

        double thresholdDAC;
        if(pe<3){
          thresholdDAC=s_th[idif][ichip]*mean_inputDAC+i_th[idif][ichip];
          thresholdDAC=round(thresholdDAC);
        }else if(pe==3){
          thresholdDAC=threshold[idif][ichip];
          thresholdDAC=round(thresholdDAC);
        }
        EditCon->Change_trigth((int)thresholdDAC);
      }
      EditCon->Write(configName);
      EditCon->Clear();
    }//ichip
  }//idif
}

