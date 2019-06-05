#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>

#include <TROOT.h>
#include "TApplication.h"
#include <THStack.h>
#include <TMultiGraph.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TGraph.h>
#include <TBox.h>
#include <TGraphErrors.h>
#include <TF1.h>
#include <TSpectrum.h>

#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgEditXML.h"
#include "wgColor.h"
#include "wgFit.h"
#include "wgFitConst.h"
#include "wgGetHist.h"

using namespace std;

void MakeDir(string& str);
void AnaXML(string& inputFileName1,string& inputFileName2,string& outputXMLDirName,string& outputIMGDirName);

int main(int argc, char** argv){
  int opt;
  wgConst *con = new wgConst;
  con->GetENV();
  string inputFileName1("");
  string inputFileName2("");
  string outputXMLDirName("");
  string outputIMGDirName=con->IMGDATA_DIRECTORY;
  string logoutputDir=con->LOG_DIRECTORY;

  OperateString *OpStr = new OperateString;
  Logger *Log = new Logger;
  CheckExist *check = new CheckExist;

  Log->Initialize();
  Log->Write("start calibration...");

  while((opt = getopt(argc,argv, "f:i:o:h")) !=-1 ){
    switch(opt){
      case 'f':
        inputFileName1=optarg;
        if(!check->XmlFile(inputFileName1)){ 

          cout<<"!!Error!! "<<inputFileName1.c_str()<<"doesn't exist!!";
          Log->eWrite(Form("Error!!target:%s doesn't exist",inputFileName1.c_str()));
          return 1;
        }   
        Log->Write(Form("target:%s",inputFileName1.c_str()));
        break;

      case 'i':
        inputFileName2=optarg;
        if(!check->XmlFile(inputFileName2)){ 

          cout<<"!!Error!! "<<inputFileName2.c_str()<<"doesn't exist!!";
          Log->eWrite(Form("Error!!target:%s doesn't exist",inputFileName2.c_str()));
          return 1;
        }   
        Log->Write(Form("target:%s",inputFileName2.c_str()));
        break;

      case 'o':
        outputXMLDirName = optarg; 
        break;

      case 'h':
        cout <<"this program is for calibration from the data of wgAnaHistSummary. "<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose initial calib_result.xml(must)"<<endl;
        cout <<"  -i (char*) : choose final calib_result.xml(must)"<<endl;
        cout <<"  -o (char*) : choose output directory (default: /home/data/calibration) "<<endl;
        exit(0);
      default:
        cout <<"this program is for calibration from the data of wgAnaHistSummary. "<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose initial calib_result.xml(must)"<<endl;
        cout <<"  -i (char*) : choose final calib_result.xml(must)"<<endl;
        cout <<"  -o (char*) : choose output directory (default: /home/data/calibration) "<<endl;
        exit(0);
    }   
  }

  if(inputFileName1==""){
    cout << "!!ERROR!! please input filename." <<endl;
    cout << "if you don't know how to input, please see help."<<endl;
    cout << "help : ./wgAnaHistSummary -h" <<endl;
    exit(1);
  }

  if(outputXMLDirName==""){
    outputXMLDirName = "/home/data/calibration";
  } 

  outputIMGDirName = Form("%s",con->CALIBDATA_DIRECTORY);
    

  Log->Write(Form("READING DIRECTORY : %s",inputFileName1.c_str()));
  Log->Write(Form("OUTPUT DIRECTORY : %s",outputXMLDirName.c_str()));

  cout << " *****  READING DIRECTORY     :" << inputFileName1 << "  *****" << endl;
  cout << " *****  OUTPUT XML DIRECTORY :" << outputXMLDirName << "  *****" << endl;
  cout << " *****  OUTPUT IMAGE DIRECTORY :" << outputIMGDirName << "  *****" << endl;

  delete check;
  delete OpStr;

  AnaXML(inputFileName1,
      inputFileName2,
      outputXMLDirName,
      outputIMGDirName
      );
  Log->Write("end summarizeing ... " );
  delete Log;  
}


//******************************************************************
void AnaXML(string& inputFileName1, string& inputFileName2,string& outputXMLDirName,string& outputIMGDirName){

  string xmlfile("");
  string name("");
  wgEditXML *Edit = new wgEditXML();
  double Gain[2][NCHIPS][32][2];

  xmlfile=inputFileName1;
  Edit->Open(xmlfile);
  for(int idif=0;idif<2;idif++){
    for(unsigned int i=0;i<NCHIPS;i++){
      int ichip=i;
      for(int j=0;j<32;j++){
        int ich=j;
        name=Form("Gain");
        Gain[idif][ichip][ich][0]=Edit->Calib_GetValue(name,idif+1,ichip,ich);
      }
    }
  }
  Edit->Close();

  xmlfile=inputFileName2;
  Edit->Open(xmlfile);
  for(int idif=0;idif<2;idif++){
    for(unsigned int i=0;i<NCHIPS;i++){
      int ichip=i;
      for(int j=0;j<32;j++){
        int ich=j;
        name=Form("Gain");
        Gain[idif][ichip][ich][1]=Edit->Calib_GetValue(name,idif+1,ichip,ich);
      }
    }
  }
  Edit->Close();
  delete Edit;

  TCanvas *c1 = new TCanvas("c1","c1");
  c1->Divide(2);
  TH2D * h_Gain[2];
  for(int idif=0;idif<2;idif++){
    h_Gain[idif]= new TH2D(Form("h_Gain_DIF%d",idif+1),Form("h_Gain_DIF%d",idif+1),20,0,20,20,-0.2,0.2);
    h_Gain[idif]->SetTitle(Form("Difference of Gain DIF%d;chip;Differential",idif+1));
    h_Gain[idif]->SetStats(0);
  }

  for(int idif=0;idif<2;idif++){
    for(unsigned int i=0;i<NCHIPS;i++){
      int ichip=i;
      for(int j=0;j<32;j++){
        int ich=j;
        double diff=(Gain[idif][ichip][ich][0]-Gain[idif][ichip][ich][1])/(Gain[idif][ichip][ich][0]+Gain[idif][ichip][ich][1]);
        h_Gain[idif]->Fill(ichip,diff);
      }
    }
  }

  TBox* box = new TBox(0,-100,100,100);
  box->SetFillColor(kRed);
  box->SetLineColor(kRed);
  box->SetFillStyle(3004);
  box->IsTransparent();
  
  for(int idif=0;idif<2;idif++){
    c1->cd(idif+1);
    h_Gain[idif]->Draw("colz");
    box->Draw("same");
  }
  c1->Print(Form("%s/GainDiff.png",outputIMGDirName.c_str()));


}

//******************************************************************
void MakeDir(string& str){
  CheckExist *check = new CheckExist;
  if(!check->Dir(str)){
    system(Form("mkdir %s",str.c_str()));
  }
  delete check;
}

