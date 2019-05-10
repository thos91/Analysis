#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>

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

#define TWIN_PEAKS      1
#define LONELY_MOUNTAIN 0

#define ONE_PE 0
#define TWO_PE 1

using namespace std;

vector<string> GetIncludeFileName(const string& inputDirName);
void MakeDir(const string& outputDir, unsigned n_chips = NCHIPS);
double cal_mean(vector<double>);
void PreCalib(const vector<string>& inputDirName, const string& outputXMLDirName, const string& outputIMGDirName, int mode, unsigned n_difs = NDIFS, unsigned n_chips = NCHIPS, unsigned n_chans = NCHANNELS);

void print_help(const char * program_name) {
  cout <<  program_name << "draws the inputDAC vs Gain graph and creates the calibration_card.xml file.\n"
	"  -h         : help\n"
	"  -f (char*) : input directory (mandatory)\n"
	"  -o (char*) : output directory (default: same as input directory)\n"
	"  -i (char*) : output image directory (default: image directory)\n"
	"  -n (int)   : number of DIFs (default is 2)\n"
	"  -x (int)   : number of chips per DIF (default is 20)\n"
	"  -y (int)   : number of channels per chip (default is 36)\n";
	"  -m (int)   : 0:use only 2pe peak. 1:use 1pe and 2pe peaks (default:0)\n";
  exit(0);
}

template < typename T>
pair<bool, int > findInVector(const vector<T>& vecOfElements, const T& element)
{
  pair<bool, int > result;
  // Find given element in vector
  auto it = find(vecOfElements.begin(), vecOfElements.end(), element);
  if (it != vecOfElements.end()) {
	result.second = distance(vecOfElements.begin(), it);
	result.first = true;
  }
  else {
	result.first = false;
	result.second = -1;
  }
  return result;
}

int main(int argc, char** argv){
  int opt;
  int mode = 0;
  int n_difs = NDIFS, n_chips = NCHIPS, n_chans = NCHANNELS;
  wgConst con;
  con.GetENV();
  string inputDirName("");
  string outputXMLDirName("");
  string outputIMGDirName = con.IMGDATA_DIRECTORY;
  string logoutputDir = con.LOG_DIRECTORY;

  OperateString OpStr;
  CheckExist check;

  while((opt = getopt(argc,argv, "f:o:i:n:x:y:m:h")) !=-1 ){
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
	case 'm':
	  mode = atoi(optarg); 
	  break;
	case 'h':
	  print_help(argv[0]);
	  break;
	default:
	  print_help(argv[0]);
    }   
  }

  if(inputDirName == "") {
    Log.eWrite("[wgPreCalib] No input directory");
    exit(1);
  }

  if(outputXMLDirName == "") {
    outputXMLDirName = con.CALIBDATA_DIRECTORY + OpStr.GetName(inputDirName);
    MakeDir(outputXMLDirName);
  } 

  MakeDir(outputIMGDirName);
  for(int idif = 0; idif < n_difs; idif++)
    MakeDir( outputIMGDirName + "/dif" + to_string(idif + 1) );

  Log.Write(" *****  READING DIRECTORY      :" + OpStr.GetName(inputDirName)     + "  *****");
  Log.Write(" *****  OUTPUT XML DIRECTORY   :" + OpStr.GetName(outputXMLDirName) + "  *****");
  Log.Write(" *****  OUTPUT IMAGE DIRECTORY :" + OpStr.GetName(outputIMGDirName) + "  *****");

  vector<string> ReadFile = GetIncludeFileName(inputDirName); 

  PreCalib(ReadFile, outputXMLDirName, outputIMGDirName, mode, n_difs, n_chips, n_chans);

  Log.Write("[" + OpStr.GetName(inputDirName) + "][wgPreCalib] Finished");
}

