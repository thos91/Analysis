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
#include "TLine.h"

// user includes
#include "wgFileSystemTools.hpp"

#include "wgEditXML.hpp"
#include "wgColor.hpp"
#include "wgFitConst.hpp"
#include "wgAnaHistSummary.hpp"
#include "wgLogger.hpp"

using namespace wagasci_tools;

//******************************************************************
void ModeSelect(int mode, std::bitset<M>& flags) {
  if(mode == 1 || mode >= 10) flags[SELECT_NOISE]        = true;
  if(mode == 2 || mode >= 10) flags[SELECT_DIFF]         = true;
  if(mode == 3 || mode >= 11) flags[SELECT_CHARGE_NOHIT] = true;
  if(mode == 4 || mode == 12) flags[SELECT_CHARGE_HIT]   = true;
  if ( mode < 0  || mode > 12 )
    throw std::invalid_argument("Mode " + std::to_string(mode) + " not recognized"); 
}

//******************************************************************
void MakeSummaryXmlFile(const std::string& dir, const bool overwrite, const unsigned ichip, const unsigned n_chans) {
  wgEditXML Edit;
  
  std::string outputxmlfile("");
  outputxmlfile = dir + "/Summary_chip" + std::to_string(ichip) + ".xml";
  if( (check_exist::XmlFile(outputxmlfile) && overwrite) || !check_exist::XmlFile(outputxmlfile) )
    Edit.SUMMARY_Make(outputxmlfile, n_chans);
  else
    throw wgInvalidFile("File " + outputxmlfile + " already exists and overwrite mode is not set");
}

