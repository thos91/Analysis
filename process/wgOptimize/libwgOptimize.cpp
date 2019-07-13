// system includes
#include <string>
#include <vector>

// json includes
#include <nlohmann/json.hpp>

// user includes
#include "wgEditXML.hpp"
#include "wgEditConfig.hpp"
#include "wgFileSystemTools.hpp"
#include "wgConst.hpp"
#include "wgExceptions.hpp"
#include "wgTopology.hpp"
#include "wgLogger.hpp"
#include "wgOptimize.hpp"

using namespace wagasci_tools;

int wgOptimize(const char * x_threshold_card,
               const char * x_calibration_card,
               const char * x_config_xml_file,
               const char * x_wagasci_config_dif_dir,
               const int mode,
               const int inputDAC,
               const int pe) {

  // ================ Argument parsing ================= //
  
  std::string threshold_card;
  if (x_threshold_card != NULL) threshold_card = x_threshold_card;
  else threshold_card = "";
  std::string calibration_card;
  if (x_calibration_card != NULL) calibration_card = x_calibration_card;
  else calibration_card = "";
  std::string wagasci_config_dif_dir;
  if (x_wagasci_config_dif_dir != NULL) wagasci_config_dif_dir = x_wagasci_config_dif_dir;
  else wagasci_config_dif_dir = "";
  std::string config_xml_file;
  if (x_threshold_card != NULL) config_xml_file = x_config_xml_file;
  else return ERR_CONFIG_XML_FILE_NOT_FOUND;

  if (mode == OP_INPUTDAC_MODE) return ERR_NOT_IMPLEMENTED_YET;
  
  // ================ Get the electronics topology ================= //
  
  Topology * topol;
  try {
    topol = new Topology(config_xml_file);
  }
  catch (const exception& e) {
    Log.eWrite("[wgOptimize] " + string(e.what()));
    return ERR_TOPOLOGY;
  }
  
  // ================ Sanity check on the arguments ================= //
  
  if ((inputDAC % 20) != 1 || inputDAC < 1 || inputDAC > 241) {
    Log.eWrite("[wgOptimize] Input DAC must be in {1,21,41,61,81,101,121,141,161,181,201,221,241}");
    return ERR_WRONG_INPUTDAC_VALUE;
  }
  if ( threshold_card.empty() || !check_exist::XmlFile(threshold_card) ) {
    Log.eWrite("[wgOptimize] Threshold card not found");
    return ERR_THRESHOLD_CARD_NOT_FOUND;
  }
  if ( (mode == OP_INPUTDAC_MODE) && (calibration_card.empty() || !check_exist::XmlFile(calibration_card)) ) {
    Log.eWrite("[wgOptimize] A valid calibration card is needed in OP_INPUTDAC_MODE");
    return ERR_CALIBRATION_CARD_NOT_FOUND;
  }
  if ( !check_exist::XmlFile(config_xml_file)) {
    Log.eWrite("[wgOptimize] Pyrame config file doesn't exist : " + config_xml_file);
    return ERR_CONFIG_XML_FILE_NOT_FOUND;
  }
  if ( pe < 1 && pe > 3 ) {
    Log.eWrite("Photo-electrons (" + to_string(pe) + ") must be in {1,2,3}");
    return ERR_WRONG_PE_VALUE;
  }
  if ( mode != OP_THRESHOLD_MODE && mode != OP_INPUTDAC_MODE ) {
    Log.eWrite("Mode not recognized : " + to_string(mode));
    return ERR_WRONG_MODE;
  }

  // ================ Read the threshold card ================= //

  u3vector optimized_threshold(topol->n_difs);
  u3vector slope_th_iDAC(topol->n_difs);
  u3vector intercept_th_iDAC(topol->n_difs);

  try {
    wgEditXML Edit;
    Edit.Open(threshold_card);
    for (unsigned idif = 0; idif < topol->n_difs; ++idif) {
      unsigned idif_id = idif + 1; 
      for (unsigned ichip = 0; ichip < topol->dif_map[idif_id].size(); ++ichip) {
        unsigned ichip_id = ichip + 1;
        optimized_threshold[idif].push_back(u1vector());
        slope_th_iDAC[idif].push_back(u1vector());
        intercept_th_iDAC[idif].push_back(u1vector());  
        for (unsigned ichan = 0; ichan < topol->dif_map[idif_id][ichip_id]; ++ichan) {
          // pre-calibration mode
          if (mode == OP_THRESHOLD_MODE) {
            // Get the optimal threshold for pe photo-electron equivalent
            optimized_threshold[idif][ichip][ichan] = Edit.OPT_GetValue("threshold_" + to_string(pe), idif, ichip, ichan, inputDAC);
          }
          // post-calibration mode
          else if (mode == OP_INPUTDAC_MODE) {
            // s_th is the slope of the linear fit of the inputDAC (x) vs optimal
            // threshold for the given p.e. equivalend (y)
            // i_th is the intercept of the linear fit of the inputDAC (x) vs
            // optimal threshold for the given p.e. equivalend (y)
            slope_th_iDAC    [idif][ichip].push_back(Edit.OPT_GetChanValue("s_th" + to_string(pe), idif, ichip, ichan));
            intercept_th_iDAC[idif][ichip].push_back(Edit.OPT_GetChanValue("i_th" + to_string(pe), idif, ichip, ichan));
          }
        }
      }
    }
    Edit.Close();
  }
  catch (const exception& e) {
    Log.eWrite("[wgOptimize] Error when reading the threshold card file : " + string(e.what()));
    return ERR_THRESHOLD_CARD_READ;
  }

  // ================ Read the calibration card ================= //

  d3vector slope_iDAC_gain(topol->n_difs);
  d3vector intercept_iDAC_gain(topol->n_difs);

  try {
    wgEditXML Edit;
    // Get the slope and intercept of the inputDAC(x) vs Gain(y) graph from the
    // calibration_card.xml file
    if(mode == OP_INPUTDAC_MODE) {
      Edit.Open(calibration_card);
      for (unsigned idif = 0; idif < topol->n_difs; ++idif) {
        unsigned idif_id = idif + 1;
        slope_iDAC_gain.push_back(d2vector());
        intercept_iDAC_gain.push_back(d2vector());
        for (unsigned ichip = 0; ichip < topol->dif_map[idif_id].size(); ++ichip) {
          unsigned ichip_id = ichip + 1;
          for (unsigned ichan = 0; ichan < topol->dif_map[idif_id][ichip_id]; ++ichan) {
            slope_iDAC_gain    [idif][ichip][ichan] = Edit.PreCalib_GetValue(string("s_Gain"), idif, ichip, ichan);
            intercept_iDAC_gain[idif][ichip][ichan] = Edit.PreCalib_GetValue(string("i_Gain"), idif, ichip, ichan);
          }
        }
      }
      Edit.Close();
    }
  }
  catch (const exception& e) {
    Log.eWrite("[wgOptimize] Error when reading the calibration card file : " + string(e.what()));
    return ERR_CALIBRATION_CARD_READ;
  }

  // ================ Edit the SPIROC2D configuration files for every GDCC, DIF, ASU, channel ================= //

  try {
    for (auto const & gdcc : topol->gdcc_map) {
      unsigned igdcc_id = gdcc.first;
      for (auto const & dif : gdcc.second) {
        unsigned rel_idif_id = dif.first;
        unsigned abs_idif_id = topol->GetAbsDif(gdcc.first, dif.first);
        unsigned idif = abs_idif_id - 1;
        for (auto const & chip : dif.second) {
          unsigned ichip_id = chip.first;
          unsigned ichip = ichip_id - 1;
          // unsigned n_channels = chip.second;
          
          string configName(wagasci_config_dif_dir + "/wagasci_config_gdcc" + to_string(igdcc_id) +
                            "_dif" + to_string(rel_idif_id) + "_chip" + to_string(ichip_id) + ".txt");

          if( !check_exist::TxtFile(configName) ) {
            Log.eWrite("[wgOptimize] bitstream file doesn't exist : " + configName);
            return ERR_BITSTREAM_FILE_NOT_FOUND;
          }
          wgEditConfig edit_config(configName, false);
          // In mode OP_THRESHOLD_MODE edit the threshold of each chip
          // to the optimal value taken from threshold_card.xml
          if (mode == OP_THRESHOLD_MODE) {
            edit_config.Change_trigth_and_adj(optimized_threshold[idif][ichip]);
          }
          edit_config.Write(configName);
          edit_config.Clear();
        } // chip
      } // dif
    } // gdcc
  }
  catch (const exception& e) {
    Log.eWrite("[wgOptimize] " + string(e.what()));
    return ERR_WG_OPTIMIZE;
  }
  return WG_SUCCESS;
}
