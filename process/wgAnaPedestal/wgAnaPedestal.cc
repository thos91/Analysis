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
#include "wgExceptions.h"
#include "wgEditXML.h"
#include "wgColor.h"
#include "wgFit.h"
#include "wgFitConst.h"
#include "wgGetHist.h"

using namespace std;
// This little function tries to guess the threshold level (0.5, 1.5 or 2.5)
// given the value of the noise rate.
double NoiseToPe(double);
void MakeDir(const string& str);
void MakeSummaryXmlFile(const string& str, bool overwrite, unsigned n_chips = NCHIPS, unsigned n_chans = NCHANNELS);
void AnaPedestal(const string& inputDirName, const string& outputXMLDirName, const string& outputIMGDirName, unsigned n_chips = NCHIPS, unsigned n_chans = NCHANNELS);
void print_help(const char * program_name) {
  cout <<  program_name << " is used to summarize the information contained in the\n"
	"xml files created by the wgAnaHist program.\n"
	"  -h         : help\n"
	"  -f (char*) : input directory (mandatory)\n"
	"  -o (char*) : output directory (default: same as input directory)\n"
	"  -r         : overwrite mode\n";
  exit(0);
}
  
//******************************************************************
int main(int argc, char** argv){

  int opt;
  bool overwrite = false;
  wgConst con;
  con.GetENV();
  string inputDirName("");
  string configFileName("");
  string outputIMGDirName=con.IMGDATA_DIRECTORY;
  string outputXMLDirName("");
  string outputDirName("");
  string logoutputDir=con.LOG_DIRECTORY;

  OperateString OpStr;
  CheckExist check;

  while((opt = getopt(argc,argv, "f:o:hr")) !=-1 ){
    switch(opt){
      case 'f':
        inputDirName=optarg;
        if(!check.Dir(inputDirName)){ 
          cout<<"!!Error!! "<< inputDirName.c_str() << "doesn't exist!!";
          Log.eWrite("[" + OpStr.GetName(inputDirName) + "][wgAnaHistSummary] target doesn't exist");
          return 1;
        }   
        break;

      case 'o':
        outputXMLDirName = optarg; 
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
  outputIMGDirName = outputIMGDirName + "/" + outputDirName;

  Log.Write(" *****  READING DIRECTORY      :" + OpStr.GetName(inputDirName)     + "  *****");
  Log.Write(" *****  OUTPUT XML DIRECTORY   :" + OpStr.GetName(outputXMLDirName) + "  *****");
  Log.Write(" *****  OUTPUT IMAGE DIRECTORY :" + OpStr.GetName(outputIMGDirName) + "  *****");

  MakeDir(outputIMGDirName);
  MakeDir(outputXMLDirName);
  MakeSummaryXmlFile(inputDirName, overwrite);
  AnaPedestal(inputDirName, outputXMLDirName, outputIMGDirName);

  Log.Write("[" + OpStr.GetName(inputDirName) + "][wgAnaPedestal] wgAnaPedestal finished");
}

//******************************************************************
void AnaPedestal(const string& inputDirName, const string& outputXMLDirName, const string& outputIMGDirName, const unsigned n_chips, const unsigned n_chans) {
  wgEditXML Edit;
  string xmlfile("");
  int trig_th    [n_chips];
  int gain_th    [n_chips];
  int inputDAC   [n_chips][n_chans];
  int ampDAC     [n_chips][n_chans];
  int adjDAC     [n_chips][n_chans];
  double charge  [n_chips][n_chans][MEMDEPTH][2];
  double gain    [n_chips][n_chans][MEMDEPTH];
  double Noise   [n_chips][n_chans][2];
  double pe_level[n_chips][n_chans];

  wgColor wgColor;

  TH1F *h_Pedestal[n_chips];
  TH1F *h_Gain    [n_chips];
  TH1F *h_Gain_all[n_chips];
  TH1F *h_Noise   [n_chips];

  //*** Define histgram ***//
  for(unsigned ichip = 0; ichip < n_chips; ichip++) {
    h_Pedestal[ichip]=new TH1F(Form("h_pedestal_chip%d", ichip), Form("h_pedestal_chip%d", ichip), n_chans * 26 + 10, -5, n_chans * 26 + 5);
    h_Pedestal[ichip]->SetTitle(Form("pedestal chip:%d;ch*26+col;ADC count",ichip));
    h_Pedestal[ichip]->SetMarkerStyle(8);
    h_Pedestal[ichip]->SetMarkerSize(0.3);
    h_Pedestal[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
    h_Pedestal[ichip]->SetStats(0);
    
    h_Gain_all[ichip]=new TH1F(Form("h_Gain_all_chip%d", ichip), Form("h_Gain_all_chip%d", ichip), n_chans * 26 + 10, -5, 32 * 26 + 5);
    h_Gain_all[ichip]->SetTitle(Form("Gain chip:%d;ch*26+col;ADC count",ichip));
    h_Gain_all[ichip]->SetMarkerStyle(8);
    h_Gain_all[ichip]->SetMarkerSize(0.3);
    h_Gain_all[ichip]->SetStats(0);
    h_Gain_all[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);

    h_Gain[ichip]=new TH1F(Form("h_Gain_chip%d", ichip), Form("h_Gain_chip%d", ichip), 34, -1, 33);
    h_Gain[ichip]->SetTitle(Form("Gain chip:%d;ch;ADC count", ichip));
    h_Gain[ichip]->SetMarkerStyle(8);
    h_Gain[ichip]->SetMarkerSize(0.3);
    h_Gain[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
    h_Gain[ichip]->SetStats(0);
    
    h_Noise[ichip]=new TH1F(Form("h_Noise_chip%d", ichip) ,Form("h_Noise_chip%d", ichip), 34, -1, 33);
    h_Noise[ichip]->SetTitle(Form("Noise chip:%d;ch;Noise Rate[Hz]",ichip));
    h_Noise[ichip]->SetMarkerStyle(8);
    h_Noise[ichip]->SetMarkerSize(0.3);
    h_Noise[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
    h_Noise[ichip]->SetStats(0);
  }

  //*** Read data ***//

  //These are the chip%d/ch%d.xml files that were produced by the wgAnaHist
  // program. You need to run that program in the appropriate mode to generate
  // these xml files
  for(unsigned ichip = 0; ichip < n_chips; ichip++) {
    for(unsigned ichan = 0; ichan < n_chans; ichan++) {
		Edit.Open(inputDirName + "/chip" + to_string(ichip) + "/ch" + to_string(ichan) + ".xml");
      if(ichan == 0) {
        trig_th[ichip] = Edit.GetConfigValue(string("trigth"));
        gain_th[ichip] = Edit.GetConfigValue(string("gainth"));
      }
	  // Fill the arrays with the data read from the xml files
      inputDAC[ichip][ichan]    = Edit.GetConfigValue (string("inputDAC"));
      ampDAC  [ichip][ichan]    = Edit.GetConfigValue (string("HG"));
      adjDAC  [ichip][ichan]    = Edit.GetConfigValue (string("trig_adj"));
      Noise   [ichip][ichan][0] = Edit.GetChValue     (string("NoiseRate"));
      Noise   [ichip][ichan][1] = Edit.GetChValue     (string("NoiseRate_e"));
	  // Guess the threshold value (in p.e.) given the dark noise rate
      pe_level[ichip][ichan]    = NoiseToPe(Noise[ichip][ichan][0]);

      for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
		// Pedestal position
        charge[ichip][ichan][icol][0] = Edit.GetColValue(string("charge_nohit"), icol);
		// 1 p.e. position for high gain preamp
        charge[ichip][ichan][icol][1] = Edit.GetColValue(string("charge_lowHG"), icol);
        gain[ichip][ichan][icol]      = charge[ichip][ichan][icol][1] - charge[ichip][ichan][icol][0];
      }
      Edit.Close();
    }
  }
  
  //*** Fill data ***//
  // Fill the Summary_chip%d.xml file with the data
  // Basically the only new information that is added to the files is the pe_level and the gain
  for(unsigned ichip = 0; ichip < n_chips; ichip++) {
    Edit.Open( outputXMLDirName + "/Summary_chip" + to_string(ichip) + ".xml" );
    Edit.SUMMARY_SetGlobalConfigValue(string("tr_th"), trig_th[ichip], 0);
    Edit.SUMMARY_SetGlobalConfigValue(string("gs_th"), gain_th[ichip], 0);
    for(unsigned ichan = 0; ichan < n_chans; ichan++){
      Edit.SUMMARY_SetChConfigValue(string("inputDAC"), inputDAC[ichip][ichan], ichan, NO_CREATE_NEW_MODE);
      Edit.SUMMARY_SetChConfigValue(string("ampDAC"),   ampDAC[ichip][ichan],   ichan, NO_CREATE_NEW_MODE);
      Edit.SUMMARY_SetChConfigValue(string("adjDAC"),   adjDAC[ichip][ichan],   ichan, NO_CREATE_NEW_MODE);
      Edit.SUMMARY_SetChFitValue(string("Gain"),        gain[ichip][ichan][0],  ichan, NO_CREATE_NEW_MODE);
      Edit.SUMMARY_SetChFitValue(string("Noise"),       Noise[ichip][ichan][0], ichan, NO_CREATE_NEW_MODE);
      Edit.SUMMARY_SetChFitValue(string("pe_level"),    pe_level[ichip][ichan], ichan, CREATE_NEW_MODE);

      h_Noise[ichip]->Fill(ichan,Noise[ichip][ichan][0]);
      h_Gain[ichip]->Fill(ichan,gain[ichip][ichan][0]);
      for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
		// Pedestal position
        Edit.SUMMARY_SetChFitValue("ped_" + to_string(icol),     charge[ichip][ichan][icol][0], ichan, NO_CREATE_NEW_MODE);
		// 1 p.e. peak position for high gain preamp
        Edit.SUMMARY_SetChFitValue("ped_ref_" + to_string(icol), charge[ichip][ichan][icol][1], ichan, CREATE_NEW_MODE);
        h_Pedestal[ichip]->Fill(ichan * 26 + icol, charge[ichip][ichan][icol][0]);
        h_Gain_all[ichip]->Fill(ichan * 26 + icol, gain  [ichip][ichan][icol]);
      }
    }
    Edit.Write();
    Edit.Close();
  }

  //*** Plot data ***//

  TCanvas *c1 = new TCanvas("c1","c1");
  TLegend *l_Noise[n_chips];
  TLegend *l_Gain[n_chips];
  TLegend *l_Gain_all[n_chips];
  TLegend *l_Pedestal[n_chips];
  for(unsigned int i=0;i<n_chips;i++){
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

//******************************************************************
double NoiseToPe(const double noise){
  if      (noise >  u_limit_1pe)                        return 0.5;
  else if (noise >= l_limit_1pe && noise < u_limit_1pe) return 1.0;
  else if (noise >= u_limit_2pe && noise < l_limit_1pe) return 1.5;
  else if (noise >= l_limit_2pe && noise < u_limit_2pe) return 2.0;
  else if (noise >= u_limit_3pe && noise < l_limit_2pe) return 2.5;
  else if (noise >= l_limit_3pe && noise < u_limit_3pe) return 3.0;
  else if (noise <  l_limit_3pe && noise > 0)           return 3.5;
  else                                                  return 0; 
}
