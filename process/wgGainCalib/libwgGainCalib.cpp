// system C++ includes
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <bitset>
#include <map>
#include <unordered_map>

// boost includes
#include <boost/filesystem.hpp>
#include <boost/make_unique.hpp>

// ROOT includes
#include "TMultiGraph.h"
#include "TCanvas.h"
#include "TString.h"
#include "TGraphErrors.h"
#include "TH1.h"
#include "TF1.h"
#include "TVectorD.h"

// user includes
#include "wgConst.hpp"
#include "wgErrorCodes.hpp"
#include "wgNumericTools.hpp"
#include "wgFileSystemTools.hpp"
#include "wgEditXML.hpp"
#include "wgLogger.hpp"
#include "wgGainCalib.hpp"

namespace wg = wagasci_tools;

//******************************************************************
int wgGainCalib(const char * x_input_run_dir,
                const char * x_output_xml_dir,
                const char * x_output_img_dir,
                const bool only_wallmrd,
                const bool only_wagasci) {
  
  /////////////////////////////////////////////////////////////////////////////
  //                          Check arguments sanity                         //
  /////////////////////////////////////////////////////////////////////////////
  
  std::string input_run_dir (x_input_run_dir);
  std::string output_xml_dir(x_output_xml_dir);
  std::string output_img_dir(x_output_img_dir);

  if (only_wagasci & only_wallmrd) {
    Log.eWrite("[wgGainCalib] the only-wagasci and only-wallmrd flags "
               "are mutually exclusive");
    return ERR_WRONG_MODE;
  }
  if (input_run_dir.empty() || !wg::check_exist::directory(input_run_dir)) {
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

  gErrorIgnoreLevel = kError;
  gROOT->SetBatch(kTRUE);
  
  /////////////////////////////////////////////////////////////////////////////
  //                               Get topology                              //
  /////////////////////////////////////////////////////////////////////////////

  std::unique_ptr<Topology> topol;
  try {
    topol = boost::make_unique<Topology>(input_run_dir,
                                         TopologySourceType::gain_tree);
  } catch (const std::exception& except) {
    Log.eWrite("Failed to get topology from the gain folder tree (" +
               input_run_dir + ")" + except.what());
    return ERR_TOPOLOGY;
  }

  if (only_wagasci) {
    topol->dif_map.erase(0); // WallMRD north top
    topol->dif_map.erase(1); // WallMRD north bottom
    topol->dif_map.erase(2); // WallMRD south top
    topol->dif_map.erase(3); // WallMRD south bottom
    topol->n_difs -= 4;
  } else if (only_wallmrd) {
    topol->dif_map.erase(4); // WAGASCI upstream top
    topol->dif_map.erase(5); // WAGASCI upstream side
    topol->dif_map.erase(6); // WAGASCI downstream top
    topol->dif_map.erase(7); // WAGASCI downstream side
    topol->n_difs -= 4;
  }

  /////////////////////////////////////////////////////////////////////////////
  //                        List the input DAC values                        //
  /////////////////////////////////////////////////////////////////////////////
  
  gain_calib::DirList idac_dir_list = wg::list::list_directories(input_run_dir,
                                                                 true);
  std::vector<unsigned> v_idac;
  for (auto const& idac_directory : idac_dir_list) {
    unsigned idac =
        wg::string::extract_integer(wg::get_stats::basename(idac_directory));
    if (idac > MAX_VALUE_8BITS) continue;
    else v_idac.push_back(idac);
  }
  if (v_idac.size() <= 0) {
    Log.eWrite("No iDAC folder found in the gain folder tree : " + input_run_dir);
    return ERR_INPUT_FILE_NOT_FOUND;
  }

  /////////////////////////////////////////////////////////////////////////////
  //                             Allocate memory                             //
  /////////////////////////////////////////////////////////////////////////////

  gain_calib::Charge    charge_hit;
  gain_calib::Charge    sigma_hit;
  gain_calib::LinearFit slope;
  gain_calib::LinearFit intercept;
  gain_calib::BadChannels bad_channels;
  
  for (auto const& idac : v_idac) {
    for (unsigned ipeu = 0; ipeu < NUM_PE; ++ipeu) {
      for (auto const& dif: topol->dif_map) {
        unsigned dif_id = dif.first;
        unsigned n_chips = dif.second.size();
        charge_hit[idac][ipeu][dif_id].resize(n_chips);
        sigma_hit [idac][ipeu][dif_id].resize(n_chips);
        for (auto const& chip: dif.second) {
          unsigned ichip = chip.first;
          unsigned n_chans = topol->dif_map[dif_id][ichip];
          charge_hit[idac][ipeu][dif_id][ichip].resize(n_chans);
          sigma_hit [idac][ipeu][dif_id][ichip].resize(n_chans);
        }
      }
    }
  }
     
  for (auto const& dif: topol->dif_map) {
    unsigned dif_id = dif.first;
    unsigned n_chips = dif.second.size();
    slope       [dif_id].resize(n_chips);
    intercept   [dif_id].resize(n_chips);
    bad_channels[dif_id].resize(n_chips);
    for (auto const& chip: dif.second) {
      unsigned ichip = chip.first;
      unsigned n_chans = topol->dif_map[dif_id][ichip];
      slope       [dif_id][ichip].resize(n_chans);
      intercept   [dif_id][ichip].resize(n_chans);
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  //                        Create output directories                        //
  /////////////////////////////////////////////////////////////////////////////

  // ============ Create output_xml_dir ============ //
  try { wg::make::directory(output_xml_dir); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[wgGainCalib] " + std::string(e.what()));
    return ERR_FAILED_CREATE_DIRECTORY;
  }

  // ============ Create output_img_dir ============ //
  for (auto const& dif: topol->dif_map) {
    unsigned dif_id = dif.first;
    try { wg::make::directory(output_img_dir + "/dif" + std::to_string(dif_id)); }
    catch (const wgInvalidFile& e) {
      Log.eWrite("[wgGainCalib] " + std::string(e.what()));
      return ERR_FAILED_CREATE_DIRECTORY;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  //                       Read Summary_chipX.xml files                      //
  /////////////////////////////////////////////////////////////////////////////

  // input DAC
  for (auto const& idac_dir : idac_dir_list) {
    unsigned idac =
        wg::string::extract_integer(wg::get_stats::basename(idac_dir));
    if (idac > MAX_VALUE_8BITS) continue;
    
    // PEU
    gain_calib::DirList pe_dir_list = wg::list::list_directories(idac_dir, true);
    for (auto const& peu_dir : pe_dir_list) {
      unsigned pe_level_from_dir =
          wg::string::extract_integer(wg::get_stats::basename(peu_dir));
      unsigned ipe = -1;
      if      (pe_level_from_dir == 1) ipe = ONE_PE;
      else if (pe_level_from_dir == 2) ipe = TWO_PE;
      else continue;

      // DIF
      for (auto const& dif: topol->dif_map) {
        unsigned dif_id = dif.first;
        if (only_wagasci && dif_id < 4) continue;
        if (only_wallmrd && dif_id >= 4) continue;
        std::string dif_id_directory(peu_dir + "/wgAnaHistSummary/Xml/dif_" +
                                     std::to_string(dif_id));
        
        // Chip
        for (auto const& chip: dif.second) {
          unsigned ichip = chip.first;
        
          // ************* Open XML file ************* //
          
          std::string xmlfile(dif_id_directory + "/Summary_chip" +
                              std::to_string(ichip) + ".xml");
          wgEditXML xml;
          try { xml.Open(xmlfile); }
          catch (const std::exception& e) {
            Log.eWrite("[wgGainCalib] " + std::string(e.what()));
            return ERR_FAILED_OPEN_XML_FILE;
          }

          // ************* Read XML file ************* //
          
          for (unsigned ichan = 0; ichan < chip.second; ++ichan) {

#ifdef DEBUG_WG_GAIN_CALIB
            unsigned pe_level_from_xml;
            try {
              pe_level_from_xml =
                  xml.SUMMARY_GetChFitValue(std::string("pe_level"), ichan);
            } catch (const std::exception & e) {
              Log.eWrite("failed to read photo electrons equivalent "
                         "threshold from XML file");
              return ERR_FAILED_OPEN_XML_FILE;
            }
            if (pe_level_from_dir != pe_level_from_xml) {
              Log.eWrite("The PEU values read from XML file and "
                         "from folder name are different");
            }
#endif // DEBUG_WG_GAIN_CALIB

            // Among the first 5 columns, choose the column with the highest
            // charge. If the threshold calibration is not perfect, sometimes
            // lower PEU peaks crawl back in the histograms. If nothing is found
            // in the first 5 columns, use the next 5.
            int max_charge = 0;
            unsigned max_col = 5;
            while (max_charge == 0 && max_col < 15) {
              for (unsigned icol = 0; icol < max_col; ++icol) {
                int tmp = xml.SUMMARY_GetChFitValue(
                    "charge_hit_" + std::to_string(icol), ichan);
                if (!isnan(tmp) && tmp > max_charge) {
                  max_charge = tmp;
                  max_col = icol;
                }
              }
              if (max_charge == 0) max_col += 5;
            }
            if (max_charge == 0) max_charge = -1;
            
            charge_hit[idac][ipe][dif_id][ichip][ichan] = max_charge;
            sigma_hit [idac][ipe][dif_id][ichip][ichan] = xml.SUMMARY_GetChFitValue(
                "sigma_hit_" + std::to_string(max_col), ichan);
          }
          xml.Close();
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
  std::vector<double> double_idac_err(v_idac.size(), 1);
  TVectorD root_idac_err(v_idac.size(), double_idac_err.data());

  // DIF
  for (auto const& dif: topol->dif_map) {
    unsigned dif_id = dif.first;

    // CHIP
    for (auto const& chip: dif.second) {
      unsigned ichip = chip.first;
      unsigned n_chans = chip.second;
      
      std::unique_ptr<TCanvas> canvas(new TCanvas("canvas", "inputDAC vs gain",
                                                  1280, 720));
      std::unique_ptr<TMultiGraph> multi_graph(new TMultiGraph());
      multi_graph->SetMinimum(gain_calib::MIN_GAIN);
      multi_graph->SetMaximum(gain_calib::MAX_GAIN);
      std::vector<std::unique_ptr<TGraphErrors>> graphs(n_chans);

      // Check for non physical (bad) channels ////////////////////////////////
      
      for (auto const& idac : v_idac) {
        for (unsigned ichan = 0; ichan < n_chans; ++ichan) {
          if (!Topology::IsWallMRDChannelEnabled(dif_id, ichip, ichan))
            continue;
          double charge_2pe = charge_hit[idac][TWO_PE][dif_id][ichip][ichan];
          double charge_1pe = charge_hit[idac][ONE_PE][dif_id][ichip][ichan];
          double sigma_2pe = sigma_hit  [idac][TWO_PE][dif_id][ichip][ichan];
          double sigma_1pe = sigma_hit  [idac][ONE_PE][dif_id][ichip][ichan];
          bad_channels[dif_id][ichip][ichan] =
              wg::numeric::is_unphysical_gain(charge_1pe, charge_2pe);
          bad_channels[dif_id][ichip][ichan] =
              wg::numeric::is_unphysical_sigma(sigma_1pe, sigma_2pe);
        }
      }

      // CHANNEL
      for (unsigned ichan = 0; ichan < chip.second; ++ichan) {
        if (bad_channels[dif_id][ichip][ichan] ||
            !Topology::IsWallMRDChannelEnabled(dif_id, ichip, ichan))
          continue;
        
        // IDAC
        TVectorD root_gain(v_idac.size());
        TVectorD root_gain_err(v_idac.size());
        for (std::size_t i_idac = 0; i_idac < v_idac.size(); ++i_idac) {
          double charge_2pe =
              charge_hit[v_idac[i_idac]][TWO_PE][dif_id][ichip][ichan];
          double charge_1pe =
              charge_hit[v_idac[i_idac]][ONE_PE][dif_id][ichip][ichan];
          double sigma_2pe =
              sigma_hit[v_idac[i_idac]][TWO_PE][dif_id][ichip][ichan];
          double sigma_1pe =
              sigma_hit[v_idac[i_idac]][ONE_PE][dif_id][ichip][ichan];
          root_gain    (i_idac) = charge_2pe - charge_1pe;
          root_gain_err(i_idac) = std::sqrt(std::pow(sigma_1pe, 2) +
                                            std::pow(sigma_2pe, 2));
        }

        std::unique_ptr<TGraphErrors> graph(
            new TGraphErrors(root_idac, root_gain, root_idac_err, root_gain_err));
        graphs[ichan] = std::move(graph);
        
        if (graphs[ichan] == nullptr) {
          std::stringstream ss;
          ss << "Failed to create TGraphErrors for dif " << dif_id <<
              " chip " << ichip << " channel " << ichan;
          Log.eWrite(ss.str());
          return ERR_WRONG_CHANNEL_VALUE;
        }
        
#ifdef ROOT_HAS_NOT_MINUIT2
        MUTEX.lock();
#endif
        graphs[ichan]->Fit("pol1", "QE", "same");
#ifdef ROOT_HAS_NOT_MINUIT2
        MUTEX.unlock();
#endif
        graphs[ichan]->SetMarkerColor(632);
        graphs[ichan]->SetMarkerSize(1);
        graphs[ichan]->SetMarkerStyle(8);
        auto linear_fit = graphs[ichan]->GetFunction("pol1");
        linear_fit->SetLineColor(kGreen);
        intercept[dif_id][ichip][ichan] = linear_fit->GetParameter(0);
        slope    [dif_id][ichip][ichan] = linear_fit->GetParameter(1);
          
        multi_graph->Add(graphs[ichan].get());
      }
      
      TString title;
      title.Form("chip%d;inputDAC;gain", ichip);
      multi_graph->SetTitle(title);
      multi_graph->Draw("ap");
      canvas->Update();
      canvas->Modified();
      TString image;
      image.Form("%s/dif%d/inputDAC_vs_gain_chip%d.png",
                 output_img_dir.c_str(), dif_id, ichip);
      canvas->Print(image);
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  //                              gain_card.xml                              //
  /////////////////////////////////////////////////////////////////////////////

  wgEditXML xml;

  xml.GainCalib_Make(output_xml_dir + "/gain_card.xml", *topol);
  xml.Open(output_xml_dir + "/gain_card.xml");

  // DIF
  for (auto const& dif: topol->dif_map) {
    unsigned dif_id = dif.first;
    // CHIP
    for (auto const& chip: dif.second) {
      unsigned ichip = chip.first;
      double slope_mean = wg::numeric::mean(slope[dif_id][ichip]);
      double intercept_mean = wg::numeric::mean(intercept[dif_id][ichip]);
      // CHANNEL
      for (unsigned ichan = 0; ichan < chip.second; ++ichan) {
        if (!Topology::IsWallMRDChannelEnabled(dif_id, ichip, ichan))
          continue;
        if (bad_channels[dif_id][ichip][ichan]) {
          slope[dif_id][ichip][ichan] = slope_mean;
          intercept[dif_id][ichip][ichan] = intercept_mean;
        }
        xml.GainCalib_SetValue(std::string("slope_gain"),
                               slope[dif_id][ichip][ichan],
                               dif_id, ichip, ichan);
        xml.GainCalib_SetValue(std::string("intercept_gain"),
                               intercept[dif_id][ichip][ichan],
                               dif_id, ichip, ichan);
      }
    }
  }

  xml.Write();
  xml.Close();

  wg::make::bad_channels_file(bad_channels, charge_hit, v_idac,
                              output_xml_dir + "/bad_channels.cvs");
  
  return WG_SUCCESS;
}
