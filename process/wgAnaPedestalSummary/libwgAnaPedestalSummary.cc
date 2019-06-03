// system includes
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <dirent.h>

// boost includes
#include <boost/filesystem.hpp>

// ROOT includes
#include <TROOT.h>
#include "TApplication.h"
#include <THStack.h>
#include <TMultiGraph.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TF1.h>
#include <TSpectrum.h>

// user includes
#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgExceptions.h"
#include "wgEditXML.h"
#include "wgColor.h"
#include "wgFit.h"
#include "wgFitConst.h"
#include "wgGetHist.h"
#include "Const.h"
#include "wgAnaPedestalSummary.hpp"

//******************************************************************
vector<string> GetIncludeFileName(const string& inputDir){
  OperateString OpStr;
  DIR *dp;
  struct dirent *entry;
  vector<string> openxmlfile;

  // Open the input directory
  dp = opendir(inputDir.c_str());
  if(dp == NULL)
	throw wgInvalidFile("opendir: failed to open directory");

  // Fill the openxmlfile vector of strings with the path of all the files and
  // directories contained inside the input directory
  while( (entry = readdir(dp)) != NULL ) {
	// Ignore hidden files and directories
    if( (entry->d_name[0]) != '.' )
	  openxmlfile.push_back( inputDir + "/" + string(entry->d_name) );
  }
  closedir(dp);
  return openxmlfile;
} 

