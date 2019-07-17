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
#include "wgConst.hpp"
#include "wgFileSystemTools.hpp"
#include "wgFit.hpp"
#include "wgFitConst.hpp"
#include "wgEditXML.hpp"
#include "wgLogger.hpp"
#include "wgTopology.hpp"
#include "wgAnaHist.hpp"

using namespace wagasci_tools;

//******************************************************************
void ModeSelect(const int mode, bitset<M>& flag){
  if ( mode == 1 || mode >= 10 )               flag[SELECT_DARK_NOISE] = true;
  if ( mode == 2 || mode >= 10 )               flag[SELECT_PEDESTAL]   = true;
  if ( mode == 3 || mode == 10 || mode >= 20 ) flag[SELECT_CHARGE]     = true;
  if ( mode == 4 || mode == 11 || mode >= 20 ) flag[SELECT_CHARGE_HG]  = true;
  if ( mode < 0  || mode > 20 )
    throw invalid_argument("Mode " + to_string(mode) + " not recognized"); 
}

//******************************************************************
int wgAnaHist(const char * x_inputFile,
              const char * x_configFile,
              const char * x_outputXMLDir,
              const char * x_outputIMGDir,
              int mode,
              const unsigned long flags_ulong,
              const unsigned idif_id) {

  string inputFile(x_inputFile);
  string configFile(x_configFile);
  string outputXMLDir(x_outputXMLDir);
  string outputIMGDir(x_outputIMGDir);
  wgEditXML Edit;
  

  // =========== FLAGS decoding =========== //

  bitset<M> flags(flags_ulong);
  
  // Set the correct flags according to the mode
  try { ModeSelect(mode, flags); }
  catch (const exception& e) {
    Log.eWrite("[wgAnaHist] Failed to " + string(e.what()));
    exit(1);
  }

  // =========== Arguments sanity check =========== //

  if(inputFile.empty() || !check_exist::RootFile(inputFile)) {
    Log.eWrite("[wgAnaHist] Input file not found : " + inputFile);
    return ERR_EMPTY_INPUT_FILE;
  }
  if ( flags[SELECT_CONFIG] && ( configFile.empty() || !check_exist::XmlFile(configFile)) ) {
    Log.eWrite("[wgAnaHist] Pyrame xml configuration file doesn't exist : " + configFile);
    exit(1);
  }
  if ( idif_id <= 0 || idif_id > NDIFS ) {
    Log.eWrite("[wgAnaHist] wrong DIF number : " + to_string(idif_id) );
    return ERR_WRONG_DIF_VALUE;
  }

  // =========== Topology =========== //

  Topology * topol;
  try {
    topol = new Topology(configFile);
  }
  catch (const exception& e) {
    Log.eWrite("[wgAnaHist] " + string(e.what()));
    return ERR_TOPOLOGY;
  }
  unsigned n_chips = topol->dif_map[idif_id].size();

  if ( n_chips <= 0 || n_chips > NCHIPS ) {
    Log.eWrite("[wgAnaHist] wrong number of chips : " + to_string(n_chips) );
    return ERR_WRONG_CHIP_VALUE;
  }

  // =========== Create output directories =========== //

  // ======= Create outputXMLDir ======= //
  try { MakeDir(outputXMLDir); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[wgAnaPedestal] " + string(e.what()));
    return ERR_FAILED_CREATE_DIRECTORY;
  }
  // ======= Create outputIMGDir ======= //
  if( flags[SELECT_PRINT] ) {
    for ( unsigned ichip = 1; ichip <= n_chips; ichip++ ) {
      unsigned n_chans = topol->dif_map[idif_id][ichip];
      for ( unsigned ichan_id = 1; ichan_id <= n_chans; ichan_id++ ) {
        string outputIMGChipChanDir(outputIMGDir + "/chip" + to_string(ichip) + "/chan" + to_string(ichan_id));
        try { MakeDir(outputIMGChipChanDir); }
        catch (const wgInvalidFile& e) {
          Log.eWrite("[wgAnaPedestal] " + string(e.what()));
          return ERR_FAILED_CREATE_DIRECTORY;
        }
      }
    }
  }

  // ======================================================== //
  //                                                          //
  //                        MAIN LOOP                         //
  //                                                          //
  // ======================================================== //
  
  try {
    wgFit Fit(inputFile, outputIMGDir);

    bool first_time = true;
    int start_time = 0;
    int stop_time = 0;

    ///////////////////////////////////////////////////////////////////////////
    //                               Chip loop                               //
    ///////////////////////////////////////////////////////////////////////////

    for (unsigned ichip_id = 1; ichip_id <= n_chips; ichip_id++) {
      unsigned n_chans = topol->dif_map[idif_id][ichip_id];

      // ============ Create outputXMLChipDir ============ //
      string outputXMLChipDir(outputXMLDir + "/chip" + to_string(ichip_id));
      try { MakeDir(outputXMLChipDir); }
      catch (const wgInvalidFile& e) {
        Log.eWrite("[wgAnaHist] " + string(e.what()));
        return ERR_FAILED_CREATE_DIRECTORY;
      }
	
      // v[channel][0] = global 10-bit discriminator threshold
      // v[channel][1] = global 10-bit gain selection discriminator threshold
      // v[channel][2] = adjustable input 8-bit DAC
      // v[channel][3] = adjustable 6-bit high gain (HG) preamp feedback capacitance
      // v[channel][4] = adjustable 4-bit discriminator threshold
      vector<vector<int>> config; // n_chans * 5 parameters

      Log.Write("[wgAnaHist] Analyzing chip " + to_string(ichip_id));
      // Read the SPIROC2D configuration parameters from the configFile (the xml
      // configuration file used during acquisition) into the "config" vector.
      if( flags[SELECT_CONFIG] ) {
        unsigned gdcc = topol->GetGdccDifPair(idif_id).first;
        unsigned dif = topol->GetGdccDifPair(idif_id).second;
        if ( ! Edit.GetConfig(configFile, gdcc, dif, ichip_id, n_chans, config) ) {
          Log.eWrite("[wgAnaHist] DIF " + to_string(idif_id) + ", chip " + to_string(ichip_id) +
                     " : failed to get bitstream parameters");
          return ERR_FAILED_GET_BISTREAM;
        }
      }

      /////////////////////////////////////////////////////////////////////////
      //                             Channel loop                            //
      /////////////////////////////////////////////////////////////////////////
      
      for(unsigned ichan_id = 1; ichan_id <= n_chans; ichan_id++) {
        // Open the outputxmlfile as an XML file
        string outputxmlfile(outputXMLChipDir + "/chan" + to_string(ichan_id) + ".xml");
        try {
          if( !check_exist::XmlFile(outputxmlfile) || flags[SELECT_OVERWRITE] )
            Edit.Make(outputxmlfile, idif_id, ichip_id, ichan_id);
          Edit.Open(outputxmlfile);
        }
        catch (const exception& e) {
          Log.eWrite("[wgAnaHist] Failed to open XML file : " + string(e.what()));
          return ERR_FAILED_OPEN_XML_FILE;
        }
	  
        // ******************* FILL THE XML FILES ********************//

        try {
          if (first_time) {
            start_time = Fit.GetHist->Get_start_time();
            stop_time  = Fit.GetHist->Get_stop_time();
            first_time = false;
          }
          Edit.SetConfigValue(string("start_time"), start_time);
          Edit.SetConfigValue(string("stop_time"),  stop_time);
          Edit.SetConfigValue(string("difid"),      idif_id);
          Edit.SetConfigValue(string("chipid"),     ichip_id);
          Edit.SetConfigValue(string("chanid"),     ichan_id);

          //************ SELECT_CONFIG ************//

          if ( flags[SELECT_CONFIG] ) {
            // Write the parameters values contained in the config vector into the
            // outputxmlfile
            Edit.SetConfigValue(string("trigth"),   config[ichan_id - 1][GLOBAL_THRESHOLD_INDEX], CREATE_NEW_MODE);
            Edit.SetConfigValue(string("gainth"),   config[ichan_id - 1][GLOBAL_GS_INDEX],        CREATE_NEW_MODE);
            Edit.SetConfigValue(string("inputDAC"), config[ichan_id - 1][ADJ_INPUTDAC_INDEX],     CREATE_NEW_MODE);
            Edit.SetConfigValue(string("HG"),       config[ichan_id - 1][ADJ_AMPDAC_INDEX],       CREATE_NEW_MODE);
            Edit.SetConfigValue(string("trig_adj"), config[ichan_id - 1][ADJ_THRESHOLD_INDEX],    CREATE_NEW_MODE);
          }

          //************* SELECT_DARK_NOISE *************//

          if ( flags[SELECT_DARK_NOISE] ) {  //for bcid
            double fit_bcid[2] = {0, 0};
            // calculate the dark noise rate for chip "ichip_id" and channel "ichan_id" and
            // save the mean and standard deviation in fit_bcid[0] and fit_bcid[1]
            // respectively.
            Fit.noise_rate(ichip_id, ichan_id, fit_bcid, flags[SELECT_PRINT]);
            // Save the noise rate and its standard deviation in the outputxmlfile xml
            // file
            Edit.SetChValue(string("noise_rate"), fit_bcid[0], CREATE_NEW_MODE); // mean
            Edit.SetChValue(string("sigma_rate"), fit_bcid[1], CREATE_NEW_MODE); // standard deviation
          }

          //************* SELECT_PEDESTAL *************//

          if ( flags[SELECT_PEDESTAL] ) {
            double fit_charge_nohit[3] = {0, 0, 0};
            for(unsigned icol_id = 1; icol_id <= MEMDEPTH; icol_id++) {
              // Calculate the pedestal value and its sigma
              Fit.charge_nohit(ichip_id, ichan_id, icol_id, fit_charge_nohit, flags[SELECT_PRINT]);
              Edit.SetColValue(string("charge_nohit"), icol_id, fit_charge_nohit[0], CREATE_NEW_MODE);
              Edit.SetColValue(string("sigma_nohit"),  icol_id, fit_charge_nohit[1], CREATE_NEW_MODE);
            } 
          }

          //************* SELECT_CHARGE *************//

          if ( flags[SELECT_CHARGE] ) {
            double fit_charge[3] = {0, 0, 0};
            for(unsigned icol_id = 1; icol_id <= MEMDEPTH; icol_id++) {
              Fit.charge_hit(ichip_id, ichan_id, icol_id, fit_charge, flags[SELECT_PRINT]);
              Edit.SetColValue(string("charge_hit"), icol_id, fit_charge[0], CREATE_NEW_MODE);
              Edit.SetColValue(string("sigma_hit") , icol_id, fit_charge[1], CREATE_NEW_MODE);
            }
          }

          //************* SELECT_CHARGE_HG *************//

          if ( flags[SELECT_CHARGE_HG] ) {
            double fit_charge_HG[3] = {0, 0, 0};
            for(unsigned icol_id = 1; icol_id <= MEMDEPTH; icol_id++) {
              Fit.charge_hit_HG(ichip_id, ichan_id, icol_id, fit_charge_HG, flags[SELECT_PRINT]);
              Edit.SetColValue(string("charge_hit_HG"), icol_id, fit_charge_HG[0], CREATE_NEW_MODE);
              Edit.SetColValue(string("sigma_hit_HG"),  icol_id, fit_charge_HG[1], CREATE_NEW_MODE);
            }
          }

          Edit.Write();
          Edit.Close();
        }
        catch (const exception& e) {
          Log.eWrite("[wgAnaHist] chip " + to_string(ichip_id) +
                     ", chan " + to_string(ichan_id) + " : " + string(e.what()));
          return ERR_FAILED_WRITE;
        } // try (write to xml files)
      } // ichan_id
    } //ichip_id
  } // try (wgFit)
  catch (const exception& e) {
    Log.eWrite("[wgAnaHist] " + string(e.what()));
    return ERR_FAILED_OPEN_HIST_FILE;
  }
  return WG_SUCCESS;
}
