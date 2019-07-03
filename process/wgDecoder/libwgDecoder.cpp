// system C++ includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <limits>

// system C includes
#include "unistd.h"
#include <bits/stdc++.h>

// boost includes
#include <boost/filesystem.hpp>

// ROOT includes
#include "TDirectory.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TTree.h"
#include "TInterpreter.h"

// user includes
#include "wgConst.hpp"
#include "wgFileSystemTools.hpp"
#include "wgExceptions.hpp"
#include "wgGetCalibData.hpp"
#include "wgChannelMap.hpp"
#include "wgDecoder.hpp"
#include "wgLogger.hpp"

using namespace std;
using namespace wagasci_tools;

int wgDecoder(const char * x_input_raw_file,
              const char * x_calibration_dir,
              const char * x_output_dir,
              const bool overwrite,
              unsigned maxEvt,
              unsigned dif,
              unsigned n_chips,
              unsigned n_channels) {

  string input_raw_file(x_input_raw_file);
  string calibration_dir(x_calibration_dir);
  string output_dir(x_output_dir);
  string output_file_name = GetName(input_raw_file)+"_tree.root";
  wgConst con;

  // ===================================================================== //
  //                         Arguments sanity check                        //
  // ===================================================================== //

  // ======== maxEvt ========= //
  
  if ( maxEvt == 0 ) {
    maxEvt = MAX_EVENT;
  }

  // ======== input_raw_file ========= //
  
  if (input_raw_file.empty() || !check_exist::RawFile(input_raw_file)) { 
    Log.eWrite("[wgDecoder] Input file doesn't exist : " + input_raw_file);
    return ERR_INPUT_FILE_NOT_FOUND;
  }

  // ======== pedestal_card ========= //

  if (calibration_dir.empty()) {
    calibration_dir = con.CONF_DIRECTORY;
  }

  // ======== dif ========= //

  if (dif > NDIFS) {
    Log.eWrite("[wgDecoder] The DIF number must be {1-" + to_string(NDIFS) + "}");
    exit(1);
  }
  
  // ======== n_chips ========= //

  if( n_chips > NCHIPS ) {
    Log.eWrite("[wgDecoder] The number of chips per DIF must be {1-"
               + to_string(NCHIPS) + "}");
    exit(1);
  }
  if ( n_chips == 0 ) n_chips = NCHIPS;

  // ======== n_channels ========= //
  
  if( n_channels > NCHANNELS ) {
    Log.eWrite("[wgDecoder] The number of channels per DIF must be {1-"
               + to_string(NCHANNELS) + "}");
    exit(1);
  }
  if ( n_channels == 0 ) n_channels = NCHANNELS;
  
  // ============ pyrame_log_file ============ //

  // This is not the output log file but the log file that should be already
  // present in the input folder and was created together with the .raw file
  string pyrame_log_file  = GetName(input_raw_file);
  size_t pos  = pyrame_log_file.rfind("_ecal_dif_") ;
  pyrame_log_file = GetPath(input_raw_file) + pyrame_log_file.substr(0, pos) + ".log";

  // ============ Create output_dir ============ //
  
  try { MakeDir(output_dir); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[Decoder] " + string(e.what()));
    return ERR_CANNOT_CREATE_DIRECTORY;
  }

  Log.Write("[Decoder] READING FILE     :" + input_raw_file      );
  Log.Write("[Decoder] OUTPUT TREE FILE :" + output_file_name );
  Log.Write("[Decoder] OUTPUT DIRECTORY :" + output_dir      );

  // ===================================================================== //
  //                  Allocate the raw data Raw_t class                    //
  // ===================================================================== //
  
  Raw_t rd(n_chips, n_channels);
  rd.spill      = -1;
  rd.spill_flag = n_chips;

  // ===================================================================== //
  //                        DIF number detection                           //
  // ===================================================================== //

  // If the number of DIFs is not provided as an argument, try to infer it from
  // the file name
  if (dif == 0) {
    if ((pos = input_raw_file.find("dif_1_1_")) != string::npos)
      try {
        dif = stoi(input_raw_file.substr(pos + 8, pos + 9));
      } catch (const invalid_argument & e) {
        Log.eWrite("[Decoder] failed to read the DIF number from the file name : " + string(e.what()));
      } else if ((pos = input_raw_file.find("dif")) != string::npos)
      try {
        dif = stoi(input_raw_file.substr(pos + 3, pos + 4));
      } catch (const invalid_argument & e) {
        Log.eWrite("[Decoder] failed to read the DIF number from the file name : " + string(e.what()));
      } else {
      Log.eWrite("[Decoder] Error: DIF ID number not given nor found");
      return ERR_WRONG_DIF_VALUE;
    }
  }

  // ===================================================================== //
  //                             Get mapping                               //
  // ===================================================================== //
  
  // Get the geometrical information (position in space) for each channel
  wgChannelMap Map;
  {
    vector<int> pln, ch, grid;
    vector<double> x, y, z;
    for(unsigned ichip = 0; ichip < n_chips; ichip++) {
      Map.GetMap( dif,
                  ichip,
                  rd.pln [ichip].size(),
                  rd.view,
                  rd.pln [ichip].data(),
                  rd.ch  [ichip].data(),
                  rd.grid[ichip].data(),
                  rd.x   [ichip].data(),
                  rd.y   [ichip].data(),
                  rd.z   [ichip].data());
    }
  }

  // ===================================================================== //
  //                        Get calibration data                           //
  // ===================================================================== //
  
  // Read the value of pedestal, TDC ramp and gain from the calibration files
  bool pedestal_is_calibrated = false;
  bool adc_is_calibrated = false;
  bool tdc_is_calibrated = false;
  try {
    wgGetCalibData calib(calibration_dir, dif);
    try {
      if ((pedestal_is_calibrated = calib.isPedestalCalibrated()))
        calib.GetPedestal(dif, rd.pedestal);
    } catch (const exception & e ) {
      pedestal_is_calibrated = false;
      Log.Write("[wgDecoder] pedestal is not calibrated yet : " + string(e.what()));
    }
    try {
      if ((adc_is_calibrated = calib.isADCCalibrated()))
        calib.GetADC(dif, rd.gain);
    } catch (const exception & e ) {
      adc_is_calibrated = false;
      Log.Write("[wgDecoder] ADC is not calibrated yet : " + string(e.what()));
    }
    try {
      if ((tdc_is_calibrated = calib.isTDCCalibrated()))
        calib.GetTDC(dif, rd.tdc_slope, rd.tdc_intcpt);
    } catch (const exception & e ) {
      tdc_is_calibrated = false;
      Log.Write("[wgDecoder] TDC is not calibrated yet : " + string(e.what()));
    }  
  } catch (const exception & e ) {
    Log.Write("[wgDecoder] detector is not calibrated yet : " + string(e.what())); 
  }

  // ===================================================================== //
  //                        Read Pyrame log file                           //
  // ===================================================================== //

  if( check_exist::LogFile(pyrame_log_file) ) {
    
    // Will be filled with  v[0]: start_time, v[1]: stop_time, v[2]: nb_data_pkts, v[3]: nb_lost_pkts
    vector<int> v_log(4,0);
    wgEditXML *Edit = new wgEditXML();
    Edit->GetLog(pyrame_log_file, v_log);
    delete Edit;

    time_t current_time = time(0);
    tm *tm = localtime(&current_time);
    string this_year(Form("%04d-%02d-%02d-%02d-%02d", tm->tm_year+1900, 1, 1, 0, 0));
    string next_year(Form("%04d-%02d-%02d-%02d-%02d", tm->tm_year+1900+1, 1, 1, 0, 0));
    struct tm date;
    strptime(this_year.c_str(), "%Y-%m-%d-%H-%M", &date);
    int DATETIME_STR = (int) mktime(&date);
    strptime(next_year.c_str(), "%Y-%m-%d-%H-%M", &date);
    int DATETIME_END = (int) mktime(&date);
    int DATETIME_BIN = DATETIME_END - DATETIME_STR;

    // Start time of the acquisition that produced the .raw file
    TH1D * h_start_time = new TH1D("start_time", "Time the acquisition started", DATETIME_BIN, DATETIME_STR, DATETIME_END);
    h_start_time -> Fill(v_log[0], v_log[0]);
    h_start_time -> Fill(v_log[0] + 1, v_log[0] - DATETIME_END);
    // Stop time of the acquisition that produced the .raw file
    TH1D * h_stop_time = new TH1D("stop_time","Time the acquisition stopped", DATETIME_BIN, DATETIME_STR, DATETIME_END);
    h_stop_time -> Fill(v_log[1], v_log[1]);
    h_stop_time -> Fill(v_log[1] + 1, v_log[1] - DATETIME_END);
    // Number of data packets acquired as reported by Pyrame in the acquisition log file
    TH1D * h_nb_data_pkts = new TH1D("nb_data_pkts","Number of data packets", DATETIME_BIN, DATETIME_STR, DATETIME_END);
    h_nb_data_pkts -> Fill(v_log[2]);
    // Number of lost packets acquired as reported by Pyrame in the acquisition log file
    TH1D * h_nb_lost_pkts = new TH1D("nb_lost_pkts","Number of lost packests", DATETIME_BIN, DATETIME_STR, DATETIME_END);
    h_nb_lost_pkts -> Fill(v_log[3]);

    h_start_time  ->Write();
    h_stop_time   ->Write();
    h_nb_data_pkts->Write();
    h_nb_lost_pkts->Write();
  }
  else Log.eWrite("PYRAME_LOG_FILE: " + pyrame_log_file + " doesn't exist!");

  // ===================================================================== //
  //                      Create TTree branches                            //
  // ===================================================================== //

  TTree * tree = new TTree("tree", "ROOT tree containing decoded data");
  tree->Branch("spill"       ,&rd.spill            ,"spill/I"                                                       );
  tree->Branch("spill_mode"  ,&rd.spill_mode       ,"spill_mode/I"                                                  );
  tree->Branch("spill_flag"  ,&rd.spill_flag       ,"spill_flag/I"                                                  );
  tree->Branch("spill_count" ,&rd.spill_count      ,"spill_count/I"                                                 );
  tree->Branch("bcid"        ,rd.bcid.data()       ,Form("bcid[%d][%d]/I"           ,n_chips,             MEMDEPTH ));
  tree->Branch("charge"      ,rd.charge.data()     ,Form("charge[%d][%d][%d]/I"     ,n_chips, n_channels, MEMDEPTH ));
  tree->Branch("time"        ,rd.time.data()       ,Form("time[%d][%d][%d]/I"       ,n_chips, n_channels, MEMDEPTH ));
  tree->Branch("gs"          ,rd.gs.data()         ,Form("gs[%d][%d][%d]/I"         ,n_chips, n_channels, MEMDEPTH ));
  tree->Branch("hit"         ,rd.hit.data()        ,Form("hit[%d][%d][%d]/I"        ,n_chips, n_channels, MEMDEPTH ));
  tree->Branch("chipid_tag"  ,rd.chipid_tag.data() ,Form("chipid_tag[%d]/I"         ,n_chips                       ));
  tree->Branch("chipid"      ,rd.chipid.data()     ,Form("chipid[%d]/I"             ,n_chips                       ));
  tree->Branch("col"         ,rd.col.data()        ,Form("col[%d]/I"                ,                     MEMDEPTH ));
  tree->Branch("chipch"      ,rd.chipch.data()     ,Form("chipch[%d]/I"             ,         n_channels           ));
  tree->Branch("chip"        ,rd.chip.data()       ,Form("chip[%d]/I"               ,n_chips                       ));
  tree->Branch("debug"       ,rd.debug.data()      ,Form("debug[%d]/I"              ,n_chips                       ));
  tree->Branch("view"        ,&rd.view             ,"view/I"                                                        );
  tree->Branch("pln"         ,rd.pln.data()        ,Form("pln[%d][%d]/I"            ,n_chips, n_channels           ));
  tree->Branch("ch"          ,rd.ch.data()         ,Form("ch[%d][%d]/I"             ,n_chips, n_channels           ));
  tree->Branch("grid"        ,rd.grid.data()       ,Form("grid[%d][%d]/I"           ,n_chips, n_channels           ));
  tree->Branch("x"           ,rd.x.data()          ,Form("x[%d][%d]/D"              ,n_chips, n_channels           ));
  tree->Branch("y"           ,rd.y.data()          ,Form("y[%d][%d]/D"              ,n_chips, n_channels           ));
  tree->Branch("z"           ,rd.z.data()          ,Form("z[%d][%d]/D"              ,n_chips, n_channels           ));
  if (pedestal_is_calibrated) {
    tree->Branch("pedestal"  ,rd.pedestal.data()   ,Form("pedestal[%d][%d][%d]/D"   ,n_chips, n_channels, MEMDEPTH ));
  }
  if (adc_is_calibrated) {
    tree->Branch("pe"        ,rd.pe.data()         ,Form("pe[%d][%d][%d]/D"         ,n_chips, n_channels, MEMDEPTH ));
    tree->Branch("gain"      ,rd.gain.data()       ,Form("gain[%d][%d][%d]/D"       ,n_chips, n_channels, MEMDEPTH ));
  }
  if (tdc_is_calibrated) {
    tree->Branch("time_ns"   ,rd.time_ns.data()    ,Form("time_ns[%d][%d][%d]/D"    ,n_chips, n_channels, MEMDEPTH ));
    tree->Branch("tdc_slope" ,rd.tdc_slope.data()  ,Form("tdc_slope[%d][%d][%d]/D"  ,n_chips, n_channels, 2        ));
    tree->Branch("tdc_intcpt",rd.tdc_intcpt.data() ,Form("tdc_intcpt[%d][%d][%d]/D" ,n_chips, n_channels, 2        ));
  }

  // ===================================================================== //
  //                         Create the output file                        //
  // ===================================================================== //

  TFile * outputTFile;
  TString output_file_path(output_dir + "/" + output_file_name);
  if (!overwrite) {
    if (check_exist::RootFile(output_file_path)) {
      Log.eWrite("[Decoder] Error:" + string(output_file_path.Data()) +
                 " already exists!");
      return ERR_CANNOT_OVERWRITE_OUTPUT_FILE;
    }
    outputTFile = new TFile(output_file_path, "create");
  } else {
    outputTFile = new TFile(output_file_path, "recreate");
  }
  
  // ===================================================================== //
  //     ============================================================      //
  //                          READ THE RAW FILE                            //
  //     ============================================================      //
  // ===================================================================== //




  
  Log.Write("[Decoder] *****  Finished reading file  *****");
  Log.Write("[Decoder] *****  with " + to_string(ievent) + " entries  *****");
  Log.Write("[Decoder] *****  BAD data : " + to_string(bad_data) + " *****");

  outputTFile->cd();
  tree->Write();
  outputTFile->Close();
  Log.Write("[Decoder] Decode is finished");
  return DE_SUCCESS;
}
