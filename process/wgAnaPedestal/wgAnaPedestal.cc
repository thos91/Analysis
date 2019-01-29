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

double NoiseToPe(double);
void MakeDir(string& str);
void MakeXMLFILE(string& str, bool overwrite);
void ReadXML(string& inputDirName,string& outputXMLDirName,string& outputIMGDirName);

//******************************************************************
int main(int argc, char** argv){

  int opt;
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

  while((opt = getopt(argc,argv, "f:o:hr")) !=-1 ){
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

      case 'r':
        overwrite = true; 
        break;

      case 'h':
        cout <<"this program is for summarizing the wgAnaHist outputs. "<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose input directory you wanna read(must)"<<endl;
        cout <<"  -o (char*) : choose output directory (default: input directory) "<<endl;
        cout <<"  -r         : overwrite mode"<<endl;
        exit(0);
      default:
        cout <<"this program is for summarizing the wgAnaHist outputs. "<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose input directory you wanna read(must)"<<endl;
        cout <<"  -o (char*) : choose output directory (default: input directory) "<<endl;
        cout <<"  -r         : overwrite mode"<<endl;
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

  MakeDir(outputIMGDirName);
  MakeDir(outputXMLDirName);
  MakeXMLFILE(inputDirName,overwrite);
  ReadXML(inputDirName,outputXMLDirName,outputIMGDirName);

  Log->Write(Form("[%s][wgAnaHistSummary]end wgAnaHistSummary.",inputDirName.c_str() ));
  delete Log;  
}

//******************************************************************
void ReadXML(string& inputDirName, string& outputXMLDirName,string& outputIMGDirName){
  wgEditXML *Edit = new wgEditXML();
  string xmlfile("");
  int trig_th[NCHIPS];
  int gain_th[NCHIPS];
  int inputDAC[NCHIPS][32];
  int ampDAC[NCHIPS][32];
  int adjDAC[NCHIPS][32];
  double charge[NCHIPS][32][MEMDEPTH][2];
  double gain[NCHIPS][32][MEMDEPTH];
  double Noise[NCHIPS][32][2];
  double pe_level[NCHIPS][32];

  wgColor wgColor;

  TH1F *h_Pedestal[NCHIPS];
  TH1F *h_Gain[NCHIPS];
  TH1F *h_Gain_all[NCHIPS];
  TH1F *h_Noise[NCHIPS];

  //*** Define histgram ***//
  for(unsigned int i=0;i<NCHIPS;i++){
    int ichip=i;
    h_Pedestal[ichip]=new TH1F(Form("h_pedestal_chip%d",ichip),Form("h_pedestal_chip%d",ichip),32*26+10,-5,32*26+5);
    h_Pedestal[ichip]->SetTitle(Form("pedestal chip:%d;ch*26+col;ADC count",ichip));
    h_Pedestal[ichip]->SetMarkerStyle(8);
    h_Pedestal[ichip]->SetMarkerSize(0.3);
    h_Pedestal[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
    h_Pedestal[ichip]->SetStats(0);
    
    h_Gain_all[ichip]=new TH1F(Form("h_Gain_all_chip%d",ichip),Form("h_Gain_all_chip%d",ichip),32*26+10,-5,32*26+5);
    h_Gain_all[ichip]->SetTitle(Form("Gain chip:%d;ch*26+col;ADC count",ichip));
    h_Gain_all[ichip]->SetMarkerStyle(8);
    h_Gain_all[ichip]->SetMarkerSize(0.3);
    h_Gain_all[ichip]->SetStats(0);
    h_Gain_all[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);

    h_Gain[ichip]=new TH1F(Form("h_Gain_chip%d",ichip),Form("h_Gain_chip%d",ichip),34,-1,33);
    h_Gain[ichip]->SetTitle(Form("Gain chip:%d;ch;ADC count",ichip));
    h_Gain[ichip]->SetMarkerStyle(8);
    h_Gain[ichip]->SetMarkerSize(0.3);
    h_Gain[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
    h_Gain[ichip]->SetStats(0);
    
    h_Noise[ichip]=new TH1F(Form("h_Noise_chip%d",ichip),Form("h_Noise_chip%d",ichip),34,-1,33);
    h_Noise[ichip]->SetTitle(Form("Noise chip:%d;ch;Noise Rate[Hz]",ichip));
    h_Noise[ichip]->SetMarkerStyle(8);
    h_Noise[ichip]->SetMarkerSize(0.3);
    h_Noise[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
    h_Noise[ichip]->SetStats(0);
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
        name="trigth";
        trig_th[ichip]=Edit->GetConfigValue(name);
        name="gainth";
        gain_th[ichip]=Edit->GetConfigValue(name);
      }
      name="inputDAC";
      inputDAC[ichip][ich]=Edit->GetConfigValue(name);
      name="HG";
      ampDAC[ichip][ich]=Edit->GetConfigValue(name);
      name="trig_adj";
      adjDAC[ichip][ich]=Edit->GetConfigValue(name);
      name="NoiseRate";
      Noise[ichip][ich][0]=Edit->GetChValue(name);
      name="NoiseRate_e";
      Noise[ichip][ich][1]=Edit->GetChValue(name);
      pe_level[ichip][ich]=NoiseToPe(Noise[ichip][ich][0]);

      for(unsigned int k=0;k<MEMDEPTH;k++){
        int icol=k;
        name="charge_nohit";
        charge[ichip][ich][icol][0]=Edit->GetColValue(name,icol); 
        name="charge_lowHG";
        charge[ichip][ich][icol][1]=Edit->GetColValue(name,icol);
        gain[ichip][ich][icol]=charge[ichip][ich][icol][1]-charge[ichip][ich][icol][0];
      }
      Edit->Close();
    }
  }
  
  //*** Fill data ***//
  for(unsigned int i=0;i<NCHIPS;i++){
    int ichip=i;
    xmlfile=Form("%s/Summary_chip%d.xml",outputXMLDirName.c_str(),ichip);
    Edit->Open(xmlfile);
    name="tr_th";
    Edit->SUMMARY_SetGlobalConfigValue(name,trig_th[ichip],0);
    name="gs_th";
    Edit->SUMMARY_SetGlobalConfigValue(name,gain_th[ichip],0);
    for(unsigned int j=0;j<32;j++){
      int ich=j;
      name="inputDAC";
      Edit->SUMMARY_SetChConfigValue(name,inputDAC[ichip][ich],ich,0);
      name="ampDAC";
      Edit->SUMMARY_SetChConfigValue(name,ampDAC[ichip][ich],ich,0);
      name="adjDAC";
      Edit->SUMMARY_SetChConfigValue(name,adjDAC[ichip][ich],ich,0);
      name="Gain";
      Edit->SUMMARY_SetChFitValue(name,gain[ichip][ich][0],ich,0);
      name="Noise";
      Edit->SUMMARY_SetChFitValue(name,Noise[ichip][ich][0],ich,0);
      name="pe_level";
      Edit->SUMMARY_SetChFitValue(name,pe_level[ichip][ich],ich,1);
      h_Noise[ichip]->Fill(ich,Noise[ichip][ich][0]);
      h_Gain[ichip]->Fill(ich,gain[ichip][ich][0]);
      for(unsigned int k=0;k<MEMDEPTH;k++){
        int icol=k;
        name=Form("ped_%d",icol);
        Edit->SUMMARY_SetChFitValue(name,charge[ichip][ich][icol][0],ich,0);
        name=Form("ped_ref_%d",icol);
        Edit->SUMMARY_SetChFitValue(name,charge[ichip][ich][icol][1],ich,1);
        h_Pedestal[ichip]->Fill(ich*26+icol,charge[ichip][ich][icol][0]);
        h_Gain_all[ichip]->Fill(ich*26+icol,gain[ichip][ich][icol]);
      }
    }
    Edit->Write();
    Edit->Close();
  }

  TCanvas *c1 = new TCanvas("c1","c1");
  TLegend *l_Noise[NCHIPS];
  TLegend *l_Gain[NCHIPS];
  TLegend *l_Gain_all[NCHIPS];
  TLegend *l_Pedestal[NCHIPS];
  for(unsigned int i=0;i<NCHIPS;i++){
    int ichip = i;
    l_Noise[ichip]=new TLegend(0.75,0.84,0.90,0.90,Form("chip:%d",ichip));
    l_Noise[ichip]->SetBorderSize(1);
    l_Noise[ichip]->SetFillStyle(0);
    l_Noise[ichip]->AddEntry(h_Noise[ichip],"Noise Rate","p");
    h_Noise[ichip]->Draw("P HIST");
    l_Noise[ichip]->Draw();
    c1->Print(Form("%s/Summary_Noise_chip%d.png",outputIMGDirName.c_str(),ichip));

    l_Gain[ichip]=new TLegend(0.75,0.81,0.90,0.90,Form("chip:%d",ichip));
    l_Gain[ichip]->SetBorderSize(1);
    l_Gain[ichip]->SetFillStyle(0);
    l_Gain[ichip]->AddEntry(h_Gain[ichip],"Gain","p");
    h_Gain[ichip]->Draw("P HIST");
    l_Gain[ichip]->Draw();
    c1->Print(Form("%s/Summary_Gain_chip%d.png",outputIMGDirName.c_str(),ichip));
    
    l_Gain_all[ichip]=new TLegend(0.75,0.75,0.90,0.90,Form("chip:%d",ichip));
    l_Gain_all[ichip]->SetBorderSize(1);
    l_Gain_all[ichip]->SetFillStyle(0);
    l_Gain_all[ichip]->AddEntry(h_Gain_all[ichip],Form("Gain"),"p");
    h_Gain_all[ichip]->Draw("P HIST");
    l_Gain_all[ichip]->Draw();
    c1->Print(Form("%s/Summary_Gain_all_chip%d.png",outputIMGDirName.c_str(),ichip));

    l_Pedestal[ichip]=new TLegend(0.75,0.84,0.90,0.90,Form("chip:%d",ichip));
    l_Pedestal[ichip]->SetBorderSize(0);
    l_Pedestal[ichip]->SetFillStyle(0);
    l_Pedestal[ichip]->AddEntry(h_Pedestal[ichip],Form("Pedestal \n\t chip:%d",ichip),"p");
    c1->DrawFrame(-5,begin_ped,32*26+5,end_ped);
    h_Pedestal[ichip]->Draw("same P HIST");
    l_Pedestal[ichip]->Draw();
    c1->Print(Form("%s/Summary_Pedestal_chip%d.png",outputIMGDirName.c_str(),ichip));
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

//******************************************************************
double NoiseToPe(double noise){
  if(noise>u_limit_1pe){ return 0.5;
  }else if(noise>=l_limit_1pe && noise<u_limit_1pe){ return 1.0;
  }else if(noise>=u_limit_2pe && noise<l_limit_1pe){ return 1.5;
  }else if(noise>=l_limit_2pe && noise<u_limit_2pe){ return 2.0;
  }else if(noise>=u_limit_3pe && noise<l_limit_2pe){ return 2.5;
  }else if(noise>=l_limit_3pe && noise<u_limit_3pe){ return 3.0;
  }else if( noise<l_limit_3pe && noise>0.){ return 3.5;
  }else{  return 0.; } 
}



