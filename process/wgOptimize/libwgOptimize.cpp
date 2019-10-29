// system includes
#include <string>
#include <vector>

// json includes
#include <nlohmann/json.hpp>

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
#include "wgOptimize.hpp"

using namespace wagasci_tools;

int wgOptimize(const char * x_threshold_card,
               const char * x_gain_card,
               const char * x_config_xml_file,
               const char * x_wagasci_config_dif_dir,
               const unsigned mode,
               const unsigned pe,
               const unsigned inputDAC) {

  // ================ Argument parsing ================= //
  
  std::string threshold_card("");
  if (x_threshold_card != NULL) threshold_card = x_threshold_card;
  std::string gain_card("");
  if (x_gain_card != NULL) gain_card = x_gain_card;
  std::string wagasci_config_dif_dir("");
  if (x_wagasci_config_dif_dir != NULL) wagasci_config_dif_dir = x_wagasci_config_dif_dir;
  std::string config_xml_file("");
  if (x_config_xml_file != NULL) config_xml_file = x_config_xml_file;

  // ================ Sanity check on the arguments ================= //
  
  if ((inputDAC % 20) != 1 || inputDAC < 1 || inputDAC > 241) {
    Log.eWrite("[wgOptimize] Input DAC must be in {1,21,41,61,81,101,121,141,161,181,201,221,241}");
    return ERR_WRONG_INPUTDAC_VALUE;
  }
  if ( threshold_card.empty() || !check_exist::xml_file(threshold_card) ) {
    Log.eWrite("[wgOptimize] Threshold card not found");
    return ERR_THRESHOLD_CARD_NOT_FOUND;
  }
  if ( (mode == OP_INPUTDAC_MODE) && (gain_card.empty() || !check_exist::xml_file(gain_card)) ) {
    Log.eWrite("[wgOptimize] A valid calibration card is needed in OP_INPUTDAC_MODE");
    return ERR_GAIN_CARD_NOT_FOUND;
  }
  if ( !check_exist::xml_file(config_xml_file)) {
    Log.eWrite("[wgOptimize] Pyrame config file doesn't exist : " + config_xml_file);
    return ERR_CONFIG_XML_FILE_NOT_FOUND;
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
  
  Topology * topol;
  try {
    topol = new Topology(config_xml_file);
  }
  catch (const std::exception& e) {
    Log.eWrite("[wgOptimize] " + std::string(e.what()));
    return ERR_TOPOLOGY;
  }
  
  // ================ Read the threshold card ================= //

  u3vector optimized_threshold(topol->n_difs);
  u3vector slope_iDAC_th(topol->n_difs);
  u3vector intercept_iDAC_th(topol->n_difs);

  try {
    wgEditXML Edit;
    Edit.Open(threshold_card);
    for (unsigned idif = 0; idif < topol->n_difs; ++idif) {
      for (unsigned ichip = 0; ichip < topol->dif_map[idif].size(); ++ichip) {
        optimized_threshold[idif].push_back(u1vector());
        slope_iDAC_th[idif].push_back(u1vector());
        intercept_iDAC_th[idif].push_back(u1vector());  
        for (unsigned ichan = 0; ichan < topol->dif_map[idif][ichip]; ++ichan) {
          // Get the optimal threshold for pe photo-electron equivalent
          if (mode == OP_THRESHOLD_MODE) {
          optimized_threshold[idif][ichip][ichan] = Edit.OPT_GetValue("threshold_" + std::to_string(pe), idif, ichip, ichan, inputDAC);
          } else if (mode == OP_INPUTDAC_MODE) {
            // s_th is the slope of the linear fit of the inputDAC (x) vs optimal
            // threshold for the given p.e. equivalend (y)
            // i_th is the intercept of the linear fit of the inputDAC (x) vs
            // optimal threshold for the given p.e. equivalend (y)
            slope_iDAC_th    [idif][ichip].push_back(Edit.OPT_GetChanValue("slope_threshold" + std::to_string(pe), idif, ichip, ichan));
            intercept_iDAC_th[idif][ichip].push_back(Edit.OPT_GetChanValue("intercept_threshold" + std::to_string(pe), idif, ichip, ichan));
          }
        }
      }
    }
    Edit.Close();
  }
  catch (const std::exception& e) {
    Log.eWrite("[wgOptimize] Error when reading the threshold card file : " + std::string(e.what()));
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
              average_slope += Edit.GainCalib_GetValue("slope_gain", idif, ichip, ichan, icol);
              average_intercept += Edit.GainCalib_GetValue("intercept_gain", idif, ichip, ichan, icol);
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
      Log.eWrite("[wgOptimize] Error when reading the calibration card file : " + std::string(e.what()));
      return ERR_GAIN_CARD_READ;
    }
  }

  // ================ Edit the SPIROC2D configuration files for every GDCC, DIF, ASU, channel ================= //

  try {
    for (auto const & gdcc : topol->gdcc_map) {
      unsigned igdcc_id = gdcc.first;
      for (auto const & dif : gdcc.second) {
        unsigned rel_idif = dif.first;
        unsigned abs_idif = topol->GetAbsDif(gdcc.first, dif.first);
        unsigned idif = abs_idif;
        for (auto const & chip : dif.second) {
          unsigned ichip = chip.first;

          std::string configName(wagasci_config_dif_dir + "/wagasci_bitstream_gdcc" + std::to_string(igdcc_id) +
                                 "_dif" + std::to_string(rel_idif) + "_chip" + std::to_string(ichip) + ".txt");

          if( !check_exist::txt_file(configName) ) {
            Log.eWrite("[wgOptimize] bitstream file doesn't exist : " + configName);
            return ERR_BITSTREAM_FILE_NOT_FOUND;
          }
          wgEditConfig edit_config(configName, false);
          if (mode == OP_THRESHOLD_MODE) {
          edit_config.Change_trigth_and_adj(optimized_threshold[idif][ichip]);
          } else if (mode == OP_INPUTDAC_MODE) {
            for (unsigned ichan = 0; ichan < chip.second; ++ichan) {
              unsigned optimized_input_dac = (WG_NOMINAL_GAIN - intercept_iDAC_gain[idif][ichip][ichan]) / slope_iDAC_gain[idif][ichip][ichan];
              edit_config.Change_inputDAC(ichan, optimized_input_dac);
              optimized_threshold[idif][ichip][ichan] = intercept_iDAC_th[idif][ichip][ichan] + slope_iDAC_th[idif][ichip][ichan] * optimized_input_dac;
            } // chan
           edit_config.Change_trigth_and_adj(optimized_threshold[idif][ichip]); 
          }
          edit_config.Write(configName);
          edit_config.Clear();
        } // chip
      } // dif
    } // gdcc
  }
  catch (const std::exception& e) {
    Log.eWrite("[wgOptimize] " + std::string(e.what()));
    return ERR_WG_OPTIMIZE;
  }
  return WG_SUCCESS;
}