//******************************************************************
void PreCalib(const vector<string> &inputFileName, const string& outputXMLDirName, const string& outputIMGDirName, const int mode, const unsigned n_difs, const unsigned n_chips, const unsigned n_chans) {

  OperateString OpStr;
  size_t nFiles = inputFileName.size();
  int inputDAC[nFiles];

  // Save all the inputDAC values in the inputDAC[] array
  wgEditXML Edit;
  for(unsigned iFN = 0; iFN < nFiles; iFN++) {
    Edit.Open(inputFileName[iFN] + "/Summary_chip0.xml");
    inputDAC[iFN] = Edit.SUMMARY_GetChConfigValue(string("inputDAC"), NO_CREATE_NEW_MODE);
    Edit.Close();
  }

  if ( mode != TWIN_PEAKS || mode != LONELY_MOUNTAIN )
	throw invalid_argument("[wgPreCalib] unknown mode : " + to_string(mode));

  // Remove duplicates and sort the list of inputDACs
  vector<int> list_inputDAC;
  bool add_list_inputDAC = false;
  list_inputDAC.push_back(inputDAC[0]);
  for(unsigned iFN = 1; iFN < nFiles; iFN++) {
    add_list_inputDAC = true;
    for(unsigned l = 0; l < list_inputDAC.size(); l++) {
      if(list_inputDAC[l] == inputDAC[iFN]) {
        add_list_inputDAC = false;
      }
    }
    if(add_list_inputDAC)list_inputDAC.push_back(inputDAC[iFN]);
  }
  sort(list_inputDAC.begin(), list_inputDAC.end());

  unsigned size_inputDAC = list_inputDAC.size();

  int ndif = 0, npe = 0;
  double Gain[n_difs][n_chips][n_chans][size_inputDAC][2];
  double Pedestal[n_difs][n_chips][n_chans][size_inputDAC][2][MEMDEPTH];

  // ======================================================//
  //              Read Gain and Pedestal                   //
  // ======================================================//
  
  for(unsigned iFN = 0; iFN < nFiles; iFN++) {
    for(unsigned ichip = 0; ichip < n_chips; ichip++) {

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
	  
      pos = inputFileName[iFN].find("_pe") + 3;
      if(inputFileName[iFN][pos] == '1') npe = 0;
      else if(inputFileName[iFN][pos] == '2') npe = 1;
	  else throw wgElementNotFound("failed to guess photo electrons from file name: " + inputFileName[iFN]);

      Edit.Open(inputFileName[iFN] + "/Summary_chip" + to_string(ichip) + ".xml");
	  // Read the value of the inputDAC from the Summary_chip%d.xml file
      inputDAC[iFN] = Edit.SUMMARY_GetChConfigValue(string("inputDAC"), NO_CREATE_NEW_MODE);
	  
	  pair<bool, int> iDAC = findInVector(list_inputDAC, inputDAC[iFN]);
	  if (iDAC.first == false) {
		Log.eWrite("[" + OpStr.GetName(inputFileName[iFN]) + "][wgPreCalib] inputDAC value (" + to_string(inputDAC[iFN]) + ") not found in the list");
		continue;
	  }

	  // ===== read Gain and Pedestal ===== //
	  
      for(unsigned ichan = 0; ichan < n_chans; ichan++) {
        // This gain should be twice the true gain.
        Gain[ndif][ichip][ichan][iDAC.second][npe] = Edit.SUMMARY_GetChFitValue(string("Gain"), ichan);
        if(mode == TWIN_PEAKS){
		  // If we use both 1pe and 2ped, get the pedestal position
          Edit.SUMMARY_GetPedFitValue(Pedestal[ndif][ichip][ichan][iDAC.second][npe], ichan);
        }
      }
      Edit.Close();
    }
  }

  // ======================================================//
  //            Draw the inputDAC vs Gain graph            //
  // ======================================================//

  double slope[n_difs][n_chips][n_chans];
  double inter[n_difs][n_chips][n_chans];

  for(unsigned idif = 0; idif < n_difs; idif++) {
    for(unsigned ichip = 0; ichip < n_chips; ichip++) {
      TMultiGraph * mg = new TMultiGraph();
      TGraphErrors * g_Dist[n_chans]; 
      TGraph * g_Gain0[n_chans];
      TGraph * g_Gain1[n_chans];
      TGraph * g_ped[MEMDEPTH];

	  // ===== plot the Pedestal ===== //
	  
      TCanvas *c1 = new TCanvas("c1","c1");
      if( mode == TWIN_PEAKS ) {
        c1->Divide(4,4);
        for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
          double x_ch [n_chans * size_inputDAC];
          double y_ped[n_chans * size_inputDAC];
          for(unsigned int l = 0; l < size_inputDAC; l++) {
            for(unsigned ichan = 0; ichan < n_chans; ichan++) {
              x_ch [ichan + n_chans * l] = ichan;
              y_ped[ichan + n_chans * l] = Pedestal[idif][ichip][ichan][l][1][icol];
            }
          }

          g_ped[icol] = new TGraph(n_chans * 2 * size_inputDAC, x_ch, y_ped);
          g_ped[icol]->SetMarkerColor(880+icol);
          g_ped[icol]->SetMarkerSize(0.5);
          g_ped[icol]->SetMaximum(700);
          g_ped[icol]->SetMinimum(350);
          g_ped[icol]->SetTitle(Form("chip%d col%d;ch;ped",ichip,icol));
          c1->cd(icol+1);
          c1->SetGrid();
          g_ped[icol]->Draw("ap");
        }
        c1->Print(Form("%s/dif%d/ped_chip%d.png",outputIMGDirName.c_str(),idif+1,ichip));
        delete c1;
        for(unsigned icol = 0; icol < MEMDEPTH; icol++) delete g_ped[icol];
      }

	  // ===== calculate the gain average over all the channels  ===== //
	  
      double mean_Dist[size_inputDAC];
	  for(unsigned l = 0; l < size_inputDAC; l++) {
		for(unsigned ichan = 0; ichan < n_chans; ichan++) {
          if(mode == LONELY_MOUNTAIN)
            mean_Dist[l] += Gain[idif][ichip][ichan][l][TWO_PE];
		  else if(mode == TWIN_PEAKS)
            mean_Dist[l] += (Gain[idif][ichip][ichan][l][TWO_PE] - Gain[idif][ichip][ichan][l][ONE_PE]);
        }
		if(mode == LONELY_MOUNTAIN)	mean_Dist[l] /= (double) (2 * n_chans);
		else if(mode == TWIN_PEAKS) mean_Dist[l] /= (double) n_chans;
      }

      for(unsigned ichan = 0; ichan < n_chans; ichan++) {
        double x_inputDAC[size_inputDAC];
        double ex_inputDAC[size_inputDAC];
        double y_Dist[size_inputDAC];
        double ey_Dist[size_inputDAC];
        double y_Gain0[size_inputDAC];
        double y_Gain1[size_inputDAC];
        for(unsigned l = 0; l < size_inputDAC; l++) {
          x_inputDAC[l] = list_inputDAC[l];
          ex_inputDAC[l] = 1.;
          if(mode == LONELY_MOUNTAIN) {
			// If Gain lies too far from the mean_Dist, set ey_Dist as 0.5 
            if( (mean_Dist[l] + 6. > 0.5 * Gain[idif][ichip][ichan][l][TWO_PE]) && (mean_Dist[l] - 6. < 0.5 * Gain[idif][ichip][ichan][l][TWO_PE]) ) {
              y_Dist[l] = 0.5 * Gain[idif][ichip][ichan][l][TWO_PE];
              ey_Dist[l] = 0.5;
            }
			else{
              y_Dist[l] = 0.5* Gain[idif][ichip][ichan][l][TWO_PE];
              ey_Dist[l] = 20;
            }
          }
		  else if(mode == TWIN_PEAKS) {
            if( (mean_Dist[l] + 6. > Gain[idif][ichip][ichan][l][TWO_PE] - Gain[idif][ichip][ichan][l][ONE_PE]) && (mean_Dist[l] - 6. < Gain[idif][ichip][ichan][l][TWO_PE] - Gain[idif][ichip][ichan][l][ONE_PE]) ) {
              y_Dist[l] = Gain[idif][ichip][ichan][l][TWO_PE] - Gain[idif][ichip][ichan][l][ONE_PE];
              ey_Dist[l] = 0.5;
            }
			else{
              y_Dist[l] = Gain[idif][ichip][ichan][l][TWO_PE] - Gain[idif][ichip][ichan][l][ONE_PE];
              ey_Dist[l] = 20.;
            }
            y_Gain0[l]=Gain[idif][ichip][ichan][l][ONE_PE];    
            y_Gain1[l]=Gain[idif][ichip][ichan][l][TWO_PE];    
          }
        }
        g_Dist[ichan] = new TGraphErrors(size_inputDAC, x_inputDAC, y_Dist, ex_inputDAC, ey_Dist);
        TF1 *f_Dist  = new TF1("f_Dist","[0]*x+[1]");
        f_Dist->SetLineColor(kGreen);
        g_Dist [ichan]->Fit("f_Dist","Q+ E","same"); 
        g_Dist [ichan]->SetMarkerColor(632);
        g_Dist [ichan]->SetMarkerSize(1);
        g_Dist [ichan]->SetMarkerStyle(8);
        mg->Add(g_Dist[ichan]);

        if(mode == TWIN_PEAKS) {
          g_Gain0[ichan] = new TGraph(size_inputDAC,x_inputDAC,y_Gain0);
          g_Gain0[ichan]->SetMarkerColor(600);
          g_Gain0[ichan]->SetMarkerSize(1);
          g_Gain0[ichan]->SetMarkerStyle(8);
          g_Gain1[ichan] = new TGraph(size_inputDAC,x_inputDAC,y_Gain1);
          g_Gain1[ichan]->SetMarkerColor(616);
          g_Gain1[ichan]->SetMarkerSize(1);
          g_Gain1[ichan]->SetMarkerStyle(8);
          mg->Add(g_Gain0[ichan]);
          mg->Add(g_Gain1[ichan]);
        }

        slope[idif][ichip][ichan] = f_Dist->GetParameter(0);
        inter[idif][ichip][ichan] = f_Dist->GetParameter(1);
      } // n_chans
      c1 = new TCanvas("c1","c1");

      mg->SetTitle(Form("chip%d;inputDAC;gain",ichip));
      mg->Draw("ap");
      c1->Print(Form("%s/dif%d/chip%d.png",outputIMGDirName.c_str(),idif+1,ichip));
      for(unsigned ichan = 0; ichan < n_chans; ichan++) {
        delete g_Dist[ichan];
        if(mode == TWIN_PEAKS) {
          delete g_Gain0[ichan];
          delete g_Gain1[ichan];
        }
      }
      delete mg;
      delete c1;
    } // n_chips
  } // n_difs

  // ======================================================//
  //                   calibration_card.xml                //
  // ======================================================//
  
  Edit.PreCalib_Make(outputXMLDirName + "/calibration_card.xml");
  Edit.Open(outputXMLDirName + "/calibration_card.xml");
  for(unsigned idif = 0; idif < n_difs; idif++) {
    for(unsigned ichip = 0; ichip < n_chips; ichip++) {
      for(unsigned ichan = 0; ichan < n_chans; ichan++) {
        Edit.PreCalib_SetValue(string("s_Gain"), idif+1, ichip, ichan, slope[idif][ichip][ichan], NO_CREATE_NEW_MODE);
        Edit.PreCalib_SetValue(string("i_Gain"), idif+1, ichip, ichan, inter[idif][ichip][ichan], NO_CREATE_NEW_MODE);
        if(mode == TWIN_PEAKS) {
          for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
            vector<double> v_ped;
			for(unsigned l = 0; l < size_inputDAC; l++) {
			  // Ignore pedestals less than 350 and over 680
			  if( Pedestal[idif][ichip][ichan][l][ONE_PE][icol] < 350 || Pedestal[idif][ichip][ichan][l][ONE_PE][icol] > 680)
				continue;
			  v_ped.push_back(Pedestal[idif][ichip][ichan][l][ONE_PE][icol]);
			}
			// take the average of the pedestal over the various inputDAC values
            unsigned int v_ped_size = v_ped.size();
            double v_ped_sum = 0.;
            for(unsigned m = 0; m < v_ped_size; m++) {
              v_ped_sum += v_ped[m];
            }
            double v_ped_mean = v_ped_sum / v_ped_size;
            Edit.PreCalib_SetValue("ped_" + to_string(icol), idif+1, ichip, ichan, v_ped_mean, CREATE_NEW_MODE);
          }
        }
      }
    }
  }
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
