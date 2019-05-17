#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>

#include <THStack.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TH1D.h>
#include <TH2D.h>

#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgEditXML.h"
#include "wgColor.h"
#include "wgFit.h"
#include "wgFitConst.h"
#include "wgGetHist.h"

using namespace std;

void MakeDir(const string& str);
void ModeSelect(int mode);
void MakeSummaryXmlFile(const string& str, bool overwrite, unsigned n_chips = NCHIPS, unsigned n_chans = NCHANNELS);
void wgAnaHistSummary(const string& inputDirName, const string& outputXMLDirName, const string& outputIMGDirName, int mode, unsigned n_chips = NCHIPS, unsigned n_chans = NCHANNELS);
void print_help(const char * program_name) {
  cout <<  program_name << " summarizes the wgAnaHist output into a TO-DO.\n"
	"  -h         : help\n"
	"  -f (char*) : input directory (mandatory)\n"
	"  -o (char*) : output directory (default: same as input directory)\n"
	"  -i (char*) : output directory for plots and images (default: WAGASCI_IMGDIR) "
	"  -n (int)   : number of DIFs (default is 2)\n"
	"  -x (int)   : number of chips per DIF (default is 20)\n"
	"  -y (int)   : number of channels per chip (default is 36)\n"
	"  -p         : print plots and images\n"
	"  -r         : overwrite mode\n"
	"  -m (int)   : mode (default:10)\n"
	"   ===   mode  === \n"
	"   10 : Noise Rate + Gain\n"
	"   11 : Noise Rate + Gain + Pedestal\n"
	"   12 : Noise Rate + Gain + Pedestal + Raw Charge\n";
  exit(0);
}


  
bool SELECT_Noise;
bool SELECT_Gain;
bool SELECT_Pedestal;
bool SELECT_RawCharge;
bool SELECT_Print;
//******************************************************************
int main(int argc, char** argv){

  int opt;
  int mode = 10;
  int n_chips = NCHIPS, n_chans = NCHANNELS;
  bool overwrite = false;
  wgConst con;
  con.GetENV();
  string inputDirName("");
  string configFileName("");
  string outputXMLDirName("");
  string outputDirName("");
  string logoutputDir(con.LOG_DIRECTORY);
  string outputIMGDirName(con.IMGDATA_DIRECTORY);
  
  OperateString OpStr;
  CheckExist check;

  while((opt = getopt(argc,argv, "f:o:i:hm:rp")) !=-1 ){
    switch(opt){
	case 'f':
	  inputDirName=optarg;
	  if(!check.Dir(inputDirName)){ 
		Log.eWrite("[" + OpStr.GetName(inputDirName) + "][wgAnaHistSummary] target doesn't exist");
		return 1;
	  }   
	  break;
	case 'o':
	  outputXMLDirName = optarg; 
	  break;
	case 'i':
	  outputIMGDirName = optarg;
	  break;
	case 'x':
	  n_chips = atoi(optarg);
	  break;
	case 'y':
	  n_chans = atoi(optarg);
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
	  print_help(argv[0]);
	  break;
	default:
	  print_help(argv[0]);
    }   
  }

  if(inputDirName == "") {
    Log.eWrite("[wgAnaPedestal] No input directory");
    exit(1);
  }

  if(outputXMLDirName == "") outputXMLDirName = inputDirName;

  outputDirName = OpStr.GetName(inputDirName);
  outputIMGDirName = Form("%s/%s",outputIMGDirName.c_str(),outputDirName.c_str());

  Log.Write(" *****  READING DIRECTORY      :" + OpStr.GetName(inputDirName)     + "  *****");
  Log.Write(" *****  OUTPUT XML DIRECTORY   :" + OpStr.GetName(outputXMLDirName) + "  *****");
  Log.Write(" *****  OUTPUT IMAGE DIRECTORY :" + OpStr.GetName(outputIMGDirName) + "  *****");
  
  if(SELECT_Print) MakeDir(outputIMGDirName);
  MakeDir(outputXMLDirName);
  MakeSummaryXmlFile(inputDirName, overwrite);
  wgAnaHistSummary(inputDirName, outputXMLDirName, outputIMGDirName, mode, n_chips, n_chans);
  
  Log.Write("[" + OpStr.GetName(inputDirName) + "][wgAnaHistSummary] Finished");
}

//******************************************************************
void ModeSelect(int mode) {
  if(mode==0) print_help("wgAnaHistSummary");
  if(mode == 1 || mode >= 10) SELECT_Noise     = true;
  if(mode == 2 || mode >= 10) SELECT_Gain      = true;
  if(mode == 3 || mode >= 11) SELECT_Pedestal  = true;
  if(mode == 4 || mode == 12) SELECT_RawCharge = true;
}

