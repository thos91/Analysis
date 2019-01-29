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
#include <TH1F.h>
#include <TH2F.h>
#include <TGraph.h>
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

vector<string> GetIncludeFileName(string& inputDirName);
void MakeDir(string& str);
void AnaXML(vector<string> &inputDirName,string& outputXMLDirName,string& outputIMGDirName);

int main(int argc, char** argv){
  int opt;
  wgConst *con = new wgConst;
  con->GetENV();
  string inputDirName("");
  string outputXMLDirName("");
  string outputDirName("");
  string outputIMGDirName=con->IMGDATA_DIRECTORY;
  string logoutputDir=con->LOG_DIRECTORY;

  OperateString *OpStr = new OperateString;
  Logger *Log = new Logger;
  CheckExist *check = new CheckExist;

  Log->Initialize();
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
    outputXMLDirName = Form("%s",con->CALIBDATA_DIRECTORY);
  } 

  outputDirName = OpStr->GetName(inputDirName);
  outputIMGDirName = Form("%s/%s",outputIMGDirName.c_str(),outputDirName.c_str());
  MakeDir(outputIMGDirName);

  Log->Write(Form("READING DIRECTORY : %s",inputDirName.c_str()));
  Log->Write(Form("OUTPUT DIRECTORY : %s",outputXMLDirName.c_str()));

  cout << " *****  READING DIRECTORY     :" << inputDirName << "  *****" << endl;
  cout << " *****  OUTPUT XML DIRECTORY :" << outputXMLDirName << "  *****" << endl;
  cout << " *****  OUTPUT IMAGE DIRECTORY :" << outputIMGDirName << "  *****" << endl;

  delete check;
  delete OpStr;

  vector<string> ReadFile = GetIncludeFileName(inputDirName); 
  cout << " Finish  reading file"<<endl;

  AnaXML(ReadFile,
      outputXMLDirName,
      outputIMGDirName
      );
  Log->Write("end summarizeing ... " );
  delete Log;  
}


//******************************************************************
vector<string> GetIncludeFileName(string& inputDirName){
  DIR *dp;
  struct dirent *entry;
  vector<string> openxmlfile;
  dp = opendir(inputDirName.c_str());
  if(dp==NULL){
    cout << " !! WARNING !! no data is in "<< inputDirName << endl;
    return openxmlfile;
  }

  while( (entry = readdir(dp))!=NULL ){
    if((entry->d_name[0])!='.'){
      openxmlfile.push_back(Form("%s/%s",inputDirName.c_str(),entry->d_name));
      cout << "ReadFile : " << inputDirName << "/" << entry->d_name << endl;
    }
  }
  closedir(dp);
  return openxmlfile;
} 


