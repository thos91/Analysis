// system C++ includes
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <bitset>


// ROOT includes
#include <THStack.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TH1.h>
#include <TFile.h>
#include <TTree.h>

// user includes
#include "wgConst.hpp"
#include "wgFileSystemTools.hpp"
#include "wgErrorCodes.hpp"
#include "wgFitConst.hpp"
#include "wgEditConfig.hpp"
#include "wgChangeConfig.hpp"
#include "wgLogger.hpp"

using namespace wagasci_tools;

int wgChangeConfig(const char * x_input_file,
                   const char * x_output_file,
                   const unsigned long flags_ulong,
                   const int value,
                   const unsigned mode,
                   const unsigned channel) {
  
  
  std::bitset<WG_CHANGE_CONFIG_FLAGS> flags(flags_ulong);
  std::string input_file(x_input_file);
  std::string output_file(x_output_file);
  
  if (flags[OVERWRITE_FLAG] && output_file.empty()) output_file = input_file;
  
  if (input_file.empty() || !check_exist::txt_file(input_file)) {
    Log.eWrite("[wgChangeConfig] input file doesn't exists : " + input_file);
    return ERR_INPUT_FILE_NOT_FOUND;
  }

  if (!flags[OVERWRITE_FLAG] && flags[EDIT_FLAG] && check_exist::txt_file(output_file)) {
    Log.eWrite("[wgChangeConfig] overwrite flag must be set in edit mode");
    return ERR_OVERWRITE_FLAG_NOT_SET;
  }

  if (!output_file.empty()) {
    try { make::directory(get_stats::dirname(output_file)); }
    catch (const wgInvalidFile& e) {
      Log.eWrite("[wgAnaHistSummary] " + std::string(e.what()));
      return ERR_FAILED_CREATE_DIRECTORY;
    }
  }

  try {
    bool is_bitstream_string = false;
    wgEditConfig EditConfig(input_file, is_bitstream_string);
  
    // Sanity check of the passed arguments
  
    if (flags[EDIT_FLAG]) {
      if (mode == EC_TRIGGER_THRESHOLD || mode == EC_GAIN_SELECT_THRESHOLD || mode == EC_CHIPID) {
        if ((mode == EC_TRIGGER_THRESHOLD || mode == EC_GAIN_SELECT_THRESHOLD) && value > (int) MAX_VALUE_10BITS) {
          Log.eWrite("[wgChangeConfig][" + get_stats::basename(input_file) + "] value is out of range : " + std::to_string(value));
          return ERR_VALUE_OUT_OF_RANGE;
        }
        else if (mode == EC_CHIPID && value > (int) MAX_VALUE_8BITS) {
          Log.eWrite("[wgChangeConfig][" + get_stats::basename(input_file) + "] value is out of range : " + std::to_string(value));
          return ERR_VALUE_OUT_OF_RANGE;
        }
      }
      else if (mode == EC_INPUT_DAC || mode == EC_HG_LG_AMPLIFIER || mode == EC_THRESHOLD_ADJUSTMENT) {
        if (channel > NCHANNELS) {
          Log.eWrite("[wgChangeConfig][" + get_stats::basename(input_file) + "] channel is out of range : " + std::to_string(channel));
          return ERR_WRONG_CHANNEL_VALUE;
        }
        else if (mode == EC_INPUT_DAC) {
          if (value > (int) MAX_VALUE_8BITS) {
            Log.eWrite("[wgChangeConfig][" + get_stats::basename(input_file) + "] value is out of range : " + std::to_string(value));
            return ERR_VALUE_OUT_OF_RANGE;
          }
        }
        else if (mode == EC_HG_LG_AMPLIFIER) {
          if (value > (int) MAX_VALUE_6BITS) {
            Log.eWrite("[wgChangeConfig][" + get_stats::basename(input_file) + "] value is out of range : " + std::to_string(value));
            return ERR_VALUE_OUT_OF_RANGE;
          }
        }
        else if (mode == EC_THRESHOLD_ADJUSTMENT) {
          if (value > (int) MAX_VALUE_4BITS) {
            Log.eWrite("[wgChangeConfig][" + get_stats::basename(input_file) + "] value is out of range : " + std::to_string(value));
            return ERR_VALUE_OUT_OF_RANGE;
          }
        }
      }
      else if (mode == EC_INPUT_DAC_REFERENCE) {
        if (value != 0 && value != 1) {
          Log.eWrite("[wgChangeConfig][" + get_stats::basename(input_file) + "] value is out of range : " + std::to_string(value));
          return ERR_VALUE_OUT_OF_RANGE;
        }
      }
      else {
        Log.eWrite("[wgChangeConfig][" + get_stats::basename(input_file) + "] mode not recognized : " + std::to_string(mode));
        return ERR_WRONG_MODE;
      }
    } // if (flags[EDIT_FLAG])
    else {
      EditConfig.CheckAll();
      return WG_SUCCESS;
    }
  
    try {
      // Global threshold
      if (mode == EC_TRIGGER_THRESHOLD ) {
        EditConfig.Change_trigth(value);
      }
      // Gain select threshold
      else if (mode == EC_GAIN_SELECT_THRESHOLD) {
        EditConfig.Change_gainth(value);
      }
      // CHIP ID
      else if (mode == EC_CHIPID) {
        EditConfig.Change_chipid(value);
      }
      else if (mode == EC_INPUT_DAC || mode == EC_HG_LG_AMPLIFIER || mode == EC_THRESHOLD_ADJUSTMENT) {

        // ************************** ALL CHANNELS ************************** //
		
        if (channel == NCHANNELS) {
          for(unsigned ichan = 0; ichan < NCHANNELS; ichan++) {
            // Input DAC
            if (mode == EC_INPUT_DAC ) {
              EditConfig.Change_inputDAC(ichan, value);
            }
            // HG and LG preamplifier feedback capacitor 
            else if (mode == EC_HG_LG_AMPLIFIER ) {
              EditConfig.Change_ampDAC(ichan, value);
            }
            // Threshold fine tuning
            else if (mode == EC_THRESHOLD_ADJUSTMENT ) {
              EditConfig.Change_trigadj(ichan, value);
            }
          }
        }

        // ************************** SINGLE CHANNEL ************************** //
		
        else {
          // Input DAC
          if (mode == EC_INPUT_DAC) {
            EditConfig.Change_inputDAC(channel, value);
          }
          // HG and LG preamplifier feedback capacitor 
          else if (mode == EC_HG_LG_AMPLIFIER) {	
            EditConfig.Change_ampDAC(channel, value);
          }
          // Threshold fine tuning
          else if (mode == EC_THRESHOLD_ADJUSTMENT) {
            EditConfig.Change_trigadj(channel, value);
          } 
        }
      }

      // input DAC Voltage Reference (1 = internal 4.5V   0 = internal 2.5V)
	  
      else if (mode == EC_INPUT_DAC_REFERENCE ) {
        EditConfig.Change_1bitparam(value, GLOBAL_INPUT_DAC_REF_START);
      }
      EditConfig.Write(output_file);
    }
    catch (const std::exception &e) {
      Log.eWrite("[wgChangeConfig][" + input_file + "] failed to write value : " + e.what());
      return ERR_FAILED_WRITE;
    }
  } // catch the exception thrown by the EditConfig constructor
  catch (const std::exception& e) {
    Log.eWrite("[wgChangeConfig][" + input_file + "] failed to open the input file : " + e.what());
  }
  return WG_SUCCESS;
}