//******************************************************************
int wgAnaHistSummary(const char * x_inputDir,
                     const char * x_outputXMLDir,
                     const char * x_outputIMGDir,
                     const int mode,
                     const bool overwrite,
                     const bool print) {

  std::string inputDir(x_inputDir);
  std::string outputXMLDir(x_outputXMLDir);
  std::string outputIMGDir(x_outputIMGDir);

  wgColor wgColor;

  if(inputDir.empty() || !check_exist::Dir(inputDir)) {
    Log.eWrite("[wgAnaHistSummary] No input directory");
    return ERR_EMPTY_INPUT_FILE;
  }
  if(outputXMLDir.empty()) outputXMLDir = inputDir;
    
  std::bitset<M> flags;
  flags[SELECT_PRINT] = print;
  // Set the correct flags according to the mode
  try { ModeSelect(mode, flags); }
  catch (const std::exception& e) {
    Log.eWrite("[wgAnaHistSummary] " + std::string(e.what()));
    return ERR_WRONG_MODE;
  }

  // ============ Count number of chips and channels ============ //
  unsigned n_chips = HowManyDirectories(inputDir);
  std::vector<unsigned> n_chans;
  for (unsigned ichip = 1; ichip <= n_chips; ichip++) {
    n_chans.push_back(HowManyFilesWithExtension(inputDir + "/chip" + std::to_string(ichip), "xml"));
    // std::cout << "chip = " << ichip << " : channels = " << n_chans[ichip]  << "\n";
  }
  
  // ============ Create outputXMLDir ============ //
  try { MakeDir(outputXMLDir); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[wgAnaHistSummary] " + std::string(e.what()));
    return ERR_FAILED_CREATE_DIRECTORY;
  }

  // ============ Create outputIMGDir ============ //
  if( flags[SELECT_PRINT] ) {
    try { MakeDir(outputIMGDir); }
    catch (const wgInvalidFile& e) {
      Log.eWrite("[wgAnaHistSummary] " + std::string(e.what()));
      return ERR_FAILED_CREATE_DIRECTORY;
    }
  }

  Log.Write(" *****  READING DIRECTORY      :" + inputDir     + "  *****");
  Log.Write(" *****  OUTPUT XML DIRECTORY   :" + outputXMLDir + "  *****");
  Log.Write(" *****  OUTPUT IMAGE DIRECTORY :" + outputIMGDir + "  *****");



  try {
    std::string xmlfile("");
    int start_time, stop_time, difid;

    ///////////////////////////////////////////////////////////////////////////
    //                          Variables declaration                        //
    ///////////////////////////////////////////////////////////////////////////
    
    std::vector<int>                                    trig_th           (n_chips);
    std::vector<int>                                    gain_th           (n_chips);
    std::vector<int>                                    chipid            (n_chips);
    std::vector<std::vector<int>>                       inputDAC          (n_chips);
    std::vector<std::vector<int>>                       ampDAC            (n_chips);
    std::vector<std::vector<int>>                       adjDAC            (n_chips);
    std::vector<std::vector<int>>                       chanid            (n_chips);  
    std::vector<std::vector<int>>                       noise             (n_chips);
    std::vector<std::vector<int>>                       noise_error       (n_chips);
    std::vector<std::vector<int>>                       pe_level          (n_chips);
    std::vector<std::vector<std::array<int, MEMDEPTH>>> charge_nohit      (n_chips);
    std::vector<std::vector<std::array<int, MEMDEPTH>>> charge_nohit_error(n_chips);
    std::vector<std::vector<std::array<int, MEMDEPTH>>> charge_hit        (n_chips);
    std::vector<std::vector<std::array<int, MEMDEPTH>>> charge_hit_error  (n_chips);
    
    std::vector<TH1D *> h_Charge_Nohit(n_chips);
    std::vector<TH1D *> h_Diff        (n_chips);
    std::vector<TH1D *> h_Charge_Hit  (n_chips);
    std::vector<TH1D *> h_Noise       (n_chips);

    //*** Define histograms ***//
    if(flags[SELECT_PRINT]) {
      for(unsigned ichip = 0; ichip < n_chips; ichip++) {
        unsigned ichip_id = ichip + 1;
        if(flags[SELECT_CHARGE_NOHIT]) {
          TString charge_nohit;
          charge_nohit.Form("h_charge_nohit_chip%d", ichip_id);
          h_Charge_Nohit[ichip] = new TH1D(charge_nohit, charge_nohit, n_chans[ichip] * 26 + 10, -5, n_chans[ichip] * 26 + 5);
          charge_nohit.Form("charge_nohit chip:%d;ch*26+col;ADC count", ichip_id);
          h_Charge_Nohit[ichip]->SetTitle(charge_nohit);
          h_Charge_Nohit[ichip]->SetMarkerStyle(8);
          h_Charge_Nohit[ichip]->SetMarkerSize(0.3);
          h_Charge_Nohit[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
          h_Charge_Nohit[ichip]->SetStats(0);
        }

        if(flags[SELECT_CHARGE_HIT]) {
          TString charge_hit;
          charge_hit.Form("h_Charge_Hit_chip%d", ichip_id);
          h_Charge_Hit[ichip] = new TH1D(charge_hit, charge_hit, n_chans[ichip] * 26 + 10, -5, n_chans[ichip] * 26 + 5);
          charge_hit.Form("Diff chip:%d;ch*26+col;ADC count", ichip_id);
          h_Charge_Hit[ichip]->SetTitle(charge_hit);
          h_Charge_Hit[ichip]->SetMarkerStyle(8);
          h_Charge_Hit[ichip]->SetMarkerSize(0.3);
          h_Charge_Hit[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
          h_Charge_Hit[ichip]->SetStats(0);
        }

        if(flags[SELECT_DIFF]) {
          TString diff;
          diff.Form("h_Diff_chip%d", ichip_id);
          h_Diff[ichip] = new TH1D(diff, diff, n_chans[ichip] * 26 + 10, -5, n_chans[ichip] * 26 + 5);
          diff.Form("Diff chip:%d;ch*26+col;ADC count", ichip_id);
          h_Diff[ichip]->SetTitle(diff);
          h_Diff[ichip]->SetMarkerStyle(8);
          h_Diff[ichip]->SetMarkerSize(0.3);
          h_Diff[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
          h_Diff[ichip]->SetStats(0);
        }

        if(flags[SELECT_NOISE]) {
          TString noise;
          noise.Form("h_Noise_chip%d", ichip_id);
          h_Noise[ichip] = new TH1D(noise, noise, 34, -1, 33);
          noise.Form("Noise chip:%d;ch;Noise Rate[Hz]", ichip_id);
          h_Noise[ichip]->SetTitle(noise);
          h_Noise[ichip]->SetMarkerStyle(8);
          h_Noise[ichip]->SetMarkerSize(0.3);
          h_Noise[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
          h_Noise[ichip]->SetStats(0);
        }
      }
    }

    //*** Read data ***//
    wgEditXML Edit;
    try { Edit.Open(inputDir + "/chip1/chan1.xml"); }
    catch (const wgInvalidFile & e) {
      Log.eWrite("[wgAnaHist] " + std::string(e.what()));
      return ERR_FAILED_OPEN_XML_FILE;
    }
    start_time   = Edit.GetConfigValue(std::string("start_time"));
    stop_time    = Edit.GetConfigValue(std::string("stop_time"));
    difid        = Edit.GetConfigValue(std::string("difid"));
    Edit.Close();
     
    for(unsigned ichip = 0; ichip < n_chips; ichip++) {
      unsigned ichip_id = ichip + 1;
      charge_nohit      [ichip].reserve(n_chans[ichip]);
      charge_nohit_error[ichip].reserve(n_chans[ichip]);
      charge_hit        [ichip].reserve(n_chans[ichip]);
      charge_hit_error  [ichip].reserve(n_chans[ichip]);
      
      for(unsigned ichan = 0; ichan < n_chans[ichip]; ichan++) {
        unsigned ichan_id = ichan + 1;
        xmlfile = inputDir + "/chip" + std::to_string(ichip_id) + "/chan" + std::to_string(ichan_id) + ".xml";
        try { Edit.Open(xmlfile); }
        catch (const wgInvalidFile & e) {
          Log.eWrite("[wgAnaHist]" + std::string(e.what()));
          return ERR_FAILED_OPEN_XML_FILE;
        }
        if(ichan == 0 ) {
          trig_th[ichip] = Edit.GetConfigValue(std::string("trigth"));
          gain_th[ichip] = Edit.GetConfigValue(std::string("gainth"));
          chipid[ichip]  = Edit.GetConfigValue(std::string("chipid"));
        }
        inputDAC[ichip].push_back(Edit.GetConfigValue(std::string("inputDAC")));
        ampDAC[ichip].push_back(Edit.GetConfigValue(std::string("HG")));
        adjDAC[ichip].push_back(Edit.GetConfigValue(std::string("trig_adj")));
        chanid[ichip].push_back(Edit.GetConfigValue(std::string("chanid")));
        noise[ichip].push_back(Edit.GetChValue(std::string("noise_rate")));
        noise_error[ichip].push_back(Edit.GetChValue(std::string("sigma_rate")));
        pe_level[ichip].push_back(NoiseToPe(noise[ichip][ichan]));

        for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
          unsigned icol_id = icol + 1;
          if( flags[SELECT_CHARGE_NOHIT] || flags[SELECT_DIFF] ) {
            charge_nohit      [ichip][ichan][icol] = Edit.GetColValue(std::string("charge_nohit"), icol_id);
            charge_nohit_error[ichip][ichan][icol] = Edit.GetColValue(std::string("sigma_nohit"),  icol_id);
          }
          if( flags[SELECT_CHARGE_HIT] || flags[SELECT_DIFF] ) { 
            charge_hit      [ichip][ichan][icol] = Edit.GetColValue(std::string("charge_hit_HG"), icol_id);
            charge_hit_error[ichip][ichan][icol] = Edit.GetColValue(std::string("sigma_hit_HG"),  icol_id);
          }
        }
        Edit.Close();
      }
    }

    //*** Fill data ***//
    for(unsigned ichip = 0; ichip < n_chips; ichip++) {
      unsigned ichip_id = ichip + 1;

      try { MakeSummaryXmlFile(outputXMLDir, overwrite, ichip_id, n_chans[ichip]); }
      catch (const std::exception& e) {
        Log.eWrite("[wgAnaHist][" + outputXMLDir + "] " + std::string(e.what()));
        return ERR_FAILED_CREATE_XML_FILE;
      }
      
      wgEditXML Edit;

      std::string xmlfile(outputXMLDir + "/Summary_chip" + std::to_string(ichip_id) + ".xml");
      try { Edit.Open(xmlfile); }
      catch (const wgInvalidFile & e) {
        Log.eWrite("[wgAnaHist] " + xmlfile + " : " + std::string(e.what()));
        return ERR_FAILED_OPEN_XML_FILE;
      }
      Edit.SUMMARY_SetGlobalConfigValue(std::string("start_time"), start_time,     NO_CREATE_NEW_MODE);
      Edit.SUMMARY_SetGlobalConfigValue(std::string("stop_time"),  stop_time,      NO_CREATE_NEW_MODE);
      Edit.SUMMARY_SetGlobalConfigValue(std::string("difid"),      difid,          NO_CREATE_NEW_MODE);
      Edit.SUMMARY_SetGlobalConfigValue(std::string("trigth"),     trig_th[ichip], NO_CREATE_NEW_MODE);
      Edit.SUMMARY_SetGlobalConfigValue(std::string("gainth"),     gain_th[ichip], NO_CREATE_NEW_MODE);
      Edit.SUMMARY_SetGlobalConfigValue(std::string("chipid"),     chipid [ichip], NO_CREATE_NEW_MODE);
      Edit.SUMMARY_SetGlobalConfigValue(std::string("n_chans"),    n_chans[ichip], NO_CREATE_NEW_MODE);

      for(unsigned ichan = 0; ichan < n_chans[ichip]; ichan++) {
        unsigned ichan_id = ichan + 1;
        
        Edit.SUMMARY_SetChConfigValue(std::string("chanid"),   chanid  [ichip][ichan], ichan_id, NO_CREATE_NEW_MODE);
        Edit.SUMMARY_SetChConfigValue(std::string("inputDAC"), inputDAC[ichip][ichan], ichan_id, NO_CREATE_NEW_MODE);
        Edit.SUMMARY_SetChConfigValue(std::string("ampDAC"),   ampDAC  [ichip][ichan], ichan_id, NO_CREATE_NEW_MODE);
        Edit.SUMMARY_SetChConfigValue(std::string("adjDAC"),   adjDAC  [ichip][ichan], ichan_id, NO_CREATE_NEW_MODE);
        Edit.SUMMARY_SetChFitValue(std::string("pe_level"),    pe_level[ichip][ichan], ichan_id, NO_CREATE_NEW_MODE);
        if(flags[SELECT_NOISE]) {
          Edit.SUMMARY_SetChFitValue(std::string("noise_rate"),    noise      [ichip][ichan], ichan_id, NO_CREATE_NEW_MODE);
          Edit.SUMMARY_SetChFitValue(std::string("sigma_rate"),   noise_error[ichip][ichan], ichan_id, NO_CREATE_NEW_MODE);
          if(flags[SELECT_PRINT]) h_Noise[ichip]->Fill(ichan, noise[ichip][ichan]);
        }

        for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
        unsigned icol_id = icol + 1;
          
          if(flags[SELECT_CHARGE_NOHIT]) {
            Edit.SUMMARY_SetChFitValue("charge_nohit_" + std::to_string(icol_id), charge_nohit      [ichip][ichan][icol], ichan_id, NO_CREATE_NEW_MODE);
            Edit.SUMMARY_SetChFitValue("sigma_nohit_" + std::to_string(icol_id),  charge_nohit_error[ichip][ichan][icol], ichan_id, NO_CREATE_NEW_MODE);
            if(flags[SELECT_PRINT]) h_Charge_Nohit[ichip]->Fill(ichan * 26 + icol, charge_hit[ichip][ichan][icol]);
          }
          if(flags[SELECT_CHARGE_HIT]) {
            Edit.SUMMARY_SetChFitValue("charge_hit_" + std::to_string(icol_id), charge_hit      [ichip][ichan][icol], ichan_id, NO_CREATE_NEW_MODE);
            Edit.SUMMARY_SetChFitValue("sigma_hit_" + std::to_string(icol_id),  charge_hit_error[ichip][ichan][icol], ichan_id, NO_CREATE_NEW_MODE);
            if(flags[SELECT_PRINT]) h_Charge_Hit[ichip]->Fill(ichan * 26 + icol, charge_hit[ichip][ichan][icol]);
          }
          if(flags[SELECT_DIFF]) {
            int diff = charge_hit[ichip][ichan][icol] - charge_nohit[ichip][ichan][icol];
            int diff_error = std::sqrt(std::pow(charge_hit_error[ichip][ichan][icol], 2) - std::pow(charge_nohit_error[ichip][ichan][icol], 2));
            Edit.SUMMARY_SetChFitValue("diff_" + std::to_string(icol_id),       diff,       ichan_id, NO_CREATE_NEW_MODE);
            Edit.SUMMARY_SetChFitValue("sigma_diff_" + std::to_string(icol_id), diff_error, ichan_id, NO_CREATE_NEW_MODE);
            if(flags[SELECT_PRINT]) h_Diff[ichip]->Fill(ichan * 26 + icol, diff);
          }
        }
      }
      Edit.Write();
      Edit.Close();
    }

    if(flags[SELECT_PRINT]) {
      Double_t width = 1280;
      Double_t heigth = 720;

      for(unsigned ichip = 0; ichip < n_chips; ichip++) {
        unsigned ichip_id = ichip + 1;
      
        if(flags[SELECT_NOISE]) {  
          TCanvas * canvas = new TCanvas("c1", "c1", width, heigth);
          TLegend * l_Noise;
          TString name;

          name.Form("chip:%d", ichip_id);
          l_Noise=new TLegend(0.75, 0.84, 0.90, 0.90, name);
          l_Noise->SetBorderSize(1);
          l_Noise->SetFillStyle(0);
          name.Form("Noise Rate \n\t chip:%d", ichip_id);
          l_Noise->AddEntry(h_Noise[ichip], name, "p");
          h_Noise[ichip]->SetMarkerSize(2);
          h_Noise[ichip]->Draw("P HIST");
          l_Noise->Draw();
          name.Form("%s/Summary_Noise_chip%d.png", outputIMGDir.c_str(), ichip_id);
          canvas->Print(name);
          
          delete l_Noise;
          delete h_Noise[ichip];
          delete canvas;
        }

        if(flags[SELECT_DIFF]) {
          TCanvas * canvas = new TCanvas("c1", "c1", width, heigth);
          TString name;
          TLegend * l_Diff;
          std::vector<TLine*> line_Diff(n_chans[ichip]);

          name.Form("chip:%d", ichip_id);
          l_Diff = new TLegend(0.75, 0.75, 0.90, 0.90, name);
          l_Diff->SetBorderSize(1);
          l_Diff->SetFillStyle(0);
          name.Form("Diff (%d pe) \n\t chip:%d", pe_level[ichip][0], ichip_id);
          l_Diff->AddEntry(h_Diff[ichip], name, "p");
          canvas->DrawFrame(-5, 1, 32 * 26 + 5, WG_NOMINAL_GAIN * pe_level[ichip][0] * 4);
          h_Diff[ichip]->Draw("same P HIST");
          l_Diff->Draw();
          for (unsigned ichan = 0; ichan < n_chans[ichip]; ichan++) {
            line_Diff[ichan] = new TLine(ichan * 26 + 21, canvas->GetUymin(), ichan * 26 + 21, canvas->GetUymax());
            line_Diff[ichan]->SetLineStyle(7); // Dotted line
            line_Diff[ichan]->SetLineColor(17); // Grey line
            line_Diff[ichan]->Draw(); 
          }
          name.Form("%s/Summary_Diff_chip%d.png", outputIMGDir.c_str(), ichip_id);
          canvas->Print(name);

          for (unsigned ichan = 0; ichan < n_chans[ichip]; ichan++) delete line_Diff[ichan];
          delete l_Diff;
          delete h_Diff[ichip];
          delete canvas;
        }

        if(flags[SELECT_CHARGE_HIT]) {
          TCanvas * canvas = new TCanvas("c1", "c1", width, heigth);
          TString name;
          TLegend * l_Charge_Hit;
          std::vector<TLine*> line_Charge_Hit(n_chans[ichip]);

          name.Form("chip:%d", ichip_id);
          l_Charge_Hit = new TLegend(0.75, 0.75, 0.90, 0.90, name);
          l_Charge_Hit->SetBorderSize(1);
          l_Charge_Hit->SetFillStyle(0);
          name.Form("Charge_Hit (%d pe) \n\t chip:%d", pe_level[ichip][0], ichip_id);
          l_Charge_Hit->AddEntry(h_Charge_Hit[ichip], name, "p");
          canvas->DrawFrame(-5, WG_BEGIN_CHARGE_HIT_HG, 32 * 26 + 5, WG_END_CHARGE_HIT_HG);
          h_Charge_Hit[ichip]->Draw("same P HIST");
          l_Charge_Hit->Draw();
          for (unsigned ichan = 0; ichan < n_chans[ichip]; ichan++) {
            line_Charge_Hit[ichan] = new TLine(ichan * 26 + 21, canvas->GetUymin(), ichan * 26 + 21, canvas->GetUymax());
            line_Charge_Hit[ichan]->SetLineStyle(7); // Dotted line
            line_Charge_Hit[ichan]->SetLineColor(17); // Grey line
            line_Charge_Hit[ichan]->Draw(); 
          }
          name.Form("%s/Summary_Charge_Hit_chip%d.png", outputIMGDir.c_str(), ichip_id);
          canvas->Print(name);

          for (unsigned ichan = 0; ichan < n_chans[ichip]; ichan++) delete line_Charge_Hit[ichan];
          delete l_Charge_Hit;
          delete h_Charge_Hit[ichip];
          delete canvas;
        }

        if(flags[SELECT_CHARGE_NOHIT]) {
          TCanvas * canvas = new TCanvas("c1", "c1", width, heigth);
          TString name;
          TLegend * l_Charge_Nohit;
          std::vector<TLine*> line_Charge_Nohit(n_chans[ichip]);

          name.Form("chip:%d", ichip_id);
          l_Charge_Nohit = new TLegend(0.75, 0.75, 0.90, 0.90, name);
          l_Charge_Nohit->SetBorderSize(1);
          l_Charge_Nohit->SetFillStyle(0);
          name.Form("Charge_Nohit \n\t chip:%d", ichip_id);
          l_Charge_Nohit->AddEntry(h_Charge_Nohit[ichip], name, "p");
          canvas->DrawFrame(-5, WG_BEGIN_CHARGE_NOHIT, 32 * 26 + 5, WG_END_CHARGE_NOHIT);
          h_Charge_Nohit[ichip]->Draw("same P HIST");
          l_Charge_Nohit->Draw();
          for (unsigned ichan = 0; ichan < n_chans[ichip]; ichan++) {
            line_Charge_Nohit[ichan] = new TLine(ichan * 26 + 21, canvas->GetUymin(), ichan * 26 + 21, canvas->GetUymax());
            line_Charge_Nohit[ichan]->SetLineStyle(7); // Dotted line
            line_Charge_Nohit[ichan]->SetLineColor(17); // Grey line
            line_Charge_Nohit[ichan]->Draw(); 
          }
          name.Form("%s/Summary_Charge_Nohit_chip%d.png", outputIMGDir.c_str(), ichip_id);
          canvas->Print(name);

          for (unsigned ichan = 0; ichan < n_chans[ichip]; ichan++) delete line_Charge_Nohit[ichan];
          delete l_Charge_Nohit;
          delete h_Charge_Nohit[ichip];
          delete canvas;
        }
      } // chips
    } // SELECT_PRINT
  } // try
  catch (const std::exception& e) {
    Log.eWrite("[wgAnaHistSummary] " + std::string(e.what()));
    return ERR_WG_ANA_HIST_SUMMARY;
  } 
  return WG_SUCCESS;
}
