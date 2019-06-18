//
// system includes
#include <iostream>
#include <string>
// system C includes
#include <getopt.h>
// user includes
#include "Const.hpp"
#include "wgTools.hpp"
#include "wgErrorCode.hpp"
#include "wgScurve.hpp"
#include "wgEditXML.hpp"
#include "wgColor.hpp"
#include "wgFit.hpp"
#include "wgFitConst.hpp"
#include "wgGetHist.hpp"



void print_help(const char * program_name) {
  cout <<  program_name << " summarizes the wgAnaHist output into a TO-DO.\n"
  "  -h         : help\n"
  "  -f (char*) : input directory (mandatory)\n"
  "  -o (char*) : output directory (default: same as input directory)\n";
  exit(0);
}

//******************************************************************
int main(int argc, char** argv){

  int opt;
  wgConst *con = new wgConst;
  con->GetENV();
  string inputDirName("");
  string outputXMLDirName("");
  string outputIMGDirName=con->IMGDATA_DIRECTORY;
  string logoutputDir=con->LOG_DIRECTORY;

  OperateString *OpStr = new OperateString;
  Logger *Log = new Logger;
  CheckExist *check = new CheckExist;

//  Log->Initialize();
  Log->Write("start calibration...");

  while((opt = getopt(argc,argv, "f:o:h")) !=-1 ){
    switch(opt){
      case 'f':
        inputDirName=optarg;
        if(!check->Dir(inputDirName)){ 
          cout<<"!!Error!! "<<inputDirName.c_str()<<"doesn't exist!!";
          Log->eWrite(Form("Error!!target:%s doesn't exist",inputDirName.c_str()));
          return 1;
        }   
        Log->Write(Form("target:%s",inputDirName.c_str()));
        break;

      case 'o':
        outputXMLDirName = optarg; 
        break;

      case 'h':
        cout <<"this program is for calibration from the data of wgAnaHistSummary. "<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose input directory you wanna read(must)"<<endl;
        cout <<"  -o (char*) : choose output directory (default: input directory) "<<endl;
        exit(0);
      default:
        cout <<"this program is for calibration from the data of wgAnaHistSummary. "<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose input directory you wanna read(must)"<<endl;
        cout <<"  -o (char*) : choose output directory (default: input directory) "<<endl;
        exit(0);
    }   
  }

  if(inputDirName==""){
    cout << "!!ERROR!! please input filename." <<endl;
    cout << "if you don't know how to input, please see help."<<endl;
    cout << "help : ./wgAnaHistSummary -h" <<endl;
    exit(1);
  }

  if(outputXMLDirName==""){
    outputXMLDirName=con->CALIBDATA_DIRECTORY;
  }
  outputIMGDirName = Form("%s/%s",outputIMGDirName.c_str(),OpStr->GetName(inputDirName).c_str());

  Log->Write(Form("READING DIRECTORY : %s",inputDirName.c_str()));
  Log->Write(Form("OUTPUT DIRECTORY : %s",outputXMLDirName.c_str()));

  cout << " *****  READING DIRECTORY     :" << inputDirName << "  *****" << endl;
  cout << " *****  OUTPUT XML DIRECTORY :" << outputXMLDirName << "  *****" << endl;
  cout << " *****  OUTPUT IMAGE DIRECTORY :" << outputIMGDirName << "  *****" << endl;

  delete con;
  delete check;
  delete OpStr;

  int result;
  if ( (result =  wgScurve(inputDirName.c_str(), 
                  outputXMLDirName.c_str(), 
                  outputIMGDirName.c_str())) != SCURVE_SUCCESS){
    Log->eWrite("[" + OpStr->GetName(inputDirName) + "][wgScurve] wgScurve returned error " + to_string(result));
    exit(1);
  }

  Log->Write("[" + OpStr->GetName(inputDirName) + "][wgScurve] finished");
  exit(0);

}

