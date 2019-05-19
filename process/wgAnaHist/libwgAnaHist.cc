// system C++ includes
#include <string>
#include <iostream>
#include <vector>

// boost includes
#include <boost/filesystem.hpp>

// system C includes
#include <bits/stdc++.h>

// ROOT includes
#include <THStack.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TH1.h>

// user includes
#include "Const.h"
#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgFit.h"
#include "wgFitConst.h"
#include "wgEditXML.h"
#include "wgAnaHist.hpp"

//******************************************************************
void ModeSelect(const int mode, bitset<M>& flag){
  if ( mode == 1 || mode >= 10 )               flag[SELECT_DARK_NOISE]     = true;
  if ( mode == 2 || mode >= 11 )               flag[SELECT_PEDESTAL]       = true;
  if ( mode == 3 || mode == 10 || mode >= 20 ) flag[SELECT_CHARGE_LOW]     = true;
  if ( mode == 4 || mode == 12 || mode >= 20 ) flag[SELECT_CHARGE_HG_LOW]  = true;
  if ( mode == 5 || mode == 13 || mode >= 20 ) flag[SELECT_CHARGE_HG_HIGH] = true;
  if ( mode < 0  || mode > 20 )
	throw invalid_argument("Mode " + to_string(mode) + " not recognized"); 
}

