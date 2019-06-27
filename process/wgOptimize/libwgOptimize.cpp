// system includes
#include <string>
#include <vector>

// json includes
#include <nlohmann/json.hpp>

// user includes
#include "wgEditXML.hpp"
#include "wgEditConfig.hpp"
#include "wgFileSystemTools.hpp"
#include "wgErrorCode.hpp"
#include "wgConst.hpp"
#include "wgExceptions.hpp"
#include "wgOptimize.hpp"
#include "wgTopology.hpp"
#include "wgLogger.hpp"

using namespace std;

int wgOptimize(const char * x_threshold_card,
               const char * x_calibration_card,
               const char * x_config_xml_file,
               const char * x_wagasci_config_dif_dir,
               const int mode,
               const int inputDAC,
               const int pe) {

  // ================ Argument parsing ================= //
  
  CheckExist check;
  string threshold_card;
  if (x_threshold_card != NULL) threshold_card = x_threshold_card;
  else threshold_card = "";
  string calibration_card;
  if (x_calibration_card != NULL) calibration_card = x_calibration_card;
  else calibration_card = "";
  string wagasci_config_dif_dir;
  if (x_wagasci_config_dif_dir != NULL) wagasci_config_dif_dir = x_wagasci_config_dif_dir;
  else wagasci_config_dif_dir = "";
  string config_xml_file;
  if (x_threshold_card != NULL) config_xml_file = x_config_xml_file;
  else return ERR_CONFIG_XML_FILE_NOT_FOUND;

  // ================ Get the electronics topology ================= //
  
  Topology * topol;
  try {
    topol = new Topology(config_xml_file);
  }
  catch (const exception& e) {
    Log.eWrite("[wgOptimize] " + string(e.what()));
    return ERR_TOPOLOGY;
  }
  unsigned n_difs = topol->n_difs;
  unsigned n_chips = topol->max_chips;
  unsigned n_channels = topol->max_channels;
  
  // ================ Sanity check on the arguments ================= //
  
  if ( (mode == 0) && ( ((inputDAC % 20) != 1) || (inputDAC < 1) || (inputDAC > 241) ) ) {
    Log.eWrite("[wgOptimize] Input DAC must be in {1,21,41,61,81,101,121,141,161,181,201,221,241}");
    return ERR_WRONG_INPUTDAC_VALUE;
  }
  if ( threshold_card.empty() || !check.XmlFile(threshold_card) ) {
    Log.eWrite("[wgOptimize] Threshold card not found");
    return ERR_THRESHOLD_CARD_NOT_FOUND;
  }
  if ( (mode == OP_INPUTDAC_MODE) && (calibration_card.empty() || !check.XmlFile(calibration_card)) ) {
    Log.eWrite("[wgOptimize] A valid calibration card is needed in OP_INPUTDAC_MODE");
    return ERR_CALIBRATION_CARD_NOT_FOUND;
  }
  if ( !check.XmlFile(config_xml_file)) {
    Log.eWrite("[wgOptimize] Pyrame config file doesn't exist : " + config_xml_file);
    return ERR_CONFIG_XML_FILE_NOT_FOUND;
  }
  if ( (mode == OP_THRESHOLD_MODE) && ( ((inputDAC % 20) != 1) || (inputDAC < 1) || (inputDAC > 241) ) ) {
    Log.eWrite("Input DAC (" + to_string(inputDAC) + ") must be in {1,21,41,61,81,101,121,141,161,181,201,221,241}");
    return ERR_WRONG_INPUTDAC_VALUE;
  }
  if ( pe != 1 && pe != 2 && pe != 3 ) {
    Log.eWrite("Photo-electrons (" + to_string(pe) + ") must be in {1,2,3}");
    return ERR_WRONG_PE_VALUE;
  }
  if ( n_difs > NDIFS ) {
    Log.eWrite("Number of DIFs (" + to_string(n_difs) + ") must be less than " + to_string(NDIFS));
    return ERR_WRONG_DIF_VALUE;
  }
  if ( n_chips > NCHIPS ) {
    Log.eWrite("Number of chips (" + to_string(n_chips) + ") must be less than " + to_string(NCHIPS));
    return ERR_WRONG_CHIP_VALUE;
  }
  if ( mode != OP_THRESHOLD_MODE && mode != OP_INPUTDAC_MODE ) {
    Log.eWrite("Mode not recognized : " + to_string(mode));
    return ERR_WRONG_MODE;
  }

  // ================ Read the threshold card ================= //

  d2vector threshold(n_difs, n_chips);
  d2vector s_th(n_difs, n_chips);
  d2vector i_th(n_difs, n_chips);
  threshold.fill(-1);
  s_th.fill(0);
  i_th.fill(-1);
  try {
    wgEditXML Edit;
    Edit.Open(threshold_card);
    for(unsigned idif = 0; idif < n_difs; idif++) {
      for(unsigned ichip = 0; ichip < n_chips; ichip++) {
        // pre-calibration mode
        if (mode == OP_THRESHOLD_MODE) {
          // Get the optimal threshold for pe photo-electron equivalent
          threshold[idif][ichip] = Edit.OPT_GetValue("threshold_" + to_string(pe), idif, ichip, inputDAC);
        }
        // post-calibration mode
        else if (mode == OP_INPUTDAC_MODE) {
          if ( pe < 3 ) {
            // s_th is the slope of the linear fit of the inputDAC (x) vs optimal
            // threshold for the given p.e. equivalend (y)
            // i_th is the intercept of the linear fit of the inputDAC (x) vs
            // optimal threshold for the given p.e. equivalend (y)
            s_th[idif][ichip]=Edit.OPT_GetChipValue("s_th" + to_string(pe), idif, ichip);
            i_th[idif][ichip]=Edit.OPT_GetChipValue("i_th" + to_string(pe), idif, ichip);
          } else {
            // Get the optimal threshold for 2.5 photo-electron equivalent
            // for what value of the inputDAC??
            threshold[idif][ichip]= Edit.OPT_GetChipValue(string("threshold_3"), idif, ichip);
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

  d3vector s_Gain(n_difs, n_chips, n_channels);
  d3vector i_Gain(n_difs, n_chips, n_channels);
  s_Gain.fill(0);
  i_Gain.fill(-1);
  try {
    wgEditXML Edit;
    // Get the slope and intercept of the inputDAC(x) vs Gain(y) graph from the
    // calibration_card.xml file
    if(mode == OP_INPUTDAC_MODE) {
      Edit.Open(calibration_card);
      for(unsigned idif = 0; idif < n_difs; idif++) {
        for(unsigned ichip = 0; ichip < n_chips; ichip++) {
          for(unsigned ichan = 0; ichan < n_channels; ichan++) {
            s_Gain[idif][ichip][ichan]= Edit.PreCalib_GetValue(string("s_Gain"), idif, ichip, ichan);
            i_Gain[idif][ichip][ichan]= Edit.PreCalib_GetValue(string("i_Gain"), idif, ichip, ichan);
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
    for(auto const & igdcc_id : topol->gdcc_map) {
      for ( auto const & idif_id : igdcc_id.second) {
        unsigned idif = topol->GetAbsDif(igdcc_id.first, idif_id.first) - 1;
        for ( auto const & ichip_id : idif_id.second) {
          unsigned ichip = ichip_id.first - 1;
          string configName(wagasci_config_dif_dir + "/wagasci_config_gdcc" + igdcc_id.first +
                            "_dif" + idif_id.first + "_chip" + ichip_id.first + ".txt");

          if( !check.TxtFile(configName) ) {
            Log.eWrite("[wgOptimize] bitstream file doesn't exist : " + configName);
            return ERR_BITSTREAM_FILE_NOT_FOUND;
          }
          wgEditConfig EditCon(configName, false);
	  
          // In mode 0 edit the threshold of each chip to the optimal value taken
          // from threshold_card.xml
          if( mode == OP_THRESHOLD_MODE ) {
            EditCon.Change_trigth(threshold[idif][ichip]);
          }
	  
          // In mode 1 edit the inputDAC of each channel of each chip to the
          // optimal value calculated from calibration_card.xml
          else if( mode == OP_INPUTDAC_MODE ) {
            double mean_inputDAC = 0.;

            for(unsigned ichan = 0; ichan < ichip_id.second; ichan++) {
              double inputDAC = 0.;
              try {
                if(s_Gain[idif][ichip][ichan] == 0.) {
                  inputDAC = 121.;
                  mean_inputDAC += inputDAC;
                }
                else {
                  // This is the inputDAC value corresponding to a Gain of 40
                  inputDAC = (40. - i_Gain[idif][ichip][ichan]) / s_Gain[idif][ichip][ichan];
                  if (inputDAC < 1.)   inputDAC = 1.;
                  if (inputDAC > 250.) inputDAC = 250.;
                  mean_inputDAC += inputDAC;
                }
                inputDAC=round(inputDAC);
                EditCon.Change_inputDAC(ichan, inputDAC);
              }
              catch (const exception& e) {
                Log.eWrite("[wgOptimize] error setting the optimized inputDAC " + to_string(inputDAC) +
                           "( igdcc " + to_string(igdcc_id.first) + "idif " + to_string(idif_id.first) +
                           ", chip " + to_string(ichip_id.first) + ", channel " + to_string(ichan) + ") : " + e.what());
                return ERR_INPUTDAC_WRITE;
              }
            } // chan loop
            mean_inputDAC /= (double) n_channels;

            // In mode 1 set the chip-wise threshold to the value corresponding to
            // mean_inputDAC
            double thresholdDAC = -1;
            try {
              if ( pe < 3 ) {
                // Use the slope and intercept of the inputDAC(x) vs threshold(y) graph
                thresholdDAC = s_th[idif][ichip] * mean_inputDAC + i_th[idif][ichip];
                thresholdDAC = round(thresholdDAC);
              }
              else if( pe == 3) {
                // For 2.5. p.e. just read the threshold from the threshold card file
                thresholdDAC = threshold[idif][ichip];
                thresholdDAC = round(thresholdDAC);
              }
              // Set the global threshold of the chip
              EditCon.Change_trigth((int)thresholdDAC);
            }
            catch (const exception& e) {
              Log.eWrite("[wgOptimize] error setting the optimized threshold " + to_string(thresholdDAC) +
                         "( igdcc " + to_string(igdcc_id.first) + "idif " + to_string(idif_id.first) +
                         ", chip " + to_string(ichip_id.first) + e.what());
              return ERR_THRESHOLD_WRITE;
            }
          }
          EditCon.Write(configName);
          EditCon.Clear();
        }//ichip
      }//idif
    }//igdcc
  }
  catch (const exception& e) {
    Log.eWrite("[wgOptimize] " + string(e.what()));
    return ERR_OPTIMIZE_GENERIC_ERROR;
  }
  return OP_SUCCESS;
}
