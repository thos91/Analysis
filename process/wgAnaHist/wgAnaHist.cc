#include <string>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>

#include <THStack.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TH1.h>

#include "Const.h"
#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgFit.h"
#include "wgFitConst.h"
#include "wgEditXML.h"

using namespace std;

void MakeDir(string& outputDir);
void MakeXMLFILE(string& outputDir, bool overwrite);
void ModeSelect(int mode);
void PrintSelect(int print, int i);
void ReadData(string& inputFileName, string& outputDir, string& configFileName,int mode,int ndif,int print,string& outputIMGDir,int ichip);

bool SELECT_CONFIG;
bool SELECT_BCID;
bool SELECT_CHARGE_LOW;
bool SELECT_CHARGE_NOHIT;
bool SELECT_CHARGE_HG_LOW;
bool SELECT_CHARGE_HG_HIGH;
bool PRE_SELECT_PRINT;
int SELECT_PRINT;
int start_time; 
int stop_time;
//******************************************************************
int main(int argc, char** argv){
  int opt;
  int mode=0;
  int print=0;
  int ndif=1;
  bool overwrite=false;
  wgConst *con = new wgConst;
  con->GetENV();
  string inputFileName("");
  string configFileName("");
  string outputDir=con->XMLDATA_DIRECTORY;
  string outputIMGDir=con->IMGDATA_DIRECTORY;
  string logoutputDir=con->LOG_DIRECTORY;

  OperateString *OpStr = new OperateString;

  Logger *Log = new Logger;
  CheckExist *Check = new CheckExist;

  Log->Initialize();

  while((opt = getopt(argc,argv, "f:d:m:i:o:c:hp:r")) !=-1 ){
    switch(opt){
      case 'f':
        inputFileName=optarg;
        if(!Check->RootFile(inputFileName)){ 
          cout<<"!!Error!! "<<inputFileName.c_str()<<" doesn't exist!!";
          Log->eWrite(Form("[%s][wgAnaHist]Error!!target doesn't exist",inputFileName.c_str()));
          return 1;
        }   
        cout << "== readfile :" << inputFileName.c_str() << " ==" << endl;
        Log->Write(Form("[%s][wgAnaHist]start wgAnaHist",inputFileName.c_str()));
        break;
      case 'd':
        ndif = atoi(optarg);
        break;
      case 'm':
        mode = atoi(optarg);
        break;
      case 'i':
        configFileName = optarg;
        SELECT_CONFIG=true;
        if(!Check->XmlFile(configFileName)){ 
          cout<<"!!Error!! "<<configFileName.c_str()<<" doesn't exist!!";
          Log->eWrite(Form("[%s][wgAnaHist]Error!!target doesn't exist",configFileName.c_str()));
          return 1;
        }   
        cout << "== configure xmlfile :" << configFileName.c_str() << " ==" << endl;
        Log->Write(Form("[%s][wgAnaHist]read config file:%s",inputFileName.c_str(),configFileName.c_str()));  
        break;

      case 'o':
        outputDir = optarg; 
        break;
      case 'c':
        outputIMGDir = optarg; 
        break;

      case 'p':
        print = atoi(optarg);
        PRE_SELECT_PRINT=true;
        break;

      case 'r':
        overwrite = true; 
        break;

      case 'h':
        cout <<"this program is for caliburation "<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h        : help"<<endl;
        cout <<"  -f (char*): choose inputfile(hist file)"<<endl;
        cout <<"  -i (char*): choose config file (.xml)"<<endl;
        cout <<"  -o (char*): outputdir (default= XML_DIRECTORY)"<<endl;
        cout <<"  -d (int)  : choose dif number"<<endl;
        cout <<"  -m (int)  : choose fit mode"<<endl;
        cout <<"  -p (int)  : choose print mode"<<endl;
        cout <<"  -r        : overwrite mode"<<endl;
        cout <<"   ===   mode  === "<<endl;
        cout <<"   1 : only BCID "<<endl;
        cout <<"   2 : only charge_nohit "<<endl;
        cout <<"   3 : only charge(low range) "<<endl;
        cout <<"   4 : only charge_HG(low range) "<<endl;
        cout <<"   5 : only charge_HG(high range) "<<endl;
        cout <<"   10 : BCID + charge(low) "<<endl;
        cout <<"   11 : BCID + charge_nohit + charge(low)"<<endl;
        cout <<"   12 : BCID + charge_nohit + charge_HG(low) "<<endl;
        cout <<"   13 : BCID + charge_nohit + charge_HG(high) "<<endl;
        cout <<"   20 : BCID + charge_nohit + charge_HG(low) + charge_HG(high)"<<endl; 
        exit(0);
      default:
        cout <<"this program is for fitting to _hist.root file "<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h        : help"<<endl;
        cout <<"  -f (char*): choose inputfile you wanna read(must)"<<endl;
        cout <<"  -i (char*): choose config file (.xml)"<<endl;
        cout <<"  -o (char*): outputdir (default= XML_DIRECTORY)"<<endl;
        cout <<"  -d (int)  : choose dif number"<<endl;
        cout <<"  -m (int)  : choose mode"<<endl;
        cout <<"  -p (int)  : choose print mode"<<endl;
        cout <<"  -r        : overwrite mode"<<endl;
        cout <<"   ===   mode  === "<<endl;
        cout <<"   1 : only BCID "<<endl;
        cout <<"   2 : only charge_nohit "<<endl;
        cout <<"   3 : only charge(low range) "<<endl;
        cout <<"   4 : only charge_HG(low range) "<<endl;
        cout <<"   5 : only charge_HG(high range) "<<endl;
        cout <<"   10 : BCID + charge(low) "<<endl;
        cout <<"   11 : BCID + charge_nohit + charge(low)"<<endl;
        cout <<"   12 : BCID + charge_nohit + charge_HG(low) "<<endl;
        cout <<"   13 : BCID + charge_nohit + charge_HG(high) "<<endl;
        cout <<"   20 : BCID + charge_nohit + charge_HG(low) + charge_HG(high)"<<endl; 
        exit(0);
    }   
  }

  if(inputFileName==""){
    cout << "!!ERROR!! please input filename." <<endl;
    cout << "if you don't know how to input, please see help."<<endl;
    cout << "help : ./wgAnaHist -h" <<endl;
    exit(1);
  }

  string DirName = OpStr->GetNameBeforeLastUnderBar(inputFileName);
  delete OpStr;

  outputDir = Form("%s/%s",outputDir.c_str(),DirName.c_str());
  outputIMGDir = Form("%s/%s",outputIMGDir.c_str(),DirName.c_str());
  delete Check;

  MakeDir(outputDir);
  if(PRE_SELECT_PRINT)  MakeDir(outputIMGDir);
  
  MakeXMLFILE(outputDir,overwrite);
  for(int ichip=0;ichip<(int)NCHIPS;ichip++){
    ReadData(inputFileName, outputDir, configFileName, mode, ndif, print, outputIMGDir,ichip);
  }

  Log->Write(Form("[%s][wgAnaHist]finish analyzing histgram",inputFileName.c_str()));
  delete Log;  
}