//******************************************************************
int AnaHist(const char * x_inputFileName,
			const char * x_configFileName,
			const char * x_outputDir,
			const char * x_outputIMGDir,
			const int mode,
			const unsigned long flags_ulong,
			const unsigned idif,
			const unsigned n_chips,
			const unsigned n_chans) {

  string inputFileName(x_inputFileName);
  string configFileName(x_configFileName);
  string outputDir(x_outputDir);
  string outputIMGDir(x_outputIMGDir);
  wgEditXML Edit;
  CheckExist Check;

  bitset<M> flags(flags_ulong);
  
  // Set the correct flags according to the mode
  try {
	ModeSelect(mode, flags);
  }
  catch (const exception& e) {
	Log.eWrite("[wgAnaHist][" + outputDir + "] " + string(e.what()));
	return ERR_WRONG_MODE;
  }

  // ============ Create outputDir ============ //
  if( !Check.Dir(outputDir) ) {
	boost::filesystem::path dir(outputDir);
	if( !boost::filesystem::create_directory(dir) ) {
	  Log.eWrite("[wgAnaHist][" + outputDir + "] failed to create directory");
	  return ERR_CANNOT_CREATE_DIRECTORY;
	}
  }
  // ============ Create outputIMGDir ============ //
  if( flags[SELECT_PRINT] && !Check.Dir(outputIMGDir) ) {
	boost::filesystem::path dir(outputIMGDir);
	if( !boost::filesystem::create_directory(dir) ) {
	  Log.eWrite("[wgAnaHist][" + outputIMGDir + "] failed to create directory");
	  return ERR_CANNOT_CREATE_DIRECTORY;
	}
  }
  try {
	wgFit Fit(inputFileName);
	if( flags[SELECT_PRINT] )
	  Fit.SetoutputIMGDir(outputIMGDir);

	for (unsigned ichip = 0; ichip < n_chips; ichip++) {
	  // ============ Create outputChipDir ============ //
	  string outputChipDir(outputDir + "/chip" + to_string(ichip));
	  if ( !Check.Dir(outputChipDir) ) {
		boost::filesystem::path dir(outputChipDir);
		if ( !boost::filesystem::create_directory(dir) ) {
		  Log.eWrite("[wgAnaHist][" + outputChipDir + "] failed to create directory");
		  return ERR_CANNOT_CREATE_DIRECTORY;
		}
	  }
	
	  // v[channel][0] = global 10-bit discriminator threshold
	  // v[channel][1] = global 10-bit gain selection discriminator threshold
	  // v[channel][2] = adjustable input 8-bit DAC
	  // v[channel][3] = adjustable 6-bit high gain (HG) preamp feedback capacitance
	  // v[channel][4] = adjustable 4-bit discriminator threshold
	  vector<vector<int>> config; // n_chans * 5 parameters

	  Log.Write("[wgAnaHist] Analyzing chip " + to_string(ichip + 1));
	  // Read the SPIROC2D configuration parameters from the configFileName (the xml
	  // configuration file used during acquisition) into the "config" vector.
	  if( flags[SELECT_CONFIG] )
		if ( ! Edit.GetConfig(configFileName, idif, ichip + 1, n_chans, config) ) {
		  Log.eWrite("[wgAnaHist][" + configFileName + "] DIF " + to_string(idif) + ", chip " + to_string(ichip) +
					 " : failed to get bitstream parameters");
		  return ERR_FAILED_GET_BISTREAM;
		}

	  // For the idif DIF and ichip chip, loop over all the channels
	  for(unsigned ichan = 0; ichan < n_chans; ichan++) {
		// Open the outputxmlfile as an XML file
		string outputxmlfile(outputChipDir + "/ch" + to_string(ichan) + ".xml");
		try {
		  if( !Check.XmlFile(outputxmlfile) || flags[OVERWRITE] )
			Edit.Make(outputxmlfile, ichip, ichan);
		  Edit.Open(outputxmlfile);
		}
		catch (const exception& e) {
		  Log.eWrite("[wgAnaHist][" + outputxmlfile + "] Failed to open XML file : " + string(e.what()));
		  return ERR_FAILED_OPEN_XML_FILE;
		}
	  
		// ******************** FILL THE XML FILES **********************//

		try {
		  int start_time;
		  int stop_time;
		  if(ichip == 0) {
			start_time = Fit.GetHist->Get_start_time();
			stop_time  = Fit.GetHist->Get_stop_time();
		  }
		  Edit.SetConfigValue(string("start_time"), start_time);
		  Edit.SetConfigValue(string("stop_time"), stop_time);

		  //************ SELECT_CONFIG ************//

		  if ( flags[SELECT_CONFIG] ) {
			// Write the parameters values contained in the config vector into the
			// outputxmlfile
			Edit.SetConfigValue(string("trigth"),   config[ichan][GLOBAL_THRESHOLD_INDEX], CREATE_NEW_MODE);
			Edit.SetConfigValue(string("gainth"),   config[ichan][GLOBAL_GS_INDEX], CREATE_NEW_MODE);
			Edit.SetConfigValue(string("inputDAC"), config[ichan][ADJ_INPUTDAC_INDEX], CREATE_NEW_MODE);
			Edit.SetConfigValue(string("HG"),       config[ichan][ADJ_AMPDAC_INDEX], CREATE_NEW_MODE);
			Edit.SetConfigValue(string("trig_adj"), config[ichan][ADJ_THRESHOLD_INDEX], CREATE_NEW_MODE);
		  }

		  //************* SELECT_DARK_NOISE *************//

		  if ( flags[SELECT_DARK_NOISE] ) {  //for bcid
			double x_bcid[2] = {0, 0};
			// calculate the dark noise rate for chip "ichip" and channel "ichan" and
			// save the mean and standard deviation in x_bcid[0] and x_bcid[1]
			// respectively.
			Fit.NoiseRate(ichip, ichan, x_bcid, (int) flags[SELECT_PRINT]);
			// Save the noise rate and its standard deviation in the outputxmlfile xml
			// file
			Edit.SetChValue(string("NoiseRate"),   x_bcid[0], CREATE_NEW_MODE); // mean
			Edit.SetChValue(string("NoiseRate_e"), x_bcid[1], CREATE_NEW_MODE); // standard deviation
		  }

		  //************* SELECT_PEDESTAL *************//

		  if ( flags[SELECT_PEDESTAL] ) {
			double x_nohit[3] = {0, 0, 0};
			for(int icol = 0; icol < MEMDEPTH; icol++) {
			  // Calculate the pedestal value and its sigma
			  Fit.charge_nohit(ichip, ichan, icol, x_nohit, (int) flags[SELECT_PRINT]);
			  Edit.SetColValue(string("charge_nohit"), icol, x_nohit[0], CREATE_NEW_MODE);
			  Edit.SetColValue(string("sigma_nohit"),  icol, x_nohit[1], CREATE_NEW_MODE);
			} 
		  }

		  //************* SELECT_CHARGE_LOW *************//

		  if ( flags[SELECT_CHARGE_LOW] ) {
			double x_low[3] = {0, 0, 0};
			Fit.low_pe_charge(ichip, ichan, x_low, (int) flags[SELECT_PRINT]);
			Edit.SetChValue(string("charge_low"),x_low[0], CREATE_NEW_MODE);
			Edit.SetChValue(string("sigma_low") ,x_low[1], CREATE_NEW_MODE);
		  }

		  //************* SELECT_CHARGE_HG_LOW *************//

		  if ( flags[SELECT_CHARGE_HG_LOW] ) {
			double x_low_HG[3] = {0, 0, 0};
			for(int icol = 0; icol < MEMDEPTH; icol++) {
			  Fit.low_pe_charge_HG(ichip, ichan, icol, x_low_HG, (int) flags[SELECT_PRINT]);
			  Edit.SetColValue(string("charge_lowHG"), icol, x_low_HG[0], CREATE_NEW_MODE);
			  Edit.SetColValue(string("sigma_lowHG"),  icol, x_low_HG[1], CREATE_NEW_MODE);
			}
		  }

		  //************* SELECT_CHARGE_HG_HIGH *************//

		  if ( flags[SELECT_CHARGE_HG_HIGH] ) {
			for(int icol = 0; icol < MEMDEPTH; icol++) {
			  double x_high_HG[3] = {0, 0};
			  Fit.GainSelect(ichip, ichan, icol, x_high_HG, (int) flags[SELECT_PRINT]);
			  Edit.SetColValue(string("GS_eff_m"), icol, x_high_HG[0], CREATE_NEW_MODE);
			  Edit.SetColValue(string("GS_eff_e"), icol, x_high_HG[1], CREATE_NEW_MODE);
			}
		  }
		  Edit.Write();
		  Edit.Close();
		}
		catch (const exception& e) {
		  Log.eWrite("[wgAnaHist][" + outputxmlfile + "] chip " + to_string(ichip) +
					 ", chan " + to_string(ichan) + " : " + string(e.what()));
		  return ERR_FAILED_WRITE;
		} // try (write to xml files)
	  } // ichan
	} //ichip
  } // try (wgFit)
  catch (const exception& e) {
	Log.eWrite("[wgAnaHist][" + inputFileName + "] " + string(e.what()));
	return ERR_FAILED_OPEN_HIST_FILE;
  }
  return AH_SUCCESS;
}
