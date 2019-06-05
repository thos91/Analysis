#include <string>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>

#include <THStack.h>
#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TMultiGraph.h>
#include <TLegend.h>
#include <TH1.h>

#include "Const.hpp"
#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgFitConst.hpp"
#include "wgEditXML.h"
#include "wgEditConfig.h"

using namespace std;

void MakeDir(string& str);
void MakeXML(string& str);
void AnaXML(string& inputDirName, string& outputXMLDirName,string& outputIMGDirName,int idif ,int ichip);

//#define DEBUG_OPTIMIZE
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

  Log->Initialize();
  Log->Write("start summarize calibration...");

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
    outputXMLDirName="/home/data/calibration";
  }
  outputIMGDirName = Form("%s/%s",outputIMGDirName.c_str(),OpStr->GetName(inputDirName).c_str());

  Log->Write(Form("READING DIRECTORY : %s",inputDirName.c_str()));
  Log->Write(Form("OUTPUT DIRECTORY : %s",outputXMLDirName.c_str()));

  cout << " *****  READING DIRECTORY     :" << inputDirName << "  *****" << endl;
  cout << " *****  OUTPUT XML DIRECTORY :" << outputXMLDirName << "  *****" << endl;
  cout << " *****  OUTPUT IMAGE DIRECTORY :" << outputIMGDirName << "  *****" << endl;

  delete check;
  delete OpStr;

  cout << " Finish reading file. "<<endl;
  MakeXML(outputXMLDirName);

  int nchip=NCHIPS;
  //int nchip=1;
  for(int idif=0;idif<2;idif++){  
    for(int ichip=0;ichip<nchip;ichip++){
      AnaXML(inputDirName,
          outputXMLDirName,
          outputIMGDirName,
          idif,
          ichip
          );
    }
  }
  Log->Write("end summarizeing ... " );
  delete Log;  
}

