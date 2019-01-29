#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>

#include <THStack.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TH1.h>
#include <TFile.h>
#include <TTree.h>


#include "Const.h"
#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgFitConst.h"
#include "wgEditConfig.h"

//#define DEBUG_CHANGECONFIG
using namespace std;

//******************************************************************
int main(int argc, char** argv){
  int opt;
  int mode=-1;
  int subbin=-1;
  int value=-1;
  int ichip=-1;
  bool edit=false;
  bool overwrite=false;
  bool checkopt=false;
  bool mppc_data=false;
  wgConst *con = new wgConst;
  con->GetENV();
  string inputFile("");
  string outputFile("");
  string outputPath("");

  Logger *Log = new Logger;
  CheckExist *Check = new CheckExist;
  OperateString *Opstr = new OperateString;

  Log->Initialize();
  Log->Write("start change configure...");

  while((opt = getopt(argc,argv, "f:o:rcm:b:v:ht:")) !=-1 ){
    switch(opt){
      case 'f':
        inputFile=optarg;
        break;

      case 'o':
        outputFile=optarg;
        outputPath=Opstr->GetPath(outputFile); 
        if(!Check->Dir(outputPath)){ 
          cout<<"!!Error!! "<< Opstr->GetPath(outputFile) <<" doesn't exist!!"<<endl;
          Log->eWrite(Form("Error!!outputFile:%s cannot be made!",outputFile.c_str()));
          return 1;
        } 
        cout << "== output File :" << outputFile.c_str() << " ==" << endl;
        Log->Write(Form("outputFile:%s",inputFile.c_str()));
        break;

      case 'r':
        overwrite = true;
        cout << "== overwrite mode ==" << endl;
        break;

      case 'c':
        edit = true;
        cout << "== edit mode ==" << endl;
        break;
      
      case 't':
        mppc_data = true;
        ichip = atoi(optarg);
        cout << "== fine tuning mode ==" << endl;
        break;

      case 'm':
        mode = atoi(optarg);
        break;

      case 'b':
        subbin = atoi(optarg);
        break;

      case 'v':
        value = atoi(optarg);
        break;

      case 'h':
        cout <<"This program is for check or change configure file. "<<endl;
        cout <<"  -h        : help"<<endl;
        cout <<"  -f (char*): choose input file(txt)"<<endl;
        cout <<"  -o (char*): enter output file name(default:test.txt)"<<endl;
        cout <<"  -r        : over write mode"<<endl;
        cout <<"  -c        : edit mode (default:check mode)"<<endl;
        cout <<"  -m        : type of value you change  "<<endl;
        cout <<"              0 : trigger threshold    (10bit) "<<endl;
        cout <<"              1 : gainselect threshold (10bit) "<<endl;
        cout <<"              2 : inputDAC             (8bit)"<<endl;
        cout <<"              3 : HG & LG amplifier    (6bit) "<<endl;
        cout <<"              4 : threshold adjutment  (4bit)"<<endl;
        cout <<"              5 : inputDAC reference   (1bit)"<<endl;
        cout <<"  -b        : channel (36:all channels) "<<endl;
        cout <<"  -v        : value "<<endl;
        cout <<"  -t        : chip (fine tuning mode*) " << endl;
        cout <<"  *fine tuning mode is the mode that takes mppcs' breakdown voltage into account. "<<endl;
        cout <<"  *inputDAC is automaticcaly set to erace it, "<<endl;
        cout <<"    incase of mode 2 : inputDAC=(value)+(tuning) , "<<endl;
        exit(0);
      default:
        cout <<"This program is for check or change configure file. "<<endl;
        cout <<"  -h        : help"<<endl;
        cout <<"  -f (char*): choose input file(txt)"<<endl;
        cout <<"  -o (char*): enter output file name(default:test.txt)"<<endl;
        cout <<"  -r        : over write mode"<<endl;
        cout <<"  -c        : edit mode (default:check mode)"<<endl;
        cout <<"  -m (int)  : type of value you change  "<<endl;
        cout <<"              0 : trigger threshold    (10bit) "<<endl;
        cout <<"              1 : gainselect threshold (10bit) "<<endl;
        cout <<"              2 : inputDAC             (8bit)"<<endl;
        cout <<"              3 : HG & LG amplifier    (6bit) "<<endl;
        cout <<"              4 : threshold adjutment  (4bit)"<<endl;
        cout <<"              5 : inputDAC reference   (1bit)"<<endl;
        cout <<"  -b (int)  : channel (36:all channels) "<<endl;
        cout <<"  -v (int)  : value "<<endl;
        cout <<"  -t        : chip (fine tuning mode*) " << endl;
        cout <<"  *fine tuning mode is the mode that takes mppcs' breakdown voltage into account. "<<endl;
        cout <<"  *inputDAC is automaticcaly set to erace it, "<<endl;
        cout <<"    incase of mode 2 : inputDAC=(value)+(tuning) , "<<endl;
        exit(0);
    }   
  }

  if(overwrite && outputFile=="") outputFile = inputFile;

  // open txtfile ...
  wgEditConfig *EditConfig = new wgEditConfig();
  if(inputFile==""){
    cout << "!!ERROR!! please input filename." <<endl;
    cout << "if you don't know how to input, please see help."<<endl;
    cout << "help : ./wgChangeConfig -h" <<endl;
    return 1;
  }else if(!Check->TxtFile(inputFile)){ 
    cout<<"!!Error!! "<<inputFile.c_str()<<" doesn't exist!!" <<endl;
    Log->eWrite(Form("Error!!inputFile:%s doesn't exist",inputFile.c_str()));
    return 1;
  }else if(!overwrite && edit && Check->TxtFile(outputFile)){
    cout<<"!!Error!! "<<inputFile.c_str()<<", overwrite is forbiddend !!" <<endl;
    Log->eWrite(Form("Error!!inputFile:%s,overwrite is forbiddend",inputFile.c_str()));
    return 1;
  }else{
    EditConfig->Open(inputFile);
    cout << "== input File :" << inputFile.c_str() << " ==" << endl;
    Log->Write(Form("inputFile:%s",inputFile.c_str()));
  }

  //fine tuning mode...
  if(mppc_data){
    EditConfig->Get_MPPCinfo(ichip);
  }

  // check option value ....
  if(edit){
    if(mode==0||mode==1){
      if(value<0 || value>1023){
        cout << " WARNING!! value is out of range." <<endl;
      }else{
        checkopt=true;
      }
    }else if(mode>=2 && mode<=4){
      if(subbin<0 || subbin>36){
        cout << " WARNING!! Select subbin." <<endl;
      }else{
        if(mode==2){
          if(value<0||value>255){
            cout << " WARNING!! value is out of range." <<endl;
          }else{
            checkopt=true;
          }
        }else if(mode==3){
          if(value<0||value>63){
            cout << " WARNING!! value is out of range." <<endl;
          }else{
            checkopt=true;
          }
        }else if(mode==4){
          if(value<0||value>15){
            cout << " WARNING!! value is out of range." <<endl;
          }else{
            checkopt=true;
          }
        }
      }
    }else if(mode==5){
     if(value == 0 || value==1) checkopt=true;
     else cout << "WARNING !! value is out of range"<<endl;
    }else{
      cout << " WARNING!! Select correct mode." <<endl; 
    } 
  }else{
    EditConfig->CheckAll();
    return 0;
  }
  //**************************

  if(checkopt){
    if(mode==0){
      EditConfig->Change_trigth(value);
    }else if(mode==1){
      EditConfig->Change_gainth(value);
    }else if(mode==2||mode==3||mode==4){
      if(subbin==36){
        for(int ich=0;ich<32;ich++){
          if(mode==2){
            if(mppc_data){
              EditConfig->Change_inputDAC(ich,value,1);
            }else{
              EditConfig->Change_inputDAC(ich,value,0);
            }
          }else if(mode==3){
            EditConfig->Change_ampDAC(ich,value);
          }else if(mode==4){
            EditConfig->Change_trigadj(ich,value);
          } 
        }
      }else{
        if(mode==2){
          if(mppc_data){
            EditConfig->Change_inputDAC(subbin,value,1);
          }else{
            EditConfig->Change_inputDAC(subbin,value,0);
          }
        }else if(mode==3){
          EditConfig->Change_ampDAC(subbin,value);
        }else if(mode==4){
          EditConfig->Change_trigadj(subbin,value);
        } 
      }
    }else if(mode==5){
      EditConfig->Change_1bitparam(value,36);
    }
    EditConfig->Write(outputFile);
  }
  delete Opstr;
  delete EditConfig;
  delete Check;
  delete Log;  
}

