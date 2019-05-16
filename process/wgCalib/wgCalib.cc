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

#define TWIN_PEAKS      1
#define LONELY_MOUNTAIN 0

#define ONE_PE   0
#define TWO_PE   1
#define THREE_PE 2

using namespace std;

vector<string> GetIncludeFileName(const string& inputDirName);
void MakeDir(const string& outputDir, unsigned n_chips = NCHIPS);
void Calib(const vector<string>& inputDirName, const string& outputXMLDirName, const string& outputIMGDirName, int mode, int pe, unsigned n_difs = NDIFS, unsigned n_chips = NCHIPS, unsigned n_chans = NCHANNELS);

void print_help(const char * program_name) {
  cout <<  program_name << " TO-DO\n"
	"  -h         : help\n"
	"  -f (char*) : input directory (mandatory)\n"
	"  -o (char*) : output directory (default: same as input directory)\n"
	"  -i (char*) : output image directory (default: image directory)\n"
	"  -n (int)   : number of DIFs (default is 2)\n"
	"  -x (int)   : number of chips per DIF (default is 20)\n"
	"  -y (int)   : number of channels per chip (default is 36)\n"
	"  -m (int)   : 0:use only 2pe peak. 1:use 1pe and 2pe peaks (default is 0)\n"
	"  -p (int)   : photo-electrons (default is 2)\n";
  exit(0);
}

int main(int argc, char** argv){
  int opt;
  int mode=0;
  int pe=2;
  unsigned n_difs = NDIFS, n_chips = NCHIPS, n_chans = NCHANNELS;
  wgConst con;
  con.GetENV();
  string inputDirName("");
  string outputXMLDirName("");
  string outputIMGDirName=con.IMGDATA_DIRECTORY;
  string logoutputDir=con.LOG_DIRECTORY;

  OperateString OpStr;
  CheckExist check;

  Log.Write("Start calibration...");

  while((opt = getopt(argc,argv, "f:o:i:m:p:h")) !=-1 ){
    switch(opt){
	case 'f':
	  inputDirName=optarg;
	  if(!check.Dir(inputDirName)){ 
		Log.eWrite("[" + OpStr.GetName(inputDirName) + "][wgPreCalib] target doesn't exist");
		return 1;
	  }   
	  break;
	case 'o':
	  outputXMLDirName = optarg; 
	  break;
	case 'i':
	  outputIMGDirName = optarg; 
	  break;
	case 'n':
	  n_difs = atoi(optarg);
	  break;
	case 'x':
	  n_chips = atoi(optarg);
	  break;
	case 'y':
	  n_chans = atoi(optarg);
	  break;
	case 'p':
	  pe = atoi(optarg); 
	  break;
	case 'm':
	  mode = atoi(optarg); 
	  break;
	case 'h':
	  print_help(argv[0]);
	  break;
	  exit(0);
	default:
	  print_help(argv[0]);
    }   
  }

  if(inputDirName == "") {
    Log.eWrite("[wgPreCalib] No input directory");
    exit(1);
  }

  if(outputXMLDirName==""){
    outputXMLDirName = OpStr.GetPath(inputDirName);
    outputXMLDirName = OpStr.GetName(outputXMLDirName);
    outputXMLDirName = con.CALIBDATA_DIRECTORY + "/" + outputXMLDirName;
    MakeDir(outputXMLDirName);
  } 

  outputIMGDirName = OpStr.GetPath(inputDirName);
  outputIMGDirName = OpStr.GetName(outputIMGDirName);
  outputIMGDirName = con.CALIBDATA_DIRECTORY + "/" + outputIMGDirName + "/image";
    
  MakeDir(outputIMGDirName);
  for(unsigned idif = 0; idif < n_difs; idif++)
    MakeDir(outputIMGDirName + "/dif" + to_string(idif+1));

  Log.Write(" *****  READING DIRECTORY      : " + OpStr.GetName(inputDirName)     + "  *****");
  Log.Write(" *****  OUTPUT XML DIRECTORY   : " + OpStr.GetName(outputXMLDirName) + "  *****");
  Log.Write(" *****  OUTPUT IMAGE DIRECTORY : " + OpStr.GetName(outputIMGDirName) + "  *****");

  vector<string> ReadFile = GetIncludeFileName(inputDirName); 

  Calib(ReadFile, outputXMLDirName,	outputIMGDirName, mode,	pe, n_difs, n_chips, n_chans);
}

