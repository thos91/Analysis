// system includes
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <dirent.h>

// system C includes
#include <cstdlib>

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
#include "wgFileSystemTools.hpp"
#include "wgErrorCode.hpp"
#include "wgExceptions.hpp"
#include "wgEditXML.hpp"
#include "wgColor.hpp"
#include "wgFit.hpp"
#include "wgFitConst.hpp"
#include "wgGetHist.hpp"
#include "wgConst.hpp"
#include "wgAnaPedestalSummary.hpp"
#include "wgLogger.hpp"

using namespace wagasci_tools;

//******************************************************************
int wgAnaPedestalSummary(const char * x_inputDir,
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
  CheckExist Check;

  if (inputDir.empty() || !Check.Dir(inputDir)) {
    Log.eWrite("[wgAnaPedestal] input directory doesn't exist");
    return ERR_EMPTY_INPUT_FILE;
  }
  
  if (outputXMLDir.empty()) {
    outputXMLDir = con.CALIBDATA_DIRECTORY;
  }

  // ============ Create outputXMLDir ============ //
  try { MakeDir(outputXMLDir); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[wgAnaPedestalSummary] " + string(e.what()));
    return ERR_CANNOT_CREATE_DIRECTORY;
  }

  // ============ Create outputIMGDir ============ //
  try { MakeDir(outputIMGDir); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[wgAnaPedestalSummary] " + string(e.what()));
    return ERR_CANNOT_CREATE_DIRECTORY;
  }

  Log.Write(" *****  READING DIRECTORY      :" + GetName(inputDir)     + "  *****");
  Log.Write(" *****  OUTPUT XML DIRECTORY   :" + GetName(outputXMLDir) + "  *****");
  Log.Write(" *****  OUTPUT IMAGE DIRECTORY :" + GetName(outputIMGDir) + "  *****");
  
  // Open the input directory and fill the ReadFile vector with the list of
  // files in it
  vector<string> inputFile;
  try {
	inputFile = ListFilesWithExtension(inputDir); 
  } catch (const wgInvalidFile& e) {
	Log.eWrite("[wgAnaPedestalSummary] " + string(e.what()));
	return ERR_GET_FILE_LIST;
  }

  int nFiles = inputFile.size();
  if (nFiles == 0) {
    Log.Write("[wgAnaPedestalSummary] input directory seems empty");
    return ERR_GET_FILE_LIST;
  }
  
  string xmlfile("");
  wgEditXML Edit;
  unsigned dif = 0;
  unsigned npe = ONE_PE;
  // Gain[][][][ONE_PE] is the position of the 1 p.e. peak relative to the pedestal
  // Gain[][][][TWO_PE] is the position of the 2 p.e. peak relative to the pedestal

  vector<vector<vector<vector<array<double, 2>>>>> ped_nohit (n_difs, vector<vector<vector<array<double, 2>>>>
														 (n_chips, vector<vector<array<double, 2>>>
														  (n_chans, vector<array<double, 2>>
														   (MEMDEPTH, array<double, 2>()))));
  
  vector<vector<vector<vector<array<double, 2>>>>> ped_ref (n_difs, vector<vector<vector<array<double, 2>>>>
														 (n_chips, vector<vector<array<double, 2>>>
														  (n_chans, vector<array<double, 2>>
														   (MEMDEPTH, array<double, 2>()))));
	
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
		Log.eWrite("[wgAnaPedestalSummary] " + string(e.what()));
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
		  Log.eWrite("failed to guess photo electrons equivalent threshold (npe = " + to_string(npe) + ")");
		  return ERR_WRONG_PE_VALUE;
		}

		for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
		  // charge_nohit peak (slighly shifted with respect to the pedestal)
		  ped_nohit    [dif][ichip][ichan][icol][npe] = Edit.SUMMARY_GetChFitValue("ped_" + to_string(icol), ichan);
		  // charge_HG peak (npe p.e. peak for high gain preamp)
		  // Extract the one photo-electron peak and store it in the ped_ref
		  // variable. This variable is called like this because it will serve as
		  // a reference to calculate the nominal value of the pedestal:
		  // nominal pedestal = pedestal reference - gain
		  ped_ref[dif][ichip][ichan][icol][npe] = Edit.SUMMARY_GetChFitValue("ped_ref_" + to_string(icol), ichan);
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

  vector<TH1D*> h_Gain  (n_difs);
  vector<TH2D*> h_Gain2D(n_difs);
  TString name;

  for(unsigned idif = 0; idif < n_difs; idif++) {
	// xbins = 80, xlow = 20, xup = 60 
	name.Form("h_Gain_DIF%d", idif + 1);
	h_Gain [idif] = new TH1D(name, name, 80, 20, 60);
	// xbins = 20, xlow = 0, xup = 20, ybins = 40, ylow = 0, yup = 80
	name.Form("h_Gain2D_DIF%d", idif + 1);
	h_Gain2D[idif] = new TH2D(name, name, 20, 0, 20, 40, 0, 80);
  }

  // ************* Fill the Gain and Gain2D histograms ************* //
  for(unsigned idif = 0; idif < n_difs; idif++) {
	for(unsigned ichip = 0; ichip < n_chips; ichip++) {
	  for(unsigned ichan = 0; ichan < n_chans; ichan++) {
		for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
		  // Difference between the 2 p.e. peak and the 1 p.e. peak (i.e. the gain value)
		  double Gain = ped_ref[idif][ichip][ichan][icol][TWO_PE] - ped_ref[idif][ichip][ichan][icol][ONE_PE];
		  h_Gain[idif]->Fill(Gain);
		  // fill ichip bin with weight DIST
		  h_Gain2D[idif]->Fill(ichip, Gain);
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
  name = outputIMGDir + "/Gain.png";
  c1->Print(name);


  /*************************************************************************
   *                              PEDESTAL                                 *
   *************************************************************************/
  
  TH1D *h_nominal_ped[MEMDEPTH];
  TH1D *h_ped_shift[MEMDEPTH];
  // xbins = 60, xlow = -50, xup = 10
  TH1D *h_ped_shift_global = new TH1D("h_ped_shift_global","h_ped_shift_global", abs(ped_diff_max - ped_diff_min), ped_diff_min, ped_diff_max);
  
  h_ped_shift_global->SetTitle("pedestal shift;adc count;nEntry");
  for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
	// xbins = 300, xlow = 400, xup = 700
	name.Form("h_nominal_ped_%d",icol);
    h_nominal_ped[icol] = new TH1D(name, "h_nominal_ped", abs(end_ped - begin_ped), begin_ped, end_ped);
	// xbins = 100, xlow = -50, xup = 50
	name.Form("h_ped_shift_%d",icol);
    h_ped_shift[icol] = new TH1D(name, "h_ped_shift", abs(ped_diff_max - ped_diff_min), ped_diff_min, ped_diff_max);
    h_nominal_ped[icol]->SetLineColor(kBlue);
    h_ped_shift[icol]->SetLineColor(kRed);
	name.Form("pedestal shift col%d;adc count;nEntry",icol);
    h_ped_shift[icol]->SetTitle(name);
  }

  /********************** PEDESTAL_CARD.XML ************************/
  
  xmlfile = outputXMLDir + "/pedestal_card.xml";
  Edit.Calib_Make(xmlfile, n_difs, n_chips, n_chans);
  try {
	Edit.Open(xmlfile);
  }
  catch (const exception& e) {
	Log.eWrite("[wgAnaPedestalSummary] " + string(e.what()));
	return ERR_FAILED_OPEN_XML_FILE;
  }

  double gain;
  for(unsigned idif = 0; idif < n_difs; idif++) {
    for(unsigned ichip = 0; ichip < n_chips; ichip++) {
      for(unsigned ichan = 0; ichan < n_chans; ichan++) {
        for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
		  gain = ped_ref[idif][ichip][ichan][icol][TWO_PE] - ped_ref[idif][ichip][ichan][icol][ONE_PE];
		  Edit.Calib_SetValue("pe1_" + to_string(icol),  idif+1, ichip, ichan, ped_ref[idif][ichip][ichan][icol][ONE_PE], NO_CREATE_NEW_MODE);
		  Edit.Calib_SetValue("pe2_" + to_string(icol),  idif+1, ichip, ichan, ped_ref[idif][ichip][ichan][icol][TWO_PE], NO_CREATE_NEW_MODE);
		  Edit.Calib_SetValue("gain_" + to_string(icol), idif+1, ichip, ichan, gain, NO_CREATE_NEW_MODE);
		  // nominal_pedestal = 1 p.e. peak - gain
          double nominal_pedestal = ped_ref[idif][ichip][ichan][icol][ONE_PE] - gain;
		  double measured_pedestal = ped_nohit[idif][ichip][ichan][icol][ONE_PE];
          Edit.Calib_SetValue("ped_" + to_string(icol), idif+1, ichip, ichan, nominal_pedestal, CREATE_NEW_MODE);
          h_nominal_ped[icol]->Fill(nominal_pedestal);

		  // Pedestal when there is no hit
		  // If the pedestal (charge_nohit) for one pe threshold and the
		  // pedestal for two pe threshold are significantly different, there is
		  // something wrong!
		  if ( abs(measured_pedestal - ped_nohit[idif][ichip][ichan][icol][TWO_PE]) / measured_pedestal > PEDESTAL_DIFFERENCE_WARNING_THRESHOLD ) {
			Log.eWrite("[wgAnaPedestalSummary] Difference between 1 pe pedestal_nohit (" + to_string(measured_pedestal) +
					   ") and 2 pe pedestal_nohit (" + to_string(ped_nohit[idif][ichip][ichan][icol][TWO_PE])+
					   ") is greater than " + to_string(int(PEDESTAL_DIFFERENCE_WARNING_THRESHOLD * 100)) + "%"); 
		  }
		  
          Edit.Calib_SetValue("ped_nohit_" + to_string(icol), idif+1, ichip, ichan, measured_pedestal, CREATE_NEW_MODE);
		  // Difference between the nominal_pedestal and the pedestal_nohit 
          h_ped_shift[icol]->Fill(nominal_pedestal - measured_pedestal);
          h_ped_shift_global->Fill(nominal_pedestal - measured_pedestal);
        }
      }
    }
  }

  Edit.Write();
  Edit.Close();

  delete c1;
  c1 = new TCanvas("c1","c1");
  c1->Divide(4,4);
  for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
    c1->cd(icol + 1);
    h_nominal_ped[icol]->Draw("");
    h_ped_shift[icol]->Draw("");
  }
  TString image(outputIMGDir + "/pedestal_shift.png");
  c1->Print(image);
  delete c1;
  c1=new TCanvas("c1","c1");
  h_ped_shift_global->Draw("");
  image = outputIMGDir + "/pedestal_shift_all.png";
  c1->Print(image);

  return APS_SUCCESS;
}
