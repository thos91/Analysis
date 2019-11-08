// system C++ includes
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <thread>
#include <cmath>

// boost includes
#include <boost/filesystem.hpp>
#include <boost/make_unique.hpp>

// ROOT includes
#include "TMultiGraph.h"
#include "TCanvas.h"
#include "TString.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TVectorD.h"
#include "TError.h"

// user includes
#include "wgConst.hpp"
#include "wgErrorCodes.hpp"
#include "wgNumericTools.hpp"
#include "wgFileSystemTools.hpp"
#include "wgEditXML.hpp"
#include "wgLogger.hpp"
#include "wgFit.hpp"
#include "wgEnableThreadSafety.hpp"
#include "wgGainCalibUgly.hpp"

namespace wg = wagasci_tools;
namespace gc = gain_calib; 

namespace gain_calib {
namespace ugly {
std::vector<std::thread> THREADS;
}
}

//******************************************************************
void wgGainCalibUglyWorker(gc::ugly::GainVector &gain,
                           gc::ugly::GainVector &sigma_gain,
                           const std::string& hist_file,
                           const std::map<unsigned, unsigned>& chip_map);

//******************************************************************
int wgGainCalibUgly(const char * x_input_run_dir,
                    const char * x_xml_config_file,
                    const char * x_output_xml_dir,
                    const char * x_output_img_dir,
                    const bool ignore_wagasci) {
  
  /////////////////////////////////////////////////////////////////////////////
  //                          Check arguments sanity                         //
  /////////////////////////////////////////////////////////////////////////////
  
  std::string input_run_dir (x_input_run_dir);
  std::string xml_config_file(x_xml_config_file);
  std::string output_xml_dir(x_output_xml_dir);
  std::string output_img_dir(x_output_img_dir);
  
  if (input_run_dir.empty() || !wg::check_exist::directory(input_run_dir)) {
    Log.eWrite("[wgGainCalibUgly] input directory doesn't exist");
    return ERR_EMPTY_INPUT_FILE;
  }
  if (xml_config_file.empty() || !wg::check_exist::xml_file(xml_config_file)) {
    Log.eWrite("[wgGainCalibUgly] Pyrame XML configuration file doesn't exist");
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

  Log.Write(" *****  READING DIRECTORY       : " + input_run_dir);
  Log.Write(" *****  READING PYRAME XML      : " + xml_config_file);
  Log.Write(" *****  OUTPUT XML DIRECTORY    : " + output_xml_dir);
  Log.Write(" *****  OUTPUT IMAGE DIRECTORY  : " + output_img_dir);

  gErrorIgnoreLevel = kError;
  wgEnableThreadSafety();
  // TH1::AddDirectory(false);
  gROOT->SetBatch(kTRUE);
 
  /////////////////////////////////////////////////////////////////////////////
  //                               Get topology                              //
  /////////////////////////////////////////////////////////////////////////////

  std::unique_ptr<Topology> topol;
  try {
    topol = boost::make_unique<Topology>(xml_config_file, TopologySourceType::xml_file);
  } catch (const std::exception& except) {
    Log.eWrite("Failed to get topology from the Pyrame XML file (" +
               xml_config_file + ")" + except.what());
    return ERR_TOPOLOGY;
  }
  
  if (ignore_wagasci) {
    topol->dif_map.erase(4);
    topol->dif_map.erase(5);
    topol->dif_map.erase(6);
    topol->dif_map.erase(7);
    topol->n_difs -= 4;
  }

  /////////////////////////////////////////////////////////////////////////////
  //                        List the input DAC values                        //
  /////////////////////////////////////////////////////////////////////////////
  gc::DirList idac_dir_list = wg::list::list_directories(input_run_dir, true);
  std::vector<unsigned> v_idac;
  for (auto const& idac_dir : idac_dir_list) {
    unsigned idac = wg::string::extract_integer(wg::get_stats::basename(idac_dir));
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

  gc::ugly::THREADS.reserve(topol->n_difs * v_idac.size());
  Log.Write("number of threads " + std::to_string(topol->n_difs * v_idac.size()));
  
  gc::ugly::Gain gain;
  gc::ugly::Gain sigma_gain;
  gc::LinearFit slope;
  gc::LinearFit intercept;
  gain_calib::BadChannels bad_channels;
  
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
    Log.eWrite("[wgGainCalibUgly] " + std::string(e.what()));
    return ERR_FAILED_CREATE_DIRECTORY;
  }

  // ============ Create output_img_dir ============ //
  for (auto const& dif: topol->dif_map) {
    unsigned dif_id = dif.first;
    try { wg::make::directory(output_img_dir + "/dif" + std::to_string(dif_id)); }
    catch (const wgInvalidFile& e) {
      Log.eWrite("[wgGainCalibUgly] " + std::string(e.what()));
      return ERR_FAILED_CREATE_DIRECTORY;
    }
  }
  
  /////////////////////////////////////////////////////////////////////////////
  //                              Fit histograms                             //
  /////////////////////////////////////////////////////////////////////////////
  
  // input DAC
  for (auto const& idac_dir : idac_dir_list) {
    unsigned idac = wg::string::extract_integer(wg::get_stats::basename(idac_dir));
    if (idac > MAX_VALUE_8BITS) continue;

    // PEU (fixed to 2)
    gc::DirList peu_dir_list = wg::list::list_directories(idac_dir, true);
    for (auto const& peu_dir : peu_dir_list) {
      unsigned peu_level = wg::string::extract_integer(wg::get_stats::filename(peu_dir));
      if (peu_level != gc::ugly::PEU_LEVEL) continue;

      // DIF
      std::string hist_directory(peu_dir + "/wgMakeHist");
      gc::DirList hist_files = wg::list::list_files(hist_directory, true, ".root");
      if (hist_files.empty()) return ERR_FAILED_GET_FILE_LIST;

      for (const auto& hist_file : hist_files) {
        unsigned dif_id = wg::string::extract_dif_id(hist_file);
        if (ignore_wagasci && dif_id >= 4) continue;

        gc::ugly::THREADS.emplace_back(std::thread {wgGainCalibUglyWorker,
                std::ref(gain[idac][dif_id]), std::ref(sigma_gain[idac][dif_id]),
                hist_file, topol->dif_map[dif_id]});
      }
    }
  }

  for (auto&& thread : gc::ugly::THREADS) {
    thread.join();
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
      unsigned n_chans = chip.second;
      
      auto canvas = new TCanvas("canvas", "inputDAC vs gain", 1280, 720);
      auto multi_graph = new TMultiGraph();
      std::vector<TGraphErrors *> graphs(n_chans);

      // Check for non physical (bad) channels ////////////////////////////////
      
      for (auto const& idac : v_idac) {
        for (unsigned ichan = 0; ichan < n_chans; ++ichan) {
          bad_channels[dif_id][ichip][ichan] =
              wg::numeric::is_unphysical_gain(gain[idac][dif_id][ichip][ichan]);
          bad_channels[dif_id][ichip][ichan] = wg::numeric::is_unphysical_sigma(
              sigma_gain[idac][dif_id][ichip][ichan]);
        }
      }

      // CHANNEL
      for (unsigned ichan = 0; ichan < n_chans; ++ichan) {
        if (bad_channels[dif_id][ichip][ichan]) continue;

        // IDAC
        TVectorD root_gain(v_idac.size());
        TVectorD root_gain_err(v_idac.size());
        for (std::size_t i_idac = 0; i_idac < v_idac.size(); ++i_idac) {
          root_gain    (i_idac) = gain[v_idac[i_idac]][dif_id][ichip][ichan];
          root_gain_err(i_idac) = sigma_gain[v_idac[i_idac]][dif_id][ichip][ichan];
        }
        graphs[ichan] = new TGraphErrors(root_idac, root_gain,
                                         root_idac_err, root_gain_err);
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
      image.Form("%s/dif%d/inputDAC_vs_gain_ugly_chip%d.png",
                 output_img_dir.c_str(), dif_id, ichip);
      canvas->Print(image);
      delete multi_graph;
      delete canvas;
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

  wg::make::bad_channels_file(bad_channels, gain, v_idac,
                              output_xml_dir + "/bad_channels.cvs");

  return WG_SUCCESS;
}

void wgGainCalibUglyWorker(gc::ugly::GainVector &gain,
                           gc::ugly::GainVector &sigma_gain,
                           const std::string& hist_file,
                           const std::map<unsigned, unsigned>& chip_map) {
  if (gain.empty() || sigma_gain.empty()) return;
  
  std::unique_ptr<wgFit> fit;
  try { fit = boost::make_unique<wgFit>(hist_file); }
  catch (const std::exception& except) {
    Log.eWrite("Failed to open the hist.root file (" +
               hist_file + ") : " + except.what());
    gain.clear();
    sigma_gain.clear();
    return;
  }

  // CHIP
  for (auto const &chip : chip_map) {
    unsigned ichip = chip.first;
    unsigned n_chans = chip.second;
          
    // CHANNEL
    for (unsigned ichan = 0; ichan < n_chans; ++ichan) {
      double gain_fit[2];
#ifdef ROOT_HAS_NOT_MINUIT2
      MUTEX.lock();
#endif
      // FIXME: CONSIDER THE OTHER COLUMNS TOO
      // 2 : look for only 2 peaks
      // false : do not plot anything
      // true : do not fit
      try {
        fit->Gain(ichip, ichan, 0, gain_fit, 2, false, true);
      } catch (const wgElementNotFound &except) {
        gain[ichip][ichan] = std::nan("wgElementNotFound");
        sigma_gain[ichip][ichan] = std::nan("wgElementNotFound");
      }
#ifdef ROOT_HAS_NOT_MINUIT2
      MUTEX.unlock();
#endif
      gain[ichip][ichan] = gain_fit[0];
      sigma_gain[ichip][ichan]= gain_fit[1];
    }
  }
}
