// system C++ includes
#include <string>
#include <fstream>
#include <iostream>
#include <vector>

// system C includes
#include <bits/stdc++.h>

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

#include "wgFitConst.hpp"
#include "wgEditConfig.hpp"
#include "wgChangeConfig.hpp"
#include "wgLogger.hpp"

using namespace std;
using namespace wagasci_tools;

int wgChangeConfig(const char * x_inputFile,
                   const char * x_outputFile,
                   const unsigned long flags_ulong,
                   const int value,
                   const int mode,
                   const int chip,
                   const int channel) {
  
  
  bitset<WG_CHANGE_CONFIG_FLAGS> flags(flags_ulong);
  string inputFile(x_inputFile);
  string outputFile(x_outputFile);

  if(flags[OVERWRITE_FLAG] && outputFile.empty()) outputFile = inputFile;
  
  if(inputFile.empty() || !check_exist::TxtFile(inputFile)) {
    Log.eWrite("[wgChangeConfig] input file doesn't exists : " + inputFile);
    return ERR_INPUT_FILE_NOT_FOUND;
  }

  if(!flags[OVERWRITE_FLAG] && flags[EDIT_FLAG] && check_exist::TxtFile(outputFile)) {
    Log.eWrite("[wgChangeConfig] overwrite flag must be set in edit mode");
    return ERR_OVERWRITE_FLAG_NOT_SET;
  }
  
  try { MakeDir(GetPath(outputFile)); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[wgAnaHistSummary] " + string(e.what()));
    return ERR_CANNOT_CREATE_DIRECTORY;
  }

	  // fine tuning mode
	  if( flags[MPPC_DATA_FLAG] ) {
		try { EditConfig.Get_MPPCinfo(chip); }
		catch (const exception& e) {
		  Log.eWrite("[wgChangeConfig][" + OpStr.GetName(inputFile) + "] failed to get MPPC info :" + e.what());
		}
	  }

    // fine tuning mode
    if( flags[MPPC_DATA_FLAG] ) {
      try { EditConfig.Get_MPPCinfo(ichip); }
      catch (const exception& e) {
        Log.eWrite("[wgChangeConfig] failed to get MPPC info :" + string(e.what()));
      }
    }

    // Sanity check of the passed arguments
  
	  if(flags[EDIT_FLAG]) {
		if( mode == EC_TRIGGER_THRESHOLD || mode == EC_GAIN_SELECT_THRESHOLD) {
		  if(value < 0 || value > MAX_VALUE_10BITS) {
			Log.eWrite("[wgChangeConfig][" + OpStr.GetName(inputFile) + "] value is out of range : " + to_string(value));
			return ERR_VALUE_OUT_OF_RANGE;
		  }
		}
		else if( mode == EC_INPUT_DAC || mode == EC_HG_LG_AMPLIFIER || mode == EC_THRESHOLD_ADJUSTMENT) {
		  if(channel < 0 || channel > NCHANNELS) {
			Log.eWrite("[wgChangeConfig][" + OpStr.GetName(inputFile) + "] channel is out of range : " + to_string(channel));
			return ERR_CHANNEL_OUT_OF_RANGE;
		  }
		  else if( mode == EC_INPUT_DAC) {
			if(value < 0 || value > MAX_VALUE_8BITS){
			  Log.eWrite("[wgChangeConfig][" + OpStr.GetName(inputFile) + "] value is out of range : " + to_string(value));
			  return ERR_VALUE_OUT_OF_RANGE;
			}
		  }
		  else if( mode == EC_HG_LG_AMPLIFIER) {
			if(value < 0 || value > MAX_VALUE_6BITS) {
			  Log.eWrite("[wgChangeConfig][" + OpStr.GetName(inputFile) + "] value is out of range : " + to_string(value));
			  return ERR_VALUE_OUT_OF_RANGE;
			}
		  }
		  else if( mode == EC_THRESHOLD_ADJUSTMENT) {
			if(value < 0 || value > MAX_VALUE_4BITS){
			  Log.eWrite("[wgChangeConfig][" + OpStr.GetName(inputFile) + "] value is out of range : " + to_string(value));
			  return ERR_VALUE_OUT_OF_RANGE;
			}
		  }
		}
		else if(mode == EC_INPUT_DAC_REFERENCE) {
		  if(value != 0 && value != 1) {
			Log.eWrite("[wgChangeConfig][" + OpStr.GetName(inputFile) + "] value is out of range : " + to_string(value));
			return ERR_VALUE_OUT_OF_RANGE;
		  }
		}
		else {
		  Log.eWrite("[wgChangeConfig][" + OpStr.GetName(inputFile) + "] mode not recognized : " + to_string(mode));
		  return ERR_WRONG_MODE;
		}
	  } // if(flags[EDIT_FLAG])
	  else {
		EditConfig.CheckAll();
		return EC_SUCCESS;
	  }
  
	  try {
		  // Global threshold
		  if( mode == EC_TRIGGER_THRESHOLD ) {
			EditConfig.Change_trigth(value);
		  }
		  // Gain select threshold
		  else if( mode == EC_GAIN_SELECT_THRESHOLD) {
			EditConfig.Change_gainth(value);
		  }
		  else if( mode == EC_INPUT_DAC || mode == EC_HG_LG_AMPLIFIER || mode == EC_THRESHOLD_ADJUSTMENT) {

          // ************************** ALL CHANNELS ************************** //
		
          if(channel == NCHANNELS) {
            for(unsigned ichan = 0; ichan < NCHANNELS; ichan++) {
              // Input DAC
              if( mode == EC_INPUT_DAC ) {
                if(flags[MPPC_DATA_FLAG]) {
                  EditConfig.Change_inputDAC(ichan, value);
                }
                else {
                  EditConfig.Change_inputDAC(ichan, value);
                }
              }
              // HG and LG preamplifier feedback capacitor 
              else if( mode == EC_HG_LG_AMPLIFIER ) {
                EditConfig.Change_ampDAC(ichan, value);
              }
              // Threshold fine tuning
              else if( mode == EC_THRESHOLD_ADJUSTMENT ) {
                EditConfig.Change_trigadj(ichan, value);
              }
            }
          }

          // ************************** SINGLE CHANNEL ************************** //
		
          else {
            // Input DAC
            if( mode == EC_INPUT_DAC ) {
              if(flags[MPPC_DATA_FLAG]){
                EditConfig.Change_inputDAC(channel, value);
              }
              else {
                EditConfig.Change_inputDAC(channel, value);
              }
            }
            // HG and LG preamplifier feedback capacitor 
            else if( mode == EC_HG_LG_AMPLIFIER ) {	
              EditConfig.Change_ampDAC(channel, value);
            }
            // Threshold fine tuning
            else if( mode == EC_THRESHOLD_ADJUSTMENT ) {
              EditConfig.Change_trigadj(channel, value);
            } 
          }
        }

        // input DAC Voltage Reference (1 = internal 4.5V   0 = internal 2.5V)
	  
		  else if( mode == EC_INPUT_DAC_REFERENCE ) {
			EditConfig.Change_1bitparam(value, GLOBAL_INPUT_DAC_REF_START);
		  }
		  EditConfig.Write(outputFile);
	  }
	  catch (const exception &e) {
		Log.eWrite("[wgChangeConfig][" + OpStr.GetName(inputFile) + "] failed to write value :" + e.what());
		return ERR_FAILED_WRITE;
	  }
	} // catch the exception thrown by the EditConfig constructor
	catch (const exception& e) {
	  Log.eWrite("[wgChangeConfig][" + OpStr.GetName(inputFile) + "] failed to open the input file : " + e.what());
	}
  }
  return EC_SUCCESS;
}
