// system includes
#include <string>

// boost includes
#include <boost/make_unique.hpp>

// user includes
#include "wgFileSystemTools.hpp"
#include "wgErrorCodes.hpp"
#include "wgLogger.hpp"
#include "wgMakeHist.hpp"
#include "wgTopology.hpp"
#include "wgRawData.hpp"
#include "wgGetTree.hpp"
#include "wgColor.hpp"

namespace wg = wagasci_tools;

int wgDQCheck(const char * x_tree_file,
              const char * x_xml_config_file,
              const char * x_calib_dir,
              const char * x_output_dir,
              const unsigned long ul_flags,
              int dif_id) {

  /////////////////////////////////////////////////////////////////////////////
  //                          Check argument sanity                          //
  /////////////////////////////////////////////////////////////////////////////

  std::bitset<makehist::NFLAGS> flags(ul_flags);
  std::string tree_file(x_tree_file);
  std::string xml_config_file(x_xml_config_file);
  std::string calib_dir(x_calib_dir);
  std::string output_dir(x_output_dir);

  if (tree_file.empty() || !wg::check_exist::root_file(tree_file)) {
    Log.eWrite("[wgDQCheck] Input tree file not found : " + tree_file);
    return ERR_EMPTY_INPUT_FILE;
  }
  if (xml_config_file.empty() || !wg::check_exist::root_file(xml_config_file)) {
    Log.eWrite("[wgDQCheck] Input xml config file not found : " + xml_config_file);
    return ERR_EMPTY_INPUT_FILE;
  }
  if (calib_dir.empty()) {
    wgEnvironment env;
    calib_dir = env.CONF_DIRECTORY;
  }
  if (!wg::check_exist::directory(calib_dir)) {
    Log.eWrite("[wgMakeHist] Directory containing card files not found : " +
               calib_dir);
    return ERR_EMPTY_INPUT_FILE;
  }
  if (output_dir.empty()) {
    wgEnvironment env;
    output_dir = env.DQ_DIRECTORY;
  }
  if (!wg::check_exist::directory(output_dir)) {
    wg::make::directory(output_dir);
  }

  /////////////////////////////////////////////////////////////////////////////
  //                                 Topology                                //
  /////////////////////////////////////////////////////////////////////////////


  std::unique_ptr<Topology> topol;
  try {
    topol = boost::make_unique<Topology>(xml_config_file,
                                         TopologySourceType::xml_file);
  } catch (const std::exception& except) {
    Log.eWrite("[wgDQCheck] Failed to get topology from XML file (" +
               xml_config_file + ")" + except.what());
    return ERR_TOPOLOGY;
  }

  if (dif_id == -1)
    dif_id = wg::string::extract_dif_id(tree_file);
  if (dif_id == -1)
    return ERR_WRONG_DIF_VALUE;
  
  unsigned n_chips = topol->dif_map[dif_id].size();

  if (n_chips == 0 || n_chips > NCHIPS) {
    Log.eWrite("[wgMakeHist] wrong number of chips : " +
               std::to_string(n_chips) );
    return ERR_WRONG_CHIP_VALUE;
  }

  /////////////////////////////////////////////////////////////////////////////
  //                                Read TTree                               //
  /////////////////////////////////////////////////////////////////////////////

  Raw_t rd(n_chips);
  
  std::unique_ptr<wgGetTree> tree;
  try {
    tree = boost::make_unique<wgGetTree>(tree_file, rd, dif_id);
  } catch (const std::exception& except) {
    Log.eWrite("[wgDQCheck] Failed to open TTree file (" +
               tree_file + ")" + except.what());
    return ERR_FAILED_OPEN_TREE_FILE;
  }

  Int_t n_events = tree->tree->GetEntries();

  /////////////////////////////////////////////////////////////////////////////
  //                            Create histograms                            //
  /////////////////////////////////////////////////////////////////////////////

  makehist::ChipChanHist h_dark_hit    (n_chips);
  makehist::ChipChanHist h_pe_hit      (n_chips);
  makehist::ChipChanHist h_charge_nohit(n_chips);
  makehist::ChipChanColHist h_time_hit (n_chips);
  makehist::ChipChanHist h_bcid_hit    (n_chips);

  int min_bin = 0;
  int max_bin = MAX_VALUE_12BITS;
  int bin     = MAX_VALUE_12BITS;
  
  for (unsigned ichip = 0; ichip < n_chips; ++ichip) {
    unsigned n_chans = topol->dif_map[dif_id][ichip];
    if (flags[makehist::SELECT_CHARGE_HG])
      h_dark_hit[ichip].resize(n_chans);
    if (flags[makehist::SELECT_PEU])
      h_pe_hit[ichip].resize(n_chans);
    if (flags[makehist::SELECT_PEDESTAL])
      h_charge_nohit[ichip].resize(n_chans);
    if (flags[makehist::SELECT_TIME]) { 
      h_time_hit[ichip].resize(n_chans);
    }
    if (flags[makehist::SELECT_DARK_NOISE] | flags[makehist::SELECT_TIME]) 
      h_bcid_hit     [ichip].resize(n_chans); 
    for (unsigned ichan = 0; ichan < n_chans; ++ichan) {
        TString h_name;
        if (flags[makehist::SELECT_CHARGE_HG]) {
          // ADC count when there is a hit (hit bit is one) and the high gain
          // preamp is selected
          h_name.Form("dark_hit_dif%u_chip%u_ch%u", dif_id, ichip, ichan);
          h_dark_hit[ichip][ichan] = new TH1I(h_name, h_name, bin,
                                              min_bin, max_bin);
          h_dark_hit[ichip][ichan]->SetLineColor(wgColor::wgcolors[ichan]);
        }
        if (flags[makehist::SELECT_PEU]) {
          // Photo-electrons
          h_name.Form("pe_hit_dif%u_chip%u_ch%u", dif_id, ichip, ichan);
          h_pe_hit[ichip][ichan] = new TH1I(h_name, h_name, bin,
                                                  min_bin, max_bin);
          h_pe_hit[ichip][ichan]->SetLineColor(wgColor::wgcolors[ichan]);
        }
        if (flags[makehist::SELECT_PEDESTAL]) {
          // ADC count when there is not hit (hit bit is zero)
          h_name.Form("charge_nohit_dif%u_chip%u_ch%u",
                      dif_id, ichip, ichan);
          h_charge_nohit[ichip][ichan] = new TH1I(h_name, h_name, bin,
                                                  min_bin, max_bin);
          h_charge_nohit[ichip][ichan]->SetLineColor(
              wgColor::wgcolors[ichan + MEMDEPTH * 2 + 2]);
        }
        if (flags[makehist::SELECT_TIME]) { 
          // TDC count when there is a hit (hit bit is one)
          h_name.Form("time_hit_dif%u_chip%u_ch%u", dif_id, ichip, ichan);
          h_time_hit[ichip][ichan] = new TH1I(h_name, h_name, bin,
                                              min_bin, max_bin);
          h_time_hit[ichip][ichan]->SetLineColor(wgColor::wgcolors[ichan]);
        }
      if (flags[makehist::SELECT_DARK_NOISE] | flags[makehist::SELECT_TIME]) {
        // BCID
        TString h_name;
        h_name.Form("bcid_hit_dif%u_chip%u_ch%u", dif_id, ichip, ichan);
        h_bcid_hit[ichip][ichan] = new TH1I(h_name, h_name, MAX_VALUE_16BITS,
                                            0, MAX_VALUE_16BITS);
        h_bcid_hit[ichip][ichan]->SetLineColor(kBlack);
      }
    } //end ch
  } //end chip


  
  /////////////////////////////////////////////////////////////////////////////
  //                                Event loop                               //
  /////////////////////////////////////////////////////////////////////////////
  
  for (Int_t ievent = 0; ievent < n_events; ++ievent) {

    if (ievent % 1000 == 0)
      Log.Write("[wgMakeHist] Event number = " + std::to_string(ievent) +
                " / " + std::to_string(n_events));
    // Read one event
    tree->tree->GetEntry(ievent);

    return WG_SUCCESS;
  }

  namespace data_quality {
  int bunch_structure() {
    // TODO: 
    return WG_SUCCESS;
  }

  int gain() {
    // TODO: 
    return WG_SUCCESS;
  }

  int dark_noise() {
    // TODO: 
    return WG_SUCCESS;
  }
  }
