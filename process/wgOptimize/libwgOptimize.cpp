// system includes
#include <string>
#include <vector>
#include <memory>

// json includes
#include <nlohmann/json.hpp>

// boost includes
#include <boost/filesystem.hpp>
#include <boost/make_unique.hpp>

// user includes
#include "wgEditXML.hpp"
#include "wgEditConfig.hpp"
#include "wgFileSystemTools.hpp"
#include "wgErrorCodes.hpp"
#include "wgConst.hpp"
#include "wgExceptions.hpp"
#include "wgTopology.hpp"
#include "wgLogger.hpp"
#include "wgFitConst.hpp"
#include "wgEnvironment.hpp"
#include "wgOptimize.hpp"

using namespace wagasci_tools;

int wgOptimize(const char * x_threshold_card,
               const char * x_gain_card,
               const char * x_topology_source,
               const char * x_bitstream_dir,
               const unsigned mode,
               const unsigned pe,
               const unsigned inputDAC) {

  // ================ Argument parsing ================= //
  
  std::string threshold_card("");
  if (x_threshold_card != NULL)
    threshold_card = x_threshold_card;
  std::string gain_card("");
  if (x_gain_card != NULL)
    gain_card = x_gain_card;
  std::string topology_source("");
  if (x_topology_source != NULL)
    topology_source = x_topology_source;
  std::string bitstream_dir("");
  if (x_bitstream_dir != NULL)
    bitstream_dir = x_bitstream_dir;

  // ================ Sanity check on the arguments ================= //

  if (mode > NUM_OPTIMIZE_MODES) {
    Log.eWrite("Mode not recognized : " + std::to_string(mode));
    return ERR_WRONG_MODE;
  }
  if ( pe != 1 && pe != 2 ) {
    Log.eWrite("Photo-electrons (" + std::to_string(pe) + ") must be in {1,2}");
    return ERR_WRONG_PE_VALUE;
  }
  if ( threshold_card.empty() || !check_exist::xml_file(threshold_card) ) {
    Log.eWrite("[wgOptimize] Threshold card not found");
    return ERR_THRESHOLD_CARD_NOT_FOUND;
  }
  if ( (mode == OP_INPUTDAC_MODE) && (gain_card.empty() || !check_exist::xml_file(gain_card)) ) {
    Log.eWrite("[wgOptimize] A valid calibration card is needed in OP_INPUTDAC_MODE");
    return ERR_GAIN_CARD_NOT_FOUND;
  }
  if ( pe != 1 && pe != 2 ) {
    Log.eWrite("Photo-electrons (" + std::to_string(pe) + ") must be in {1,2}");
    return ERR_WRONG_PE_VALUE;
  }
  if ( mode != OP_THRESHOLD_MODE && mode != OP_INPUTDAC_MODE ) {
    Log.eWrite("Mode not recognized : " + std::to_string(mode));
    return ERR_WRONG_MODE;
  }

  // ================ Get the electronics topology ================= //
  
  std::unique_ptr<Topology> topol;
  try {
    nlohmann::json json = nlohmann::json::parse(topology_source);
    topol = boost::make_unique<Topology>(topology_source,
                                         TopologySourceType::json_string);
  } catch (...) {
    try {
      topol.reset(new Topology(topology_source, TopologySourceType::xml_file));
    } catch (const std::exception& except) {
      Log.eWrite("Topology string (" + topology_source + ") is not a valid"
                 "JSON string or a path to a Pyrame XML config file : " + except.what());
      return ERR_TOPOLOGY;
    }
  }
  
  // ================ Read the threshold card ================= //

  u3vector optimized_threshold(topol->n_difs);
  u3vector slope_iDAC_th(topol->n_difs);
  u3vector intercept_iDAC_th(topol->n_difs);

  try {
    wgEditXML Edit;
    Edit.Open(threshold_card);
    for (const auto& dif : topol->dif_map) {
      unsigned idif = dif.first;
      for (const auto& chip : dif.second) {
        unsigned ichip = chip.first;
        optimized_threshold[idif].push_back(u1vector());
        slope_iDAC_th[idif].push_back(u1vector());
        intercept_iDAC_th[idif].push_back(u1vector());  
        for (unsigned ichan = 0; ichan < chip.second; ++ichan) {
          // Get the optimal threshold for pe photo-electron equivalent
          switch (mode) {
            case OP_THRESHOLD_MODE:
              optimized_threshold[idif][ichip].push_back(Edit.OPT_GetValue(
                  "threshold_" + std::to_string(pe), idif, ichip, ichan, inputDAC));
            case OP_INPUTDAC_MODE:
              // s_th is the slope of the linear fit of the inputDAC (x) vs optimal
              // threshold for the given p.e. equivalend (y)
              // i_th is the intercept of the linear fit of the inputDAC (x) vs
              // optimal threshold for the given p.e. equivalend (y)
              slope_iDAC_th[idif][ichip].push_back(Edit.OPT_GetChanValue(
                  "slope_threshold" + std::to_string(pe), idif, ichip, ichan));
              intercept_iDAC_th[idif][ichip].push_back(Edit.OPT_GetChanValue(
                  "intercept_threshold" + std::to_string(pe), idif, ichip, ichan));
          }
        }
      }
    }
    Edit.Close();
  }
  catch (const std::exception& e) {
    Log.eWrite("[wgOptimize] Error when reading the threshold card file : " +
               std::string(e.what()));
    return ERR_THRESHOLD_CARD_READ;
  }

  // ================ Read the gain card file ================= //

  d3vector slope_iDAC_gain(topol->n_difs);
  d3vector intercept_iDAC_gain(topol->n_difs);

  if (mode == OP_INPUTDAC_MODE) {
    // Get the slope and intercept of the inputDAC(x) vs Gain(y) graph from the
    // gain_card.xml file
    try {
      wgEditXML Edit;
      Edit.Open(gain_card);
      for (auto const& dif: topol->dif_map) {
        unsigned idif = dif.first;
        slope_iDAC_gain.push_back(d2vector());
        intercept_iDAC_gain.push_back(d2vector());
        for (auto const& chip: dif.second) {
          unsigned ichip = chip.first;
          for (unsigned ichan = 0; ichan < chip.second; ++ichan) {
            double average_slope = 0, average_intercept = 0;
            for (unsigned icol = 0; icol < MEMDEPTH; ++icol) {
              average_slope += Edit.GainCalib_GetValue(
                  "slope_gain", idif, ichip, ichan, icol);
              average_intercept += Edit.GainCalib_GetValue(
                  "intercept_gain", idif, ichip, ichan, icol);
            }
            average_slope /= MEMDEPTH;
            average_intercept /= MEMDEPTH;
            slope_iDAC_gain    [idif][ichip][ichan] = average_slope;
            intercept_iDAC_gain[idif][ichip][ichan] = average_intercept;
          }
        }
      }
      Edit.Close();
    }
    catch (const std::exception& e) {
      Log.eWrite("[wgOptimize] Error when reading the calibration card file : "
                 + std::string(e.what()));
      return ERR_GAIN_CARD_READ;
    }
  }

  // ==== Edit the SPIROC2D configuration files for every DIF, ASU, channel ==== //

  try {
    for (auto const & dif : topol->dif_map) {
      unsigned idif = dif.first;
      for (auto const & chip : dif.second) {
        unsigned ichip = chip.first;

        std::stringstream ss;
        ss << bitstream_dir << "/wagasci_bitstream_dif" << idif << "_chip" <<
            std::setw(2) << std::setfill('0') << ichip << ".txt";
        std::string bitstream_file(ss.str());

        if (!check_exist::txt_file(bitstream_file)) {
          Log.Write("[wgOptimize] bitstream file doesn't exist : " + bitstream_file);
          wgEnvironment env;
          if (!check_exist::txt_file(env.CONF_DIRECTORY + "/wagasci_bitstream_template.txt")) {
            Log.eWrite("[wgOptimize] template bitstream file doesn't exist");
            return ERR_BITSTREAM_FILE_NOT_FOUND;
          }
          try {
          boost::filesystem::copy_file(
              env.CONF_DIRECTORY + "/wagasci_bitstream_template.txt",
              bitstream_file, boost::filesystem::copy_option::overwrite_if_exists);
          } catch (const boost::filesystem::filesystem_error &exception) {
            Log.eWrite("[wgOptimize] Failed to copy template bitstream file : " +
                       std::string(exception.what()));
            return ERR_BITSTREAM_FILE_NOT_FOUND;
          }
        }

        wgEditConfig edit_config(bitstream_file, false);

        edit_config.Change_trigth_and_adj(optimized_threshold[idif][ichip]);
        
        if (mode == OP_INPUTDAC_MODE) {
          for (unsigned ichan = 0; ichan < chip.second; ++ichan) {
            unsigned optimized_input_dac =
                (WG_NOMINAL_GAIN - intercept_iDAC_gain[idif][ichip][ichan]) /
                slope_iDAC_gain[idif][ichip][ichan];
            edit_config.Change_inputDAC(ichan, optimized_input_dac);
            optimized_threshold[idif][ichip][ichan] =
                intercept_iDAC_th[idif][ichip][ichan] +
                slope_iDAC_th[idif][ichip][ichan] * optimized_input_dac;
          } // chan
        }
        edit_config.Write(bitstream_file);
        edit_config.Clear();
      } // chip
    } // dif
  }
  catch (const std::exception& e) {
    Log.eWrite("[wgOptimize] " + std::string(e.what()));
    return ERR_WG_OPTIMIZE;
  }
  return WG_SUCCESS;
}