//******************************************************************
void MakeDir(string& outputDir){
  cout << " *****  OUTPUT XML FILE     :" << outputDir << "  *****" << endl;
  system(Form("mkdir %s",outputDir.c_str()));
  for(unsigned int i=0;i<NCHIPS;i++){
    string outputChipDir = Form("%s/chip%d",outputDir.c_str(),i);
    system(Form("mkdir %s",outputChipDir.c_str()));
  }
}

//******************************************************************
void MakeXMLFILE(string& outputDir,bool overwrite){
  cout << " *****  MAKING XML FILE  ***** "<< endl;
  wgEditXML *Edit = new wgEditXML();
  CheckExist *Check = new CheckExist();
  string outputxmlfile("");
  for(unsigned int i=0;i<NCHIPS;i++){
    int ichip=i;
    for(unsigned int j=0;j<32;j++){
      int ich=j;
      outputxmlfile=Form("%s/chip%d/ch%d.xml",outputDir.c_str(),ichip,ich);
      if(!Check->XmlFile(outputxmlfile)){
        Edit->Make(outputxmlfile,ichip,ich);
      }else{
        if(overwrite) Edit->Make(outputxmlfile,ichip,ich);
      }
    }
  }
  delete Edit;
  delete Check;
}

//******************************************************************
void ModeSelect(int mode){
  if(mode==0){
        cout <<" ===   mode  === "<<endl;
        cout <<" 1 : only BCID "<<endl;
        cout <<" 2 : only charge_nohit "<<endl;
        cout <<" 3 : only charge(low range) "<<endl;
        cout <<" 4 : only charge_HG(low range) "<<endl;
        cout <<" 5 : only charge_HG(high range) "<<endl;
        cout <<" 10 : BCID + charge(low) "<<endl;
        cout <<" 11 : BCID + charge_nohit + charge(low)"<<endl;
        cout <<" 12 : BCID + charge_nohit + charge_HG(low) "<<endl;
        cout <<" 13 : BCID + charge_nohit + charge_HG(high) "<<endl;
        cout <<" 20 : BCID + charge_nohit + charge_HG(low) + charge_HG(high)"<<endl; 
  }
  if(mode==1||mode>=10) SELECT_BCID=true;
  if(mode==2||mode>=11) SELECT_CHARGE_NOHIT=true;
  if(mode==3||mode==10||mode==11) SELECT_CHARGE_LOW=true;
  if(mode==4||mode==12||mode>=20) SELECT_CHARGE_HG_LOW=true;

}