//******************************************************************
void wgAnaHistSummary(const string& inputDirName, const string& outputXMLDirName, const string& outputIMGDirName, const int mode, const unsigned n_chips, const unsigned n_chans) {
  ModeSelect(mode);

  wgEditXML *Edit = new wgEditXML();
  string xmlfile("");
  int start_time,stop_time;
  int trig_th[n_chips];
  int gain_th[n_chips];
  int inputDAC[n_chips][n_chans];
  int ampDAC[n_chips][n_chans];
  //int adjDAC[n_chips][n_chans];
  double charge[n_chips][n_chans];
  double rawcharge[n_chips][n_chans][MEMDEPTH][2];
  double e_rawcharge[n_chips][n_chans][MEMDEPTH][2];
  double Noise[n_chips][n_chans][2];

  wgColor wgColor;

  TH1D *h_Pedestal[n_chips];
  TH1D *h_Gain[n_chips];
  TH1D *h_rawcharge[n_chips];
  TH1D *h_Noise[n_chips];

  //*** Define histgram ***//
  if(SELECT_Print){
	for(unsigned ichip = 0; ichip < n_chips; ichip++) {
      if(SELECT_Pedestal){ 
        h_Pedestal[ichip]=new TH1D(Form("h_pedestal_chip%d",ichip),Form("h_pedestal_chip%d",ichip),n_chans*26+10,-5,n_chans*26+5);
        h_Pedestal[ichip]->SetTitle(Form("pedestal chip:%d;ch*26+col;ADC count",ichip));
        h_Pedestal[ichip]->SetMarkerStyle(8);
        h_Pedestal[ichip]->SetMarkerSize(0.3);
        h_Pedestal[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
        h_Pedestal[ichip]->SetStats(0);
      }

      if(SELECT_RawCharge){ 
        h_rawcharge[ichip]=new TH1D(Form("h_rawcharge__chip%d",ichip),Form("h_rawcharge_chip%d",ichip),n_chans*26+10,-5,n_chans*26+5);
        h_rawcharge[ichip]->SetTitle(Form("Gain chip:%d;ch*26+col;ADC count",ichip));
        h_rawcharge[ichip]->SetMarkerStyle(8);
        h_rawcharge[ichip]->SetMarkerSize(0.3);
        h_rawcharge[ichip]->SetStats(0);
        h_rawcharge[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
      }

      if(SELECT_Gain){ 
        h_Gain[ichip]=new TH1D(Form("h_Gain_chip%d",ichip),Form("h_Gain_chip%d",ichip),34,-1,33);
        h_Gain[ichip]->SetTitle(Form("Gain chip:%d;ch;ADC count",ichip));
        h_Gain[ichip]->SetMarkerStyle(8);
        h_Gain[ichip]->SetMarkerSize(0.3);
        h_Gain[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
        h_Gain[ichip]->SetStats(0);
      }

      if(SELECT_Noise){ 
        h_Noise[ichip]=new TH1D(Form("h_Noise_chip%d",ichip),Form("h_Noise_chip%d",ichip),34,-1,33);
        h_Noise[ichip]->SetTitle(Form("Noise chip:%d;ch;Noise Rate[Hz]",ichip));
        h_Noise[ichip]->SetMarkerStyle(8);
        h_Noise[ichip]->SetMarkerSize(0.3);
        h_Noise[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
        h_Noise[ichip]->SetStats(0);
      }
    }
  }

  //*** Read data ***//
  for(unsigned ichip = 0; ichip < n_chips; ichip++) {
    for(unsigned ichan = 0; ichan < n_chans; ichan++) {
      xmlfile=Form("%s/chip%d/ch%d.xml",inputDirName.c_str(),ichip,ichan);
      Edit->Open(xmlfile);
      if(ichan ==0 ) {
        if(ichip == 0) {
          start_time=Edit->GetConfigValue(string("start_time"));
          stop_time=Edit->GetConfigValue(string("stop_time"));
        }
        trig_th[ichip]=Edit->GetConfigValue(string("trigth"));
        gain_th[ichip]=Edit->GetConfigValue(string("gainth"));
      }
      inputDAC[ichip][ichan]=Edit->GetConfigValue(string("inputDAC"));
      ampDAC[ichip][ichan]=Edit->GetConfigValue(string("HG"));
	  // adjDAC[ichip][ichan]=Edit->GetConfigValue(string("trig_adj"));
      if(SELECT_Noise){ 
        Noise[ichip][ichan][0]=Edit->GetChValue(string("NoiseRate"));
        Noise[ichip][ichan][1]=Edit->GetChValue(string("NoiseRate_e"));
      }

      if(SELECT_Gain){ 
        charge[ichip][ichan]=Edit->GetChValue(string("charge_low"));
      }

      for(unsigned icol = 0; icol < MEMDEPTH; icol++){
        if(SELECT_Pedestal){ 
          rawcharge[ichip][ichan][icol][0]=Edit->GetColValue(string("charge_nohit"),icol); 
          e_rawcharge[ichip][ichan][icol][0]=Edit->GetColValue(string("sigma_nohit"),icol); 
        }

        if(SELECT_RawCharge){ 
          rawcharge[ichip][ichan][icol][1]=Edit->GetColValue(string("charge_lowHG"),icol); 
          e_rawcharge[ichip][ichan][icol][1]=Edit->GetColValue(string("sigma_lowHG"),icol); 
        }
      }
      Edit->Close();
    }
  }

  //*** Fill data ***//
  for(unsigned ichip = 0; ichip < n_chips; ichip++) {
    Edit->Open(outputXMLDirName + "/Summary_chip" + to_string(ichip) + ".xml");
    Edit->SUMMARY_SetGlobalConfigValue(string("start_time"),start_time,0);
    Edit->SUMMARY_SetGlobalConfigValue(string("stop_time"),stop_time,0);
    Edit->SUMMARY_SetGlobalConfigValue(string("trigth"),trig_th[ichip],0);
    Edit->SUMMARY_SetGlobalConfigValue(string("gainth"),gain_th[ichip],0);
    for(unsigned ichan = 0; ichan < n_chans; ichan++){
      Edit->SUMMARY_SetChConfigValue(string("inputDAC"),inputDAC[ichip][ichan],ichan,0);
      Edit->SUMMARY_SetChConfigValue(string("ampDAC"),ampDAC[ichip][ichan],ichan,0);
      /*
		name="adjDAC";
		Edit->SUMMARY_SetChConfigValue(string(),adjDAC[ichip][ichan],ichan,0);
      */
      if(SELECT_Gain){
        double Gain = charge[ichip][ichan];  
        Edit->SUMMARY_SetChFitValue(string("Gain"), Gain, ichan, NO_CREATE_NEW_MODE);
        if(SELECT_Print) h_Gain[ichip]->Fill(ichan, Gain);
      }

      if(SELECT_Noise){
        Edit->SUMMARY_SetChFitValue(string("Noise"), Noise[ichip][ichan][0], ichan, NO_CREATE_NEW_MODE); 
        if(SELECT_Print) h_Noise[ichip]->Fill(ichan, Noise[ichip][ichan][0]);
      }

      for(unsigned icol = 0; icol < MEMDEPTH; icol++){
        if(SELECT_Pedestal){
          Edit->SUMMARY_SetChFitValue("ped_" + to_string(icol), rawcharge[ichip][ichan][icol][0], ichan, CREATE_NEW_MODE);
          Edit->SUMMARY_SetChFitValue("eped_" + to_string(icol), e_rawcharge[ichip][ichan][icol][0], ichan, CREATE_NEW_MODE);
          if(SELECT_Print) h_Pedestal[ichip]->Fill(ichan*26+icol, rawcharge[ichip][ichan][icol][0]);
        }

        if(SELECT_RawCharge){
          Edit->SUMMARY_SetChFitValue("raw_" + to_string(icol), rawcharge[ichip][ichan][icol][1], ichan, CREATE_NEW_MODE);
          Edit->SUMMARY_SetChFitValue("eraw_" + to_string(icol), e_rawcharge[ichip][ichan][icol][1], ichan, CREATE_NEW_MODE);
          if(SELECT_Print) h_rawcharge[ichip]->Fill(ichan*26+icol, rawcharge[ichip][ichan][icol][1]);
        }
      }
    }
    Edit->Write();
    Edit->Close();
  }

  if(SELECT_Print){
    TCanvas *c1 = new TCanvas("c1","c1");
    TLegend *l_Noise[n_chips];
    TLegend *l_Gain[n_chips];
    TLegend *l_rawcharge[n_chips];
    TLegend *l_Pedestal[n_chips];
    for(unsigned int i=0;i<n_chips;i++){
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
        c1->DrawFrame(-5,begin_ped,n_chans*26+5,end_ped);
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
void MakeDir(const string& str){
  CheckExist check;
  if(!check.Dir(str)) system(("mkdir -p " + str).c_str());
}

//******************************************************************
void MakeSummaryXmlFile(const string& str, const bool overwrite, const unsigned n_chips, const unsigned n_chans) {
  wgEditXML Edit;
  CheckExist check;
  string outputxmlfile("");
  for(unsigned ichip = 0; ichip < n_chips; ichip++) {
	outputxmlfile = str + "/Summary_chip" + to_string(ichip) + ".xml";
    if( (check.XmlFile(outputxmlfile) && overwrite) || !check.XmlFile(outputxmlfile) )
	  Edit.SUMMARY_Make(outputxmlfile, n_chans);
	else
	  throw wgInvalidFile("File " + str + " already exists and overwrite mode is not set");
  }
}

