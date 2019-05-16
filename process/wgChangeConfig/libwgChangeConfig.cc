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
#include "Const.h"
#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgFitConst.h"
#include "wgEditConfig.h"
#include "wgChangeConfig.hpp"

using namespace std;

int wgChangeConfig(const char * x_inputFile, const char * x_outputFile, const unsigned long x_flags, const int value, const int mode, const int ichip, const int channel) {
  
  // open txtfile ...
  OperateString OpStr;
  CheckExist Check;
  bitset<4> flags(x_flags);
  string inputFile(x_inputFile);
  string outputFile(x_outputFile);

  // check the existence and integrity of the input file
  if(inputFile == "") {
    Log.eWrite("[wgChangeConfig] No input file");
    return ERR_EMPTY_INPUT_FILE;
  }
  else if(!Check.TxtFile(inputFile)) {
    Log.eWrite("[wgChangeConfig][" + inputFile + "] input file doesn't exist");
    return ERR_INPUT_FILE_NOT_FOUND;
  }
  else if(!flags[OVERWRITE_FLAG] && flags[EDIT_FLAG] && Check.TxtFile(outputFile)) {
    Log.eWrite("[wgChangeConfig][" + OpStr.GetName(inputFile) + "] overwrite flag must be set in edit mode");
    return ERR_OVERWRITE_FLAG_NOT_SET;
  }
  else {
	try {
	  // Open the bitstream file
	  wgEditConfig EditConfig(inputFile, false); 

	  // fine tuning mode
	  if( flags[MPPC_DATA_FLAG] ) {
		try { EditConfig.Get_MPPCinfo(ichip); }
		catch (const exception& e) {
		  Log.eWrite("[wgChangeConfig][" + OpStr.GetName(inputFile) + "] failed to get MPPC info :" + e.what());
		}
	  }

	  // Sanity check of the passed arguments
  
	  if(flags[EDIT_FLAG]) {
		if( mode == EC_TRIGGER_THRESHOLD || mode == EC_GAIN_SELECT_THRESHOLD) {
		  if(value < 0 || value > MAX_VALUE_10BITS) {
			Log.eWrite("[wgChangeConfig][" + OpStr.GetName(inputFile) + "] value is out of range : " + to_string(value));
			return ERR_VALUE_OUT_OF_RANGE;
		  }
		  else {
			flags[CHECKOPT_FLAG] = true;
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
			else {
			  flags[CHECKOPT_FLAG]=true;
			}
		  }
		  else if( mode == EC_HG_LG_AMPLIFIER) {
			if(value < 0 || value > MAX_VALUE_6BITS) {
			  Log.eWrite("[wgChangeConfig][" + OpStr.GetName(inputFile) + "] value is out of range : " + to_string(value));
			  return ERR_VALUE_OUT_OF_RANGE;
			}
			else {
			  flags[CHECKOPT_FLAG]=true;
			}
		  }
		  else if( mode == EC_THRESHOLD_ADJUSTMENT) {
			if(value < 0 || value > MAX_VALUE_4BITS){
			  Log.eWrite("[wgChangeConfig][" + OpStr.GetName(inputFile) + "] value is out of range : " + to_string(value));
			  return ERR_VALUE_OUT_OF_RANGE;
			}
			else {
			  flags[CHECKOPT_FLAG]=true;
			}
		  }
		}
		else if(mode == EC_INPUT_DAC_REFERENCE) {
		  if(value == 0 || value == 1) flags[CHECKOPT_FLAG] = true;
		  else {
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
		if(flags[CHECKOPT_FLAG]) {
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
