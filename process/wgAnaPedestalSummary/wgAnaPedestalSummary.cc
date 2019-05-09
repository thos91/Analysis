#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
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
#include "wgExceptions.h"
#include "wgEditXML.h"
#include "wgColor.h"
#include "wgFit.h"
#include "wgFitConst.h"
#include "wgGetHist.h"

using namespace std;

vector<string> GetIncludeFileName(const string& inputDirName);
void MakeDir(const string& str);
void AnaPedestalSummary(const vector<string>& inputDirName, const string& outputXMLDirName, const string& outputIMGDirName, unsigned n_difs = NDIFS, unsigned n_chips = NCHIPS, unsigned n_chans = NCHANNELS);

void print_help(const char * program_name) {
  cout <<  program_name << " TO DO.\n"
	"  -h         : help\n"
	"  -f (char*) : input directory (mandatory)\n"
	"  -o (char*) : output directory (default: same as input directory)\n"
	"  -n (int)   : number of DIFs (default is 2)\n"
	"  -x (int)   : number of chips per DIF (default is 20)\n"
	"  -y (int)   : number of channels per chip (default is 36)\n";
  exit(0);
}

int main(int argc, char** argv){
  int opt, n_difs = NDIFS, n_chips = NCHIPS, n_chans = NCHANNELS;
  wgConst con;
  con.GetENV();
  string inputDirName("");
  string outputXMLDirName("");
  string outputDirName("");
  string outputIMGDirName=con.IMGDATA_DIRECTORY;
  string logoutputDir=con.LOG_DIRECTORY;

  OperateString OpStr;
  CheckExist check;

  while((opt = getopt(argc,argv, "f:o:h")) !=-1 ){
    switch(opt){
	case 'f':
	  inputDirName=optarg;
	  if(!check.Dir(inputDirName)){ 
		Log.eWrite("[" + OpStr.GetName(inputDirName) + "][wgAnaPedestalSummary] target doesn't exist");
		return 1;
	  }   
	  break;
	case 'o':
	  outputXMLDirName = optarg; 
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

  if(outputXMLDirName==""){
    outputXMLDirName = con.CALIBDATA_DIRECTORY;
  } 

  outputDirName = OpStr.GetName(inputDirName);
  outputIMGDirName = outputIMGDirName + "/" + outputDirName;
  MakeDir(outputIMGDirName);

  Log.Write(" *****  READING DIRECTORY      :" + OpStr.GetName(inputDirName)     + "  *****");
  Log.Write(" *****  OUTPUT XML DIRECTORY   :" + OpStr.GetName(outputXMLDirName) + "  *****");
  Log.Write(" *****  OUTPUT IMAGE DIRECTORY :" + OpStr.GetName(outputIMGDirName) + "  *****");

  // Open the input directory and fill the ReadFile vector with the list of
  // files in it
  vector<string> ReadFile;
  try {
	ReadFile = GetIncludeFileName(inputDirName); 
  } catch (const wgInvalidFile& e) {
	Log.eWrite("[" + OpStr.GetName(inputDirName) + "][wgAnaPedestalSummary] " + e.what());
	exit (0);
  }

  AnaPedestalSummary(ReadFile, outputXMLDirName, outputIMGDirName, n_difs, n_chips, n_chans);

  Log.Write("[" + OpStr.GetName(inputDirName) + "][wgAnaPedestalSummary] Finished");
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


//******************************************************************
void AnaPedestalSummary(const vector<string> &inputFileName, const string& outputXMLDirName, const string& outputIMGDirName, const unsigned n_difs, const unsigned n_chips, const unsigned n_chans) {

  int nFiles = inputFileName.size();
  if(nFiles%2!=0){
    Log.Write("!! ERROR !! : the number of data is not enough to calbration!!");
    return;
  }
  string xmlfile("");
  string name("");
  wgEditXML Edit;
  int ndif=0;
  int npe=0;
  // Gain[][][][0] is the position of the 1 p.e. peak relative to the pedestal
  // Gain[][][][1] is the position of the 2 p.e. peak relative to the pedestal
  double Gain   [n_difs][n_chips][n_chans][2];
  double ped    [n_difs][n_chips][n_chans][MEMDEPTH];
  double ped_ref[n_difs][n_chips][n_chans][MEMDEPTH];

  //*** Read data ***
  for(int iFN = 0; iFN < nFiles; iFN++) {
    for(unsigned ichip = 0; ichip < n_chips; ichip++) {
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

      pos = inputFileName[iFN].find("_pe") + 3;
      if(inputFileName[iFN][pos] == '1') npe = 0;
      else if(inputFileName[iFN][pos] == '2') npe = 1;
	  else throw wgElementNotFound("failed to guess photo electrons from file name: " + inputFileName[iFN]);

	  string xmlfile(inputFileName[iFN] + "/Summary_chip" + to_string(ichip) + ".xml");
      Edit.Open(xmlfile);
      for(unsigned ichan = 0; ichan < n_chans; ichan++) {
		// Position of the npe p.e. peak relative to the pedestal
        Gain[ndif][ichip][ichan][npe] = Edit.SUMMARY_GetChFitValue(string("Gain"), ichan);
        if( npe == 1 ) {
          for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
			// Pedestal position
            ped    [ndif][ichip][ichan][icol] = Edit.SUMMARY_GetChFitValue("ped_" + to_string(icol), ichan);
			// 1 p.e. peak position for high gain preamp
            ped_ref[ndif][ichip][ichan][icol] = Edit.SUMMARY_GetChFitValue("ped_ref_" + to_string(icol), ichan);
          }
        }
      }
      Edit.Close();
    }
  }

  // Define the histograms
  TCanvas *c1 = new TCanvas("c1","c1");
  c1->Divide(n_difs,2);
  TH1F * h_Gain[n_difs];
  TH2F * h_Gain2[n_difs];
  for(unsigned idif = 0; idif < n_difs; idif++) {
	// xbins = 80, xlow = 20, xup = 60 
    h_Gain [idif]= new TH1F(Form("h_Gain_DIF%d", idif+1),Form("h_Gain_DIF%d",idif+1), 80, 20, 60);
	// xbins = 20, xlow = 0, xup = 20, ybins = 40, ylow = 0, yup = 80
    h_Gain2[idif]= new TH2F(Form("h_Gain2_DIF%d",idif+1),Form("h_Gain_DIF%d",idif+1), 20, 0, 20, 40, 0, 80);
  }

  // Fill the histograms
  for(unsigned idif = 0; idif < n_difs; idif++) {
    for(unsigned ichip = 0; ichip < n_chips; ichip++) {
      for(unsigned ichan = 0; ichan < n_chans; ichan++) {
		// Difference between the 2 p.e. peak and the 1 p.e. peak (i.e. the gain value)
        double DIST = Gain[idif][ichip][ichan][1] - Gain[idif][ichip][ichan][0];
        h_Gain[idif] ->Fill(DIST);
		// fill ichip bin with weight DIST
        h_Gain2[idif]->Fill(ichip, DIST);
      }
    }
  }

  // Print the histograms
  for(unsigned idif = 0; idif < n_difs; idif++) {
    c1->cd(idif * 2 + 1);
    c1->GetPad(idif * 2 + 1)->SetLogy(1);
    h_Gain[idif]->Draw();
    c1->cd(idif * 2 + 2);
    c1->GetPad(idif * 2 + 2)->SetLogy(0);
    c1->SetLogz(1);
    h_Gain2[idif]->Draw("colz");
  }
  c1->Print(Form("%s/Gain.png",outputIMGDirName.c_str()));

  TH1F *h1[MEMDEPTH];
  TH1F *h2[MEMDEPTH];
  // xbins = 60, xlow = -50, xup = 10
  TH1F *h3 = new TH1F("h3","h1", 60, -50, 10);
  h3->SetTitle("pedestal shift;adc count;nEntry");
  for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
	// xbins = 300, xlow = 400, xup = 700
    h1[icol] = new TH1F(Form("h1_%d",icol), "h1", 300, 400, 700);
	// xbins = 100, xlow = -50, xup = 50
    h2[icol] = new TH1F(Form("h2_%d",icol), "h2", 100, -50, 50);
    h1[icol]->SetLineColor(kBlue);
    h2[icol]->SetLineColor(kRed); 
    h2[icol]->SetTitle(Form("pedestal shift col%d;adc count;nEntry",icol));
  }


  Edit.Calib_Make(outputXMLDirName + "/pedestal_card.xml");
  Edit.Open(xmlfile);
  for(unsigned idif = 0; idif < n_difs; idif++) {
    for(unsigned ichip = 0; ichip < n_chips; ichip++) {
      for(unsigned ichan = 0; ichan < n_chans; ichan++) {
        Edit.Calib_SetValue(string("pe1"),  idif+1, ichip, ichan, Gain[idif][ichip][ichan][0], NO_CREATE_NEW_MODE);
        Edit.Calib_SetValue(string("pe2"),  idif+1, ichip, ichan, Gain[idif][ichip][ichan][1], NO_CREATE_NEW_MODE);
        Edit.Calib_SetValue(string("Gain"), idif+1, ichip, ichan, Gain[idif][ichip][ichan][1] - Gain[idif][ichip][ichan][0], NO_CREATE_NEW_MODE);
        for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
		  // nominal_pedestal = 1 p.e. peak - 2 * gain
          double nominal_pedestal = ped_ref[idif][ichip][ichan][icol] - 2.0 * (Gain[idif][ichip][ichan][1] - Gain[idif][ichip][ichan][0]);		  
          Edit.Calib_SetValue("ped_" + to_string(icol), idif+1, ichip, ichan, nominal_pedestal, CREATE_NEW_MODE);
          h1[icol]->Fill(nominal_pedestal);

		  // Pedestal when there is no hit
          Edit.Calib_SetValue("ped_nohit" + to_string(icol), idif+1, ichip, ichan, ped[idif][ichip][ichan][icol], CREATE_NEW_MODE);
		  // Difference between the nominal_pedestal and the pedestal_nohit 
          h2[icol]->Fill(nominal_pedestal - ped[idif][ichip][ichan][icol]);
          h3->Fill(nominal_pedestal - ped[idif][ichip][ichan][icol]);
        }
      }
    }
  }

  Edit.Write();
  Edit.Close();

  delete c1;
  c1=new TCanvas("c1","c1");
  c1->Divide(4,4);
  for(unsigned icol = 0; icol < MEMDEPTH; icol++){
    c1->cd(icol+1);
    h1[icol]->Draw("");
    h2[icol]->Draw("");
  }
  c1->Print(Form("%s/pedestal_shift.png",outputIMGDirName.c_str()));
  delete c1;
  c1=new TCanvas("c1","c1");
  h3->Draw("");
  c1->Print(Form("%s/pedestal_shift_all.png",outputIMGDirName.c_str()));
}

//******************************************************************
void MakeDir(const string& str){
  CheckExist check;
  if(!check.Dir(str)) system(("mkdir -p " + str).c_str());
}