//******************************************************************
void AnaXML(string& inputDirName, string& outputXMLDirName,string& outputIMGDirName,int idif ,int ichip){

  double pe_center1[13][32];
  double pe_width1[13][32];
  double pe_center2[13][32];
  double pe_width2[13][32];
  double pe_center_fit[2][13];
  double pe_center_fit_e[2][13];
  double ch[32],ch_e[32];
  for(int ich=0;ich<32;ich++){
    ch[ich]=ich;
    ch_e[ich]=0.;
  }
  double inputDAC[13],inputDAC_e[13];
  for(int iDAC=0;iDAC<13;iDAC++){
    inputDAC[iDAC]=1.+20.*iDAC;
    inputDAC_e[iDAC]=1.;
  }

  //*** Read data ***
  cout <<endl;
  cout << "  ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~  " <<endl;
  cout << "  ~~~ Start Reading ( dif : " << idif << ", chip : " << ichip << " ) ~~~  " <<endl;
  cout << "  ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~  " <<endl;
  wgEditXML *Edit = new wgEditXML();
  string name("");
  string xmlfile("");

  if(idif==0 && ichip == 21){
    for(unsigned int j=0;j<13;j++){
      pe_center_fit[0][j]=165.;
      pe_center_fit_e[0][j]=1.;
      pe_center_fit[1][j]=155.;
      pe_center_fit_e[0][j]=1.;
    }
  }else{
    for(unsigned int i=0;i<32;i++){
      int ich=i;
      xmlfile=Form("%s/dif%d/chip%d/ch%d.xml",inputDirName.c_str(),idif+1,ichip,ich);
      Edit->Open(xmlfile);
      for(unsigned int j=0;j<13;j++){
        int inputDAC=1+20*j;
        name="pe_center1";
        pe_center1[j][ich]=Edit->SCURVE_GetValue(name,inputDAC);
        name="pe_center2";
        pe_center2[j][ich]=Edit->SCURVE_GetValue(name,inputDAC);
        name="pe_width1";
        pe_width1[j][ich]=Edit->SCURVE_GetValue(name,inputDAC);
        name="pe_width2";
        pe_width2[j][ich]=Edit->SCURVE_GetValue(name,inputDAC);
      }
      Edit->Close();
    }
    delete Edit;

    for(unsigned int j=0;j<13;j++){
      TGraphErrors * g1 = new TGraphErrors(32,ch,pe_center1[j],ch_e,pe_width1[j]); 
      TGraphErrors * g2 = new TGraphErrors(32,ch,pe_center2[j],ch_e,pe_width2[j]); 
      TF1 *f1 = new TF1("f1","pol0");
      TF1 *f2 = new TF1("f2","pol0");
      g1->Fit("f1","Q+","same");
      g2->Fit("f2","Q+","same");
      g1->SetMarkerColor(632);
      g2->SetMarkerColor(600);
      g1->SetMarkerSize(1);
      g2->SetMarkerSize(1);
      g1->SetMarkerStyle(8);
      g2->SetMarkerStyle(8);
      pe_center_fit[0][j]=f1->GetParameter(0);
      pe_center_fit[1][j]=f2->GetParameter(0);
      if(j==0 || j==12){
      pe_center_fit_e[0][j]=5.;
      pe_center_fit_e[1][j]=5.;
      }else{
      pe_center_fit_e[0][j]=f1->GetParError(0);
      pe_center_fit_e[1][j]=f2->GetParError(0);
      }
      for(int pe=0;pe<2;pe++){
        pe_center_fit[pe][j]=round(pe_center_fit[pe][j]);
      }
#ifdef DEBUG_OPTIMIZE
      TCanvas *c1 = new TCanvas("c1","c1");
      TMultiGraph * mg = new TMultiGraph();
      mg->Add(g1);
      mg->Add(g2);
      mg->SetMaximum(175.);
      mg->SetMinimum(140.);
      mg->Draw("ap");
      c1->Print(Form("/home/rtam/pe_dif%d_chip%d_inputDAC%d.png",idif,ichip,1+j*20));
#endif
    }
  }

  TMultiGraph * mg2 = new TMultiGraph();
  TGraphErrors * gr[2];
  TF1 * f_th[2];
  TCanvas *c1 = new TCanvas("c1","c1");
  for(int pe=0;pe<2;pe++){
    gr[pe] = new TGraphErrors(13, inputDAC, pe_center_fit[pe], inputDAC_e, pe_center_fit_e[pe]);
    f_th[pe] = new TF1(Form("f_th%d",pe),"[0]*x+[1]");
    gr[pe]->Fit(Form("f_th%d",pe),"Q+","same");
    if(pe==0){
      gr[pe]->SetMarkerColor(kRed);
    }else{
      gr[pe]->SetMarkerColor(kBlue);
    }
    gr[pe]->SetMarkerSize(1);
    gr[pe]->SetMarkerStyle(8);
    mg2->Add(gr[pe]);
  }
  mg2->Draw("ap");
  c1->Print(Form("/home/rtam/pe_dif%d_chip%d.png",idif,ichip));

  Edit = new wgEditXML();
  xmlfile=Form("%s/threshold_card.xml",outputXMLDirName.c_str());
  Edit -> Open(xmlfile);
  for(int pe=0;pe<2;pe++){
    name= Form("s_th%d",pe+1);
    Edit->OPT_SetChipValue(name,idif+1,ichip,f_th[pe]->GetParameter(0),0);
    name= Form("i_th%d",pe+1);
    Edit->OPT_SetChipValue(name,idif+1,ichip,f_th[pe]->GetParameter(1),0);
  }
  name= Form("threshold_3");
  Edit->OPT_SetChipValue(name,idif+1,ichip,150,1);
  for(int j=0;j<13;j++){
    for(int pe=0;pe<2;pe++){
      name= Form("threshold_%d",pe+1);
      Edit->OPT_SetValue(name,idif+1,ichip,j*20+1,pe_center_fit[pe][j],0);
    }

    bool bool_center[2]={true,true};
#ifdef DEBUG_OPTIMIZE
    for(unsigned int i=0;i<32;i++){
      int ich=i;
      if(pe_center1[j][ich]==0){
        cout << " ** pe:1" << ", ich:" <<ich <<", inputDAC:"<<1+20*j<< " has no entry." <<endl;            continue;
      }
      if( abs(pe_center1[j][ich]-pe_center_fit[0][j]) > pe_width1[j][ich] ){
        bool_center[0] = false;
        cout << "   ==  pe:1" << ", ch:" <<ich <<", inputDAC:"<<1+20*j<< " get over!!(fit:"<<pe_center_fit[0][j]<<", data:"<< pe_center1[j][ich] << "  ==" <<endl; 
      }

      if(pe_center2[j][ich]==0){
        cout << " ** pe:2" << ", ich:" <<ich <<", inputDAC:"<<1+20*j<< " has no entry." <<endl;            continue;
      }

      if( abs(pe_center2[j][ich]-pe_center_fit[1][j]) > pe_width2[j][ich] ){
        bool_center[1] = false;
        cout << "   ==  pe:2" << ", ch:" <<ich <<", inputDAC:"<<1+20*j<< " get over!!(fit:"<<pe_center_fit[1][j]<<", data:"<< pe_center2[j][ich] << "  ==" <<endl; 
      }
    }//ich
#endif

    for(int pe=0;pe<2;pe++){
      if(!bool_center[pe]){
        cout << "pe:" << pe << " ,inputDAC:" << 1+20*j << " is BAD!!!!" <<endl;
      }
    }
  }//inputDAC
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

//******************************************************************
void MakeXML(string& outputXMLDirName){
  string name;
  wgEditXML *Edit = new wgEditXML();
  name=Form("%s/threshold_card.xml",outputXMLDirName.c_str());
  Edit->OPT_Make(name);
  delete Edit;
}


