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

using namespace wagasci_tools;

namespace gain_calib_ugly {

const unsigned PEU_LEVEL = 1;

//           ASU          CHIP    CHANNEL
typedef std::vector <std::vector <double>> GainVector;
//               iDAC               DIF
typedef std::map<unsigned, std::map<unsigned, GainVector>> Gain;
//               DIF            ASU          CHIP    CHANNEL
typedef std::map<unsigned, std::vector <std::vector <double>>> LinearFit;

typedef std::vector<std::string> DirList;

std::vector<std::thread> THREADS;
}

//******************************************************************
void wgGainCalibUglyWorker(gain_calib_ugly::GainVector &gain,
                           gain_calib_ugly::GainVector &sigma_gain,
                           std::string hist_file,
                           std::map<unsigned, unsigned> chip_map);

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
  
  if (input_run_dir.empty() || !check_exist::directory(input_run_dir)) {
    Log.eWrite("[wgGainCalibUgly] input directory doesn't exist");
    return ERR_EMPTY_INPUT_FILE;
  }
  if (xml_config_file.empty() || !check_exist::xml_file(xml_config_file)) {
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
  TH1::AddDirectory(false);
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
  gain_calib_ugly::DirList idac_dir_list = list::list_directories(input_run_dir,
                                                                  true);
  std::vector<unsigned> v_idac;
  for (auto const& idac_directory : idac_dir_list) {
    unsigned idac = string::extract_integer(get_stats::basename(idac_directory));
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

  gain_calib_ugly::THREADS.reserve(topol->n_difs * v_idac.size());
  Log.Write("number of threads " + std::to_string(topol->n_difs * v_idac.size()));
  
  gain_calib_ugly::Gain      gain;
  gain_calib_ugly::Gain      sigma_gain;
  gain_calib_ugly::LinearFit slope;
  gain_calib_ugly::LinearFit intercept;

  for (auto const& idac : v_idac) {
    for (auto const& dif: topol->dif_map) {
      unsigned idif = dif.first;
      unsigned n_chips = dif.second.size();
      gain        [idac][idif].resize(n_chips);
      sigma_gain  [idac][idif].resize(n_chips);
      for (auto const& chip: dif.second) {
        unsigned ichip = chip.first;
        unsigned n_chans = topol->dif_map[idif][ichip];
        gain        [idac][idif][ichip].resize(n_chans);
        sigma_gain  [idac][idif][ichip].resize(n_chans);
      }
    }
  }
  
  for (auto const& dif: topol->dif_map) {
    unsigned idif = dif.first;
    unsigned n_chips = dif.second.size();
    slope       [idif].resize(n_chips);
    intercept   [idif].resize(n_chips);
    for (auto const& chip: dif.second) {
      unsigned ichip = chip.first;
      unsigned n_chans = topol->dif_map[idif][ichip];
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
    Log.eWrite("[wgGainCalibUgly] " + std::string(e.what()));
    return ERR_FAILED_CREATE_DIRECTORY;
  }

  // ============ Create output_img_dir ============ //
  for (auto const& dif: topol->dif_map) {
    unsigned idif = dif.first;
    try { make::directory(output_img_dir + "/dif" + std::to_string(idif)); }
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
    unsigned idac = string::extract_integer(get_stats::basename(idac_dir));
    if (idac > MAX_VALUE_8BITS) continue;

    // PEU (fixed to 2)
    gain_calib_ugly::DirList peu_dir_list = list::list_directories(idac_dir, true);
    for (auto const& peu_dir : peu_dir_list) {
      unsigned peu_level = string::extract_integer(get_stats::filename(peu_dir));
      if (peu_level != gain_calib_ugly::PEU_LEVEL) continue;

      // DIF
      std::string hist_directory(peu_dir + "/wgMakeHist");
      gain_calib_ugly::DirList hist_files = list::list_files(hist_directory,
                                                             true, ".root");
      if (hist_files.empty()) return ERR_FAILED_GET_FILE_LIST;

      for (const auto& hist_file : hist_files) {
        unsigned dif_id = string::extract_dif_id(hist_file);
        if (ignore_wagasci && dif_id >= 4) continue;

        gain_calib_ugly::THREADS.emplace_back(std::thread {wgGainCalibUglyWorker,
                std::ref(gain[idac][dif_id]), std::ref(sigma_gain[idac][dif_id]),
                hist_file, topol->dif_map[dif_id]});
      }
    }
  }

  for(auto&& thread : gain_calib_ugly::THREADS) {
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

      // CHANNEL
      for (unsigned ichan = 0; ichan < n_chans; ++ichan) {
        TVectorD root_gain(v_idac.size());
        TVectorD root_gain_err(v_idac.size());
        for (std::size_t i_idac = 0; i_idac < v_idac.size(); ++i_idac) {
          double not_nan_gain = gain[v_idac[i_idac]][dif_id][ichip][ichan];
          double not_nan_sigma = sigma_gain[v_idac[i_idac]][dif_id][ichip][ichan];
          root_gain    (i_idac) = std::isnan(not_nan_gain) ? 0 : not_nan_gain;
          root_gain_err(i_idac) = std::isnan(not_nan_sigma) ? 0 : not_nan_sigma; 
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
  // true : no need for columns
  Edit.GainCalib_Make(output_xml_dir + "/gain_card.xml", *topol);
  Edit.Open(output_xml_dir + "/gain_card.xml");

  for (auto const& dif: topol->dif_map) {
    unsigned dif_id = dif.first;
    for (auto const& chip: dif.second) {
      unsigned ichip = chip.first;
      for (unsigned ichan = 0; ichan < chip.second; ++ichan) {
        Edit.GainCalib_SetValue(std::string("slope_gain"),
                                slope[dif_id][ichip][ichan],
                                dif_id, ichip, ichan);
        Edit.GainCalib_SetValue(std::string("intercept_gain"),
                                intercept[dif_id][ichip][ichan],
                                dif_id, ichip, ichan);
      }
    }
  }

  Edit.Write();
  Edit.Close();

  return WG_SUCCESS;
}

void wgGainCalibUglyWorker(gain_calib_ugly::GainVector &gain,
                           gain_calib_ugly::GainVector &sigma_gain,
                           std::string hist_file,
                           std::map<unsigned, unsigned> chip_map) {
  if (gain.empty() || sigma_gain.empty()) return;
  
  std::unique_ptr<wgFit> fit;
  try {
    fit =  boost::make_unique<wgFit>(hist_file);
  } catch (const std::exception& except) {
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
      // COLUMN

      // Sometimes the peak searching algorithm is failing
      // because of a doubly split peak or some other strange
      // misbehavior of a particular column. Then the gain
      // (calculated as adiacent peaks substraction) can take
      // strange non physical values. So we take the mean and
      // variance of the gain for each column and remove the
      // gains lying more than 2 standard deviations from the
      // average.
      std::array<double, MEMDEPTH> gain_col, sigma_col;
      for (unsigned icol = 0; icol < MEMDEPTH; ++icol) {
        double gain_fit[2];
#ifdef ROOT_HAS_NOT_MINUIT2
        MUTEX.lock();
#endif
        // 2 : look for only 2 peaks
        // false : do not plot anything
        // true : do not fit
        try {
          fit->Gain(ichip, ichan, icol, gain_fit, 2, false, true);
        } catch (const wgElementNotFound &except) {
          gain_col[icol] = std::nan("wgElementNotFound");
          sigma_col[icol] = std::nan("wgElementNotFound");
        }
#ifdef ROOT_HAS_NOT_MINUIT2
        MUTEX.unlock();
#endif
        gain_col[icol] = gain_fit[0];
        sigma_col[icol] = gain_fit[1];
      }
      double mean = wagasci_tools::numeric::mean(gain_col);
      double std_dev = wagasci_tools::numeric::standard_deviation(gain_col);
      std::vector<double> gain_col_purged, sigma_col_purged;
      for (unsigned icol = 0; icol < MEMDEPTH; ++icol) {
        if (TMath::Abs(gain_col[icol] - mean) <= 2 * std_dev) {
          gain_col_purged.push_back(gain_col[icol]);
        }
      }
      gain[ichip][ichan] = wagasci_tools::numeric::mean(gain_col_purged);
      sigma_gain[ichip][ichan] = std_dev;
      //  std::cout << "mean " << mean << " std_dev " << std_dev << "\n";
      //   std::cout << "gain " << gain[ichip][ichan] << "\n";
    }
  }
}
