// system includes
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

// systemc C includes
#include <cstdbool>

// boost includes
#include <boost/filesystem.hpp>

// ROOT includes
#include <THStack.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TLine.h>

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
#include "wgAnaPedestal.hpp"
#include "wgLogger.hpp"

#define CHARGE_NOHIT_PEAK 0
#define CHARGE_HG_PEAK    1

using namespace wagasci_tools;

//******************************************************************

int wgAnaPedestal(const char * x_inputDir,
                  const char * x_outputXMLDir,
                  const char * x_outputIMGDir,
                  const bool overwrite,
                  const unsigned n_chips,
                  const unsigned n_chans) {

  string inputDir(x_inputDir);
  string outputXMLDir(x_outputXMLDir);
  string outputIMGDir(x_outputIMGDir);

  CheckExist Check;
  wgEditXML Edit;
  string xmlfile("");

  if( inputDir.empty() || !Check.Dir(inputDir) ) {
    Log.eWrite("[wgAnaPedestal] No input directory");
    return ERR_EMPTY_INPUT_FILE;
  }
  
  if(outputXMLDir == "") outputXMLDir = inputDir;

  Log.Write(" *****  READING DIRECTORY      : " + GetName(inputDir)     + "  *****");
  Log.Write(" *****  OUTPUT XML DIRECTORY   : " + GetName(outputXMLDir) + "  *****");
  Log.Write(" *****  OUTPUT IMAGE DIRECTORY : " + GetName(outputIMGDir) + "  *****");


  vector<int> trig_th(n_chips);
  vector<int> gain_th(n_chips);
  vector<vector<int>> inputDAC(n_chips, vector<int>(n_chans));
  vector<vector<int>> ampDAC  (n_chips, vector<int>(n_chans));
  vector<vector<int>> adjDAC   (n_chips, vector<int>(n_chans));
  vector<vector<vector<array<double, 2>>>> charge (n_chips, vector<vector<array<double, 2>>>(n_chans, vector<array<double, 2>>(MEMDEPTH)));	
  vector<vector<array<double, MEMDEPTH>>>  gain    (n_chips, vector<array<double, MEMDEPTH>>(n_chans));
  vector<vector<double>>                   Noise   (n_chips, vector<double>(n_chans));
  vector<vector<double>>                   pe_level(n_chips, vector<double>(n_chans));

  wgColor wgColor;

  vector<TH1D *> h_Pedestal(n_chips);
  vector<TH1D *> h_Npe     (n_chips);
  vector<TH1D *> h_Gain    (n_chips);
  vector<TH1D *> h_Noise   (n_chips);

  if(outputXMLDir == "") outputXMLDir = inputDir;

  // ============ Create outputIMGDir ============ //
  outputIMGDir = outputIMGDir + "/" + GetName(inputDir);
  if ( !Check.Dir(outputIMGDir) ) {
	boost::filesystem::path dir(outputIMGDir);
	if( !boost::filesystem::create_directories(dir) ) {
	  Log.eWrite("[wgAnaHist][" + outputIMGDir + "] failed to create directory");
	  return ERR_CANNOT_CREATE_DIRECTORY;
	}
  }
  // ============ Create outputXMLDir ============ //
  outputXMLDir = outputXMLDir + "/" + GetName(inputDir);
  if ( !Check.Dir(outputXMLDir) ) {
	boost::filesystem::path dir(outputXMLDir);
	if( !boost::filesystem::create_directories(dir) ) {
	  Log.eWrite("[wgAnaHist][" + outputXMLDir + "] failed to create directory");
	  return ERR_CANNOT_CREATE_DIRECTORY;
	}
  }

  MakeSummaryXmlFile(outputXMLDir, overwrite, n_chips, n_chans);

  //*** Define histgram ***//
  TString name;
  for(unsigned ichip = 0; ichip < n_chips; ichip++) {
	name.Form("h_pedestal_chip%d", ichip);
    h_Pedestal[ichip]=new TH1D(name, name, n_chans * 26 + 10, -5, n_chans * 26 + 5);
	name.Form("pedestal chip:%d;ch*26+col;ADC count", ichip);
    h_Pedestal[ichip]->SetTitle(name);
    h_Pedestal[ichip]->SetMarkerStyle(8);
    h_Pedestal[ichip]->SetMarkerSize(0.3);
    h_Pedestal[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
    h_Pedestal[ichip]->SetStats(0);

	name.Form("h_Gain_chip%d", ichip);
	h_Gain[ichip]=new TH1D(name, name, n_chans * 26 + 10, -5, 32 * 26 + 5);
	name.Form("Gain chip:%d;ch*26+col;ADC count", ichip);
	h_Gain[ichip]->SetTitle(name);
	h_Gain[ichip]->SetMarkerStyle(8);
	h_Gain[ichip]->SetMarkerSize(0.3);
	h_Gain[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
	h_Gain[ichip]->SetStats(0);

	name.Form("h_Npe_chip%d", ichip);
	h_Npe[ichip]=new TH1D(name, name, n_chans * 26 + 10, -5, 32 * 26 + 5);
	name.Form("Gain chip:%d;ch*26+col;ADC count", ichip);
	h_Npe[ichip]->SetTitle(name);
	h_Npe[ichip]->SetMarkerStyle(8);
	h_Npe[ichip]->SetMarkerSize(0.3);
	h_Npe[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
	h_Npe[ichip]->SetStats(0);

	name.Form("h_Noise_chip%d", ichip);
	h_Noise[ichip]=new TH1D(name, name, 34, -1, 33);
	name.Form("Noise chip:%d;ch;Noise Rate[Hz]", ichip);
	h_Noise[ichip]->SetTitle(name);
	h_Noise[ichip]->SetMarkerStyle(8);
	h_Noise[ichip]->SetMarkerSize(0.3);
	h_Noise[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
	h_Noise[ichip]->SetStats(0);
  }

  //*** Read data ***//

  // These are the chip%d/ch%d.xml files that were produced by the wgAnaHist
  // program. You need to run that program in the appropriate mode to generate
  // these xml files
  for(unsigned ichip = 0; ichip < n_chips; ichip++) {
    for(unsigned ichan = 0; ichan < n_chans; ichan++) {
	  try {
		Edit.Open(inputDir + "/chip" + to_string(ichip) + "/ch" + to_string(ichan) + ".xml");
	  }
	  catch (const exception& e) {
		Log.eWrite("[" + GetName(inputDir) + "][wgAnaPedestalSummary] " + e.what());
		return ERR_FAILED_OPEN_XML_FILE;
	  }
	  
      if(ichan == 0) {
        trig_th[ichip] = Edit.GetConfigValue(string("trigth"));
        gain_th[ichip] = Edit.GetConfigValue(string("gainth"));
      }
	  // Fill the arrays with the data read from the xml files
      inputDAC[ichip][ichan] = Edit.GetConfigValue (string("inputDAC"));
      ampDAC  [ichip][ichan] = Edit.GetConfigValue (string("HG"));
      adjDAC  [ichip][ichan] = Edit.GetConfigValue (string("trig_adj"));
	  Noise   [ichip][ichan] = Edit.GetChValue     (string("NoiseRate"));
	  // Guess the threshold value (in p.e.) given the dark noise rate
	  pe_level[ichip][ichan]    = NoiseToPe(Noise[ichip][ichan]);

      for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
		// Pedestal position
        charge[ichip][ichan][icol][CHARGE_NOHIT_PEAK] = Edit.GetColValue(string("charge_nohit"), icol);
		// pe_level p.e. position for high gain preamp
        charge[ichip][ichan][icol][CHARGE_HG_PEAK] = Edit.GetColValue(string("charge_lowHG"), icol);
        gain[ichip][ichan][icol] = charge[ichip][ichan][icol][CHARGE_HG_PEAK] - charge[ichip][ichan][icol][CHARGE_NOHIT_PEAK];
      }
      Edit.Close();
    }
  }
  
  //*** Fill data ***//
  // Fill the Pedestal_chip%d.xml file with the data
  // Basically the only new information that is added to the files is the
  // pe_level and the gain
  for(unsigned ichip = 0; ichip < n_chips; ichip++) {
	try {
	  Edit.Open( outputXMLDir + "/Pedestal_chip" + to_string(ichip) + ".xml" );
	}
	catch (const exception& e) {
	  Log.eWrite("[" + GetName(inputDir) + "][wgAnaPedestalSummary] " + e.what());
	  return ERR_FAILED_OPEN_XML_FILE;
	}

	
    Edit.SUMMARY_SetGlobalConfigValue(string("trigth"), trig_th[ichip], 0);
    Edit.SUMMARY_SetGlobalConfigValue(string("gainth"), gain_th[ichip], 0);
    for(unsigned ichan = 0; ichan < n_chans; ichan++) {
      Edit.SUMMARY_SetChConfigValue(string("inputDAC"), inputDAC[ichip][ichan], ichan, NO_CREATE_NEW_MODE);
      Edit.SUMMARY_SetChConfigValue(string("ampDAC"),   ampDAC[ichip][ichan],   ichan, NO_CREATE_NEW_MODE);
      Edit.SUMMARY_SetChConfigValue(string("adjDAC"),   adjDAC[ichip][ichan],   ichan, NO_CREATE_NEW_MODE);
	  Edit.SUMMARY_SetChFitValue(string("pe_level"),    pe_level[ichip][ichan], ichan, CREATE_NEW_MODE);
	  Edit.SUMMARY_SetChFitValue(string("Noise"),       Noise[ichip][ichan],    ichan, NO_CREATE_NEW_MODE);
	  
      for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
		// Difference between the charge_HG peak (it is the pe_level p.e. peak) and
		// the charge_nohit peak (it is a multiple of the gain)
		Edit.SUMMARY_SetChFitValue("gain_" + to_string(icol),    gain[ichip][ichan][icol], ichan, NO_CREATE_NEW_MODE);
		// Pedestal position (charge_nohit peak)
        Edit.SUMMARY_SetChFitValue("ped_" + to_string(icol),     charge[ichip][ichan][icol][CHARGE_NOHIT_PEAK], ichan, NO_CREATE_NEW_MODE);
		// pe_level p.e. peak position for high gain preamp (charge_HG peak)
        Edit.SUMMARY_SetChFitValue("ped_ref_" + to_string(icol), charge[ichip][ichan][icol][CHARGE_HG_PEAK], ichan, CREATE_NEW_MODE);
		
        h_Pedestal[ichip]->Fill(ichan * 26 + icol, charge[ichip][ichan][icol][CHARGE_NOHIT_PEAK]);
		h_Npe[ichip]->     Fill(ichan * 26 + icol, charge[ichip][ichan][icol][CHARGE_HG_PEAK]);
		h_Gain[ichip]->    Fill(ichan * 26 + icol, gain  [ichip][ichan][icol]);
		h_Noise[ichip]->Fill(ichan, Noise[ichip][ichan]);
      }
    }
    Edit.Write();
    Edit.Close();
  }

  //*** Plot data ***//

  TCanvas *c1 = new TCanvas("c1", "c1", 1280, 720);

  vector<TLegend*> l_Noise   (n_chips);
  vector<TLegend*> l_Npe     (n_chips);
  vector<TLegend*> l_Gain    (n_chips);
  vector<TLegend*> l_Pedestal(n_chips);
  vector<vector<TLine*>> line_Pedestal(n_chips, vector<TLine*>(n_chans));
  vector<vector<TLine*>> line_Gain(n_chips, vector<TLine*>(n_chans));
  vector<vector<TLine*>> line_Npe(n_chips, vector<TLine*>(n_chans));

  for(unsigned ichip = 0; ichip < n_chips; ichip++) {

	name.Form("chip:%d", ichip);
    l_Pedestal[ichip] = new TLegend(0.75, 0.84, 0.90, 0.90, name);
    l_Pedestal[ichip]->SetBorderSize(0);
    l_Pedestal[ichip]->SetFillStyle(0);
	name.Form("Pedestal \n\t chip:%d", ichip);
    l_Pedestal[ichip]->AddEntry(h_Pedestal[ichip], name, "p");
    c1->DrawFrame(-5, begin_ped, 32 * 26 + 5, end_ped);
    h_Pedestal[ichip]->Draw("same P HIST");
    l_Pedestal[ichip]->Draw();
	for (unsigned ichan = 0; ichan < n_chans; ichan++) {
	  line_Pedestal[ichip][ichan] = new TLine(ichan * 26 + 21, c1->GetUymin(), ichan * 26 + 21, c1->GetUymax());
	  line_Pedestal[ichip][ichan]->SetLineStyle(7); // Dotted line
	  line_Pedestal[ichip][ichan]->SetLineColor(17); // Grey line
	  line_Pedestal[ichip][ichan]->Draw(); 
	}
	name.Form("%s/Summary_Pedestal_chip%d.png", outputIMGDir.c_str(),ichip);
    c1->Print(name);

	name.Form("chip:%d", ichip);
	l_Npe[ichip] = new TLegend(0.75, 0.81, 0.90, 0.90, name);
	l_Npe[ichip]->SetBorderSize(1);
	l_Npe[ichip]->SetFillStyle(0);
	name.Form("%.0f pe peak \n\t chip:%d", pe_level[ichip][0], ichip);
	l_Npe[ichip]->AddEntry(h_Npe[ichip], name, "p");
	c1->DrawFrame(-5, begin_low_pe_HG, 32 * 26 + 5, end_low_pe_HG);
	h_Npe[ichip]->Draw("same P HIST");
	l_Npe[ichip]->Draw();
	for (unsigned ichan = 0; ichan < n_chans; ichan++) {
	  line_Npe[ichip][ichan] = new TLine(ichan * 26 + 21, c1->GetUymin(), ichan * 26 + 21, c1->GetUymax());
	  line_Npe[ichip][ichan]->SetLineStyle(7); // Dotted line
	  line_Npe[ichip][ichan]->SetLineColor(17); // Grey line
	  line_Npe[ichip][ichan]->Draw(); 
	}
	name.Form("%s/Summary_Npe_chip%d.png", outputIMGDir.c_str(), ichip);
	c1->Print(name);

	name.Form("chip:%d", ichip);
	l_Gain[ichip] = new TLegend(0.75, 0.75, 0.90, 0.90, name);
	l_Gain[ichip]->SetBorderSize(1);
	l_Gain[ichip]->SetFillStyle(0);
	name.Form("Gain (%.0f pe) \n\t chip:%d", pe_level[ichip][0], ichip);
	l_Gain[ichip]->AddEntry(h_Gain[ichip], name, "p");
	c1->DrawFrame(-5, 1, 32 * 26 + 5, est_Gain * 4);
	h_Gain[ichip]->Draw("same P HIST");
	l_Gain[ichip]->Draw();
	for (unsigned ichan = 0; ichan < n_chans; ichan++) {
	  line_Gain[ichip][ichan] = new TLine(ichan * 26 + 21, c1->GetUymin(), ichan * 26 + 21, c1->GetUymax());
	  line_Gain[ichip][ichan]->SetLineStyle(7); // Dotted line
	  line_Gain[ichip][ichan]->SetLineColor(17); // Grey line
	  line_Gain[ichip][ichan]->Draw(); 
	}
	name.Form("%s/Summary_Gain_chip%d.png", outputIMGDir.c_str(), ichip);
	c1->Print(name);
	
	name.Form("chip:%d", ichip);
	l_Noise[ichip]=new TLegend(0.75, 0.84, 0.90, 0.90, name);
	l_Noise[ichip]->SetBorderSize(1);
	l_Noise[ichip]->SetFillStyle(0);
	l_Noise[ichip]->AddEntry(h_Noise[ichip],"Noise Rate","p");
	h_Noise[ichip]->SetMarkerSize(2);
	h_Noise[ichip]->Draw("P HIST");
	l_Noise[ichip]->Draw();
	name.Form("%s/Summary_Noise_chip%d.png", outputIMGDir.c_str(), ichip);
	c1->Print(name);
  }
  return AP_SUCCESS;
}

//******************************************************************
void MakeSummaryXmlFile(const string& str, const bool overwrite, const unsigned n_chips, const unsigned n_chans) {
  wgEditXML Edit;
  CheckExist check;
  string outputxmlfile("");
  for(unsigned ichip = 0; ichip < n_chips; ichip++) {
	outputxmlfile = str + "/Pedestal_chip" + to_string(ichip) + ".xml";
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
