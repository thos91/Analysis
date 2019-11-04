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
#include "wgFit.hpp"
#include "wgGainCalibUgly.hpp"

using namespace wagasci_tools;

#define N_IDAC 3

namespace gain_calib_ugly {
//           DIF         ASU         channel     column     inputDAC   pe
typedef std::vector<std::vector<std::vector<std::array<std::array<std::array<unsigned, NUM_PE>, N_IDAC>, MEMDEPTH>>>> ChargeVector;
typedef std::vector<std::vector<std::vector<std::array<std::array<unsigned, N_IDAC>, MEMDEPTH>>>> GainVector;
typedef std::vector<std::vector<std::vector<std::array<double, MEMDEPTH>>>> LinearFitVector;

typedef std::vector<std::string> DirList;
}
//******************************************************************
int wgGainCalibUgly(const char * x_hist_dir,
                    const char * x_xml_config_file,  
                    const char * x_output_xml_dir,
                    const char * x_output_img_dir) {
  
  /////////////////////////////////////////////////////////////////////////////
  //                          Check arguments sanity                         //
  /////////////////////////////////////////////////////////////////////////////
  
  std::string hist_dir (x_hist_dir);
  std::string xml_config_file (x_hist_dir);
  std::string output_xml_dir(x_output_xml_dir);
  std::string output_img_dir(x_output_img_dir);
  
  if (hist_dir.empty() || !check_exist::directory(hist_dir)) {
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

  Log.Write(" *****  READING DIRECTORY       : " + hist_dir);
  Log.Write(" *****  READING PYRAME XML FILE : " + xml_config_file);
  Log.Write(" *****  OUTPUT XML DIRECTORY    : " + output_xml_dir);
  Log.Write(" *****  OUTPUT IMAGE DIRECTORY  : " + output_img_dir);
  
  /////////////////////////////////////////////////////////////////////////////
  //                               Get topology                              //
  /////////////////////////////////////////////////////////////////////////////

  std::unique_ptr<Topology> topol;
  try {
    topol = boost::make_unique<Topology>(xml_config_file, TopologySourceType::xml_file);
  } catch (const std::exception& except) {
    Log.eWrite("Failed to get topology from Pyrame XML file (" +
               xml_config_file + ")" + except.what());
    return ERR_TOPOLOGY;
  }

  /////////////////////////////////////////////////////////////////////////////
  //                             Allocate memory                             //
  /////////////////////////////////////////////////////////////////////////////
  
  gain_calib_ugly::ChargeVector    charge_hit(topol->n_difs);
  gain_calib_ugly::ChargeVector    sigma_hit (topol->n_difs);
  gain_calib_ugly::GainVector      gain      (topol->n_difs);
  gain_calib_ugly::GainVector      sigma_gain(topol->n_difs);
  gain_calib_ugly::LinearFitVector slope     (topol->n_difs);
  gain_calib_ugly::LinearFitVector intercept (topol->n_difs);

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
  //                              Fit histograms                             //
  /////////////////////////////////////////////////////////////////////////////

  gain_calib::DirList dif_list = list::list_files(hist_dir, true, ".root");
  try {
    wgFit Fit(input_file, output_img_dir);

    for (auto const &chip : topol->dif_map[idif]) {
      unsigned ichip = chip.first;
      unsigned n_chans = chip.second;
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
    for (auto const& chip: dif.second) {
      unsigned ichip = chip.first;
      try { make::directory(output_img_dir + "/dif" + std::to_string(idif) +
                            "/chip" + std::to_string(ichip)); }
      catch (const wgInvalidFile& e) {
        Log.eWrite("[wgGainCalibUgly] " + std::string(e.what()));
        return ERR_FAILED_CREATE_DIRECTORY;
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
