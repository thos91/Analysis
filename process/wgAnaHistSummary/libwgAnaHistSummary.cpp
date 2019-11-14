// system includes
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <bitset>
#include <list>
#include <unordered_map>
#include <cmath>

// boost includes
#include <boost/filesystem.hpp>
#include <boost/make_unique.hpp>

#ifdef HAVE_IMAGEMAGICK
// Magick include
#include <Magick++.h>
#endif

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
#include "wgErrorCodes.hpp"
#include "wgEditXML.hpp"
#include "wgColor.hpp"
#include "wgFitConst.hpp"
#include "wgLogger.hpp"
#include "wgAnaHist.hpp"
#include "wgAnaHistSummary.hpp"

using namespace wagasci_tools;

//******************************************************************

namespace anahistsummary {
void make_summary_xml_file(const std::string& dir, const bool overwrite,
                           const unsigned ichip, const unsigned n_chans) {
  wgEditXML Edit;
  
  std::string outputxmlfile("");
  outputxmlfile = dir + "/Summary_chip" + std::to_string(ichip) + ".xml";
  if( (check_exist::xml_file(outputxmlfile) && overwrite) ||
      !check_exist::xml_file(outputxmlfile) )
    Edit.SUMMARY_Make(outputxmlfile, n_chans);
  else
    throw wgInvalidFile("File " + outputxmlfile +
                        " already exists and overwrite mode is not set");
}
} // anahistsummary

