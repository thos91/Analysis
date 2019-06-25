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
#include "wgAnaPedestal.hpp"
#include "wgLogger.hpp"

using namespace wagasci_tools;

//******************************************************************
int wgAnaPedestal(const char * x_input_run_dir,
                  const char * x_output_xml_dir,
                  const char * x_output_img_dir)
{
  string input_run_dir (x_input_run_dir);
  string output_xml_dir(x_output_xml_dir);
  string output_img_dir(x_output_img_dir);
  
  wgConst con;
  CheckExist Check;

  if (input_run_dir.empty() || !Check.Dir(input_run_dir)) {
    Log.eWrite("[wgAnaPedestal] input directory doesn't exist");
    return ERR_EMPTY_INPUT_FILE;
  }
  
  if (output_xml_dir.empty()) {
    output_xml_dir = con.CALIBDATA_DIRECTORY;
  }

  // ============ Create output_xml_dir ============ //
  try { MakeDir(output_xml_dir); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[wgAnaPedestal] " + string(e.what()));
    return ERR_CANNOT_CREATE_DIRECTORY;
  }

  // ============ Create output_img_dir ============ //
  try { MakeDir(output_img_dir); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[wgAnaPedestal] " + string(e.what()));
    return ERR_CANNOT_CREATE_DIRECTORY;
  }

  Log.Write(" *****  READING DIRECTORY      :" + GetName(input_run_dir)  + "  *****");
  Log.Write(" *****  OUTPUT XML DIRECTORY   :" + GetName(output_xml_dir) + "  *****");
  Log.Write(" *****  OUTPUT IMAGE DIRECTORY :" + GetName(output_img_dir) + "  *****");
  
  wgEditXML Edit;

  /********************************************************************************
   *                       Get topology from XML files                            *
   ********************************************************************************/

  // The topology for each acquisition (each photo-electron equivalent
  // threshold) MUST be the same. I mean same number of DIFs, chips
  // and channels
  
  Topology topol(input_run_dir, run_directory_tree);
  unsigned n_difs = topol.n_difs;

  /********************************************************************************
   *       Reserve memory for charge/simga_nohit and charge/simga_hit             *
   ********************************************************************************/

  vector<vector<vector<vector<array<int, N_PE>>>>> charge_nohit(n_difs);
  vector<vector<vector<vector<array<int, N_PE>>>>> sigma_nohit (n_difs);
  vector<vector<vector<vector<array<int, N_PE>>>>> charge_hit  (n_difs);
  vector<vector<vector<vector<array<int, N_PE>>>>> sigma_hit   (n_difs);
  vector<vector<vector<array<int, MEMDEPTH>>>> gain            (n_difs);
  vector<vector<vector<array<int, MEMDEPTH>>>> sigma_gain      (n_difs);
  
  for(unsigned idif = 0; idif < n_difs; idif++) {
    unsigned idif_id = idif + 1;
    charge_nohit[idif].resize(topol.dif_map[idif_id].size());
    sigma_nohit [idif].resize(topol.dif_map[idif_id].size());
    charge_hit  [idif].resize(topol.dif_map[idif_id].size());
    sigma_hit   [idif].resize(topol.dif_map[idif_id].size());
    gain        [idif].resize(topol.dif_map[idif_id].size());
    sigma_gain  [idif].resize(topol.dif_map[idif_id].size());
    for(unsigned ichip = 0; ichip < topol.dif_map[idif_id].size(); ichip++) {
      unsigned ichip_id = ichip + 1;
      charge_nohit[idif][ichip].resize(topol.dif_map[idif_id][ichip_id]);
      sigma_nohit [idif][ichip].resize(topol.dif_map[idif_id][ichip_id]);
      charge_hit  [idif][ichip].resize(topol.dif_map[idif_id][ichip_id]);
      sigma_hit   [idif][ichip].resize(topol.dif_map[idif_id][ichip_id]);
      gain        [idif][ichip].resize(topol.dif_map[idif_id][ichip_id]);
      sigma_gain  [idif][ichip].resize(topol.dif_map[idif_id][ichip_id]);
      for(unsigned ichan = 0; ichan < (unsigned) topol.dif_map[idif_id][ichip_id]; ichan++) {
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

  for (auto const & it : run_directory_tree) {
    unsigned ipe = it.first;
    string pe_directory = it.second;

    for(unsigned idif = 0; idif < n_difs; idif++) {
      unsigned idif_id = idif + 1;
      string idif_directory(input_run_dir + pe_directory + "/dif" + to_string(idif_id));
    
      for(unsigned ichip = 0; ichip < topol.dif_map[idif_id].size(); ichip++) {
        unsigned ichip_id = ichip + 1;
        
        // ************* Open XML file ************* //
	  
        string xmlfile(idif_directory + "/Summary_chip" + to_string(ichip_id) + ".xml");
        try { Edit.Open(xmlfile); }
        catch (const exception& e) {
          Log.eWrite("[wgAnaPedestal] " + string(e.what()));
          return ERR_FAILED_OPEN_XML_FILE;
        }

        // ************* Read XML file ************* //
	  
        for(unsigned ichan = 0; ichan < topol.dif_map[idif_id][ichip_id]; ichan++) {
          unsigned ichan_id = ichan + 1;
          
          // ************* Detect the photo electron level ************* //

          unsigned pe_level = 1;
          try { pe_level = Edit.SUMMARY_GetChFitValue(string("pe_level"), ichan_id); }
          catch (const exception & e) {
            Log.eWrite("failed to read photo electrons equivalent threshold from XML file\n");
            return ERR_WRONG_PE_VALUE;
          }
          pe_level--;  // "pe_level" starts from 1 while the local variable "pe" starts from 0
#ifdef DEBUG_WGANAPEDESTAL
          if ( ipe != pe_level ) {
            Log.eWrite("Photo electrons equivalent threshold read from XML file and corrected value are different\n");
          }
#endif // DEBUG_WGANAPEDESTAL

          for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
            unsigned icol_id = icol + 1;
            // charge_nohit peak (slighly shifted with respect to the pedestal)
            charge_nohit[idif][ichip][ichan][icol][ipe] = Edit.SUMMARY_GetChFitValue("charge_nohit_" + to_string(icol_id), ichan_id);
            sigma_nohit [idif][ichip][ichan][icol][ipe] = Edit.SUMMARY_GetChFitValue("sigma_nohit_"  + to_string(icol_id), ichan_id);
            // charge_HG peak (npe p.e. peak for high gain preamp)
            // Extract the one photo-electron peak and store it in the charge_hit
            // variable. This variable is called like this because it will serve as
            // a reference to calculate the corrected value of the pedestal:
            // corrected pedestal = pedestal reference - gain
            charge_hit[idif][ichip][ichan][icol][ipe] = Edit.SUMMARY_GetChFitValue("charge_hit_" + to_string(icol_id), ichan_id);
            sigma_hit [idif][ichip][ichan][icol][ipe] = Edit.SUMMARY_GetChFitValue("sigma_hit_"  + to_string(icol_id), ichan_id);
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

  vector<TH1D*> h_Gain  (n_difs);
  vector<TH2D*> h_Gain2D(n_difs);
  TString name;

  for(unsigned idif = 0; idif < n_difs; idif++) {
    unsigned idif_id = idif + 1;
    // xbins = 80, xlow = 20, xup = 60 
    name.Form("h_Gain_DIF%d", idif_id);
    h_Gain [idif] = new TH1D(name, name, 80, 20, 60);
    // xbins = 20, xlow = 0, xup = 20, ybins = 40, ylow = 0, yup = 80
    name.Form("h_Gain2D_DIF%d", idif_id);
    h_Gain2D[idif] = new TH2D(name, name, 20, 0, 20, 40, 0, 80);
  }

  // ************* Fill the Gain and Gain2D histograms ************* //
  for(unsigned idif = 0; idif < n_difs; idif++) {
    unsigned idif_id = idif + 1;
    for(unsigned ichip = 0; ichip < topol.dif_map[idif_id].size(); ichip++) {
      unsigned ichip_id = ichip + 1;
      for(unsigned ichan = 0; ichan < topol.dif_map[idif_id][ichip_id]; ichan++) {
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
  name = output_img_dir + "/Gain.png";
  c1->Print(name);


  /*************************************************************************
   *                              PEDESTAL                                 *
   *************************************************************************/
  
  TH1D *h_corrected_ped[MEMDEPTH];
  TH1D *h_ped_shift[MEMDEPTH];
  // xbins = 60, xlow = -50, xup = 10
  TH1D *h_ped_shift_global = new TH1D("h_ped_shift_global","h_ped_shift_global", abs(ped_diff_max - ped_diff_min), ped_diff_min, ped_diff_max);
  
  h_ped_shift_global->SetTitle("pedestal shift;adc count;nEntry");
  for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
    unsigned icol_id = icol + 1;
    // xbins = 300, xlow = 400, xup = 700
    name.Form("h_corrected_ped_%d", icol_id);
    h_corrected_ped[icol] = new TH1D(name, "h_corrected_ped", abs(end_ped - begin_ped), begin_ped, end_ped);
    // xbins = 100, xlow = -50, xup = 50
    name.Form("h_ped_shift_%d", icol_id);
    h_ped_shift[icol] = new TH1D(name, "h_ped_shift", abs(ped_diff_max - ped_diff_min), ped_diff_min, ped_diff_max);
    h_corrected_ped[icol]->SetLineColor(kBlue);
    h_ped_shift[icol]->SetLineColor(kRed);
    name.Form("pedestal shift col%d;adc count;nEntry", icol_id);
    h_ped_shift[icol]->SetTitle(name);
  }

  /********************** PEDESTAL_CARD.XML ************************/
  
  string xmlfile = output_xml_dir + "/pedestal_card.xml";
  Edit.Calib_Make(xmlfile, topol);
  try { Edit.Open(xmlfile); }
  catch (const exception& e) {
    Log.eWrite("[wgAnaPedestal] " + string(e.what()));
    return ERR_FAILED_OPEN_XML_FILE;
  }

  for(unsigned idif = 0; idif < n_difs; idif++) {
    unsigned idif_id = idif + 1;
    for(unsigned ichip = 0; ichip < topol.dif_map[idif_id].size(); ichip++) {
      unsigned ichip_id = ichip + 1;
      for(unsigned ichan = 0; ichan < topol.dif_map[idif_id][ichip_id]; ichan++) {
        unsigned ichan_id = ichan + 1;
        for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
          unsigned icol_id = icol + 1;
          Edit.Calib_SetValue("pe1_"        + to_string(icol_id), idif_id, ichip_id, ichan_id, charge_hit[idif][ichip][ichan][icol][ONE_PE], NO_CREATE_NEW_MODE);
          Edit.Calib_SetValue("pe2_"        + to_string(icol_id), idif_id, ichip_id, ichan_id, charge_hit[idif][ichip][ichan][icol][TWO_PE], NO_CREATE_NEW_MODE);
          Edit.Calib_SetValue("gain_"       + to_string(icol_id), idif_id, ichip_id, ichan_id, gain      [idif][ichip][ichan][icol],         NO_CREATE_NEW_MODE);
          Edit.Calib_SetValue("sigma_gain_" + to_string(icol_id), idif_id, ichip_id, ichan_id, sigma_gain[idif][ichip][ichan][icol],         NO_CREATE_NEW_MODE);

          // corrected_pedestal = 1 p.e. peak - gain
          // measured_pedestal = raw pedestal when there is no hit
          int corrected_pedestal       = charge_hit  [idif][ichip][ichan][icol][ONE_PE] - gain[idif][ichip][ichan][icol];
          int measured_pedestal        = charge_nohit[idif][ichip][ichan][icol][ONE_PE];
          int sigma_corrected_pedestal = sqrt(pow(sigma_hit[idif][ichip][ichan][icol][ONE_PE], 2) - pow(gain[idif][ichip][ichan][icol], 2));
          int sigma_measured_pedestal  = sigma_nohit[idif][ichip][ichan][icol][ONE_PE];
          Edit.Calib_SetValue("ped_"            + to_string(icol_id), idif_id, ichip_id, ichan_id, corrected_pedestal,       NO_CREATE_NEW_MODE);
          Edit.Calib_SetValue("sigma_ped_"      + to_string(icol_id), idif_id, ichip_id, ichan_id, sigma_corrected_pedestal, NO_CREATE_NEW_MODE);
          Edit.Calib_SetValue("meas_ped_"       + to_string(icol_id), idif_id, ichip_id, ichan_id, measured_pedestal,        CREATE_NEW_MODE);
          Edit.Calib_SetValue("sigma_meas_ped_" + to_string(icol_id), idif_id, ichip_id, ichan_id, sigma_measured_pedestal,  CREATE_NEW_MODE);

          h_corrected_ped[icol]->Fill(corrected_pedestal);
          h_ped_shift[icol]->Fill(corrected_pedestal - measured_pedestal);
          h_ped_shift_global->Fill(corrected_pedestal - measured_pedestal);

#ifdef DEBUG_WGANAPEDESTAL
          // If the pedestal (charge_nohit) for one pe threshold and the pedestal for two pe threshold are significantly different, there is
          // something wrong!
          if ( abs(measured_pedestal - charge_nohit[idif][ichip][ichan][icol][TWO_PE]) / measured_pedestal > PEDESTAL_DIFFERENCE_WARNING_THRESHOLD ) {
            Log.eWrite("[wgAnaPedestal] Difference between 1 pe pedestal_nohit (" + to_string(measured_pedestal) +
                       ") and 2 pe pedestal_nohit (" + to_string(charge_nohit[idif][ichip][ichan][icol][TWO_PE])+
                       ") is greater than " + to_string(int(PEDESTAL_DIFFERENCE_WARNING_THRESHOLD * 100)) + "%");
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

  return APS_SUCCESS;
}
