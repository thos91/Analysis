// system includes
#include <string>
#include <vector>

// system C includes
#include <cstdlib>

// boost includes
#include <boost/filesystem.hpp>

// ROOT includes
#include <TCanvas.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TString.h>

// user includes
#include "wgFileSystemTools.hpp"
#include "wgExceptions.hpp"
#include "wgEditXML.hpp"
#include "wgFitConst.hpp"
#include "wgConst.hpp"
#include "wgLogger.hpp"
#include "wgPedestalCalib.hpp"

using namespace wagasci_tools;

//******************************************************************
int wgPedestalCalib(const char * x_input_run_dir,
                    const char * x_output_xml_dir,
                    const char * x_output_img_dir)
{
  /////////////////////////////////////////////////////////////////////////////
  //                          Check arguments sanity                         //
  /////////////////////////////////////////////////////////////////////////////
  
  std::string input_run_dir (x_input_run_dir);
  std::string output_xml_dir(x_output_xml_dir);
  std::string output_img_dir(x_output_img_dir);
  
  if (input_run_dir.empty() || !check_exist::Dir(input_run_dir)) {
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

  // ============ Create output_xml_dir ============ //
  try { MakeDir(output_xml_dir); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[wgPedestalCalib] " + std::string(e.what()));
    return ERR_FAILED_CREATE_DIRECTORY;
  }

  // ============ Create output_img_dir ============ //
  try { MakeDir(output_img_dir); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[wgPedestalCalib] " + std::string(e.what()));
    return ERR_FAILED_CREATE_DIRECTORY;
  }

  Log.Write(" *****  READING DIRECTORY      : " + input_run_dir);
  Log.Write(" *****  OUTPUT XML DIRECTORY   : " + output_xml_dir);
  Log.Write(" *****  OUTPUT IMAGE DIRECTORY : " + output_img_dir);
  

  /********************************************************************************
   *                       Get topology from XML files                            *
   ********************************************************************************/

  wgEditXML Edit;
  
  // The topology for each acquisition (each photo-electron equivalent
  // threshold) MUST be the same. I mean same number of DIFs, chips
  // and channels
  
  Topology topol(input_run_dir, TopologySourceType::pedestal_tree );
  unsigned n_difs = topol.n_difs;

  /********************************************************************************
   *       Reserve memory for charge/simga_nohit and charge/simga_hit             *
   ********************************************************************************/

  std::vector<std::vector<std::vector<std::vector<std::array<int, N_PE_PEDESTAL_CALIB>>>>> charge_nohit(n_difs);
  std::vector<std::vector<std::vector<std::vector<std::array<int, N_PE_PEDESTAL_CALIB>>>>> sigma_nohit (n_difs);
  std::vector<std::vector<std::vector<std::vector<std::array<int, N_PE_PEDESTAL_CALIB>>>>> charge_hit  (n_difs);
  std::vector<std::vector<std::vector<std::vector<std::array<int, N_PE_PEDESTAL_CALIB>>>>> sigma_hit   (n_difs);
  std::vector<std::vector<std::vector<std::array<int, MEMDEPTH>>>> gain            (n_difs);
  std::vector<std::vector<std::vector<std::array<int, MEMDEPTH>>>> sigma_gain      (n_difs);
  
  for(unsigned idif = 0; idif < n_difs; idif++) {
    charge_nohit[idif].resize(topol.dif_map[idif].size());
    sigma_nohit [idif].resize(topol.dif_map[idif].size());
    charge_hit  [idif].resize(topol.dif_map[idif].size());
    sigma_hit   [idif].resize(topol.dif_map[idif].size());
    gain        [idif].resize(topol.dif_map[idif].size());
    sigma_gain  [idif].resize(topol.dif_map[idif].size());
    for(unsigned ichip = 0; ichip < topol.dif_map[idif].size(); ichip++) {
      charge_nohit[idif][ichip].resize(topol.dif_map[idif][ichip]);
      sigma_nohit [idif][ichip].resize(topol.dif_map[idif][ichip]);
      charge_hit  [idif][ichip].resize(topol.dif_map[idif][ichip]);
      sigma_hit   [idif][ichip].resize(topol.dif_map[idif][ichip]);
      gain        [idif][ichip].resize(topol.dif_map[idif][ichip]);
      sigma_gain  [idif][ichip].resize(topol.dif_map[idif][ichip]);
      for(unsigned ichan = 0; ichan < (unsigned) topol.dif_map[idif][ichip]; ichan++) {
        charge_nohit[idif][ichip][ichan].resize(MEMDEPTH);
        sigma_nohit [idif][ichip][ichan].resize(MEMDEPTH);
        charge_hit  [idif][ichip][ichan].resize(MEMDEPTH);
        sigma_hit   [idif][ichip][ichan].resize(MEMDEPTH);
      }
    }
  }
  
  /********************************************************************************
   *                              Read XML files                                  *
   ********************************************************************************/

  for (auto & pe_directory : ListDirectories(input_run_dir)) {
    unsigned pe_level_from_dir = extractIntegerFromString(GetName(pe_directory));
    unsigned ipe;
    if ( pe_level_from_dir == 1 ) ipe = ONE_PE;
    else if ( pe_level_from_dir == 2 ) ipe = TWO_PE;
    else continue;
    
    // DIF
    for (auto const & idif_directory : ListDirectories(pe_directory + "/wgAnaHistSummary/Xml")) {
      unsigned idif = extractIntegerFromString(GetName(idif_directory));
      
      for(unsigned ichip = 0; ichip < topol.dif_map[idif].size(); ichip++) {
        
        // ************* Open XML file ************* //
	  
        std::string xmlfile(idif_directory + "/Summary_chip" + std::to_string(ichip) + ".xml");
        try { Edit.Open(xmlfile); }
        catch (const std::exception& e) {
          Log.eWrite("[wgPedestalCalib] " + std::string(e.what()));
          return ERR_FAILED_OPEN_XML_FILE;
        }

        // ************* Read XML file ************* //
	  
        for(unsigned ichan = 0; ichan < topol.dif_map[idif][ichip]; ichan++) {

#ifdef DEBUG_WG_PEDESTAL_CALIB
          unsigned pe_level_from_xml;
          try { pe_level_from_xml = Edit.SUMMARY_GetChFitValue(std::string("pe_level"), ichan); }
          catch (const exception & e) {
            Log.eWrite("failed to read photo electrons equivalent threshold from XML file");
            return ERR_FAILED_OPEN_XML_FILE;
          }
          if ( pe_level_from_dir != pe_level_from_xml ) {
            Log.eWrite("The PEU values read from XML file and from folder name are different");
          }
#endif // DEBUG_WG_PEDESTAL_CALIB

          for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
            // charge_nohit peak (slighly shifted with respect to the pedestal)
            charge_nohit[idif][ichip][ichan][icol][ipe] = Edit.SUMMARY_GetChFitValue("charge_nohit_" + std::to_string(icol), ichan);
            sigma_nohit [idif][ichip][ichan][icol][ipe] = Edit.SUMMARY_GetChFitValue("sigma_nohit_"  + std::to_string(icol), ichan);
            // charge_HG peak (npe p.e. peak for high gain preamp)
            // Extract the one photo-electron peak and store it in the charge_hit
            // variable. This variable is called like this because it will serve as
            // a reference to calculate the corrected value of the pedestal:
            // corrected pedestal = pedestal reference - gain
            charge_hit[idif][ichip][ichan][icol][ipe] = Edit.SUMMARY_GetChFitValue("charge_hit_" + std::to_string(icol), ichan);
            sigma_hit [idif][ichip][ichan][icol][ipe] = Edit.SUMMARY_GetChFitValue("sigma_hit_"  + std::to_string(icol), ichan);
          }
        }
        Edit.Close();
      }
    }
  }

  /*************************************************************************
   *                                 GAIN                                  *
   *************************************************************************/

  TCanvas *c1 = new TCanvas("c1", "c1", 1280, 720);
  c1->Divide(n_difs, 2);
  
  // Define the histograms

  std::vector<TH1D*> h_Gain  (n_difs);
  std::vector<TH2D*> h_Gain2D(n_difs);
  TString name;

  for(unsigned idif = 0; idif < n_difs; idif++) {
    // xbins = 80, xlow = 20, xup = 60 
    name.Form("h_Gain_DIF%d", idif);
    h_Gain [idif] = new TH1D(name, name, 80, 20, 60);
    // xbins = 20, xlow = 0, xup = 20, ybins = 40, ylow = 0, yup = 80
    name.Form("h_Gain2D_DIF%d", idif);
    h_Gain2D[idif] = new TH2D(name, name, 20, 0, 20, 40, 0, 80);
  }

  // ************* Fill the Gain and Gain2D histograms ************* //
  for(unsigned idif = 0; idif < n_difs; idif++) {
    for(unsigned ichip = 0; ichip < topol.dif_map[idif].size(); ichip++) {
      for(unsigned ichan = 0; ichan < topol.dif_map[idif][ichip]; ichan++) {
        for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
          // Difference between the 2 p.e. peak and the 1 p.e. peak (i.e. the gain value)
          gain[idif][ichip][ichan][icol] = charge_hit[idif][ichip][ichan][icol][TWO_PE] - charge_hit[idif][ichip][ichan][icol][ONE_PE];
          sigma_gain[idif][ichip][ichan][icol] = sqrt(pow(sigma_hit[idif][ichip][ichan][icol][TWO_PE], 2) +
                                                      pow(sigma_hit[idif][ichip][ichan][icol][ONE_PE], 2));
          h_Gain[idif]->Fill(gain[idif][ichip][ichan][icol]);
          // fill ichip bin with weight DIST
          h_Gain2D[idif]->Fill(ichip, gain[idif][ichip][ichan][icol]);
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
  name = output_img_dir + "/gain.png";
  c1->Print(name);


  /*************************************************************************
   *                              PEDESTAL                                 *
   *************************************************************************/
  
  TH1D *h_corrected_ped[MEMDEPTH];
  TH1D *h_ped_shift[MEMDEPTH];
  // xbins = 60, xlow = -50, xup = 10
  TH1D *h_ped_shift_global = new TH1D("h_ped_shift_global","h_ped_shift_global", std::abs(WG_PED_DIFF_MAX - WG_PED_DIFF_MIN), WG_PED_DIFF_MIN, WG_PED_DIFF_MAX);
  
  h_ped_shift_global->SetTitle("pedestal shift;adc count;nEntry");
  for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
    // xbins = 300, xlow = 400, xup = 700
    name.Form("h_corrected_ped_%d", icol);
    h_corrected_ped[icol] = new TH1D(name, "h_corrected_ped", std::abs(WG_END_CHARGE_NOHIT - WG_BEGIN_CHARGE_NOHIT), WG_BEGIN_CHARGE_NOHIT, WG_END_CHARGE_NOHIT);
    // xbins = 100, xlow = -50, xup = 50
    name.Form("h_ped_shift_%d", icol);
    h_ped_shift[icol] = new TH1D(name, "h_ped_shift", std::abs(WG_PED_DIFF_MAX - WG_PED_DIFF_MIN), WG_PED_DIFF_MIN, WG_PED_DIFF_MAX);
    h_corrected_ped[icol]->SetLineColor(kBlue);
    h_ped_shift[icol]->SetLineColor(kRed);
    name.Form("pedestal shift col%d;adc count;nEntry", icol);
    h_ped_shift[icol]->SetTitle(name);
  }

  /********************** PEDESTAL_CARD.XML ************************/
  
  std::string xmlfile = output_xml_dir + "/pedestal_card.xml";
  Edit.Pedestal_Make(xmlfile, topol);
  try { Edit.Open(xmlfile); }
  catch (const std::exception& e) {
    Log.eWrite("[wgPedestalCalib] " + std::string(e.what()));
    return ERR_FAILED_OPEN_XML_FILE;
  }

  for(unsigned idif = 0; idif < n_difs; idif++) {
    for(unsigned ichip = 0; ichip < topol.dif_map[idif].size(); ichip++) {
      for(unsigned ichan = 0; ichan < topol.dif_map[idif][ichip]; ichan++) {
        for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
          Edit.Pedestal_SetChanValue("pe1_"        + std::to_string(icol), idif, ichip, ichan, charge_hit[idif][ichip][ichan][icol][ONE_PE], NO_CREATE_NEW_MODE);
          Edit.Pedestal_SetChanValue("pe2_"        + std::to_string(icol), idif, ichip, ichan, charge_hit[idif][ichip][ichan][icol][TWO_PE], NO_CREATE_NEW_MODE);
          Edit.Pedestal_SetChanValue("gain_"       + std::to_string(icol), idif, ichip, ichan, gain      [idif][ichip][ichan][icol],         NO_CREATE_NEW_MODE);
          Edit.Pedestal_SetChanValue("sigma_gain_" + std::to_string(icol), idif, ichip, ichan, sigma_gain[idif][ichip][ichan][icol],         NO_CREATE_NEW_MODE);

          // corrected_pedestal = 1 p.e. peak - gain
          // measured_pedestal = raw pedestal when there is no hit
          int corrected_pedestal       = charge_hit  [idif][ichip][ichan][icol][ONE_PE] - gain[idif][ichip][ichan][icol];
          int measured_pedestal        = charge_nohit[idif][ichip][ichan][icol][ONE_PE];
          int sigma_corrected_pedestal = sqrt(pow(sigma_hit[idif][ichip][ichan][icol][ONE_PE], 2) - pow(gain[idif][ichip][ichan][icol], 2));
          int sigma_measured_pedestal  = sigma_nohit[idif][ichip][ichan][icol][ONE_PE];
          Edit.Pedestal_SetChanValue("ped_"            + std::to_string(icol), idif, ichip, ichan, corrected_pedestal,       NO_CREATE_NEW_MODE);
          Edit.Pedestal_SetChanValue("sigma_ped_"      + std::to_string(icol), idif, ichip, ichan, sigma_corrected_pedestal, NO_CREATE_NEW_MODE);
          Edit.Pedestal_SetChanValue("meas_ped_"       + std::to_string(icol), idif, ichip, ichan, measured_pedestal,        CREATE_NEW_MODE);
          Edit.Pedestal_SetChanValue("sigma_meas_ped_" + std::to_string(icol), idif, ichip, ichan, sigma_measured_pedestal,  CREATE_NEW_MODE);

          h_corrected_ped[icol]->Fill(corrected_pedestal);
          h_ped_shift[icol]->Fill(corrected_pedestal - measured_pedestal);
          h_ped_shift_global->Fill(corrected_pedestal - measured_pedestal);

#ifdef DEBUG_WGANAPEDESTAL
          // If the pedestal (charge_nohit) for one pe threshold and the pedestal for two pe threshold are significantly different, there is
          // something wrong!
          if ( abs(measured_pedestal - charge_nohit[idif][ichip][ichan][icol][TWO_PE]) / measured_pedestal > PEDESTAL_DIFFERENCE_WARNING_THRESHOLD ) {
            Log.eWrite("[wgPedestalCalib] Difference between 1 pe pedestal_nohit (" + std::to_string(measured_pedestal) +
                       ") and 2 pe pedestal_nohit (" + std::to_string(charge_nohit[idif][ichip][ichan][icol][TWO_PE])+
                       ") is greater than " + std::to_string(int(PEDESTAL_DIFFERENCE_WARNING_THRESHOLD * 100)) + "%");
          }
#endif // DEBUG_WGANAPEDESTAL
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
    h_corrected_ped[icol]->Draw("");
    h_ped_shift[icol]->Draw("");
  }
  TString image(output_img_dir + "/pedestal_shift.png");
  c1->Print(image);
  delete c1;
  c1=new TCanvas("c1","c1");
  h_ped_shift_global->Draw("");
  image = output_img_dir + "/pedestal_shift_all.png";
  c1->Print(image);

  return WG_SUCCESS;
}
