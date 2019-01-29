#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>

#include <THStack.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TH1F.h>
#include <TH2F.h>

#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgEditXML.h"
#include "wgColor.h"
#include "wgFit.h"
#include "wgFitConst.h"
#include "wgGetHist.h"

using namespace std;

void MakeDir(string& str);
void ModeSelect(int mode);
void MakeXMLFILE(string& str, bool overwrite);
void ReadXML(string& inputDirName,string& outputXMLDirName,string& outputIMGDirName,int mode);

bool SELECT_Noise;
bool SELECT_Gain;
bool SELECT_Pedestal;
bool SELECT_RawCharge;
bool SELECT_Print;
//******************************************************************
int main(int argc, char** argv){

  int opt;
  int mode=10;
  bool overwrite=false;
  wgConst *con = new wgConst;
  con->GetENV();
  string inputDirName("");
  string configFileName("");
  string outputIMGDirName=con->IMGDATA_DIRECTORY;
  string outputXMLDirName("");
  string outputDirName("");
  string logoutputDir=con->LOG_DIRECTORY;

  OperateString *OpStr = new OperateString;
  Logger *Log = new Logger;
  CheckExist *check = new CheckExist;

  Log->Initialize();

  while((opt = getopt(argc,argv, "f:o:i:hm:rp")) !=-1 ){
    switch(opt){
      case 'f':
        inputDirName=optarg;
        if(!check->Dir(inputDirName)){ 
          cout<<"!!Error!! "<<inputDirName.c_str()<<"doesn't exist!!";
          Log->eWrite(Form("[%s][wgAnaHistSummary]Error!!target doesn't exist",inputDirName.c_str()));
          return 1;
        }   
        Log->Write(Form("[%s][wgAnaHistSummary]start wgAnaHistSummary",inputDirName.c_str()));
        break;

      case 'o':
        outputXMLDirName = optarg; 
        break;
      
      case 'i':
        outputIMGDirName = optarg;
        break;

      case 'm':
        mode = atoi(optarg); 
        break;

      case 'p':
        SELECT_Print = true; 
        break;

      case 'r':
        overwrite = true; 
        break;

      case 'h':
        cout <<"this program is for summarizing the wgAnaHist outputs. "<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose input directory you wanna read(must)"<<endl;
        cout <<"  -o (char*) : choose output directory (default: input directory) "<<endl;
        cout <<"  -i (char*) : choose image directory (default: WAGASCI_IMGDIR) "<<endl;
        cout <<"  -p         : Print images." << endl;
        cout <<"  -r         : overwrite mode"<<endl;
        cout <<"  -m (int)   : mode (default:10)"<<endl;
        cout <<"   ===   mode  === "<<endl;
        cout <<"   10 : Noise Rate + Gain "<<endl;
        cout <<"   11 : Noise Rate + Gain + Pedestal "<<endl;
        cout <<"   12 : Noise Rate + Gain + Pedestal + Raw Charge "<<endl;
        exit(0);
      default:
        cout <<"this program is for summarizing the wgAnaHist outputs. "<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose input directory you wanna read(must)"<<endl;
        cout <<"  -o (char*) : choose output directory (default: input directory) "<<endl;
        cout <<"  -i (char*) : choose image directory (default: WAGASCI_IMGDIR) "<<endl;
        cout <<"  -p         : Print images." << endl;
        cout <<"  -r         : overwrite mode"<<endl;
        cout <<"  -m (int)   : mode (default:10)"<<endl;
        cout <<"   ===   mode  === "<<endl;
        cout <<"   10 : Noise Rate + Gain "<<endl;
        cout <<"   11 : Noise Rate + Gain + Pedestal "<<endl;
        cout <<"   12 : Noise Rate + Gain + Pedestal + Raw Charge "<<endl;
        exit(0);
    }   
  }

  if(inputDirName==""){
    cout << "!!ERROR!! please input filename." <<endl;
    cout << "if you don't know how to input, please see help."<<endl;
    cout << "help : ./wgAnaHistSummary -h" <<endl;
    exit(1);
  }

  if(outputXMLDirName=="")outputXMLDirName=inputDirName;

  outputDirName = OpStr->GetName(inputDirName);
  outputIMGDirName = Form("%s/%s",outputIMGDirName.c_str(),outputDirName.c_str());

  cout << " *****  READING DIRECTORY     :" << inputDirName << "  *****" << endl;
  cout << " *****  OUTPUT XML DIRECTORY :" << outputXMLDirName << "  *****" << endl;
  cout << " *****  OUTPUT IMAGE DIRECTORY :" << outputIMGDirName << "  *****" << endl;

  delete check;
  delete OpStr;

  if(SELECT_Print) MakeDir(outputIMGDirName);
  MakeDir(outputXMLDirName);
  MakeXMLFILE(inputDirName,overwrite);
  ReadXML(inputDirName,outputXMLDirName,outputIMGDirName,mode);

  Log->Write(Form("[%s][wgAnaHistSummary]end wgAnaHistSummary.",inputDirName.c_str() ));
  delete Log;  
}

