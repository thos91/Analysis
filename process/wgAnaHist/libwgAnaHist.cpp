// system C++ includes
#include <string>
#include <iostream>
#include <vector>
#include <exception>
#include <bitset>
#include <iterator>

// boost includes
#include <boost/filesystem.hpp>

// ROOT includes
#include "TROOT.h"
#include "TError.h"

// user includes
#include "wgConst.hpp"
#include "wgErrorCodes.hpp"
#include "wgFileSystemTools.hpp"
#include "wgFit.hpp"
#include "wgFitConst.hpp"
#include "wgEditXML.hpp"
#include "wgLogger.hpp"
#include "wgTopology.hpp"
#include "wgAnaHist.hpp"

using namespace wagasci_tools;

// all doubles are cast to int when saving to XML files
// the unphysical values are stored as -1

//******************************************************************
int wgAnaHist(const char * x_input_hist_file,
              const char * x_xml_config_file,
              const char * x_output_xml_dir,
              const char * x_output_img_dir,
              const unsigned long ul_flags,
              unsigned dif_id) {

  std::bitset<anahist::NFLAGS> flags(ul_flags);
  std::string input_hist_file(x_input_hist_file);
  std::string xml_config_file(x_xml_config_file);
  std::string output_xml_dir(x_output_xml_dir);
  std::string output_img_dir(x_output_img_dir);
  wgEditXML xml;

  // =========== Arguments sanity check =========== //

  if(input_hist_file.empty() || !check_exist::root_file(input_hist_file)) {
    Log.eWrite("[wgAnaHist] Input file not found : " + input_hist_file);
    return ERR_EMPTY_INPUT_FILE;
  }
  if (flags[anahist::SELECT_CONFIG] &&
      (xml_config_file.empty() || !check_exist::xml_file(xml_config_file))) {
    Log.eWrite("[wgAnaHist] Xml xml configuration file doesn't exist : "
               + xml_config_file);
    return ERR_CONFIG_XML_FILE_NOT_FOUND;
  }
  if (dif_id > NDIFS) {
    Log.eWrite("[wgAnaHist] wrong DIF number : " + std::to_string(dif_id) );
    return ERR_WRONG_DIF_VALUE;
  }

  Log.Write("[wgAnaHist] *****  READING FILE     : " + input_hist_file    + "  *****");
  Log.Write("[wgAnaHist] *****  OUTPUT DIRECTORY : " + output_xml_dir     + "  *****");
  Log.Write("[wgAnaHist] *****  CONFIG FILE      : " + xml_config_file + "  *****");

  gErrorIgnoreLevel = kError;
  gROOT->SetBatch(kTRUE);
  
  // =========== Topology =========== //

  Topology * topol;
  try {
    topol = new Topology(xml_config_file);
  }
  catch (const std::exception& e) {
    Log.eWrite("[wgAnaHist] " + std::string(e.what()));
    return ERR_TOPOLOGY;
  }

  if (flags[anahist::SELECT_COMPATIBILITY])
    dif_id = std::next(topol->dif_map.begin(), dif_id)->first;
  unsigned n_chips = topol->dif_map[dif_id].size();

  if (n_chips == 0 || n_chips > NCHIPS) {
    Log.eWrite("[wgAnaHist] wrong number of chips : " + std::to_string(n_chips));
    return ERR_WRONG_CHIP_VALUE;
  }

  // =========== Create output directories =========== //

  // ======= Create output_xml_dir ======= //
  try { make::directory(output_xml_dir); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[wgAnaHist] " + std::string(e.what()));
    return ERR_FAILED_CREATE_DIRECTORY;
  }
  // ======= Create output_img_dir ======= //
  if (flags[anahist::SELECT_PRINT]) {
    for ( unsigned ichip = 0; ichip < n_chips; ichip++ ) {
      unsigned n_chans = topol->dif_map[dif_id][ichip];
      for ( unsigned ichan = 0; ichan < n_chans; ichan++ ) {
        std::string output_img_chip_chan_dir(output_img_dir +
                                             "/chip" + std::to_string(ichip) +
                                             "/chan" + std::to_string(ichan));
        try { make::directory(output_img_chip_chan_dir); }
        catch (const wgInvalidFile& e) {
          Log.eWrite("[wgAnaHist] " + std::string(e.what()));
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
    wgFit Fit(input_hist_file, output_img_dir);

    bool first_time = true;
    int start_time = 0;
    int stop_time = 0;

    ///////////////////////////////////////////////////////////////////////////
    //                               Chip loop                               //
    ///////////////////////////////////////////////////////////////////////////

    for (auto const &chip : topol->dif_map[dif_id]) {
      unsigned ichip = chip.first;
      unsigned n_chans = chip.second;

      // ============ Create output_xml_chip_dir ============ //
      std::string output_xml_chip_dir(output_xml_dir +
                                      "/chip" + std::to_string(ichip));
      try { make::directory(output_xml_chip_dir); }
      catch (const wgInvalidFile& e) {
        Log.eWrite("[wgAnaHist] " + std::string(e.what()));
        return ERR_FAILED_CREATE_DIRECTORY;
      }
	
      // v[channel][0] = global 10-bit discriminator threshold
      // v[channel][1] = global 10-bit gain selection discriminator threshold
      // v[channel][2] = adjustable input 8-bit DAC
      // v[channel][3] = adjustable 6-bit high gain (HG) preamp feedback capacitance
      // v[channel][4] = adjustable 4-bit discriminator threshold
      std::vector<std::vector<int>> config; // n_chans * 5 parameters

      Log.Write("[wgAnaHist] Analyzing chip " + std::to_string(ichip));
      // Read the SPIROC2D configuration parameters from the xml_config_file
      // (the xml configuration file used during acquisition) into the "config"
      // vector.
      if( flags[anahist::SELECT_CONFIG] ) {
        unsigned gdcc = topol->GetGdccDifPair(dif_id).first;
        unsigned dif = topol->GetGdccDifPair(dif_id).second;
        if (!xml.GetConfig(xml_config_file, gdcc, dif, ichip + 1,
                           n_chans, config)) {
          Log.eWrite("[wgAnaHist] DIF " + std::to_string(dif_id) + ", chip " +
                     std::to_string(ichip) + " : failed to get bitstream "
                     "parameters");
          return ERR_FAILED_GET_BISTREAM;
        }
      }

      /////////////////////////////////////////////////////////////////////////
      //                             Channel loop                            //
      /////////////////////////////////////////////////////////////////////////
      
      for(unsigned ichan = 0; ichan < n_chans; ichan++) {
        // Open the outputxmlfile as an XML file
        std::string outputxmlfile(output_xml_chip_dir +
                                  "/chan" + std::to_string(ichan) + ".xml");
        try {
          if( !check_exist::xml_file(outputxmlfile) ||
              flags[anahist::SELECT_OVERWRITE] )
            xml.Make(outputxmlfile, dif_id, ichip, ichan);
          xml.Open(outputxmlfile);
        }
        catch (const std::exception& e) {
          Log.eWrite("[wgAnaHist] Failed to open XML file : " +
                     std::string(e.what()));
          return ERR_FAILED_OPEN_XML_FILE;
        }
	  
        // ******************* FILL THE XML FILES ********************//
        try {
          if (first_time) {
            start_time = Fit.GetStartTime();
            stop_time  = Fit.GetStopTime();
            first_time = false;
          }
          xml.SetConfigValue(std::string("start_time"), start_time);
          xml.SetConfigValue(std::string("stop_time"),  stop_time);
          xml.SetConfigValue(std::string("difid"),      dif_id);
          xml.SetConfigValue(std::string("chipid"),     ichip);
          xml.SetConfigValue(std::string("chanid"),     ichan);

          //************ anahist::SELECT_CONFIG ************//

          if ( flags[anahist::SELECT_CONFIG] ) {
            // Write the parameters values contained in the config vector into the
            // outputxmlfile
            xml.SetConfigValue(std::string("trigth"),   config[ichan][GLOBAL_THRESHOLD_INDEX], CREATE_NEW_MODE);
            xml.SetConfigValue(std::string("gainth"),   config[ichan][GLOBAL_GS_INDEX],        CREATE_NEW_MODE);
            xml.SetConfigValue(std::string("inputDAC"), config[ichan][ADJ_INPUTDAC_INDEX],     CREATE_NEW_MODE);
            xml.SetConfigValue(std::string("HG"),       config[ichan][ADJ_AMPDAC_INDEX],       CREATE_NEW_MODE);
            xml.SetConfigValue(std::string("trig_adj"), config[ichan][ADJ_THRESHOLD_INDEX],    CREATE_NEW_MODE);
          }

          
          //************* anahist::SELECT_DARK_NOISE *************//

          if ( flags[anahist::SELECT_DARK_NOISE] ) {  //for bcid
            double fit_bcid[2] = {0, 0};
            // calculate the dark noise rate for chip "ichip" and channel
            // "ichan" and save the mean and standard deviation in fit_bcid[0]
            // and fit_bcid[1] respectively.
            Fit.NoiseRate(fit_bcid, dif_id, ichip, ichan,
                          flags[anahist::SELECT_PRINT]);
            // Save the noise rate and its standard deviation in the
            // outputxmlfile xml file
            xml.SetChValue(std::string("noise_rate"), fit_bcid[0],
                           CREATE_NEW_MODE); // mean
            xml.SetChValue(std::string("sigma_rate"), fit_bcid[1],
                           CREATE_NEW_MODE); // standard deviation
          }
          
          //************* anahist::SELECT_PEDESTAL *************//
          
          if ( flags[anahist::SELECT_PEDESTAL] ) {
            double fit_charge_nohit[3] = {0, 0, 0};
            for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
              // Calculate the pedestal value and its sigma
#ifdef ROOT_HAS_NOT_MINUIT2
              MUTEX.lock();
#endif
              try {
                Fit.ChargeNohit(fit_charge_nohit, dif_id, ichip, ichan, icol,
                                flags[anahist::SELECT_PRINT]);
              } catch (const wgElementNotFound &except) {
                std::stringstream ss;
                ss << "Histogram charge_nohit not found for "
                    "dif " << dif_id << " chip " << ichip
                   << " chan " << ichan << " : " << except.what();
                Log.eWrite(ss.str());
              } catch (const wgFitFailed &except) {
                std::stringstream ss;
                ss << "charge_nohit fit failed for "
                    "dif " << dif_id << " chip " << ichip
                   << " chan " << ichan << " : " << except.what();
                Log.eWrite(ss.str());
              }
#ifdef ROOT_HAS_NOT_MINUIT2
              MUTEX.unlock();
#endif
              xml.SetColValue(std::string("charge_nohit"), icol,
                              fit_charge_nohit[0], CREATE_NEW_MODE);
              xml.SetColValue(std::string("sigma_nohit"),  icol,
                              fit_charge_nohit[1], CREATE_NEW_MODE);
            } 
          }

          //************* anahist::SELECT_CHARGE_LG *************//

          if ( flags[anahist::SELECT_CHARGE_LG] ) {
            double fit_charge[3] = {0, 0, 0};
            for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
#ifdef ROOT_HAS_NOT_MINUIT2
              MUTEX.lock();
#endif
              try {
                Fit.ChargeHitLG(fit_charge, dif_id, ichip, ichan, icol,
                                flags[anahist::SELECT_PRINT]);
              } catch (const wgElementNotFound &except) {
                std::stringstream ss;
                ss << "Histogram charge_hit_LG not found for "
                    "dif " << dif_id << " chip " << ichip
                   << " chan " << ichan << " : " << except.what();
                Log.eWrite(ss.str());
              } catch (const wgFitFailed &except) {
                std::stringstream ss;
                ss << "charge_hit_LG fir failed for "
                    "dif " << dif_id << " chip " << ichip
                   << " chan " << ichan << " : " << except.what();
                Log.eWrite(ss.str());
              }
#ifdef ROOT_HAS_NOT_MINUIT2
              MUTEX.unlock();
#endif
              xml.SetColValue(std::string("charge_hit_LG"), icol,
                              fit_charge[0], CREATE_NEW_MODE);
              xml.SetColValue(std::string("sigma_hit_LG") , icol,
                              fit_charge[1], CREATE_NEW_MODE);
            }
          }

          //************* anahist::SELECT_CHARGE_HG *************//

          if ( flags[anahist::SELECT_CHARGE_HG] ) {
            double fit_charge_HG[3] = {0, 0, 0};
            for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
#ifdef ROOT_HAS_NOT_MINUIT2
              MUTEX.lock();
#endif
              try {
                Fit.ChargeHitHG(fit_charge_HG, dif_id, ichip, ichan, icol,
                                flags[anahist::SELECT_PRINT]);
              } catch (const wgElementNotFound &except) {
                std::stringstream ss;
                ss << "Histogram charge_hit_HG not found for "
                    "dif " << dif_id << " chip " << ichip
                   << " chan " << ichan << " : " << except.what();
                Log.eWrite(ss.str());
              } catch (const wgFitFailed &except) {
                std::stringstream ss;
                ss << "charge_hit_HG fir failed for "
                    "dif " << dif_id << " chip " << ichip
                   << " chan " << ichan << " : " << except.what();
                Log.eWrite(ss.str());
              }
#ifdef ROOT_HAS_NOT_MINUIT2
              MUTEX.unlock();
#endif
              xml.SetColValue(std::string("charge_hit_HG"), icol,
                              fit_charge_HG[0], CREATE_NEW_MODE);
              xml.SetColValue(std::string("sigma_hit_HG"),  icol,
                              fit_charge_HG[1], CREATE_NEW_MODE);
            }
          }

          xml.Write();
          xml.Close();
        }
        catch (const std::exception& e) {
          Log.eWrite("[wgAnaHist] chip " + std::to_string(ichip) +
                     ", chan " + std::to_string(ichan) + " : " +
                     std::string(e.what()));
          return ERR_FAILED_WRITE;
        } // try (write to xml files)
      } // ichan
    } //ichip
  } // try (wgFit)
  catch (const std::exception& e) {
    Log.eWrite("[wgAnaHist] " + std::string(e.what()));
    return ERR_FAILED_OPEN_HIST_FILE;
  }
  
  return WG_SUCCESS;
}
