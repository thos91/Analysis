// As always if a variable name starts from i (ichip, ichan, etc...), it means
// that it starts counting from 0. If a variable name ends with _id, it means
// that its value may NOT start from 0.

// system includes
#include <string>
#include <vector>

// boost includes
#include <boost/make_unique.hpp>

// ROOT includes
#include <TStyle.h>
#include <TCanvas.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TString.h>

// user includes
#include "wgFileSystemTools.hpp"
#include "wgErrorCodes.hpp"
#include "wgExceptions.hpp"
#include "wgEditXML.hpp"
#include "wgFitConst.hpp"
#include "wgConst.hpp"
#include "wgLogger.hpp"
#include "wgPedestalCalib.hpp"

using namespace wagasci_tools;

int wgPedestalCalib(const char * x_input_run_dir,
                    const char * x_output_xml_dir,
                    const char * x_output_img_dir) {
  
  /////////////////////////////////////////////////////////////////////////////
  //                          Check arguments sanity                         //
  /////////////////////////////////////////////////////////////////////////////
  
  std::string input_run_dir (x_input_run_dir);
  std::string output_xml_dir(x_output_xml_dir);
  std::string output_img_dir(x_output_img_dir);
  
  if (input_run_dir.empty() || !check_exist::directory(input_run_dir)) {
    Log.eWrite("[wgPedestalCalib] input directory doesn't exist");
    return ERR_EMPTY_INPUT_FILE;
  }
  
  if (output_xml_dir.empty()) {
    wgEnvironment env;
    output_xml_dir = env.CALIBDATA_DIRECTORY;
  }
  if (output_img_dir.empty()) {
    wgEnvironment env;
    output_xml_dir = env.IMGDATA_DIRECTORY;
  }

  /////////////////////////////////////////////////////////////////////////////
  //                        Create output directories                        //
  /////////////////////////////////////////////////////////////////////////////

  try { make::directory(output_xml_dir); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[wgPedestalCalib] " + std::string(e.what()));
    return ERR_FAILED_CREATE_DIRECTORY;
  }
  
  try { make::directory(output_img_dir); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[wgPedestalCalib] " + std::string(e.what()));
    return ERR_FAILED_CREATE_DIRECTORY;
  }

  Log.Write(" *****  READING DIRECTORY      : " + input_run_dir);
  Log.Write(" *****  OUTPUT XML DIRECTORY   : " + output_xml_dir);
  Log.Write(" *****  OUTPUT IMAGE DIRECTORY : " + output_img_dir);
  

  ///////////////////////////////////////////////////////////////////////////////
  //                      Get topology from XML files                          //
  ///////////////////////////////////////////////////////////////////////////////
  
  // The topology for each acquisition (each photo-electron equivalent
  // threshold) MUST be the same. I mean same number of DIFs, chips and channels

  std::unique_ptr<Topology> topol;
  try {
    topol = boost::make_unique<Topology>(input_run_dir,
                                         TopologySourceType::pedestal_tree);
  } catch (const std::exception& e) {
    Log.eWrite("Failed to get topology from " + input_run_dir +
               " directory : " + e.what());
    return ERR_TOPOLOGY;
  }
  
  unsigned n_difs = topol->n_difs;

  /////////////////////////////////////////////////////////////////////////////
  //                   Reserve memory for containers                         //
  /////////////////////////////////////////////////////////////////////////////

  ChargeVector charge_nohit;
  ChargeVector sigma_nohit;
  ChargeVector charge_hit;
  ChargeVector sigma_hit;
  GainVector   gain;
  GainVector   sigma_gain;

  for (auto const& dif : topol->dif_map) {
    unsigned dif_id = dif.first;
    unsigned n_chips = dif.second.size();
    charge_nohit[dif_id].resize(n_chips);
    sigma_nohit [dif_id].resize(n_chips);
    charge_hit  [dif_id].resize(n_chips);
    sigma_hit   [dif_id].resize(n_chips);
    gain        [dif_id].resize(n_chips);
    sigma_gain  [dif_id].resize(n_chips);
    for (auto const& chip : dif.second) {
      unsigned ichip = chip.first;
      unsigned n_channels = chip.second;
      charge_nohit[dif_id][ichip].resize(n_channels);
      sigma_nohit [dif_id][ichip].resize(n_channels);
      charge_hit  [dif_id][ichip].resize(n_channels);
      sigma_hit   [dif_id][ichip].resize(n_channels);
      gain        [dif_id][ichip].resize(n_channels);
      sigma_gain  [dif_id][ichip].resize(n_channels);
      for (unsigned ichan = 0; ichan < n_channels; ++ichan) {
        charge_nohit[dif_id][ichip][ichan].resize(MEMDEPTH);
        sigma_nohit [dif_id][ichip][ichan].resize(MEMDEPTH);
        charge_hit  [dif_id][ichip][ichan].resize(MEMDEPTH);
        sigma_hit   [dif_id][ichip][ichan].resize(MEMDEPTH);
      }
    }
  }

  ///////////////////////////////////////////////////////////////////////////
  //                            Read XML files                             //
  ///////////////////////////////////////////////////////////////////////////

  for (auto& pe_directory : list::list_directories(input_run_dir, true)) {
    unsigned pe_level_from_dir = string::extract_integer(get_stats::basename(pe_directory));
    unsigned ipe;
    if (pe_level_from_dir == 1) ipe = ONE_PE;
    else if (pe_level_from_dir == 2) ipe = TWO_PE;
    else continue;
    
    // DIF
    for (auto const& dif_directory :
             list::list_directories(pe_directory + "/wgAnaHistSummary/Xml", true)) {
      unsigned dif_id = string::extract_integer(get_stats::basename(dif_directory));
      
      for (auto const& chip : topol->dif_map[dif_id]) {
        unsigned ichip = chip.first;

        // ************* Open XML file ************* //

        wgEditXML xml;

        std::string xmlfile(dif_directory + "/Summary_chip" + std::to_string(ichip) + ".xml");
        try { xml.Open(xmlfile); }
        catch (const std::exception& e) {
          Log.eWrite("[wgPedestalCalib] " + std::string(e.what()));
          return ERR_FAILED_OPEN_XML_FILE;
        }

        // ************* Read XML file ************* //
	  
        for (unsigned ichan = 0; ichan < chip.second; ++ichan) {

#ifdef DEBUG_WG_PEDESTAL_CALIB
          unsigned pe_level_from_xml;
          try { pe_level_from_xml = xml.SUMMARY_GetChFitValue(std::string("pe_level"), ichan); }
          catch (const exception & e) {
            Log.eWrite("failed to read photo electrons equivalent threshold from XML file");
            return ERR_FAILED_OPEN_XML_FILE;
          }
          if ( pe_level_from_dir != pe_level_from_xml ) {
            Log.eWrite("The PEU values read from XML file and from folder name are different");
          }
#endif // DEBUG_WG_PEDESTAL_CALIB

          for (unsigned icol = 0; icol < MEMDEPTH; ++icol) {
            // charge_nohit peak (slighly shifted with respect to the pedestal)
            charge_nohit[dif_id][ichip][ichan][icol][ipe] =
                xml.SUMMARY_GetChFitValue("charge_nohit_" + std::to_string(icol), ichan);
            sigma_nohit [dif_id][ichip][ichan][icol][ipe] =
                xml.SUMMARY_GetChFitValue("sigma_nohit_"  + std::to_string(icol), ichan);
            // charge_HG peak (npe p.e. peak for high gain preamp)
            // Extract the one photo-electron peak and store it in the charge_hit
            // variable. This variable is called like this because it will serve as
            // a reference to calculate the corrected value of the pedestal:
            // corrected pedestal = pedestal reference - gain
            charge_hit[dif_id][ichip][ichan][icol][ipe] =
                xml.SUMMARY_GetChFitValue("charge_hit_" + std::to_string(icol), ichan);
            sigma_hit [dif_id][ichip][ichan][icol][ipe] =
                xml.SUMMARY_GetChFitValue("sigma_hit_"  + std::to_string(icol), ichan);
          }
        }
        xml.Close();
      }
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  //                                   GAIN                                  //
  /////////////////////////////////////////////////////////////////////////////

  gStyle->SetImageScaling(3.);
  
  TCanvas *c1 = new TCanvas("gain_canvas", "Pedestal Calibration Canvas", 1280, 720);
  c1->Divide(n_difs, 2);
  
  // Define the histograms

  std::map<unsigned, TH1D*> h_gain;
  std::map<unsigned, TH2D*> h_gain2D;

  for (auto const& dif : topol->dif_map) {
    unsigned dif_id = dif.first;
    TString name;
    // xbins = 80, xlow = 20, xup = 60 
    name.Form("h_gain_DIF%d", dif_id);
    h_gain[dif_id] = new TH1D(name, name, 80, 20, 60);
    h_gain[dif_id]->SetBit(kCanDelete);
    // xbins = 20, xlow = 0, xup = 20, ybins = 40, ylow = 0, yup = 80
    name.Form("h_gain2D_DIF%d", dif_id);
    h_gain2D[dif_id] = new TH2D(name, name, 20, 0, 20, 40, 0, 80);
    h_gain2D[dif_id]->SetBit(kCanDelete);
  }

  // ************* Fill the Gain and Gain2D histograms ************* //

  for (auto const& dif : topol->dif_map) {
    unsigned dif_id = dif.first;
    for (auto const& chip : dif.second) {
      unsigned ichip = chip.first;
      for (unsigned ichan = 0; ichan < chip.second; ++ichan) {
        for (unsigned icol = 0; icol < MEMDEPTH; ++icol) {
          // Difference between the 2 p.e. peak and the 1 p.e. peak (i.e. the
          // gain value)
          gain[dif_id][ichip][ichan][icol] =
              charge_hit[dif_id][ichip][ichan][icol][TWO_PE] -
              charge_hit[dif_id][ichip][ichan][icol][ONE_PE];
          sigma_gain[dif_id][ichip][ichan][icol] =
              sqrt(pow(sigma_hit[dif_id][ichip][ichan][icol][TWO_PE], 2) +
                   pow(sigma_hit[dif_id][ichip][ichan][icol][ONE_PE], 2));
          h_gain[dif_id]->Fill(gain[dif_id][ichip][ichan][icol]);
          // fill ichip bin with weight DIST
          h_gain2D[dif_id]->Fill(ichip, gain[dif_id][ichip][ichan][icol]);
        }
      }
    }
  }
  
  // Print the histograms

  {
    unsigned idif = 0;
    for (auto const& dif : topol->dif_map) {
      unsigned dif_id = dif.first;
      c1->cd(idif * 2 + 1);
      c1->GetPad(idif * 2 + 1)->SetLogy(1);
      h_gain[dif_id]->Draw();
      c1->cd(idif * 2 + 2);
      c1->GetPad(idif * 2 + 2)->SetLogy(0);
      c1->SetLogz(1);
      h_gain2D[dif_id]->Draw("colz");
      ++idif;
    }
    TString name;
    name = output_img_dir + "/gain.png";
    c1->Print(name);
    delete c1;
  }
  
  ///////////////////////////////////////////////////////////////////////////
  //                                PEDESTAL                               //
  ///////////////////////////////////////////////////////////////////////////

  TCanvas* c2 = new TCanvas("pedestal_canvas", "Pedestal Calibration Canvas", 1280, 720);
  c2->Divide(4,4);
  c2->cd();
  TH1D *h_corrected_ped[MEMDEPTH];
  TH1D *h_ped_shift[MEMDEPTH];

  for (unsigned icol = 0; icol < MEMDEPTH; icol++) {
    TString name;
    // xbins = 300, xlow = 400, xup = 700
    name.Form("h_corrected_ped_%d", icol);
    h_corrected_ped[icol] = new TH1D(name, "h_corrected_ped",
                                     std::abs(WG_END_CHARGE_NOHIT - WG_BEGIN_CHARGE_NOHIT),
                                     WG_BEGIN_CHARGE_NOHIT, WG_END_CHARGE_NOHIT);
    h_corrected_ped[icol]->SetBit(kCanDelete);
    // xbins = 100, xlow = -50, xup = 50
    name.Form("h_ped_shift_%d", icol);
    h_ped_shift[icol] = new TH1D(name, "h_ped_shift",
                                 std::abs(WG_PED_DIFF_MAX - WG_PED_DIFF_MIN),
                                 WG_PED_DIFF_MIN, WG_PED_DIFF_MAX);
    h_ped_shift[icol]->SetBit(kCanDelete);
    h_corrected_ped[icol]->SetLineColor(kBlue);
    h_ped_shift[icol]->SetLineColor(kRed);
    name.Form("pedestal shift col%d;adc count;nEntry", icol);
    h_ped_shift[icol]->SetTitle(name);
  }

  TCanvas* c3 = new TCanvas("pedestal_global_canvas", "Pedestal Calibration Canvas", 1280, 720);
  c3->cd();
  // xbins = 60, xlow = -50, xup = 10
  TH1D *h_ped_shift_global = new TH1D("h_ped_shift_global","h_ped_shift_global",
                                      std::abs(WG_PED_DIFF_MAX - WG_PED_DIFF_MIN),
                                      WG_PED_DIFF_MIN, WG_PED_DIFF_MAX);
  h_ped_shift_global->SetBit(kCanDelete);
  h_ped_shift_global->SetTitle("pedestal shift;adc count;nEntry");
  
  /********************** PEDESTAL_CARD.XML ************************/

  wgEditXML xml;
  
  std::string xmlfile = output_xml_dir + "/pedestal_card.xml";
  xml.Pedestal_Make(xmlfile, *topol);
  try { xml.Open(xmlfile); }
  catch (const std::exception& e) {
    Log.eWrite("[wgPedestalCalib] " + std::string(e.what()));
    return ERR_FAILED_OPEN_XML_FILE;
  }

  for (auto const& dif : topol->dif_map) {
    unsigned dif_id = dif.first;
    for (auto const& chip : dif.second) {
      unsigned ichip = chip.first;
      for (unsigned ichan = 0; ichan < chip.second; ++ichan) {
        for (unsigned icol = 0; icol < MEMDEPTH; ++icol) {
          xml.Pedestal_SetChanValue("pe1_"        + std::to_string(icol), dif_id, ichip, ichan,
                                     charge_hit[dif_id][ichip][ichan][icol][ONE_PE],
                                     NO_CREATE_NEW_MODE);
          xml.Pedestal_SetChanValue("pe2_"        + std::to_string(icol), dif_id, ichip, ichan,
                                     charge_hit[dif_id][ichip][ichan][icol][TWO_PE],
                                     NO_CREATE_NEW_MODE);
          xml.Pedestal_SetChanValue("gain_"       + std::to_string(icol), dif_id, ichip, ichan,
                                     gain      [dif_id][ichip][ichan][icol],
                                     NO_CREATE_NEW_MODE);
          xml.Pedestal_SetChanValue("sigma_gain_" + std::to_string(icol), dif_id, ichip, ichan,
                                     sigma_gain[dif_id][ichip][ichan][icol],
                                     NO_CREATE_NEW_MODE);

          // corrected_pedestal = 1 p.e. peak - gain
          // measured_pedestal = raw pedestal when there is no hit
          int corrected_pedestal       = charge_hit  [dif_id][ichip][ichan][icol][ONE_PE] -
                                         gain[dif_id][ichip][ichan][icol];
          int measured_pedestal        = charge_nohit[dif_id][ichip][ichan][icol][ONE_PE];
          int sigma_corrected_pedestal = sqrt(pow(sigma_hit[dif_id][ichip][ichan][icol][ONE_PE], 2) -
                                              pow(gain[dif_id][ichip][ichan][icol], 2));
          int sigma_measured_pedestal  = sigma_nohit[dif_id][ichip][ichan][icol][ONE_PE];
          xml.Pedestal_SetChanValue("ped_"            + std::to_string(icol), dif_id, ichip, ichan,
                                     corrected_pedestal,       NO_CREATE_NEW_MODE);
          xml.Pedestal_SetChanValue("sigma_ped_"      + std::to_string(icol), dif_id, ichip, ichan,
                                     sigma_corrected_pedestal, NO_CREATE_NEW_MODE);
          xml.Pedestal_SetChanValue("meas_ped_"       + std::to_string(icol), dif_id, ichip, ichan,
                                     measured_pedestal,        CREATE_NEW_MODE);
          xml.Pedestal_SetChanValue("sigma_meas_ped_" + std::to_string(icol), dif_id, ichip, ichan,
                                     sigma_measured_pedestal,  CREATE_NEW_MODE);

          h_corrected_ped[icol]->Fill(corrected_pedestal);
          h_ped_shift[icol]->Fill(corrected_pedestal - measured_pedestal);
          h_ped_shift_global->Fill(corrected_pedestal - measured_pedestal);

#ifdef DEBUG_WGANAPEDESTAL
          // If the pedestal (charge_nohit) for one pe threshold and the
          // pedestal for two pe threshold are significantly different, there is
          // something wrong!
          if ( abs(measured_pedestal - charge_nohit[dif_id][ichip][ichan][icol][TWO_PE]) /
               measured_pedestal > PEDESTAL_DIFFERENCE_WARNING_THRESHOLD ) {
            Log.eWrite("[wgPedestalCalib] Difference between 1 pe pedestal_nohit (" +
                       std::to_string(measured_pedestal) + ") and 2 pe pedestal_nohit (" +
                       std::to_string(charge_nohit[dif_id][ichip][ichan][icol][TWO_PE]) +
                       ") is greater than " +
                       std::to_string(int(PEDESTAL_DIFFERENCE_WARNING_THRESHOLD * 100)) + "%");
          }
#endif // DEBUG_WGANAPEDESTAL
        }
      }
    }
  }

  xml.Write();
  xml.Close();

  for (unsigned icol = 0; icol < MEMDEPTH; ++icol) {
    c2->cd(icol + 1);
    h_corrected_ped[icol]->Draw("");
    h_ped_shift[icol]->Draw("");
  }
  TString image(output_img_dir + "/pedestal_shift.png");
  c2->Print(image);
  delete c2;

  c3->cd();
  h_ped_shift_global->Draw("");
  image = output_img_dir + "/pedestal_shift_all.png";
  c3->Print(image);
  delete c3;
  
  return WG_SUCCESS;
}
