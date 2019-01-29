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
#include <TBox.h>
#include <TSpectrum.h>

#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgEditXML.h"
#include "wgColor.h"
#include "wgFit.h"
#include "wgFitConst.h"
#include "wgGetHist.h"
#include "wgChannelMap.h"

using namespace std;

vector<string> GetIncludeFileName(string& inputDirName);
void MakeDir(string& str);
void AnaXML(vector<string> &inputDirName,string& outputXMLDirName,string& outputIMGDirName,int mode,int pe);

int main(int argc, char** argv){
  int opt;
  int mode=0;
  int pe=2;
  wgConst *con = new wgConst;
  con->GetENV();
  string inputDirName("");
  string outputXMLDirName("");
  string outputIMGDirName=con->IMGDATA_DIRECTORY;
  string logoutputDir=con->LOG_DIRECTORY;

  OperateString *OpStr = new OperateString;
  Logger *Log = new Logger;
  CheckExist *check = new CheckExist;

  Log->Initialize();
  Log->Write("start calibration...");

  while((opt = getopt(argc,argv, "f:o:i:hm:p:")) !=-1 ){
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
      
      case 'i':
        outputIMGDirName = optarg; 
        break;
      
      case 'p':
        pe = atoi(optarg); 
        break;
      
      case 'm':
        mode = atoi(optarg); 
        break;

      case 'h':
        cout <<"this program is for calibration from the data of wgAnaHistSummary. "<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose input directory you wanna read(must)"<<endl;
        cout <<"  -o (char*) : choose output directory (default: input directory) "<<endl;
        cout <<"  -i (char*) : choose output image directory (default: image directory) "<<endl;
        cout <<"  -p (int)   : choose pe. (default:2) "<<endl;
        exit(0);
      default:
        cout <<"this program is for calibration from the data of wgAnaHistSummary. "<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose input directory you wanna read(must)"<<endl;
        cout <<"  -o (char*) : choose output directory (default: input directory) "<<endl;
        cout <<"  -i (char*) : choose output image directory (default: image directory) "<<endl;
        cout <<"  -p (int)   : choose pe. (default:2) "<<endl;
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
    outputXMLDirName = OpStr->GetPath(inputDirName);
    outputXMLDirName = OpStr->GetName(outputXMLDirName);
    outputXMLDirName = Form("%s/%s",con->CALIBDATA_DIRECTORY,outputXMLDirName.c_str());
    MakeDir(outputXMLDirName);
  } 

  /* 
  outputIMGDirName = OpStr->GetPath(inputDirName);
  outputIMGDirName = OpStr->GetName(outputIMGDirName);
  outputIMGDirName = Form("%s/%s/image",con->CALIBDATA_DIRECTORY,outputIMGDirName.c_str());
  */
    
  MakeDir(outputIMGDirName);
  for(int idif=0;idif<2;idif++){
    string str("");
    str=Form("%s/dif%d",outputIMGDirName.c_str(),idif+1);
    MakeDir(str);
  }

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
      outputIMGDirName,
      mode,
      pe
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
void AnaXML(vector<string> &inputFileName, string& outputXMLDirName,string& outputIMGDirName,int mode,int pe){

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
  double Gain[2][NCHIPS][32][3];

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

      pos = inputFileName[iFN].find("pe")+2;
      if(inputFileName[iFN][pos]=='1'){
        npe=0;
      }else if(inputFileName[iFN][pos]=='2'){
        npe=1;
      }else if(inputFileName[iFN][pos]=='3'){
        npe=2;
      }

      xmlfile=Form("%s/Summary_chip%d.xml",inputFileName[iFN].c_str(),ichip);
      Edit->Open(xmlfile);
      for(unsigned int j=0;j<32;j++){
        int ich=j;
        name="Gain";
        Gain[ndif][ichip][ich][npe]=Edit->SUMMARY_GetChFitValue(name,ich);
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
    h_Gain[idif]->SetTitle(Form("Gain DIF%d;Gain;nEntry",idif+1));
    h_Gain2[idif]= new TH2F(Form("h_Gain2_DIF%d",idif+1),Form("h_Gain_DIF%d",idif+1),20,0,20,40,0,80);
    h_Gain2[idif]->SetTitle(Form("Gain DIF%d;chip;Gain",idif+1));
    h_Gain2[idif]->SetStats(0);
  }

  for(int idif=0;idif<2;idif++){
    for(unsigned int i=0;i<NCHIPS;i++){
      int ichip=i;
      for(int j=0;j<32;j++){
        int ich=j;
        double DIST;
        if(mode==0){
          if(pe==2) DIST = Gain[idif][ichip][ich][pe-1]/pe;
          else      DIST = Gain[idif][ichip][ich][pe-1]/pe*1.05;
        }
        else if(mode==1) DIST = Gain[idif][ichip][ich][1]-Gain[idif][ichip][ich][0];
        h_Gain[idif]->Fill(DIST);
        h_Gain2[idif]->Fill(ichip,DIST);
      }
    }
  }
  
  TBox* box = new TBox(36,0,44,100);
  box->SetFillColor(kRed);
  box->SetLineColor(kRed);
  box->SetFillStyle(3004);
  box->IsTransparent();
  
  for(int idif=0;idif<2;idif++){
    c1->cd(idif*2+1);
    c1->GetPad(idif*2+1)->SetLogy(1);
    h_Gain[idif]->Draw();
    box->DrawBox(36,0,44,h_Gain[idif]->GetMaximum()*1.8);
    c1->cd(idif*2+2);
    c1->GetPad(idif*2+2)->SetLogy(0);
    c1->SetLogz(1);
    h_Gain2[idif]->Draw("colz");
  }
  if(pe==2) c1->Print(Form("%s/Gain.png",outputIMGDirName.c_str()));
  else      c1->Print(Form("%s/Gain_pe%d.png",outputIMGDirName.c_str(),pe));
  
  cout << "      ~~~   bad channel  ~~~ " <<endl; 
  Edit = new wgEditXML();
  if(pe==2){
    xmlfile=Form("%s/calib_result.xml",outputXMLDirName.c_str());
  }else{
    xmlfile=Form("%s/calib_result_pe%d.xml",outputXMLDirName.c_str(),pe);
  }
  Edit->Calib_Make(xmlfile);
  Edit->Open(xmlfile);
  string badchfilename;
  if(pe==2) badchfilename = Form("%s/bad_channel.txt",outputXMLDirName.c_str());
  else      badchfilename = Form("%s/bad_channel_pe%d.txt",outputXMLDirName.c_str(),pe);
  ofstream badch(badchfilename.c_str());
  badch << "#dif chip chipch view pln ch grid" << endl;
  int view,pln,ch,grid;
  wgChannelMap mapping;
  for(int idif=0;idif<2;idif++){
    for(unsigned int i=0;i<NCHIPS;i++){
      int ichip=i;
      for(int j=0;j<32;j++){
        int ich=j;
        if(mode==1){
          name= Form("pe1");
          Edit->Calib_SetValue(name,idif+1,ichip,ich,Gain[idif][ichip][ich][0],0);
          name= Form("pe2");
          Edit->Calib_SetValue(name,idif+1,ichip,ich,Gain[idif][ichip][ich][1],0);
        }
        double gain=0;
        if(mode==0){
          if(pe==2) gain=Gain[idif][ichip][ich][pe-1]/pe;
          else      gain=Gain[idif][ichip][ich][pe-1]/pe*1.05;
        }
        else if(mode==1) gain=Gain[idif][ichip][ich][1]-Gain[idif][ichip][ich][0];
        name= Form("Gain");
        Edit->Calib_SetValue(name,idif+1,ichip,ich,gain,0);
        if(gain<36.0 || gain>44.0){
         cout << "    (dif,chip,chipch)=("<< idif+1 << ", " << ichip <<", " << ich << ")" << endl;
         mapping.GetViewPlnCh(idif,ichip,ich,&view,&pln,&ch,&grid);
         badch 
           << idif << " " << ichip << " " << ich << " " 
           << view << " " << pln   << " " << ch << " " << grid
           << endl;
        }
      }
    }
  }
  badch.close();

  Edit->Write();
  Edit->Close();
  delete Edit;
}

//******************************************************************
void MakeDir(string& str){
  CheckExist *check = new CheckExist;
  if(!check->Dir(str)){
    system(Form("mkdir %s",str.c_str()));
  }
  delete check;
}

