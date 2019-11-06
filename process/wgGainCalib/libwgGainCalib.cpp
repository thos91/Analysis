// system C++ includes
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <thread>

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
#include "wgEnableThreadSafety.hpp"
#include "wgFileSystemTools.hpp"
#include "wgEditXML.hpp"
#include "wgLogger.hpp"
#include "wgGainCalib.hpp"

using namespace wagasci_tools;

#define N_IDAC 3

namespace gain_calib {
//           ASU          CHIP    CHANNEL
typedef std::vector <std::vector <double>> GainVector;
//               iDAC           PEU              DIF
typedef std::map<unsigned, std::array <std::map <unsigned, GainVector>, NUM_PE>> Charge;
//          iDAC                    DIF
typedef std::map<unsigned, std::map<unsigned, GainVector>> Gain;
//               DIF            ASU          CHIP    CHANNEL
typedef std::map<unsigned, std::vector <std::vector <double>>> LinearFit;

typedef std::vector<std::string> DirList;

std::vector<std::thread> THREADS;
}

//******************************************************************
int wgGainCalib(const char * x_input_run_dir,
                const char * x_output_xml_dir,
                const char * x_output_img_dir,
                const bool ignore_wallmrd) {
  
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

  gErrorIgnoreLevel = kError;
  wgEnableThreadSafety();
  TH1::AddDirectory(false);
  gROOT->SetBatch(kTRUE);
  
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

  if (ignore_wallmrd) {
    topol->dif_map.erase(0);
    topol->dif_map.erase(1);
    topol->dif_map.erase(2);
    topol->dif_map.erase(3);
    topol->n_difs -= 4;
  }

  /////////////////////////////////////////////////////////////////////////////
  //                        List the input DAC values                        //
  /////////////////////////////////////////////////////////////////////////////
  gain_calib::DirList idac_dir_list = list::list_directories(input_run_dir, true);
  std::vector<unsigned> v_idac;
  for (auto const& idac_directory : idac_dir_list) {
    unsigned idac = string::extract_integer(get_stats::basename(idac_directory));
    if (idac > MAX_VALUE_8BITS) continue;
    else v_idac.push_back(idac);
  }

  /////////////////////////////////////////////////////////////////////////////
  //                             Allocate memory                             //
  /////////////////////////////////////////////////////////////////////////////
  

  gain_calib::Charge    charge_hit;
  gain_calib::Charge    sigma_hit;
  gain_calib::Gain      gain;
  gain_calib::Gain      sigma_gain;
  gain_calib::LinearFit slope;
  gain_calib::LinearFit intercept;

  for (auto const& idac : v_idac) {
    for (unsigned ipeu = 0; ipeu < NUM_PE; ++ipeu) {
      for (auto const& dif: topol->dif_map) {
        unsigned dif_id = dif.first;
        unsigned n_chips = dif.second.size();
        charge_hit[idac][ipeu][dif_id].resize(n_chips);
        sigma_hit [dif_id][ipeu][dif_id].resize(n_chips);
        for (auto const& chip: dif.second) {
          unsigned ichip = chip.first;
          unsigned n_chans = topol->dif_map[dif_id][ichip];
          charge_hit[idac][ipeu][dif_id][ichip].resize(n_chans);
          sigma_hit [idac][ipeu][dif_id][ichip].resize(n_chans);
        }
      }
    }
  }

  for (auto const& idac : v_idac) {
    for (auto const& dif: topol->dif_map) {
      unsigned dif_id = dif.first;
      unsigned n_chips = dif.second.size();
      gain        [idac][dif_id].resize(n_chips);
      sigma_gain  [idac][dif_id].resize(n_chips);
      for (auto const& chip: dif.second) {
        unsigned ichip = chip.first;
        unsigned n_chans = topol->dif_map[dif_id][ichip];
        gain        [idac][dif_id][ichip].resize(n_chans);
        sigma_gain  [idac][dif_id][ichip].resize(n_chans);
      }
    }
  }
     
  for (auto const& dif: topol->dif_map) {
    unsigned dif_id = dif.first;
    unsigned n_chips = dif.second.size();
    slope       [dif_id].resize(n_chips);
    intercept   [dif_id].resize(n_chips);
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
  try { make::directory(output_xml_dir); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[wgGainCalib] " + std::string(e.what()));
    return ERR_FAILED_CREATE_DIRECTORY;
  }

  // ============ Create output_img_dir ============ //
  for (auto const& dif: topol->dif_map) {
    unsigned dif_id = dif.first;
    for (auto const& chip: dif.second) {
      unsigned ichip = chip.first;
      try { make::directory(output_img_dir + "/dif" + std::to_string(dif_id) +
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
  for (auto const& idac_directory : idac_dir_list) {
    unsigned idac = string::extract_integer(get_stats::basename(idac_directory));
    if (idac > MAX_VALUE_8BITS) continue;
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
        unsigned dif_id = dif.first;
        std::string dif_id_directory(pe_directory + "/wgAnaHistSummary/Xml/dif" +
                                   std::to_string(dif_id));
        // Chip
        for (auto const& chip: dif.second) {
          unsigned ichip = chip.first;
        
          // ************* Open XML file ************* //
          
          std::string xmlfile(dif_id_directory + "/Summary_chip" +
                              std::to_string(ichip) + ".xml");
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
            try {
              pe_level_from_xml = Edit.SUMMARY_GetChFitValue(std::string("pe_level"), ichan);
            } catch (const std::exception & e) {
              Log.eWrite("failed to read photo electrons equivalent threshold from XML file");
              return ERR_FAILED_OPEN_XML_FILE;
            }
            if ( pe_level_from_dir != pe_level_from_xml ) {
              Log.eWrite("The PEU values read from XML file and "
                         "from folder name are different");
            }
#endif // DEBUG_WG_GAIN_CALIB

            double charge_mean = 0, sigma_mean = 0;
            for (unsigned icol = 0; icol < MEMDEPTH; ++icol) {
              // charge_HG peak (npe p.e. peak for high gain preamp)
              charge_mean +=
                  Edit.SUMMARY_GetChFitValue("charge_hit_" + std::to_string(icol), ichan);
              sigma_mean +=
                  TMath::Power(Edit.SUMMARY_GetChFitValue("sigma_hit_" + std::to_string(icol),
                                                          ichan), 2);
            }
            charge_hit[idac][ipe][dif_id][ichip][ichan] = charge_mean / (double) MEMDEPTH;
            sigma_hit [idac][ipe][dif_id][ichip][ichan] = TMath::Sqrt(sigma_mean);
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

  // DIF
  for (auto const& dif: topol->dif_map) {
    unsigned dif_id = dif.first;

    // CHIP
    for (auto const& chip: dif.second) {
      unsigned ichip = chip.first;
      auto canvas = new TCanvas("canvas", "inputDAC vs gain", 1280, 720);
      auto multi_graph = new TMultiGraph();
      std::array<TGraphErrors *, MEMDEPTH> graphs;
      
      // CHANNEL
      for (unsigned ichan = 0; ichan < chip.second; ++ichan) {
        TVectorD root_gain(v_idac.size());
        TVectorD root_gain_err(v_idac.size());
        for (std::size_t i_idac = 0; i_idac < v_idac.size(); ++i_idac) {
          gain[v_idac[i_idac]][dif_id][ichip][ichan] =
              charge_hit[v_idac[i_idac]][TWO_PE][dif_id][ichip][ichan] -
              charge_hit[v_idac[i_idac]][ONE_PE][dif_id][ichip][ichan];
          sigma_gain[v_idac[i_idac]][dif_id][ichip][ichan]=
              std::sqrt(std::pow(sigma_hit[v_idac[i_idac]][TWO_PE][dif_id][ichip][ichan], 2) +
                        std::pow(sigma_hit[v_idac[i_idac]][ONE_PE][dif_id][ichip][ichan], 2));
          root_gain    (i_idac) = gain      [v_idac[i_idac]][dif_id][ichip][ichan];
          root_gain_err(i_idac) = sigma_gain[v_idac[i_idac]][dif_id][ichip][ichan];
        }
        graphs[ichan] = new TGraphErrors(root_idac, root_gain, root_idac_err, root_gain_err);
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
        TF1 * linear_fit  = graphs[ichan]->GetFunction("pol1");
        linear_fit->SetLineColor(kGreen);
        intercept[dif_id][ichip][ichan] = linear_fit->GetParameter(0);
        slope    [dif_id][ichip][ichan] = linear_fit->GetParameter(1);
          
        multi_graph->Add(graphs[ichan]);
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
      for (auto graph : graphs) delete graph;
      delete multi_graph;
      delete canvas;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  //                              gain_card.xml                              //
  /////////////////////////////////////////////////////////////////////////////

  wgEditXML Edit;

  Edit.GainCalib_Make(output_xml_dir + "/gain_card.xml", *topol);
  Edit.Open(output_xml_dir + "/gain_card.xml");

  for (auto const& dif: topol->dif_map) {
    unsigned dif_id = dif.first;
    for (auto const& chip: dif.second) {
      unsigned ichip = chip.first;
      for (unsigned ichan = 0; ichan < chip.second; ++ichan) {
        for (unsigned icol = 0; icol < MEMDEPTH; ++icol) {
          Edit.GainCalib_SetValue(std::string("slope_gain"),
                                  slope[dif_id][ichip][ichan],
                                  dif_id, ichip, ichan, NO_CREATE_NEW_MODE);
          Edit.GainCalib_SetValue(std::string("intercept_gain"),
                                  intercept[dif_id][ichip][ichan],
                                  dif_id, ichip, ichan, NO_CREATE_NEW_MODE);
        }
      }
    }
  }

  Edit.Write();
  Edit.Close();

  return WG_SUCCESS;
}