//******************************************************************
int wgAnaHistSummary(const char * x_input_dir,
                     const char * x_output_xml_dir,
                     const char * x_output_img_dir,
                     const unsigned long ul_flags) {

  std::bitset<anahist::NFLAGS> flags(ul_flags);
  std::string input_dir(x_input_dir);
  std::string output_xml_dir(x_output_xml_dir);
  std::string output_img_dir(x_output_img_dir);

  wgColor wgColor;

  if(input_dir.empty() || !check_exist::directory(input_dir)) {
    Log.eWrite("[wgAnaHistSummary] No input directory");
    return ERR_EMPTY_INPUT_FILE;
  }
  if(output_xml_dir.empty()) output_xml_dir = input_dir;

  // ============ Count number of chips and channels ============ //
  unsigned n_chips = list::how_many_directories(input_dir, true);
  std::vector<unsigned> n_chans;
  for (unsigned ichip = 0; ichip < n_chips; ichip++) {
    n_chans.push_back(list::how_many_files(input_dir + "/chip" +
                                           std::to_string(ichip), true, ".xml"));
  }
  
  // ============ Create output_xml_dir ============ //
  try { make::directory(output_xml_dir); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[wgAnaHistSummary] " + std::string(e.what()));
    return ERR_FAILED_CREATE_DIRECTORY;
  }

  // ============ Create output_img_dir ============ //
  if( flags[anahist::SELECT_PRINT] ) {
    try { make::directory(output_img_dir); }
    catch (const wgInvalidFile& e) {
      Log.eWrite("[wgAnaHistSummary] " + std::string(e.what()));
      return ERR_FAILED_CREATE_DIRECTORY;
    }
  }

  Log.Write("[wgAnaHistSummary] *****  READING DIRECTORY      :" + input_dir     + "  *****");
  Log.Write("[wgAnaHistSummary] *****  OUTPUT XML DIRECTORY   :" + output_xml_dir + "  *****");
  Log.Write("[wgAnaHistSummary] *****  OUTPUT IMAGE DIRECTORY :" + output_img_dir + "  *****");

  try {
    std::string xmlfile("");
    int start_time, stop_time, difid;

    ///////////////////////////////////////////////////////////////////////////
    //                          Variables declaration                        //
    ///////////////////////////////////////////////////////////////////////////

    //   CHIP        CHANNEL     COLUMN
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
    
    std::vector<std::unique_ptr<TH1D>> h_Charge_Nohit(n_chips);
    std::vector<std::unique_ptr<TH1D>> h_Charge_Hit  (n_chips);
    std::vector<std::unique_ptr<TH1D>> h_Noise       (n_chips);

    //*** Define histograms ***//
    if(flags[anahist::SELECT_PRINT]) {
      for(unsigned ichip = 0; ichip < n_chips; ichip++) {
        if(flags[anahist::SELECT_PEDESTAL]) {
          TString charge_nohit;
          charge_nohit.Form("h_charge_nohit_chip%d", ichip);
          h_Charge_Nohit[ichip] = boost::make_unique<TH1D>(
              charge_nohit, charge_nohit, n_chans[ichip] * 26 + 10,
              -5, n_chans[ichip] * 26 + 5);
          h_Charge_Nohit[ichip]->SetDirectory(0);
          charge_nohit.Form("charge_nohit chip:%d;ch*26+col;ADC count",
                            ichip);
          h_Charge_Nohit[ichip]->SetTitle(charge_nohit);
          h_Charge_Nohit[ichip]->SetMarkerStyle(8);
          h_Charge_Nohit[ichip]->SetMarkerSize(0.3);
          h_Charge_Nohit[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
          h_Charge_Nohit[ichip]->SetStats(0);
        }

        if(flags[anahist::SELECT_CHARGE_HG]) {
          TString charge_hit;
          charge_hit.Form("h_Charge_Hit_HG_chip%d", ichip);
          h_Charge_Hit[ichip] = boost::make_unique<TH1D>(
              charge_hit, charge_hit, n_chans[ichip] * 26 + 10,
              -5, n_chans[ichip] * 26 + 5);
          h_Charge_Hit[ichip]->SetDirectory(0);
          charge_hit.Form("Charge hit HG chip:%d;ch*26+col;ADC count",
                          ichip);
          h_Charge_Hit[ichip]->SetTitle(charge_hit);
          h_Charge_Hit[ichip]->SetMarkerStyle(8);
          h_Charge_Hit[ichip]->SetMarkerSize(0.3);
          h_Charge_Hit[ichip]->SetMarkerColor(wgColor::wgcolors[ichip]);
          h_Charge_Hit[ichip]->SetStats(0);
        }

        if(flags[anahist::SELECT_DARK_NOISE]) {
          TString noise;
          noise.Form("h_Noise_chip%d", ichip);
          h_Noise[ichip] = boost::make_unique<TH1D>(noise, noise,
                                                    34, -1, 33);
          noise.Form("Noise chip:%d;ch;Noise Rate[Hz]", ichip);
          h_Noise[ichip]->SetDirectory(0);
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
    try { Edit.Open(input_dir + "/chip1/chan1.xml"); }
    catch (const wgInvalidFile & e) {
      Log.eWrite("[wgAnaHist] " + std::string(e.what()));
      return ERR_FAILED_OPEN_XML_FILE;
    }
    start_time = Edit.GetConfigValue(std::string("start_time"));
    stop_time  = Edit.GetConfigValue(std::string("stop_time"));
    difid      = Edit.GetConfigValue(std::string("difid"));
    Edit.Close();
     
    for(unsigned ichip = 0; ichip < n_chips; ichip++) {
      charge_nohit      [ichip].reserve(n_chans[ichip]);
      charge_nohit_error[ichip].reserve(n_chans[ichip]);
      charge_hit        [ichip].reserve(n_chans[ichip]);
      charge_hit_error  [ichip].reserve(n_chans[ichip]);
      
      for(unsigned ichan = 0; ichan < n_chans[ichip]; ichan++) {
        xmlfile = input_dir + "/chip" + std::to_string(ichip) +
                  "/chan" + std::to_string(ichan) + ".xml";
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
        inputDAC[ichip].push_back(
            Edit.GetConfigValue(std::string("inputDAC")));
        ampDAC[ichip].push_back(
            Edit.GetConfigValue(std::string("HG")));
        adjDAC[ichip].push_back(
            Edit.GetConfigValue(std::string("trig_adj")));
        chanid[ichip].push_back(
            Edit.GetConfigValue(std::string("chanid")));
        noise[ichip].push_back(
            Edit.GetChValue(std::string("noise_rate")));
        noise_error[ichip].push_back(
            Edit.GetChValue(std::string("sigma_rate")));
        pe_level[ichip].push_back(noise_to_pe(noise[ichip][ichan]));

        for (unsigned icol = 0; icol < MEMDEPTH; icol++) {
          if (flags[anahist::SELECT_PEDESTAL]) {
            charge_nohit[ichip][ichan][icol] =
                Edit.GetColValue(std::string("charge_nohit"), icol);
            charge_nohit_error[ichip][ichan][icol] =
                Edit.GetColValue(std::string("sigma_nohit"),  icol);
          }
          if (flags[anahist::SELECT_CHARGE_HG]) { 
            charge_hit[ichip][ichan][icol] =
                Edit.GetColValue(std::string("charge_hit_HG"), icol);
            charge_hit_error[ichip][ichan][icol] =
                Edit.GetColValue(std::string("sigma_hit_HG"),  icol);
          }
        }
        Edit.Close();
      }
    }

    //*** Fill data ***//
    for(unsigned ichip = 0; ichip < n_chips; ichip++) {

      try {
        anahistsummary::make_summary_xml_file(
            output_xml_dir, flags[anahist::SELECT_OVERWRITE],
            ichip, n_chans[ichip]); }
      catch (const std::exception& e) {
        Log.eWrite("[wgAnaHist][" + output_xml_dir + "] " +
                   std::string(e.what()));
        return ERR_FAILED_CREATE_XML_FILE;
      }
      
      wgEditXML Edit;

      std::string xmlfile(output_xml_dir + "/Summary_chip" +
                          std::to_string(ichip) + ".xml");
      try { Edit.Open(xmlfile); }
      catch (const wgInvalidFile & e) {
        Log.eWrite("[wgAnaHist] " + xmlfile +
                   " : " + std::string(e.what()));
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
        
        Edit.SUMMARY_SetChConfigValue(std::string("chanid"),   chanid  [ichip][ichan], ichan, NO_CREATE_NEW_MODE);
        Edit.SUMMARY_SetChConfigValue(std::string("inputDAC"), inputDAC[ichip][ichan], ichan, NO_CREATE_NEW_MODE);
        Edit.SUMMARY_SetChConfigValue(std::string("ampDAC"),   ampDAC  [ichip][ichan], ichan, NO_CREATE_NEW_MODE);
        Edit.SUMMARY_SetChConfigValue(std::string("adjDAC"),   adjDAC  [ichip][ichan], ichan, NO_CREATE_NEW_MODE);
        Edit.SUMMARY_SetChFitValue(std::string("pe_level"),    pe_level[ichip][ichan], ichan, NO_CREATE_NEW_MODE);
        if(flags[anahist::SELECT_DARK_NOISE]) {
          Edit.SUMMARY_SetChFitValue(std::string("noise_rate"), noise      [ichip][ichan], ichan, NO_CREATE_NEW_MODE);
          Edit.SUMMARY_SetChFitValue(std::string("sigma_rate"), noise_error[ichip][ichan], ichan, NO_CREATE_NEW_MODE);
          if(flags[anahist::SELECT_PRINT]) h_Noise[ichip]->Fill(ichan, noise[ichip][ichan]);
        }

        for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
          
          if (flags[anahist::SELECT_PEDESTAL]) {
            Edit.SUMMARY_SetChFitValue("charge_nohit_" + std::to_string(icol), charge_nohit      [ichip][ichan][icol], ichan, NO_CREATE_NEW_MODE);
            Edit.SUMMARY_SetChFitValue("sigma_nohit_" + std::to_string(icol),  charge_nohit_error[ichip][ichan][icol], ichan, NO_CREATE_NEW_MODE);
            if(flags[anahist::SELECT_PRINT]) h_Charge_Nohit[ichip]->Fill(ichan * 26 + icol, charge_hit[ichip][ichan][icol]);
          }
          if (flags[anahist::SELECT_CHARGE_HG]) {
            Edit.SUMMARY_SetChFitValue("charge_hit_" + std::to_string(icol), charge_hit      [ichip][ichan][icol], ichan, NO_CREATE_NEW_MODE);
            Edit.SUMMARY_SetChFitValue("sigma_hit_" + std::to_string(icol),  charge_hit_error[ichip][ichan][icol], ichan, NO_CREATE_NEW_MODE);
            if(flags[anahist::SELECT_PRINT]) h_Charge_Hit[ichip]->Fill(ichan * 26 + icol, charge_hit[ichip][ichan][icol]);
          }
        }
      }
      Edit.Write();
      Edit.Close();
    }

    if(flags[anahist::SELECT_PRINT]) {
      Double_t width = 1280;
      Double_t heigth = 720;
      
#ifdef HAVE_IMAGEMAGICK
      std::unordered_map<std::string, std::list<Magick::Image>> images;
#endif
      for(unsigned ichip = 0; ichip < n_chips; ichip++) {
      
        if (flags[anahist::SELECT_DARK_NOISE]) {  
          std::unique_ptr<TCanvas> canvas(new TCanvas("dark", "Dark noise",
                                                      width, heigth));
          TString name;
          name.Form("chip:%d", ichip);
          auto l_Noise = new TLegend(0.75, 0.84, 0.90, 0.90, name);
          l_Noise->SetBorderSize(1);
          l_Noise->SetFillStyle(0);
          name.Form("Noise Rate \n\t chip:%d", ichip);
          l_Noise->AddEntry(h_Noise[ichip].get(), name, "p");
          h_Noise[ichip]->SetMarkerSize(2);
          h_Noise[ichip]->Draw("P HIST");
          l_Noise->Draw();
          name.Form("%s/Summary_Noise_chip%d.png",
                    output_img_dir.c_str(), ichip);
          canvas->Print(name);
#ifdef HAVE_IMAGEMAGICK
          Magick::Image noise_image;
          noise_image.read(name.Data());
          image_list["noise"].push_back(noise_image);
#endif
        }

        if (flags[anahist::SELECT_CHARGE_HG]) {
          std::unique_ptr<TCanvas> canvas(
              new TCanvas("charge", "Charge hit HG", width, heigth));
          TString name;
          TLegend * l_Charge_Hit;
          std::vector<TLine*> line_Charge_Hit(n_chans[ichip]);

          name.Form("chip:%d", ichip);
          l_Charge_Hit = new TLegend(0.75, 0.75, 0.90, 0.90, name);
          l_Charge_Hit->SetBorderSize(1);
          l_Charge_Hit->SetFillStyle(0);
          name.Form("Charge_Hit (%d pe) \n\t chip:%d",
                    pe_level[ichip][0], ichip);
          l_Charge_Hit->AddEntry(h_Charge_Hit[ichip].get(), name, "p");
          canvas->DrawFrame(-5, WG_BEGIN_CHARGE_HIT_HG,
                            32 * 26 + 5, WG_END_CHARGE_HIT_HG);
          h_Charge_Hit[ichip]->Draw("same P HIST");
          l_Charge_Hit->Draw();
          for (unsigned ichan = 0; ichan < n_chans[ichip]; ichan++) {
            line_Charge_Hit[ichan] =
                new TLine(ichan * 26 + 21, canvas->GetUymin(),
                          ichan * 26 + 21, canvas->GetUymax());
            line_Charge_Hit[ichan]->SetLineStyle(7); // Dotted line
            line_Charge_Hit[ichan]->SetLineColor(17); // Grey line
            line_Charge_Hit[ichan]->Draw(); 
          }
          name.Form("%s/Summary_Charge_Hit_chip%d.png",
                    output_img_dir.c_str(), ichip);
          canvas->Print(name);
#ifdef HAVE_IMAGEMAGICK
          Magick::Image charge_image;
          charge_image.read(name.Data());
          image_list["charge"].push_back(charge_image);
#endif
        }

        if (flags[anahist::SELECT_PEDESTAL]) {
          std::unique_ptr<TCanvas> canvas(
              new TCanvas("pedestal", "Pedestal", width, heigth));
          TString name;

          name.Form("chip:%d", ichip);
          auto l_Charge_Nohit = new TLegend(0.75, 0.75, 0.90, 0.90, name);
          l_Charge_Nohit->SetBorderSize(1);
          l_Charge_Nohit->SetFillStyle(0);
          name.Form("Charge_Nohit \n\t chip:%d", ichip);
          l_Charge_Nohit->AddEntry(h_Charge_Nohit[ichip].get(), name, "p");
          canvas->DrawFrame(-5, WG_BEGIN_CHARGE_NOHIT, 32 * 26 + 5,
                            WG_END_CHARGE_NOHIT);
          h_Charge_Nohit[ichip]->Draw("same P HIST");
          l_Charge_Nohit->Draw();
          for (unsigned ichan = 0; ichan < n_chans[ichip]; ichan++) {
            auto line_Charge_Nohit =
                new TLine(ichan * 26 + 21, canvas->GetUymin(),
                          ichan * 26 + 21, canvas->GetUymax());
            line_Charge_Nohit->SetLineStyle(7); // Dotted line
            line_Charge_Nohit->SetLineColor(17); // Grey line
            line_Charge_Nohit->Draw(); 
          }
          name.Form("%s/Summary_Charge_Nohit_chip%d.png",
                    output_img_dir.c_str(), ichip);
          canvas->Print(name);
#ifdef HAVE_IMAGEMAGICK
          Magick::Image pedestal_image;
          pedestal_image.read(name.Data());
          image_list["pedestal"].push_back(pedestal_image);
#endif
        }
      } // chips

#ifdef HAVE_IMAGEMAGICK
      for (auto const &image_list : images) {
        Magick::Color color("rgba(0,0,0,0)");
        Magick::Montage montage_settings;
        montage_settings.geometry("4096x2160-0-0");
        montage_settings.shadow(true);
        montage_settings.backgroundColor(color);
        std::stringstream tile;
        tile << n_chips << "x"  << std::floor(n_chips / 5) + 1;
        montage_settings.tile(tile);

        std::list<Magick::Image> montage_list;
        Magick::montage_images(&montage_list, image_list.begin(),
                               image_list.end(), montage_settings);

        Magick::writeImages(montage_list.begin(), montage_list.end(),
                            output_img_dir + "/charge_summary.png");
      }
#endif
    } // SELECT_PRINT
  } // try
  catch (const std::exception& e) {
    Log.eWrite("[wgAnaHistSummary] " + std::string(e.what()));
    return ERR_WG_ANA_HIST_SUMMARY;
  } 
  return WG_SUCCESS;
}