//******************************************************************
void Calib(const vector<string> &inputFileName, const string& outputXMLDirName, const string& outputIMGDirName, const int mode, const int pe, const unsigned n_difs, const unsigned n_chips, const unsigned n_chans) {

  size_t nFiles = inputFileName.size();
  if(nFiles % 2 != 0) {
    throw invalid_argument("the number of files is not enough for calibration");
  }
  wgEditXML Edit;
  int ndif=0;
  int npe=0;
  double Gain[n_difs][n_chips][n_chans][3];

  //*** Read data ***
  for(unsigned iFN = 0; iFN < nFiles; iFN++) {
    for(unsigned ichip = 0; ichip < n_chips; ichip++){

	  // ===== DIF number ===== //
	  
      int pos = inputFileName[iFN].find("dif_1_1_") + 8;
	  bool found = false;
	  for (unsigned idif = 0; idif < n_difs; idif++) {
		// This will not work for DIF >= 10
		if(inputFileName[iFN][pos] == to_string(idif + 1)) ndif = idif;
		found = true;
		break;
	  }
	  if (found == false)
		throw wgElementNotFound("failed to guess DIF number from file name: " + inputFileName[iFN]);

	  // ===== P.E. number ===== //

      pos = inputFileName[iFN].find("pe") + 2;
      if(inputFileName[iFN][pos] == '1')    npe = 0;
      else if(inputFileName[iFN][pos]=='2') npe = 1;
      else if(inputFileName[iFN][pos]=='3') npe = 2;

      Edit.Open(inputFileName[iFN] + "/Summary_chip" + to_string(ichip) + ".xml");
      for(unsigned ichan = 0; ichan < n_chans; ichan++)
        Gain[ndif][ichip][ichan][npe] = Edit.SUMMARY_GetChFitValue("Gain", ichan);
      Edit.Close();
    }
  }

  TCanvas *c1 = new TCanvas("c1","c1");
  c1->Divide(2,2);
  TH1F * h_Gain[n_difs];    // one dimensional (x: Gain)
  TH2F * h_Gain2[n_difs];   // two dimensional (x: chip, y: Gain)
  for(unsigned idif = 0; idif < n_difs; idif++) {
    h_Gain[idif]= new TH1F(Form("h_Gain_DIF%d",idif+1),Form("h_Gain_DIF%d",idif+1),80,20,60);
    h_Gain[idif]->SetTitle(Form("Gain DIF%d;Gain;nEntry",idif+1));
    h_Gain2[idif]= new TH2F(Form("h_Gain2_DIF%d",idif+1),Form("h_Gain_DIF%d",idif+1),20,0,20,40,0,80);
    h_Gain2[idif]->SetTitle(Form("Gain DIF%d;chip;Gain",idif+1));
    h_Gain2[idif]->SetStats(0);
  }
  
  // ===== DIST is the actual value of the Gain ===== //

  for(unsigned idif = 0; idif < n_difs; idif++) {
    for(unsigned ichip = 0; ichip < n_chips; ichip++){
      for(unsigned ichan = 0; ichan < n_chans; ichan++){
        double DIST;
        if(mode == LONELY_MOUNTAIN) {
          if(pe == 2) DIST = 0.5 * Gain[idif][ichip][ichan][TWO_PE];
          else {
			// TO-DO Still need to understand this formula
			DIST = Gain[idif][ichip][ichan][pe-1] / pe * 1.05;
			stringstream s;
			s << "[wgPreCalib] Gain[" << idif << "][" << ichip << "][" << ichan << "][" << pe-1 << "] / " << pe << " * 1.05 = " << DIST;
			Log.Write(s.str());
		  }
        }
        else if(mode == TWIN_PEAKS) DIST = Gain[idif][ichip][ichan][TWO_PE] - Gain[idif][ichip][ichan][ONE_PE];

	  // ===== Fill Gain and Gain2 histograms ===== //
		
        h_Gain[idif]->Fill(DIST);
        h_Gain2[idif]->Fill(ichip, DIST);
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

  // ===== Fill the calib_result.xml file with the gain and 1pe and 2pe peaks ===== //
  // ===== Write the bad channels (gain < 36.0 || gain > 44.0) into the bad_channel.txt file ===== //

  Log.eWrite("      ~~~   bad channels   ~~~"); 
  string xmlfile;
  if(pe == 2)
    xmlfile = outputXMLDirName + "/calib_result.xml";
  else
	xmlfile = outputXMLDirName + "/calib_result_" + to_string(pe) + ".xml";
  Edit.Calib_Make(xmlfile);
  Edit.Open(xmlfile);
  string badchfilename;
  if(pe==2) badchfilename = outputXMLDirName + "/bad_channel.txt";
  else      badchfilename = outputXMLDirName + "/bad_channel_pe" + to_string(pe) + ".txt";
  ofstream badch(badchfilename.c_str());
  badch << "#dif chip chipch view pln ch grid" << endl;
  int view, pln, chan, grid;
  wgChannelMap mapping;
  for(unsigned idif = 0; idif < n_difs; idif++) {
    for(unsigned ichip = 0; ichip < n_chips; ichip++) {
      for(unsigned ichan = 0; ichan < n_chans; ichan++) {
        if(mode == TWIN_PEAKS) {
          Edit.Calib_SetValue(string("pe1"), idif+1, ichip, ichan, Gain[idif][ichip][ichan][ONE_PE], NO_CREATE_NEW_MODE);
          Edit.Calib_SetValue(string("pe2"), idif+1, ichip, ichan, Gain[idif][ichip][ichan][TWO_PE], NO_CREATE_NEW_MODE);
        }
        double gain = 0;
        if(mode == LONELY_MOUNTAIN){
          if(pe==2) gain=Gain[idif][ichip][ichan][TWO_PE] / pe;
          else      gain=Gain[idif][ichip][ichan][pe-1] / pe * 1.05;
        }
        else if(mode == TWIN_PEAKS) gain = Gain[idif][ichip][ichan][TWO_PE]-Gain[idif][ichip][ichan][ONE_PE];
        Edit.Calib_SetValue(string("Gain"), idif+1, ichip, ichan, gain, NO_CREATE_NEW_MODE);
        if(gain < 36.0 || gain > 44.0){
		  Log.eWrite("    (dif,chip,chipch)=(" + to_string(idif+1) + ", " + to_string(ichip) + ", " + to_string(ichan) + ")");
		  mapping.GetViewPlnCh(idif, ichip, ichan, view, pln, chan, grid);
		  badch 
			<< idif << " " << ichip << " " << ichan << " " 
			<< view << " " << pln   << " " << chan << " " << grid
			<< endl;
        }
      }
    }
  }
  badch.close();

  Edit.Write();
  Edit.Close();
}

//******************************************************************
void MakeDir(const string& outputDir, const unsigned n_chips) {
  system( Form("mkdir -p %s", outputDir.c_str()) );
  for(unsigned i = 0; i < n_chips; i++)
    system(Form("mkdir -p %s", Form("%s/chip%u", outputDir.c_str(), i)));
}

//******************************************************************
vector<string> GetIncludeFileName(const string& inputDirName){
  OperateString OpStr;
  DIR *dp;
  struct dirent *entry;
  vector<string> openxmlfile;

  // Open the input directory
  dp = opendir(inputDirName.c_str());
  if(dp == NULL)
	throw wgInvalidFile("opendir: failed to open directory");

  // Fill the openxmlfile vector of strings with the path of all the files and
  // directories contained inside the input directory
  while( (entry = readdir(dp)) != NULL ) {
	// Ignore hidden files and directories
    if( (entry->d_name[0]) != '.' )
	  openxmlfile.push_back( inputDirName + "/" + string(entry->d_name) );
  }
  closedir(dp);
  return openxmlfile;
} 
