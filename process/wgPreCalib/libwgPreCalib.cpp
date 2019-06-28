// system C++ includes
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

// system C includes
#include <dirent.h>

// boost includes
#include <boost/filesystem.hpp>

// ROOT includes
#include <THStack.h>
#include <TMultiGraph.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TString.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TF1.h>
#include <TSpectrum.h>
#include <TVector.h>

// user includes
#include "wgFileSystemTools.hpp"

#include "wgEditXML.hpp"
#include "wgColor.hpp"
#include "wgFit.hpp"
#include "wgFitConst.hpp"
#include "wgGetHist.hpp"
#include "wgPreCalib.hpp"
#include "wgLogger.hpp"

using namespace wagasci_tools;

//******************************************************************
int wgPreCalib(const char * x_inputDir,
			   const char * x_outputXMLDir,
			   const char * x_outputIMGDir,
			   const int mode,
			   const unsigned n_difs,
			   const unsigned n_chips,
			   const unsigned n_chans) {
  
  
  wgConst con;
  string inputDir(x_inputDir);
  string outputXMLDir(x_outputXMLDir);
  string outputIMGDir(x_outputIMGDir);

  if(outputXMLDir == "") {
    outputXMLDir = con.CALIBDATA_DIRECTORY + GetName(inputDir);
	if(outputXMLDir == "") {
	  return ERR_CANNOT_CREATE_DIRECTORY;
	}
  }
  
  // ============ Create outputXMLDir ============ //
  for(unsigned ichip = 0; ichip < n_chips; ichip++) {
	string dir_name(outputXMLDir + "/chip" + to_string(ichip));
	if( !check_exist::Dir(dir_name) ) {
	  boost::filesystem::path dir(dir_name);
	  if( !boost::filesystem::create_directories(dir) ) {
		Log.eWrite("[wgPreCalib][" + dir_name + "] failed to create directory");
		return ERR_CANNOT_CREATE_DIRECTORY;
	  }
	}
  }
  // ============ Create outputIMGDir ============ //
  for(unsigned idif = 0; idif < n_difs; idif++) {
	string dir_name(outputIMGDir + "/dif" + to_string(idif + 1));
	if( !check_exist::Dir(dir_name) ) {
	  boost::filesystem::path dir(dir_name);
	  if( !boost::filesystem::create_directories(dir) ) {
		Log.eWrite("[wgPreCalib][" + dir_name + "] failed to create directory");
		return ERR_CANNOT_CREATE_DIRECTORY;
	  }
	}
  }

  vector<string> inputFiles = ListFiles(inputDir);
  unsigned n_files = inputFiles.size();
  vector<int> inputDAC(n_files);
  
  // Save all the inputDAC values in the inputDAC[] array
  wgEditXML Edit;
  for(unsigned iFN = 0; iFN < n_files; iFN++) {
    Edit.Open(inputFiles[iFN] + "/Summary_chip0.xml");
    inputDAC[iFN] = Edit.SUMMARY_GetChConfigValue(string("inputDAC"), NO_CREATE_NEW_MODE);
    Edit.Close();
  }

  if ( mode != TWIN_PEAKS && mode != LONELY_MOUNTAIN )
	throw invalid_argument("[wgPreCalib] unknown mode : " + to_string(mode));

  // Remove duplicates and sort the list of inputDACs
  vector<int> list_inputDAC;
  bool add_list_inputDAC = false;
  list_inputDAC.push_back(inputDAC[0]);
  for(unsigned iFN = 1; iFN < n_files; iFN++) {
    add_list_inputDAC = true;
    for(unsigned l = 0; l < list_inputDAC.size(); l++) {
      if(list_inputDAC[l] == inputDAC[iFN]) {
        add_list_inputDAC = false;
      }
    }
    if(add_list_inputDAC) list_inputDAC.push_back(inputDAC[iFN]);
  }
  sort(list_inputDAC.begin(), list_inputDAC.end());

  unsigned size_inputDAC = list_inputDAC.size();

  int dif = 0, npe = 0;

  // double Gain[n_difs][n_chips][n_chans][size_inputDAC][2];
  // double Pedestal[n_difs][n_chips][n_chans][size_inputDAC][2][MEMDEPTH];

  vector<vector<vector<vector<array<double, 2>>>>> Gain (n_difs, vector<vector<vector<array<double, 2>>>>
														 (n_chips, vector<vector<array<double, 2>>>
														  (n_chans, vector<array<double, 2>>
														   (size_inputDAC, array<double, 2>()))));

  vector<vector<vector<vector<vector<array<double, MEMDEPTH>>>>>> Pedestal (n_difs, vector<vector<vector<vector<array<double, MEMDEPTH>>>>>
																			(n_chips, vector<vector<vector<array<double, MEMDEPTH>>>>
																			 (n_chans, vector<vector<array<double, MEMDEPTH>>>
																			  (size_inputDAC, vector<array<double, MEMDEPTH>>
																			   (2, array<double, MEMDEPTH>())))));
																	   


  // ======================================================//
  //              Read Gain and Pedestal                   //
  // ======================================================//
  
  for(unsigned iFN = 0; iFN < n_files; iFN++) {
    for(unsigned ichip = 0; ichip < n_chips; ichip++) {

	  // ===== DIF number ===== //
	  
      int pos = inputFiles[iFN].find("dif_1_1_") + 8;
	  bool found = false;
	  for (unsigned idif = 0; idif < n_difs; idif++) {
		// This will not work for DIF >= 10
		if(inputFiles[iFN][pos] == to_string(idif + 1)) {
		  dif = idif;
		  found = true;
		  break;
		}
	  }
	  if (found == false)
		throw wgElementNotFound("failed to guess DIF number from file name: " + inputFiles[iFN]);

	  // ===== P.E. number ===== //
	  
      pos = inputFiles[iFN].find("_pe") + 3;
      if(inputFiles[iFN][pos] == '1') npe = 0;
      else if(inputFiles[iFN][pos] == '2') npe = 1;
	  else throw wgElementNotFound("failed to guess photo electrons from file name: " + inputFiles[iFN]);

      Edit.Open(inputFiles[iFN] + "/Summary_chip" + to_string(ichip) + ".xml");
	  // Read the value of the inputDAC from the Summary_chip%d.xml file
      inputDAC[iFN] = Edit.SUMMARY_GetChConfigValue(string("inputDAC"), NO_CREATE_NEW_MODE);
	  
	  pair<bool, int> iDAC = findInVector(list_inputDAC, inputDAC[iFN]);
	  if (iDAC.first == false) {
		Log.eWrite("[wgPreCalib] inputDAC value (" + to_string(inputDAC[iFN]) + ") not found in the list");
		continue;
	  }

	  // ===== read Gain and Pedestal ===== //
	  
      for(unsigned ichan = 0; ichan < n_chans; ichan++) {
        // This gain should be twice the true gain.
        Gain[dif][ichip][ichan][iDAC.second][npe] = Edit.SUMMARY_GetChFitValue(string("Gain"), ichan);
        if(mode == TWIN_PEAKS){
		  // If we use both 1pe and 2ped, get the pedestal position
          Edit.SUMMARY_GetPedFitValue(Pedestal[dif][ichip][ichan][iDAC.second][npe].data(), ichan);
        }
      }
      Edit.Close();
    }
  }

  // ======================================================//
  //            Draw the inputDAC vs Gain graph            //
  // ======================================================//

  vector<vector<vector<double>>> slope(n_difs, vector<vector<double>>(n_chips, vector<double>(n_chans)));
  vector<vector<vector<double>>> inter(n_difs, vector<vector<double>>(n_chips, vector<double>(n_chans)));
	
  for(unsigned idif = 0; idif < n_difs; idif++) {
    for(unsigned ichip = 0; ichip < n_chips; ichip++) {
      TMultiGraph * mg = new TMultiGraph();
      vector<TGraphErrors *> g_Dist(n_chans); 
      vector<TGraph *> g_Gain0(n_chans);
      vector<TGraph *> g_Gain1(n_chans);
      vector<TGraph *> g_ped(MEMDEPTH);

	  // ===== plot the Pedestal ===== //
	  
      TCanvas *c1 = new TCanvas("c1","c1");
      if( mode == TWIN_PEAKS ) {
        c1->Divide(4,4);
        for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
          TVectorD x_ch (n_chans * size_inputDAC);
          TVectorD y_ped(n_chans * size_inputDAC);
          for(unsigned int l = 0; l < size_inputDAC; l++) {
            for(unsigned ichan = 0; ichan < n_chans; ichan++) {
              x_ch [ichan + n_chans * l] = ichan;
              y_ped[ichan + n_chans * l] = Pedestal[idif][ichip][ichan][l][1][icol];
            }
          }

          g_ped[icol] = new TGraph(x_ch, y_ped);
          g_ped[icol]->SetMarkerColor(880+icol);
          g_ped[icol]->SetMarkerSize(0.5);
          g_ped[icol]->SetMaximum(700);
          g_ped[icol]->SetMinimum(350);
		  TString title;
		  title.Form("chip%d col%d;ch;ped",ichip,icol);
          g_ped[icol]->SetTitle(title);
          c1->cd(icol+1);
          c1->SetGrid();
          g_ped[icol]->Draw("ap");
        }
		TString image;
		image.Form("%s/dif%d/ped_chip%d.png",outputIMGDir.c_str(),idif+1,ichip);
        c1->Print(image);
        delete c1;
        for(unsigned icol = 0; icol < MEMDEPTH; icol++) delete g_ped[icol];
      }

	  // ===== calculate the gain average over all the channels  ===== //
	  
      vector<double> mean_Dist(size_inputDAC);
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
        TVectorD x_inputDAC(size_inputDAC);
        TVectorD ex_inputDAC(size_inputDAC);
        TVectorD y_Dist(size_inputDAC);
        TVectorD ey_Dist(size_inputDAC);
        TVectorD y_Gain0(size_inputDAC);
        TVectorD y_Gain1(size_inputDAC);
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
        g_Dist[ichan] = new TGraphErrors(x_inputDAC, y_Dist, ex_inputDAC, ey_Dist);
        TF1 *f_Dist  = new TF1("f_Dist","[0]*x+[1]");
        f_Dist->SetLineColor(kGreen);
        g_Dist [ichan]->Fit("f_Dist","Q+ E","same"); 
        g_Dist [ichan]->SetMarkerColor(632);
        g_Dist [ichan]->SetMarkerSize(1);
        g_Dist [ichan]->SetMarkerStyle(8);
        mg->Add(g_Dist[ichan]);

        if(mode == TWIN_PEAKS) {
          g_Gain0[ichan] = new TGraph(x_inputDAC, y_Gain0);
          g_Gain0[ichan]->SetMarkerColor(600);
          g_Gain0[ichan]->SetMarkerSize(1);
          g_Gain0[ichan]->SetMarkerStyle(8);
          g_Gain1[ichan] = new TGraph(x_inputDAC, y_Gain1);
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

	  TString title;
	  title.Form("chip%d;inputDAC;gain",ichip);
      mg->SetTitle(title);
      mg->Draw("ap");
	  TString image;
	  image.Form("%s/dif%d/chip%d.png",outputIMGDir.c_str(),idif+1,ichip);
      c1->Print(image);
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
  
  Edit.PreCalib_Make(outputXMLDir + "/calibration_card.xml");
  Edit.Open(outputXMLDir + "/calibration_card.xml");
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
  return PC_SUCCESS;
}

//******************************************************************
vector<string> ListFiles(const string& inputDirName){
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