//******************************************************************
void PrintSelect(int print,int i){
  int nchip=NCHIPS;
  if(PRE_SELECT_PRINT){
    if(print==nchip){
      if(i<nchip && i >= 0){ 
        SELECT_PRINT=1;
      }else{
        SELECT_PRINT=0;
      }
    }else{
      if(i==print){
        SELECT_PRINT=1;
      }else{
        SELECT_PRINT=0;
      }
    }
  }else{
    SELECT_PRINT=0;
  }
}


//******************************************************************
void ReadData(string& inputFileName, string& outputDir, string& configFileName,int mode,int ndif,int print, string& outputIMGDir,int ichip){

  cout << " *****  READING FILE     :" << inputFileName << "  *****" << endl;
  cout << "\nstart Analyzing..." <<endl;

  ModeSelect(mode);

  unsigned int i,j,k;
  int ich;
  wgEditXML *Edit = new wgEditXML();
  wgFit *Fit = new wgFit(inputFileName);
  Fit->SetoutputIMGDir(outputIMGDir);
  string outputChipDir("");
  string outputxmlfile("");
  string name("");
  vector<int> config; // 6parameters * 32 channel = 192

  i=ichip;
  cout << "start chip "<< ichip <<endl;
  outputChipDir=Form("%s/chip%d",outputDir.c_str(),ichip);
  if(SELECT_CONFIG) Edit->GetConfig(configFileName,ndif,ichip+1,config); 
  for(j=0;j<32;j++){
    ich=j;
    PrintSelect(print,ichip);
    outputxmlfile=Form("%s/ch%d.xml",outputChipDir.c_str(),ich);
    Edit->Open(outputxmlfile);
    // ***********************************
    if(ichip==0){
      start_time = Fit->GetHist->Get_start_time(); 
      stop_time  = Fit->GetHist->Get_stop_time(); 
    }
    name="start_time";
    Edit->SetConfigValue(name,start_time,0);
    name="stop_time";
    Edit->SetConfigValue(name,stop_time,0);
    if(SELECT_CONFIG){
      name="trigth";
      Edit->SetConfigValue(name,config[0+ich*6],0);
      name="gainth";
      Edit->SetConfigValue(name,config[1+ich*6],0);
      name="inputDAC";
      Edit->SetConfigValue(name,config[2+ich*6],0);
      name="HG";
      Edit->SetConfigValue(name,config[3+ich*6],0);
      /*
      name="LG";
      Edit->SetConfigValue(name,config[4+ich*6],0);
      name="trig_adj";
      Edit->SetConfigValue(name,config[5+ich*6],0);
      */
    }
    if(SELECT_BCID){  //for bcid
      double x_bcid[2]={0.,0.};
      Fit->NoiseRate(i,j,x_bcid,SELECT_PRINT);
      name="NoiseRate";
      Edit->SetChValue(name,x_bcid[0],1);
      name="NoiseRate_e";
      Edit->SetChValue(name,x_bcid[1],1);
    }
    // ***********************************
    if(SELECT_CHARGE_NOHIT){  //for nohit
      double x_nohit[3]={0.,0.,0.};
      for(k=0;k<MEMDEPTH;k++){ 
        Fit->charge_nohit(i,j,k,x_nohit,SELECT_PRINT);
        name="charge_nohit";
        Edit->SetColValue(name,k,x_nohit[0],1);
        name="sigma_nohit";
        Edit->SetColValue(name,k,x_nohit[2],1);
      } 
    }
    // ***********************************
    if(SELECT_CHARGE_LOW){  //for charge_HG (low range)
      double x_low[3]={0.,0.,0.};
      Fit->low_pe_charge(i,ich,x_low,SELECT_PRINT);
      name="charge_low";
      Edit->SetChValue(name,x_low[0],1);
      name="sigma_low";
      Edit->SetChValue(name,x_low[2],1);
    }
    
    // ***********************************
    if(SELECT_CHARGE_HG_LOW){  //for charge_HG (low range)
      double x_low_HG[3]={0.,0.,0.};
      for(k=0;k<MEMDEPTH+1;k++){ 
        Fit->low_pe_charge_HG(i,j,k,x_low_HG,SELECT_PRINT);
        name="charge_lowHG";
        Edit->SetColValue(name,k,x_low_HG[0],1);
        name="sigma_lowHG";
        Edit->SetColValue(name,k,x_low_HG[2],1);
      } 
    }
    // ***********************************
    if(SELECT_CHARGE_HG_HIGH){  //for charge_HG (high range)
      for(k=0;k<MEMDEPTH;k++){ 
        double x2[2]={0.,0.};
        Fit->GainSelect(i,j,k,x2,SELECT_PRINT);
        name="GS_eff_m";
        Edit->SetColValue(name,k,x2[0],1);
        name="GS_eff_e";
        Edit->SetColValue(name,k,x2[1],1);
      }
    }
    Edit->Write();
    Edit->Close();
  }//ich
  cout << "end Analyzing..." <<endl;
  delete Edit;
  delete Fit;
  return;
}