//******************************************************************
void AnaXML(vector<string> &inputFileName, string& outputXMLDirName,string& outputIMGDirName){

  int FN=inputFileName.size();
  if(FN%2!=0){
    cout << "!! ERROR !! : the number of data is not enough to calbration!!" << endl;
    return;
  }
  string xmlfile("");
  string name("");
  wgEditXML *Edit = new wgEditXML();
  int ndif=0;
  int npe=0;
  double Gain[2][NCHIPS][32][2];
  double ped[2][NCHIPS][32][16];
  double ped_ref[2][NCHIPS][32][16];

  cout << "  ~~~ Start Reading ~~~  " <<endl;
  //*** Read data ***
  for(int iFN=0;iFN<FN;iFN++){
    for(unsigned int i=0;i<NCHIPS;i++){
      int ichip=i;
      int pos = inputFileName[iFN].find("dif_1_1_")+8;
      if(inputFileName[iFN][pos]=='1'){
        ndif=0;
      }else if(inputFileName[iFN][pos]=='2'){
        ndif=1;
      }

      pos = inputFileName[iFN].find("_pe")+3;
      if(inputFileName[iFN][pos]=='1'){
        npe=0;
      }else if(inputFileName[iFN][pos]=='2'){
        npe=1;
      }

      xmlfile=Form("%s/Summary_chip%d.xml",inputFileName[iFN].c_str(),ichip);
      Edit->Open(xmlfile);
      for(unsigned int j=0;j<32;j++){
        int ich=j;
        name=Form("Gain");
        Gain[ndif][ichip][ich][npe]=Edit->SUMMARY_GetChFitValue(name,ich);
        if(npe==1){
          for(unsigned int k=0;k<16;k++){
            int icol=k;
            name=Form("ped_%d",icol);
            ped[ndif][ichip][ich][icol]=Edit->SUMMARY_GetChFitValue(name,ich);

            name=Form("ped_ref_%d",icol);
            ped_ref[ndif][ichip][ich][icol]=Edit->SUMMARY_GetChFitValue(name,ich);
          }
        }
      }
      Edit->Close();
    }
  }
  delete Edit;

  TCanvas *c1 = new TCanvas("c1","c1");
  c1->Divide(2,2);
  TH1F * h_Gain[2];
  TH2F * h_Gain2[2];
  for(int idif=0;idif<2;idif++){
    h_Gain[idif]= new TH1F(Form("h_Gain_DIF%d",idif+1),Form("h_Gain_DIF%d",idif+1),80,20,60);
    h_Gain2[idif]= new TH2F(Form("h_Gain2_DIF%d",idif+1),Form("h_Gain_DIF%d",idif+1),20,0,20,40,0,80);
  }

  for(int idif=0;idif<2;idif++){
    for(unsigned int i=0;i<NCHIPS;i++){
      int ichip=i;
      for(int j=0;j<32;j++){
        int ich=j;
        double DIST = Gain[idif][ichip][ich][1]-Gain[idif][ichip][ich][0];
        h_Gain[idif]->Fill(DIST);
        h_Gain2[idif]->Fill(ichip,DIST);
      }
    }
  }

  for(int idif=0;idif<2;idif++){
    c1->cd(idif*2+1);
    c1->GetPad(idif*2+1)->SetLogy(1);
    h_Gain[idif]->Draw();
    c1->cd(idif*2+2);
    c1->GetPad(idif*2+2)->SetLogy(0);
    c1->SetLogz(1);
    h_Gain2[idif]->Draw("colz");
  }
  c1->Print(Form("%s/Gain.png",outputIMGDirName.c_str()));

  TH1F *h1[16];
  TH1F *h2[16];
  TH1F *h3 = new TH1F(Form("h3"),"h1",60,-50,10);
  h3->SetTitle("pedestal shift;adc count;nEntry");
  for(int icol=0;icol<16;icol++){
    h1[icol] = new TH1F(Form("h1_%d",icol),"h1",300,400,700);
    h2[icol] = new TH1F(Form("h2_%d",icol),"h2",100,-50,50);
    h1[icol]->SetLineColor(kBlue);
    h2[icol]->SetLineColor(kRed); 
    h2[icol]->SetTitle(Form("pedestal shift col%d;adc count;nEntry",icol));
  }

  Edit = new wgEditXML();
  xmlfile=Form("%s/pedestal_card.xml",outputXMLDirName.c_str());
  Edit->Calib_Make(xmlfile);
  Edit->Open(xmlfile);
  for(int idif=0;idif<2;idif++){
    for(unsigned int i=0;i<NCHIPS;i++){
      int ichip=i;
      for(int j=0;j<32;j++){
        int ich=j;
        name= Form("pe1");
        Edit->Calib_SetValue(name,idif+1,ichip,ich,Gain[idif][ichip][ich][0],0);
        name= Form("pe2");
        Edit->Calib_SetValue(name,idif+1,ichip,ich,Gain[idif][ichip][ich][1],0);
        name= Form("Gain");
        Edit->Calib_SetValue(name,idif+1,ichip,ich,Gain[idif][ichip][ich][1]-Gain[idif][ichip][ich][0],0);
        for(int k=0;k<16;k++){
          int icol=k;
          name=Form("ped_%d",icol);
          double nominal_pedestal=ped_ref[idif][ichip][ich][icol]-2.0*(Gain[idif][ichip][ich][1]-Gain[idif][ichip][ich][0]);
          Edit->Calib_SetValue(name,idif+1,ichip,ich,nominal_pedestal ,1);
          h1[icol]->Fill(nominal_pedestal);
          name=Form("ped_nohit_%d",icol);
          Edit->Calib_SetValue(name,idif+1,ichip,ich,ped[idif][ichip][ich][icol] ,1);
          h2[icol]->Fill(nominal_pedestal-ped[idif][ichip][ich][icol]);
          h3->Fill(nominal_pedestal-ped[idif][ichip][ich][icol]);
        }
      }
    }
  }

  Edit->Write();
  Edit->Close();
  delete Edit;

  delete c1;
  c1=new TCanvas("c1","c1");
  c1->Divide(4,4);
  for(int icol=0;icol<16;icol++){
    c1->cd(icol+1);
    //h1[icol]->Draw();
    h2[icol]->Draw("");
  }
  c1->Print(Form("%s/pedestal_shift.png",outputIMGDirName.c_str()));
  delete c1;
  c1=new TCanvas("c1","c1");
  h3->Draw("");
  c1->Print(Form("%s/pedestal_shift_all.png",outputIMGDirName.c_str()));
}

//******************************************************************
void MakeDir(string& str){
  CheckExist *check = new CheckExist;
  if(!check->Dir(str)){
    system(Form("mkdir %s",str.c_str()));
  }
  delete check;
}

