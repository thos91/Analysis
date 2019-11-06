// system C++ includes
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

// boost includes
#include <boost/filesystem.hpp>
#include <boost/make_unique.hpp>

// ROOT includes
#include <TMultiGraph.h>
#include <TCanvas.h>
#include <TString.h>
#include <TGraphErrors.h>
#include <TF1.h>
#include <TVectorD.h>

// user includes
#include "wgErrorCodes.hpp"
#include "wgFileSystemTools.hpp"
#include "wgEditXML.hpp"
#include "wgLogger.hpp"
#include "wgGainCalib.hpp"

using namespace wagasci_tools;

#define N_IDAC 3

namespace gain_calib {
//           DIF         ASU         channel     column     inputDAC   pe
typedef std::vector<std::vector<std::vector<std::array<std::array<std::array<unsigned, NUM_PE>, N_IDAC>, MEMDEPTH>>>> ChargeVector;
typedef std::vector<std::vector<std::vector<std::array<std::array<unsigned, N_IDAC>, MEMDEPTH>>>> GainVector;
typedef std::vector<std::vector<std::vector<std::array<double, MEMDEPTH>>>> LinearFitVector;

typedef std::vector<std::string> DirList;
}
//******************************************************************
int wgGainCalib(const char * x_input_run_dir,
                const char * x_output_xml_dir,
                const char * x_output_img_dir) {
  
  /////////////////////////////////////////////////////////////////////////////
  //                          Check arguments sanity                         //
  /////////////////////////////////////////////////////////////////////////////
  
  std::string input_run_dir (x_input_run_dir);
  std::string output_xml_dir(x_output_xml_dir);
  std::string output_img_dir(x_output_img_dir);
  
  if (input_run_dir.empty() || !check_exist::directory(input_run_dir)) {
    Log.eWrite("[wgGainCalib] input directory doesn't exist");
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

  Log.Write(" *****  READING DIRECTORY      : " + input_run_dir);
  Log.Write(" *****  OUTPUT XML DIRECTORY   : " + output_xml_dir);
  Log.Write(" *****  OUTPUT IMAGE DIRECTORY : " + output_img_dir);
  
  /////////////////////////////////////////////////////////////////////////////
  //                               Get topology                              //
  /////////////////////////////////////////////////////////////////////////////

  std::unique_ptr<Topology> topol;
  try {
    topol = boost::make_unique<Topology>(input_run_dir, TopologySourceType::gain_tree);
  } catch (const std::exception& except) {
    Log.eWrite("Failed to get topology from the gain folder tree (" +
               input_run_dir + ")" + except.what());
    return ERR_TOPOLOGY;
  }

  /////////////////////////////////////////////////////////////////////////////
  //                             Allocate memory                             //
  /////////////////////////////////////////////////////////////////////////////
  

  gain_calib::ChargeVector    charge_hit(topol->n_difs);
  gain_calib::ChargeVector    sigma_hit (topol->n_difs);
  gain_calib::GainVector      gain      (topol->n_difs);
  gain_calib::GainVector      sigma_gain(topol->n_difs);
  gain_calib::LinearFitVector slope     (topol->n_difs);
  gain_calib::LinearFitVector intercept (topol->n_difs);

  for (auto const& dif: topol->dif_map) {
    unsigned idif = dif.first;
    unsigned n_chips = dif.second.size();
    charge_hit  [idif].resize(n_chips);
    sigma_hit   [idif].resize(n_chips);
    gain        [idif].resize(n_chips);
    sigma_gain  [idif].resize(n_chips);
    slope       [idif].resize(n_chips);
    intercept   [idif].resize(n_chips);
    for (auto const& chip: dif.second) {
      unsigned ichip = chip.first;
      unsigned n_chans = topol->dif_map[idif][ichip];
      charge_hit  [idif][ichip].resize(n_chans);
      sigma_hit   [idif][ichip].resize(n_chans);
      gain        [idif][ichip].resize(n_chans);
      sigma_gain  [idif][ichip].resize(n_chans);
      slope       [idif][ichip].resize(n_chans);
      intercept   [idif][ichip].resize(n_chans);
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  //                        Create output directories                        //
  /////////////////////////////////////////////////////////////////////////////

  // ============ Create output_xml_dir ============ //
  try { make::directory(output_xml_dir); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[wgGainCalib] " + std::string(e.what()));
    return ERR_FAILED_CREATE_DIRECTORY;
  }

  // ============ Create output_img_dir ============ //
  for (auto const& dif: topol->dif_map) {
    unsigned idif = dif.first;
    for (auto const& chip: dif.second) {
      unsigned ichip = chip.first;
      try { make::directory(output_img_dir + "/dif" + std::to_string(idif) +
                            "/chip" + std::to_string(ichip)); }
      catch (const wgInvalidFile& e) {
        Log.eWrite("[wgGainCalib] " + std::string(e.what()));
        return ERR_FAILED_CREATE_DIRECTORY;
      }
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  //                       Read Summary_chipX.xml files                      //
  /////////////////////////////////////////////////////////////////////////////

  // input DAC
  gain_calib::DirList idac_dir_list = list::list_directories(input_run_dir, true);
  std::vector<unsigned> v_idac;
  for (auto const& idac_directory : idac_dir_list) {

    unsigned i_idac = string::extract_integer(get_stats::basename(idac_directory));
    if (i_idac > MAX_VALUE_8BITS) continue;
    else v_idac.push_back(i_idac);

    // PEU
    gain_calib::DirList pe_dir_list = list::list_directories(idac_directory, true);
    for (auto const& pe_directory : list::list_directories(input_run_dir, true)) {
      unsigned pe_level_from_dir = string::extract_integer(get_stats::basename(pe_directory));
      unsigned ipe;
      if      (pe_level_from_dir == 1) ipe = ONE_PE;
      else if (pe_level_from_dir == 2) ipe = TWO_PE;
      else continue;
      // DIF
      for (auto const& dif: topol->dif_map) {
        unsigned idif = dif.first;
        std::string idif_directory(pe_directory + "/wgAnaHistSummary/Xml/dif" + std::to_string(idif));
        // Chip
        for (auto const& chip: dif.second) {
          unsigned ichip = chip.first;
        
          // ************* Open XML file ************* //
          
          std::string xmlfile(idif_directory + "/Summary_chip" + std::to_string(ichip) + ".xml");
          wgEditXML Edit;
          try { Edit.Open(xmlfile); }
          catch (const std::exception& e) {
            Log.eWrite("[wgGainCalib] " + std::string(e.what()));
            return ERR_FAILED_OPEN_XML_FILE;
          }

          // ************* Read XML file ************* //
          
          for (unsigned ichan = 0; ichan < chip.second; ++ichan) {

#ifdef DEBUG_WG_GAIN_CALIB
            unsigned pe_level_from_xml;
            try { pe_level_from_xml = Edit.SUMMARY_GetChFitValue(std::string("pe_level"), ichan); }
            catch (const std::exception & e) {
              Log.eWrite("failed to read photo electrons equivalent threshold from XML file");
              return ERR_FAILED_OPEN_XML_FILE;
            }
            if ( pe_level_from_dir != pe_level_from_xml ) {
              Log.eWrite("The PEU values read from XML file and from folder name are different");
            }
#endif // DEBUG_WG_GAIN_CALIB

            for (unsigned icol = 0; icol < MEMDEPTH; ++icol) {
              // charge_HG peak (npe p.e. peak for high gain preamp)
              charge_hit[idif][ichip][ichan][icol][i_idac][ipe] =
                  Edit.SUMMARY_GetChFitValue("charge_hit_" + std::to_string(icol), ichan);
              sigma_hit [idif][ichip][ichan][icol][i_idac][ipe] =
                  Edit.SUMMARY_GetChFitValue("sigma_hit_"  + std::to_string(icol), ichan);
            }
          }
          Edit.Close();
        }
      }
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  //                     Draw the inputDAC vs Gain graph                     //
  /////////////////////////////////////////////////////////////////////////////

  std::sort(v_idac.begin(), v_idac.end());
  std::vector<double> double_idac(v_idac.begin(), v_idac.end());
  TVectorD root_idac(double_idac.size(), double_idac.data());
  std::vector<double> v_idac_err(v_idac.size(), 1);
  TVectorD root_idac_err(v_idac.size(), v_idac_err.data());

  for (auto const& dif: topol->dif_map) {
    unsigned idif = dif.first;
    for (auto const& chip: dif.second) {
      unsigned ichip = chip.first;
      for (unsigned ichan = 0; ichan < chip.second; ++ichan) {
        TCanvas * canvas = new TCanvas("canvas", "inputDAC vs gain", 1280, 720);
        TMultiGraph * multi_graph = new TMultiGraph();
        std::array<TGraphErrors *, MEMDEPTH> graphs;
        for (unsigned icol = 0; icol < MEMDEPTH; ++icol) {
          TVectorD root_gain(v_idac.size());
          TVectorD root_gain_err(v_idac.size());
          for (auto const& i_idac : v_idac) {
            gain[idif][ichip][ichan][icol][i_idac] =
                charge_hit[idif][ichip][ichan][icol][i_idac][TWO_PE] -
                charge_hit[idif][ichip][ichan][icol][i_idac][ONE_PE];
            sigma_gain[idif][ichip][ichan][icol][i_idac] =
                std::sqrt(std::pow(sigma_hit[idif][ichip][ichan][icol][i_idac][TWO_PE], 2) +
                          std::pow(sigma_hit[idif][ichip][ichan][icol][i_idac][ONE_PE], 2));
            root_gain    [i_idac] = gain      [idif][ichip][ichan][icol][i_idac];
            root_gain_err[i_idac] = sigma_gain[idif][ichip][ichan][icol][i_idac];
          }
          graphs[icol] = new TGraphErrors(root_idac, root_gain, root_idac_err, root_gain_err);
#ifdef ROOT_HAS_NOT_MINUIT2
          MUTEX.lock();
#endif
          graphs[icol]->Fit("pol1", "QE", "same");
#ifdef ROOT_HAS_NOT_MINUIT2
          MUTEX.unlock();
#endif
          graphs[icol]->SetMarkerColor(632);
          graphs[icol]->SetMarkerSize(1);
          graphs[icol]->SetMarkerStyle(8);
          TF1 * linear_fit  = graphs[icol]->GetFunction("pol1");
          linear_fit->SetLineColor(kGreen);
          intercept[idif][ichip][ichan][icol] = linear_fit->GetParameter(0);
          slope    [idif][ichip][ichan][icol] = linear_fit->GetParameter(1);
          
          multi_graph->Add(graphs[icol]);
        }
        TString title;
        title.Form("chan%d;inputDAC;gain", ichan);
        multi_graph->SetTitle(title);
        multi_graph->Draw("ap");
        TString image;
        image.Form("%s/dif%d/chip%d/inputDAC_vs_gain_chan%d.png",
                   output_img_dir.c_str(), idif, ichip, ichan);
        canvas->Print(image);
        for (auto graph : graphs) delete graph;
        delete multi_graph;
        delete canvas;
      }
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  //                              gain_card.xml                              //
  /////////////////////////////////////////////////////////////////////////////

  wgEditXML Edit;
  Edit.GainCalib_Make(output_xml_dir + "/gain_card.xml", *topol);
  Edit.Open(output_xml_dir + "/gain_card.xml");

  for (auto const& dif: topol->dif_map) {
    unsigned idif = dif.first;
    for (auto const& chip: dif.second) {
      unsigned ichip = chip.first;
      for (unsigned ichan = 0; ichan < chip.second; ++ichan) {
        for (unsigned icol = 0; icol < MEMDEPTH; ++icol) {
          Edit.GainCalib_SetValue(
              std::string("slope_gain"),     idif, ichip, ichan, icol,
              slope    [idif][ichip][ichan][icol], NO_CREATE_NEW_MODE);
          Edit.GainCalib_SetValue(
              std::string("intercept_gain"), idif, ichip, ichan, icol,
              intercept[idif][ichip][ichan][icol], NO_CREATE_NEW_MODE);
        }
      }
    }
  }

  Edit.Write();
  Edit.Close();

  return WG_SUCCESS;
}