//******************************************************************
int AnaPedestalSummary(const char * x_inputDir,
					   const char * x_outputXMLDir,
					   const char * x_outputIMGDir,
					   const unsigned n_difs,
					   const unsigned n_chips,
					   const unsigned n_chans)
{
  string inputDir(x_inputDir);
  string outputXMLDir(x_outputXMLDir);
  string outputIMGDir(x_outputIMGDir);
  
  wgConst con;
  OperateString OpStr;
  CheckExist Check;
  if (outputXMLDir == ""){
    outputXMLDir = con.CALIBDATA_DIRECTORY;
  }

  // ============ Create outputXMLDir ============ //
  if ( !Check.Dir(outputXMLDir) ) {
	boost::filesystem::path dir(outputXMLDir);
	if( !boost::filesystem::create_directories(dir) ) {
	  Log.eWrite("[wgAnaHist][" + outputXMLDir + "] failed to create directory");
	  return ERR_CANNOT_CREATE_DIRECTORY;
	}
  }
    // ============ Create outputIMGDir ============ //
  if ( !Check.Dir(outputIMGDir) ) {
	boost::filesystem::path dir(outputIMGDir);
	if( !boost::filesystem::create_directories(dir) ) {
	  Log.eWrite("[wgAnaHist][" + outputIMGDir + "] failed to create directory");
	  return ERR_CANNOT_CREATE_DIRECTORY;
	}
  }

  // Open the input directory and fill the ReadFile vector with the list of
  // files in it
  vector<string> inputFile;
  try {
	inputFile = GetIncludeFileName(inputDir); 
  } catch (const wgInvalidFile& e) {
	Log.eWrite("[" + OpStr.GetName(inputDir) + "][wgAnaPedestalSummary] " + e.what());
	return ERR_GET_FILE_LIST;
  }

  int nFiles = inputFile.size();
  if (nFiles == 0) {
    Log.Write("[" + OpStr.GetName(inputDir) + "][wgAnaPedestalSummary] input directory seems empty");
    return ERR_GET_FILE_LIST;
  }
  
  string xmlfile("");
  wgEditXML Edit;
  unsigned dif = 0;
  unsigned npe = ONE_PE;
  // Gain[][][][ONE_PE] is the position of the 1 p.e. peak relative to the pedestal
  // Gain[][][][TWO_PE] is the position of the 2 p.e. peak relative to the pedestal
  double Gain   [n_difs][n_chips][n_chans][MEMDEPTH][2] = {};
  double ped    [n_difs][n_chips][n_chans][MEMDEPTH] = {};
  double ped_ref[n_difs][n_chips][n_chans][MEMDEPTH] = {};

  /********************************************************************************
   *                              Read XML files                                  *
   ********************************************************************************/
	
  for(int iFN = 0; iFN < nFiles; iFN++) {
    for(unsigned ichip = 0; ichip < n_chips; ichip++) {

	  // Guess the dif number from the file name
      int pos = inputFile[iFN].find("dif_1_1_") + 8;
	  dif = inputFile[iFN][pos] - '0';
	  if( dif <= n_difs)
		dif--;
	  else
		{
		  Log.eWrite("failed to guess DIF number from file name: " + inputFile[iFN]);
		  return ERR_WRONG_DIF_VALUE;
		}

	  // ************* Open XML file ************* //
	  
	  string xmlfile(inputFile[iFN] + "/Pedestal_chip" + to_string(ichip) + ".xml");
	  try {
		Edit.Open(xmlfile);
	  }
	  catch (const exception& e) {
		Log.eWrite("[" + OpStr.GetName(inputDir) + "][wgAnaPedestalSummary] " + e.what());
		return ERR_FAILED_OPEN_XML_FILE;
	  }

	  // ************* Read XML file ************* //
	  
      for(unsigned ichan = 0; ichan < n_chans; ichan++) {
		try { npe = Edit.SUMMARY_GetChFitValue(string("pe_level"), ichan); }
		catch (...) {
		  Log.eWrite("failed to read photo electrons equivalent threshold from XML file");
		  pos = inputFile[iFN].find("_pe") + 3;
		  npe = inputFile[iFN][pos] - '0';
		}
		// pe_level and dif_1_1_ start from 1 while the local variable npe starts from 0
		npe--;
		if ( npe > TWO_PE ) {
		  Log.eWrite("failed to guess photo electrons equivalent threshold");
		  return ERR_WRONG_PE_VALUE;
		}
		  
		for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
		  // charge_nohit peak
		  ped    [dif][ichip][ichan][icol] = Edit.SUMMARY_GetChFitValue("ped_" + to_string(icol), ichan);
		  // charge_HG peak (npe p.e. peak for high gain preamp)
		  ped_ref[dif][ichip][ichan][icol] = Edit.SUMMARY_GetChFitValue("ped_ref_" + to_string(icol), ichan);
		  // Position of the npe p.e. peak relative to the charge_nohit value (charge_HG - charge_nohit)
		  Gain[dif][ichip][ichan][icol][npe] = Edit.SUMMARY_GetChFitValue("gain_" + to_string(icol), ichan);
		}
	  }
	  Edit.Close();
	}
  }

  /*************************************************************************
   *                                 GAIN                                  *
   *************************************************************************/

  TCanvas *c1 = new TCanvas("c1", "c1", 1280, 720);
  c1->Divide(n_difs, 2);
  
  // Define the histograms

  TH1D * h_Gain[n_difs];
  TH2D * h_Gain2D[n_difs];
  TString name;

  for(unsigned idif = 0; idif < n_difs; idif++) {
	// xbins = 80, xlow = 20, xup = 60 
	name.Form("h_Gain_DIF%d", idif + 1);
	h_Gain [idif] = new TH1D(name, name, 80, 20, 60);
	// xbins = 20, xlow = 0, xup = 20, ybins = 40, ylow = 0, yup = 80
	name.Form("h_Gain2D_DIF%d", idif + 1);
	h_Gain2D[idif] = new TH2D(name, name, 20, 0, 20, 40, 0, 80);
  }


  // Fill the histograms
  for(unsigned idif = 0; idif < n_difs; idif++) {
	for(unsigned ichip = 0; ichip < n_chips; ichip++) {
	  for(unsigned ichan = 0; ichan < n_chans; ichan++) {
		for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
		  // Difference between the 2 p.e. peak and the 1 p.e. peak (i.e. the gain value)
		  double DIST = Gain[idif][ichip][ichan][icol][TWO_PE] - Gain[idif][ichip][ichan][icol][ONE_PE];
		  h_Gain[idif]->Fill(DIST);
		  // fill ichip bin with weight DIST
		  h_Gain2D[idif]->Fill(ichip, DIST);
		}
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
	h_Gain2D[idif]->Draw("colz");
  }
  c1->Print(Form("%s/Gain.png",outputIMGDir.c_str()));


  /*************************************************************************
   *                              PEDESTAL                                 *
   *************************************************************************/
  
  TH1D *h1[MEMDEPTH];
  TH1D *h2[MEMDEPTH];
  // xbins = 60, xlow = -50, xup = 10
  TH1D *h3 = new TH1D("h3","h1", 60, -50, 10);
  h3->SetTitle("pedestal shift;adc count;nEntry");
  for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
	// xbins = 300, xlow = 400, xup = 700
    h1[icol] = new TH1D(Form("h1_%d",icol), "h1", 300, 400, 700);
	// xbins = 100, xlow = -50, xup = 50
    h2[icol] = new TH1D(Form("h2_%d",icol), "h2", 100, -50, 50);
    h1[icol]->SetLineColor(kBlue);
    h2[icol]->SetLineColor(kRed); 
    h2[icol]->SetTitle(Form("pedestal shift col%d;adc count;nEntry",icol));
  }


  xmlfile = outputXMLDir + "/pedestal_card.xml";
  Edit.Calib_Make(xmlfile, n_difs, n_chips, n_chans);
  try {
	Edit.Open(xmlfile);
  }
  catch (const exception& e) {
	Log.eWrite("[" + OpStr.GetName(inputDir) + "][wgAnaPedestalSummary] " + e.what());
	return ERR_FAILED_OPEN_XML_FILE;
  }

  for(unsigned idif = 0; idif < n_difs; idif++) {
    for(unsigned ichip = 0; ichip < n_chips; ichip++) {
      for(unsigned ichan = 0; ichan < n_chans; ichan++) {
        for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
		  Edit.Calib_SetValue("pe1_" + to_string(icol),  idif+1, ichip, ichan, Gain[idif][ichip][ichan][icol][ONE_PE], NO_CREATE_NEW_MODE);
		  Edit.Calib_SetValue("pe2_" + to_string(icol),  idif+1, ichip, ichan, Gain[idif][ichip][ichan][icol][TWO_PE], NO_CREATE_NEW_MODE);
		  Edit.Calib_SetValue("gain_" + to_string(icol), idif+1, ichip, ichan, Gain[idif][ichip][ichan][icol][TWO_PE] - Gain[idif][ichip][ichan][icol][ONE_PE], NO_CREATE_NEW_MODE);
		  // nominal_pedestal = 1 p.e. peak - 2 * gain
          double nominal_pedestal = ped_ref[idif][ichip][ichan][icol] - 2 * (Gain[idif][ichip][ichan][icol][TWO_PE] - Gain[idif][ichip][ichan][icol][ONE_PE]);		  
          Edit.Calib_SetValue("ped_" + to_string(icol), idif+1, ichip, ichan, nominal_pedestal, CREATE_NEW_MODE);
          h1[icol]->Fill(nominal_pedestal);

		  // Pedestal when there is no hit
          Edit.Calib_SetValue("ped_nohit_" + to_string(icol), idif+1, ichip, ichan, ped[idif][ichip][ichan][icol], CREATE_NEW_MODE);
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
  TString image(outputIMGDir + "/pedestal_shift.png");
  c1->Print(image);
  delete c1;
  c1=new TCanvas("c1","c1");
  h3->Draw("");
  image = outputIMGDir + "/pedestal_shift_all.png";
  c1->Print(image);

  return APS_SUCCESS;
}