//******************************************************************
void ModeSelect(int mode){
  if(mode==0){
    cout <<"   ===   mode  === "<<endl;
    cout <<"   1 : only Noise Rate "<<endl;
    cout <<"   2 : only Gain       "<<endl;
    cout <<"   3 : only Pedestal   "<<endl;
    cout <<"   4 : only Raw Charge "<<endl;
    cout <<"   10 : Noise Rate + Gain "<<endl;
    cout <<"   11 : Noise Rate + Gain + Pedestal "<<endl;
    cout <<"   12 : Noise Rate + Gain + Pedestal + Raw Charge "<<endl;
  }
  if(mode==1||mode>=10) SELECT_Noise  =true;
  if(mode==2||mode>=10) SELECT_Gain   =true;
  if(mode==3||mode>=11) SELECT_Pedestal=true;
  if(mode==4||mode==12) SELECT_RawCharge=true;
}

//******************************************************************
void ReadXML(string& inputDirName, string& outputXMLDirName,string& outputIMGDirName,int mode){

  ModeSelect(mode);

  wgEditXML *Edit = new wgEditXML();
  string xmlfile("");
  int start_time,stop_time;
  int trig_th[NCHIPS];
  int gain_th[NCHIPS];
  int inputDAC[NCHIPS][32];
  int ampDAC[NCHIPS][32];
  //int adjDAC[NCHIPS][32];
  double charge[NCHIPS][32];
  double rawcharge[NCHIPS][32][MEMDEPTH][2];
  double e_rawcharge[NCHIPS][32][MEMDEPTH][2];
  double Noise[NCHIPS][32][2];

  wgColor wgColor;

  TH1F *h_Pedestal[NCHIPS];
  TH1F *h_Gain[NCHIPS];
  TH1F *h_rawcharge[NCHIPS];
  TH1F *h_Noise[NCHIPS];

  //*** Define histgram ***//
  if(SELECT_Print){
    for(unsigned int i=0;i<NCHIPS;i++){
      int ichip=i;
      if(SELECT_Pedestal){ 
        h_Pedestal[ichip]=new TH1F(Form("h_pedestal_chip%d",ichip),Form("h_pedestal_chip%d",ichip),32*26+10,-5,32*26+5);
        h_Pedestal[ichip]->SetTitle(Form("pedestal chip:%d;ch*26+col;ADC count",ichip));
        h_Pedestal[ichip]->SetMarkerStyle(8);
        h_Pedestal[ichip]->SetMarkerSize(0.3);
        h_Pedestal[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
        h_Pedestal[ichip]->SetStats(0);
      }

      if(SELECT_RawCharge){ 
        h_rawcharge[ichip]=new TH1F(Form("h_rawcharge__chip%d",ichip),Form("h_rawcharge_chip%d",ichip),32*26+10,-5,32*26+5);
        h_rawcharge[ichip]->SetTitle(Form("Gain chip:%d;ch*26+col;ADC count",ichip));
        h_rawcharge[ichip]->SetMarkerStyle(8);
        h_rawcharge[ichip]->SetMarkerSize(0.3);
        h_rawcharge[ichip]->SetStats(0);
        h_rawcharge[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
      }

      if(SELECT_Gain){ 
        h_Gain[ichip]=new TH1F(Form("h_Gain_chip%d",ichip),Form("h_Gain_chip%d",ichip),34,-1,33);
        h_Gain[ichip]->SetTitle(Form("Gain chip:%d;ch;ADC count",ichip));
        h_Gain[ichip]->SetMarkerStyle(8);
        h_Gain[ichip]->SetMarkerSize(0.3);
        h_Gain[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
        h_Gain[ichip]->SetStats(0);
      }

      if(SELECT_Noise){ 
        h_Noise[ichip]=new TH1F(Form("h_Noise_chip%d",ichip),Form("h_Noise_chip%d",ichip),34,-1,33);
        h_Noise[ichip]->SetTitle(Form("Noise chip:%d;ch;Noise Rate[Hz]",ichip));
        h_Noise[ichip]->SetMarkerStyle(8);
        h_Noise[ichip]->SetMarkerSize(0.3);
        h_Noise[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
        h_Noise[ichip]->SetStats(0);
      }
    }
  }

  //*** Read data ***//
  string name("");
  for(unsigned int i=0;i<NCHIPS;i++){
    int ichip=i;
    for(unsigned int j=0;j<32;j++){
      int ich=j;
      xmlfile=Form("%s/chip%d/ch%d.xml",inputDirName.c_str(),ichip,ich);
      Edit->Open(xmlfile);
      if(ich==0){
        if(ichip==0){
          name="start_time";
          start_time=Edit->GetConfigValue(name);
          name="stop_time";
          stop_time=Edit->GetConfigValue(name);
        }
        name="trigth";
        trig_th[ichip]=Edit->GetConfigValue(name);
        name="gainth";
        gain_th[ichip]=Edit->GetConfigValue(name);
      }
      name="inputDAC";
      inputDAC[ichip][ich]=Edit->GetConfigValue(name);
      name="HG";
      ampDAC[ichip][ich]=Edit->GetConfigValue(name);
      /*
      name="trig_adj";
      adjDAC[ichip][ich]=Edit->GetConfigValue(name);
      */
      if(SELECT_Noise){ 
        name="NoiseRate";
        Noise[ichip][ich][0]=Edit->GetChValue(name);
        name="NoiseRate_e";
        Noise[ichip][ich][1]=Edit->GetChValue(name);
      }

      if(SELECT_Gain){ 
        name="charge_low";
        charge[ichip][ich]=Edit->GetChValue(name);
      }

      for(unsigned int k=0;k<MEMDEPTH;k++){
        int icol=k;
        if(SELECT_Pedestal){ 
          name="charge_nohit";
          rawcharge[ichip][ich][icol][0]=Edit->GetColValue(name,icol); 
          name="sigma_nohit";
          e_rawcharge[ichip][ich][icol][0]=Edit->GetColValue(name,icol); 
        }

        if(SELECT_RawCharge){ 
          name="charge_lowHG";
          rawcharge[ichip][ich][icol][1]=Edit->GetColValue(name,icol); 
          name="sigma_lowHG";
          e_rawcharge[ichip][ich][icol][1]=Edit->GetColValue(name,icol); 
        }
      }
      Edit->Close();
    }
  }

  //*** Fill data ***//
  for(unsigned int i=0;i<NCHIPS;i++){
    int ichip=i;
    xmlfile=Form("%s/Summary_chip%d.xml",outputXMLDirName.c_str(),ichip);
    Edit->Open(xmlfile);
    name="start_time";
    Edit->SUMMARY_SetGlobalConfigValue(name,start_time,0);
    name="stop_time";
    Edit->SUMMARY_SetGlobalConfigValue(name,stop_time,0);
    name="trigth";
    Edit->SUMMARY_SetGlobalConfigValue(name,trig_th[ichip],0);
    name="gainth";
    Edit->SUMMARY_SetGlobalConfigValue(name,gain_th[ichip],0);
    for(unsigned int j=0;j<32;j++){
      int ich=j;
      name="inputDAC";
      Edit->SUMMARY_SetChConfigValue(name,inputDAC[ichip][ich],ich,0);
      name="ampDAC";
      Edit->SUMMARY_SetChConfigValue(name,ampDAC[ichip][ich],ich,0);
      /*
      name="adjDAC";
      Edit->SUMMARY_SetChConfigValue(name,adjDAC[ichip][ich],ich,0);
      */
      if(SELECT_Gain){
        name="Gain";
        double Gain = charge[ichip][ich];  
        Edit->SUMMARY_SetChFitValue(name,Gain,ich,0);
        if(SELECT_Print) h_Gain[ichip]->Fill(ich,Gain);
      }

      if(SELECT_Noise){
        name="Noise";
        Edit->SUMMARY_SetChFitValue(name,Noise[ichip][ich][0],ich,0); 
        if(SELECT_Print) h_Noise[ichip]->Fill(ich,Noise[ichip][ich][0]);
      }

      for(unsigned int k=0;k<MEMDEPTH+1;k++){
        int icol=k;
        if(SELECT_Pedestal){
          name=Form("ped_%d",icol);
          Edit->SUMMARY_SetChFitValue(name,rawcharge[ichip][ich][icol][0],ich,1);
          name=Form("eped_%d",icol);
          Edit->SUMMARY_SetChFitValue(name,e_rawcharge[ichip][ich][icol][0],ich,1);
          if(SELECT_Print) h_Pedestal[ichip]->Fill(ich*26+icol,rawcharge[ichip][ich][icol][0]);
        }

        if(SELECT_RawCharge){
          name=Form("raw_%d",icol);
          Edit->SUMMARY_SetChFitValue(name,rawcharge[ichip][ich][icol][1],ich,1);
          name=Form("eraw_%d",icol);
          Edit->SUMMARY_SetChFitValue(name,e_rawcharge[ichip][ich][icol][1],ich,1);
          if(SELECT_Print) h_rawcharge[ichip]->Fill(ich*26+icol,rawcharge[ichip][ich][icol][1]);
        }
      }
    }
    Edit->Write();
    Edit->Close();
  }

  if(SELECT_Print){
    TCanvas *c1 = new TCanvas("c1","c1");
    TLegend *l_Noise[NCHIPS];
    TLegend *l_Gain[NCHIPS];
    TLegend *l_rawcharge[NCHIPS];
    TLegend *l_Pedestal[NCHIPS];
    for(unsigned int i=0;i<NCHIPS;i++){
      int ichip = i;
      if(SELECT_Noise){
        l_Noise[ichip]=new TLegend(0.75,0.84,0.90,0.90,Form("chip:%d",ichip));
        l_Noise[ichip]->SetBorderSize(1);
        l_Noise[ichip]->SetFillStyle(0);
        l_Noise[ichip]->AddEntry(h_Noise[ichip],"Noise Rate","p");
        h_Noise[ichip]->Draw("P HIST");
        l_Noise[ichip]->Draw();
        c1->Print(Form("%s/Summary_Noise_chip%d.png",outputIMGDirName.c_str(),ichip));
        delete h_Noise[ichip];
        delete l_Noise[ichip];
      }

      if(SELECT_Gain){
        l_Gain[ichip]=new TLegend(0.75,0.81,0.90,0.90,Form("chip:%d",ichip));
        l_Gain[ichip]->SetBorderSize(1);
        l_Gain[ichip]->SetFillStyle(0);
        l_Gain[ichip]->AddEntry(h_Gain[ichip],"Gain","p");
        h_Gain[ichip]->Draw("P HIST");
        l_Gain[ichip]->Draw();
        c1->Print(Form("%s/Summary_Gain_chip%d.png",outputIMGDirName.c_str(),ichip));
        delete h_Gain[ichip];
        delete l_Gain[ichip];
      }

      if(SELECT_RawCharge){
        l_rawcharge[ichip]=new TLegend(0.75,0.75,0.90,0.90,Form("chip:%d",ichip));
        l_rawcharge[ichip]->SetBorderSize(1);
        l_rawcharge[ichip]->SetFillStyle(0);
        l_rawcharge[ichip]->AddEntry(h_rawcharge[ichip],Form("Gain"),"p");
        h_rawcharge[ichip]->Draw("P HIST");
        l_rawcharge[ichip]->Draw();
        c1->Print(Form("%s/Summary_rawcharge_chip%d.png",outputIMGDirName.c_str(),ichip));
        delete h_rawcharge[ichip];
        delete l_rawcharge[ichip];
      }

      if(SELECT_Pedestal){
        l_Pedestal[ichip]=new TLegend(0.75,0.84,0.90,0.90,Form("chip:%d",ichip));
        l_Pedestal[ichip]->SetBorderSize(0);
        l_Pedestal[ichip]->SetFillStyle(0);
        l_Pedestal[ichip]->AddEntry(h_Pedestal[ichip],Form("Pedestal \n\t chip:%d",ichip),"p");
        c1->DrawFrame(-5,begin_ped,32*26+5,end_ped);
        h_Pedestal[ichip]->Draw("same P HIST");
        l_Pedestal[ichip]->Draw();
        c1->Print(Form("%s/Summary_Pedestal_chip%d.png",outputIMGDirName.c_str(),ichip));
        delete h_Pedestal[ichip];
        delete l_Pedestal[ichip];
      }
    }
  }
}

//******************************************************************
void MakeDir(string& str){
  CheckExist *check = new CheckExist;
  if(!check->Dir(str)){
    system(Form("mkdir %s",str.c_str()));
  }
  delete check;
}

//******************************************************************
void MakeXMLFILE(string& str,bool overwrite){
  cout << " *****  MAKING XML FILE  ***** "<< endl;
  wgEditXML *Edit = new wgEditXML();
  CheckExist *check = new CheckExist();
  string outputxmlfile("");
  for(unsigned int i=0; i<NCHIPS; i++){
    int ichip = i;
    outputxmlfile = Form("%s/Summary_chip%d.xml",str.c_str(),ichip);
    if(check->XmlFile(outputxmlfile)){
      if(overwrite){
        Edit->SUMMARY_Make(outputxmlfile,ichip);
        cout << "     making " << outputxmlfile.c_str() << " ..." << endl;
      }
    }else{
      Edit->SUMMARY_Make(outputxmlfile,ichip);
      cout << "     making " << outputxmlfile.c_str() << " ..." << endl;
    }
  } 
  delete Edit;
  delete check;
}




