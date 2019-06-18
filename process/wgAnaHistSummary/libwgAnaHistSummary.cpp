// system includes
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

// boost includes
#include <boost/filesystem.hpp>

// system C includes
#include <cstdbool>
#include <bits/stdc++.h>

// ROOT includes
#include "TSystem.h"
#include "THStack.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TImage.h"
#include "TH1D.h"
#include "TH2D.h"

// user includes
#include "wgFileSystemTools.hpp"
#include "wgErrorCode.hpp"
#include "wgEditXML.hpp"
#include "wgColor.hpp"
#include "wgFitConst.hpp"
#include "wgAnaHistSummary.hpp"
#include "wgLogger.hpp"

using namespace wagasci_tools;

//******************************************************************
void ModeSelect(int mode, bitset<M>& flags) {
  if(mode == 1 || mode >= 10) flags[SELECT_NOISE]     = true;
  if(mode == 2 || mode >= 10) flags[SELECT_GAIN]      = true;
  if(mode == 3 || mode >= 11) flags[SELECT_PEDESTAL]  = true;
  if(mode == 4 || mode == 12) flags[SELECT_RAWCHARGE] = true;
  if ( mode < 0  || mode > 12 )
	throw invalid_argument("Mode " + to_string(mode) + " not recognized"); 
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
int wgAnaHistSummary(const char * x_inputDir, const char * x_outputXMLDir, const char * x_outputIMGDir, const int mode, const bool overwrite, const bool print, const unsigned n_chips, const unsigned n_chans) {

  string inputDir(x_inputDir);
  string outputXMLDir(x_outputXMLDir);
  string outputIMGDir(x_outputIMGDir);

  if(inputDir.empty()) {
    Log.eWrite("[wgAnaHistSummary] No input directory");
    return ERR_EMPTY_INPUT_FILE;
  }
  if(outputXMLDir.empty()) outputXMLDir = inputDir;
    
  bitset<M> flags;
  flags[SELECT_PRINT] = print;
  // Set the correct flags according to the mode
  try { ModeSelect(mode, flags); }
  catch (const exception& e) {
	Log.eWrite("[wgAnaHistSummary] " + string(e.what()));
	return ERR_WRONG_MODE;
  }

  // ============ Create outputXMLDir ============ //
  try { MakeDir(outputXMLDir); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[wgAnaHistSummary] " + string(e.what()));
    return ERR_CANNOT_CREATE_DIRECTORY;
  }

  // ============ Create outputIMGDir ============ //
  if( flags[SELECT_PRINT] ) {
	outputIMGDir += "/" + GetName(inputDir);
    try { MakeDir(outputIMGDir); }
    catch (const wgInvalidFile& e) {
      Log.eWrite("[wgAnaHistSummary] " + string(e.what()));
      return ERR_CANNOT_CREATE_DIRECTORY;
    }
  }

  Log.Write(" *****  READING DIRECTORY      :" + GetName(inputDir)     + "  *****");
  Log.Write(" *****  OUTPUT XML DIRECTORY   :" + GetName(outputXMLDir) + "  *****");
  Log.Write(" *****  OUTPUT IMAGE DIRECTORY :" + GetName(outputIMGDir) + "  *****");

  try { MakeSummaryXmlFile(outputXMLDir, overwrite, n_chips, n_chans); }
  catch (const exception& e) {
	Log.eWrite("[wgAnaHist][" + outputXMLDir + "] " + string(e.what()));
	return ERR_FAILED_CREATE_XML_FILE;
  }

  try {
	string xmlfile("");
	int start_time,stop_time;
	vector<int>            trig_th  (n_chips);
	vector<int>            gain_th  (n_chips);
	vector<vector<int>>    inputDAC (n_chips, vector<int>(n_chans));
	vector<vector<int>>    ampDAC   (n_chips, vector<int>(n_chans));
	vector<vector<int>>    adjDAC   (n_chips, vector<int>(n_chans));
	vector<vector<double>> charge   (n_chips, vector<double>(n_chans));

	vector<vector<vector<array<double, 2>>>> rawcharge   (n_chips, vector<vector<array<double, 2>>>(n_chans, vector<array<double, 2>>(MEMDEPTH)));
    vector<vector<vector<array<double, 2>>>> e_rawcharge (n_chips, vector<vector<array<double, 2>>>(n_chans, vector<array<double, 2>>(MEMDEPTH)));	
	vector<vector<array<double, 2>>>         Noise       (n_chips, vector<array<double, 2>>(n_chans));

	wgColor wgColor;

	vector<TH1D *> h_Pedestal (n_chips);
	vector<TH1D *> h_Gain     (n_chips);
	vector<TH1D *> h_rawcharge(n_chips);
	vector<TH1D *> h_Noise    (n_chips);

	//*** Define histgram ***//
	if(flags[SELECT_PRINT]){
	  for(unsigned ichip = 0; ichip < n_chips; ichip++) {
		if(flags[SELECT_PEDESTAL]) {
		  TString pedestal;
		  pedestal.Form("h_pedestal_chip%d", ichip);
		  h_Pedestal[ichip] = new TH1D(pedestal, pedestal, n_chans*26+10, -5, n_chans*26+5);
		  pedestal.Form("pedestal chip:%d;ch*26+col;ADC count", ichip);
		  h_Pedestal[ichip]->SetTitle(pedestal);
		  h_Pedestal[ichip]->SetMarkerStyle(8);
		  h_Pedestal[ichip]->SetMarkerSize(0.3);
		  h_Pedestal[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
		  h_Pedestal[ichip]->SetStats(0);
		}

		if(flags[SELECT_RAWCHARGE]) {
		  TString rawcharge;
		  rawcharge.Form("h_rawcharge_chip%d",ichip);
		  h_rawcharge[ichip] = new TH1D(rawcharge, rawcharge, n_chans*26+10, -5, n_chans*26+5);
		  rawcharge.Form("Gain chip:%d;ch*26+col;ADC count", ichip);
		  h_rawcharge[ichip]->SetTitle(rawcharge);
		  h_rawcharge[ichip]->SetMarkerStyle(8);
		  h_rawcharge[ichip]->SetMarkerSize(0.3);
		  h_rawcharge[ichip]->SetStats(0);
		  h_rawcharge[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
		}

		if(flags[SELECT_GAIN]) {
		  TString gain;
		  gain.Form("h_Gain_chip%d", ichip);
		  h_Gain[ichip] = new TH1D(gain, gain, 34, -1, 33);
		  gain.Form("Gain chip:%d;ch;ADC count", ichip);
		  h_Gain[ichip]->SetTitle(gain);
		  h_Gain[ichip]->SetMarkerStyle(8);
		  h_Gain[ichip]->SetMarkerSize(0.3);
		  h_Gain[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
		  h_Gain[ichip]->SetStats(0);
		}

		if(flags[SELECT_NOISE]) {
		  TString noise;
		  noise.Form("h_Noise_chip%d", ichip);
		  h_Noise[ichip] = new TH1D(noise, noise, 34, -1, 33);
		  noise.Form("Noise chip:%d;ch;Noise Rate[Hz]", ichip);
		  h_Noise[ichip]->SetTitle(noise);
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
		wgEditXML Edit;
		xmlfile = inputDir + "/chip" + to_string(ichip) + "/ch" + to_string(ichan) + ".xml";
		Edit.Open(xmlfile);
		if(ichan == 0 ) {
		  if(ichip == 0) {
			start_time   = Edit.GetConfigValue(string("start_time"));
			stop_time    = Edit.GetConfigValue(string("stop_time"));
		  }
		  trig_th[ichip] = Edit.GetConfigValue(string("trigth"));
		  gain_th[ichip] = Edit.GetConfigValue(string("gainth"));
		}
		inputDAC[ichip][ichan] = Edit.GetConfigValue(string("inputDAC"));
		ampDAC[ichip][ichan]   = Edit.GetConfigValue(string("HG"));
		adjDAC[ichip][ichan]   = Edit.GetConfigValue(string("trig_adj"));
		if(flags[SELECT_NOISE]){ 
		  Noise[ichip][ichan][0] =Edit.GetChValue(string("NoiseRate"));
		  Noise[ichip][ichan][1]=Edit.GetChValue(string("NoiseRate_e"));
		}

		if(flags[SELECT_GAIN]){ 
		  charge[ichip][ichan]=Edit.GetChValue(string("charge_low"));
		}

		for(unsigned icol = 0; icol < MEMDEPTH; icol++){
		  if(flags[SELECT_PEDESTAL]){ 
			rawcharge[ichip][ichan][icol][0]=Edit.GetColValue(string("charge_nohit"),icol); 
			e_rawcharge[ichip][ichan][icol][0]=Edit.GetColValue(string("sigma_nohit"),icol); 
		  }

		  if(flags[SELECT_RAWCHARGE]){ 
			rawcharge[ichip][ichan][icol][1]=Edit.GetColValue(string("charge_lowHG"),icol); 
			e_rawcharge[ichip][ichan][icol][1]=Edit.GetColValue(string("sigma_lowHG"),icol); 
		  }
		}
		Edit.Close();
	  }
	}

	//*** Fill data ***//
	for(unsigned ichip = 0; ichip < n_chips; ichip++) {
	  wgEditXML Edit;
	  Edit.Open(outputXMLDir + "/Summary_chip" + to_string(ichip) + ".xml");
	  Edit.SUMMARY_SetGlobalConfigValue(string("start_time"),start_time,0);
	  Edit.SUMMARY_SetGlobalConfigValue(string("stop_time"),stop_time,0);
	  Edit.SUMMARY_SetGlobalConfigValue(string("trigth"),trig_th[ichip],0);
	  Edit.SUMMARY_SetGlobalConfigValue(string("gainth"),gain_th[ichip],0);
	  for(unsigned ichan = 0; ichan < n_chans; ichan++){
		Edit.SUMMARY_SetChConfigValue(string("inputDAC"),inputDAC[ichip][ichan],ichan,0);
		Edit.SUMMARY_SetChConfigValue(string("ampDAC"),ampDAC[ichip][ichan],ichan,0);
		Edit.SUMMARY_SetChConfigValue(string("adjDAC"),adjDAC[ichip][ichan],ichan,0);
		if(flags[SELECT_GAIN]){
		  double Gain = charge[ichip][ichan];  
		  Edit.SUMMARY_SetChFitValue(string("Gain"), Gain, ichan, NO_CREATE_NEW_MODE);
		  if(flags[SELECT_PRINT]) h_Gain[ichip]->Fill(ichan, Gain);
		}

		if(flags[SELECT_NOISE]){
		  Edit.SUMMARY_SetChFitValue(string("Noise"), Noise[ichip][ichan][0], ichan, NO_CREATE_NEW_MODE); 
		  if(flags[SELECT_PRINT]) h_Noise[ichip]->Fill(ichan, Noise[ichip][ichan][0]);
		}

		for(unsigned icol = 0; icol < MEMDEPTH; icol++){
		  if(flags[SELECT_PEDESTAL]){
			Edit.SUMMARY_SetChFitValue("ped_" + to_string(icol), rawcharge[ichip][ichan][icol][0], ichan, CREATE_NEW_MODE);
			Edit.SUMMARY_SetChFitValue("eped_" + to_string(icol), e_rawcharge[ichip][ichan][icol][0], ichan, CREATE_NEW_MODE);
			if(flags[SELECT_PRINT]) h_Pedestal[ichip]->Fill(ichan*26+icol, rawcharge[ichip][ichan][icol][0]);
		  }

		  if(flags[SELECT_RAWCHARGE]){
			Edit.SUMMARY_SetChFitValue("raw_" + to_string(icol), rawcharge[ichip][ichan][icol][1], ichan, CREATE_NEW_MODE);
			Edit.SUMMARY_SetChFitValue("eraw_" + to_string(icol), e_rawcharge[ichip][ichan][icol][1], ichan, CREATE_NEW_MODE);
			if(flags[SELECT_PRINT]) h_rawcharge[ichip]->Fill(ichan*26+icol, rawcharge[ichip][ichan][icol][1]);
		  }
		}
	  }
	  Edit.Write();
	  Edit.Close();
	}

	if(flags[SELECT_PRINT]) {
	  Double_t w = 800;
	  Double_t h = 600;
	  for(unsigned int ichip = 0; ichip < n_chips; ichip++) {

		if(flags[SELECT_NOISE]) {
		  TCanvas * canvas = new TCanvas;
		  canvas->SetCanvasSize(w,h);
		  TLegend * l_Noise;
		  TString noise;
		  noise.Form("chip:%d", ichip);
		  l_Noise = new TLegend(0.75, 0.84, 0.90, 0.90, noise);
		  l_Noise->SetBorderSize(1);
		  l_Noise->SetFillStyle(0);
		  l_Noise->AddEntry(h_Noise[ichip],"Noise Rate","p");
		  h_Noise[ichip]->Draw("P HIST");
		  l_Noise->Draw();
		  gSystem->ProcessEvents();
		  TImage * image = TImage::Create();
		  image->FromPad(canvas);
		  noise.Form("%s/Summary_Noise_chip%d.png", outputIMGDir.c_str(), ichip);
		  image->WriteImage(noise);
		  delete l_Noise;
		  delete h_Noise[ichip];
		  delete canvas;
		  delete image;
		}

		if(flags[SELECT_GAIN]) {
		  TCanvas * canvas = new TCanvas;
		  canvas->SetCanvasSize(w,h);
		  TString gain;
		  TLegend * l_Gain;
		  gain.Form("chip:%d", ichip);
		  l_Gain = new TLegend(0.75, 0.81, 0.90, 0.90, gain);
		  l_Gain->SetBorderSize(1);
		  l_Gain->SetFillStyle(0);
		  l_Gain->AddEntry(h_Gain[ichip],"Gain","p");
		  h_Gain[ichip]->Draw("P HIST");
		  l_Gain->Draw();
		  gSystem->ProcessEvents();
		  TImage * image = TImage::Create();
		  image->FromPad(canvas);
		  gain.Form("%s/Summary_Gain_chip%d.png", outputIMGDir.c_str(), ichip);
		  image->WriteImage(gain);
		  delete l_Gain;
		  delete h_Gain[ichip];
		  delete canvas;
		  delete image;
		}

		if(flags[SELECT_RAWCHARGE]) {
		  TCanvas * canvas = new TCanvas;
		  canvas->SetCanvasSize(w,h);
		  TString rawcharge;
		  TLegend *l_rawcharge;
		  rawcharge.Form("chip:%d", ichip);
		  l_rawcharge = new TLegend(0.75, 0.75, 0.90, 0.90, rawcharge);
		  l_rawcharge->SetBorderSize(1);
		  l_rawcharge->SetFillStyle(0);
		  l_rawcharge->AddEntry(h_rawcharge[ichip], "Gain", "p");
		  h_rawcharge[ichip]->Draw("P HIST");
		  l_rawcharge->Draw();
		  gSystem->ProcessEvents();
		  TImage * image = TImage::Create();
		  image->FromPad(canvas);
		  rawcharge.Form("%s/Summary_rawcharge_chip%d.png", outputIMGDir.c_str(), ichip);
		  image->WriteImage(rawcharge);
		  delete l_rawcharge;
		  delete h_rawcharge[ichip];
		  delete canvas;
		  delete image;
		}

		if(flags[SELECT_PEDESTAL]) {
		  TCanvas * canvas = new TCanvas;
		  canvas->SetCanvasSize(w,h);
		  TString pedestal;
		  TLegend * l_Pedestal;
		  pedestal.Form("chip:%d", ichip);
		  l_Pedestal = new TLegend(0.75, 0.84, 0.90, 0.90, pedestal);
		  l_Pedestal->SetBorderSize(0);
		  l_Pedestal->SetFillStyle(0);
		  pedestal.Form("Pedestal \n\t chip:%d",ichip);
		  l_Pedestal->AddEntry(h_Pedestal[ichip], pedestal,"p");
		  canvas->DrawFrame(-5, begin_ped, n_chans*26+5, end_ped);
		  h_Pedestal[ichip]->Draw("same P HIST");
		  l_Pedestal->Draw();
		  gSystem->ProcessEvents();
		  TImage * image = TImage::Create();
		  image->FromPad(canvas);
		  pedestal.Form("%s/Summary_Pedestal_chip%d.png", outputIMGDir.c_str(), ichip);
		  image->WriteImage(pedestal);
		  delete l_Pedestal;
		  delete h_Pedestal[ichip];
		  delete canvas;
		  delete image;
		}
	  } // chips
	} // SELECT_PRINT
  } // try
  catch (const exception& e) {
	Log.eWrite("[wgAnaHistSummary][" + inputDir + "] " + string(e.what()));
	return ERR_WG_ANA_HIST_SUMMARY;
  } 
  return AHS_SUCCESS;
}
