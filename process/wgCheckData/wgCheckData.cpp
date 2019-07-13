#include <iostream>
#include <string>

#include "wgEditXML.h"
#include "wgEditConfig.h"
#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgConst.hpp"

using namespace std;

int optimize_level_th(int);
int optimize_level_inputDAC(int);

int main(int argc, char** argv){ 

  int opt;
  double NoiseRate=3.0;
  double u_Gain=-1.0;
  double l_Gain=-1.0;
  double l_limit_Gain=10.0;
  double u_limit_Gain=50.0;
  bool bu_Gain=false;
  bool bl_Gain=false;
  bool data_quality=true;
  int chip_num=1;

  wgEnvironment env;
  con->GetENV();
  
  string inputDirName("");
  string inputFileName("");
  string configDir("");
  string configName("");
  configDir=Form("%s/config/spiroc2d",con->CALICOE_DIRECTORY);

  Logger *Log = new Logger;
  CheckExist *check = new CheckExist;

  Log->Initialize();
  Log->Write("start checking data quality...");

  while((opt = getopt(argc,argv, "f:i:hn:u:l:")) !=-1 ){
    switch(opt){
      case 'f':
        inputDirName=optarg;
        if(!check->Dir(inputDirName)){ 
          cout<<"!!Error!! "<<inputDirName.c_str()<<"doesn't exist!!";
          Log->eWrite(Form("Error!!target:%s doesn't exist",inputDirName.c_str()));
          return 100;
        }   
        Log->Write(Form("target:%s",inputDirName.c_str()));
        break;

      case 'i':
        chip_num=atoi(optarg);
        break;

      case 'n':
        NoiseRate = atof(optarg);
        break;

      case 'u':
        u_Gain = atof(optarg);
        bu_Gain = true;
        break;

      case 'l':
        l_Gain = atof(optarg);
        bl_Gain = true;
        break;

      case 'h':
        cout <<"this program is to check the data quality by reading the output from wgAnaHistSummary. "<<endl;
        cout <<"If data quality matches the requirement, return 0. Else, return 1" <<endl;
        cout <<"(If something error happens while running, return 100!)"<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -n (int)   : threshold p.e. "<<endl;
        cout <<"  -u (int)   ; Upper Limit of Gain" << endl; 
        cout <<"  -l (int)   ; Lower Limit of Gain" << endl;
        cout <<"  -f (char*) : choose input directory you wanna read(must)"<<endl;
        exit(0);
      default:
        cout <<"this program is to check the data quality by reading the output from wgAnaHistSummary. "<<endl;
        cout <<"If data quality matches the requirement, return 0. Else, return 1" <<endl;
        cout <<"(If something error happens while running, return 100!)"<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -n (int)   : threshold p.e. "<<endl;
        cout <<"  -u (int)   ; Upper Limit of Gain " << endl;
        cout <<"  -l (int)   ; Lower Limit of Gain " << endl;
        cout <<"  -f (char*) : choose input directory you wanna read(must)"<<endl;
        exit(0);
    }   
  }

  if(inputDirName==""){
    cout << "!!ERROR!! please input filename." <<endl;
    cout << "if you don't know how to input, please see help."<<endl;
    cout << "help : ./wgAnaHistSummary -h" <<endl;
    return 100;
  }

  Log->Write(Form("READING DIRECTORY : %s",inputDirName.c_str()));
  
  wgEditXML *Edit = new wgEditXML();

  for(unsigned int i=0;i<NCHIPS;i++){
    int ichip = i;
    double pe_level[32]={-1.};
    double Gain[32]={-1.};
    int v_trigth=0;
    //int v_inputDAC[32]={0};
    //int bad_ch_th[32]={0};
    //int bad_ch_inputDAC[32]={0};
    int sum_bad_high=0;
    int sum_bad_low=0;

    string name("");
    inputFileName=Form("%s/Summary_chip%d.xml",inputDirName.c_str(),ichip);
    if(!check->XmlFile(inputFileName)){ 
      cout<<"!!Error!! "<<inputFileName.c_str()<<"doesn't exist!!";
      Log->eWrite(Form("Error!!target:%s doesn't exist",inputFileName.c_str()));
      return 100;
    }   

    Edit->Open(inputFileName);
    name="tr_th";
    v_trigth=Edit->SUMMARY_GetGlobalConfigValue(name);
    for(unsigned int j=0;j<32;j++){
      int ich = j;
      //name="inputDAC";
      //v_inputDAC[ich]=Edit->SUMMARY_GetChConfigValue(name,ich);
      name="pe_level";
      pe_level[ich]=Edit->SUMMARY_GetChFitValue(name,ich);
      name="Gain";
      Gain[ich]=Edit->SUMMARY_GetChFitValue(name,ich);
    } 
    Edit->Close();

    //Fill data
    for(unsigned int j=0;j<32;j++){
      int ich = j;
      if(pe_level[ich]>NoiseRate+0.1){
        //bad_ch_th[ich]=1;
        sum_bad_high++;
      }else if(pe_level[ich]<NoiseRate-0.1){
        //bad_ch_th[ich]=-1;
        sum_bad_low++;
      }
      /*
      if(bu_Gain&&bl_Gain){
        if(l_Gain > Gain[ich]){
          bad_ch_inputDAC[ich]=-1; 
        }else if(u_Gain < Gain[ich]){
          bad_ch_inputDAC[ich]=1; 
        }
      }else if(!bu_Gain&&bl_Gain){
        if(l_Gain > Gain[ich]){
          bad_ch_inputDAC[ich]=-1; 
        }else if(u_limit_Gain < Gain[ich]){
        }
        bad_ch_inputDAC[ich]=1; 
      }else if(bu_Gain&&!bl_Gain){
        if(l_limit_Gain > Gain[ich]){
          bad_ch_inputDAC[ich]=-1; 
        }else if(u_Gain < Gain[ich]){
          bad_ch_inputDAC[ich]=1; 
        }
      }
      */
    }
    
    //Check data quality
    configName=Form("%s/wagasci_config_chip%d.txt",configDir.c_str(),ichip+1);
    if(!check->TxtFile(configName)){ 
      cout<<"!!Error!! "<<configName.c_str()<<" doesn't exist!!"<<endl;
      Log->eWrite(Form("Error!!target:%s doesn't exist",configName.c_str()));
      return 100;
    }   

    wgEditConfig *EditCon = new wgEditConfig();
    EditCon->Get_MPPCinfo(chip_num);
    EditCon->Open(configName);

    if(sum_bad_high>0. || sum_bad_low>0.){
      if(sum_bad_high>0. && sum_bad_low>0.){
        cout <<"!!WARNING!! CHIP:" << ichip << ",threshold level over several p.e.!!" <<endl;
        cout <<"  Need to change other parameters than global trigger!!!" <<endl;
      }else if(sum_bad_high>0.){
        v_trigth =  v_trigth + optimize_level_th(sum_bad_high);
        cout <<"WARNING!! CHIP:" << ichip << ",threshold level over "<< NoiseRate<<" p.e.!!" <<endl;
        cout << "    change threshold :"<< optimize_level_th(sum_bad_high) << "  , value : "<< v_trigth <<endl;
        if(v_trigth >175){
          cout << "   trigger threshold is out of range!! trgger threhold is set to 173!" << endl;
        }
        EditCon->Change_trigth(v_trigth);
      }else if(sum_bad_low>0.){
        v_trigth = v_trigth-optimize_level_th(sum_bad_low);
        cout <<"WARNING!! CHIP:" << ichip << ",threshold level under " << NoiseRate <<" p.e.!!" <<endl;
        cout << "    change threshold :"<< -optimize_level_th(sum_bad_low) << "  , value : "<< v_trigth <<endl;
        EditCon->Change_trigth(v_trigth);
      }
      data_quality=false;
    }
    else
    {
      /*
      EditCon->Change_trigth(v_trigth);
      if(bu_Gain || bl_Gain){ 
        for (int j=0;j<32;j++){
          v_inputDAC[j]+= optimize_level_th(bad_ch_inputDAC[j]);
          EditCon->Change_inputDAC(j,v_inputDAC[j],1); 
        }   
      }else{
        for (int j=0;j<32;j++){
          EditCon->Change_inputDAC(j,v_inputDAC[j],1); 
        }
      } 
      */  
    }
    EditCon->Write(configName);
  }    
  Log->Write("end checking data quality ... " );
  delete Log;  

  if(data_quality){ cout << "data quality is OK!" << endl; return 0;}
  else{ cout << "data quality is FAULT!!"<< endl; return 1;}
  
}

//******************************************************************
int optimize_level_th(int n){
  if(n>0 && n<=4) return 1;
  else if(n>4 && n<=8) return 2;
  else if(n>8 && n<=16) return 3;
  else if(n>16 && n<=24) return 4;
  else if(n>24) return 5;
  else return 0;
}

int optimize_level_inputDAC(int n){
  if(n==0) return 0;
  if(n<0) return -40;
  if(n>0) return 40;
  else return 1;
}
