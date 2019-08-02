// system C++ includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <bitset>
#include <vector>
#include <iomanip>
#include <locale>

// boost includes
#include <boost/filesystem.hpp>

// ROOT includes
#include "TFile.h"
#include "TH1I.h"
#include "TTree.h"
#include "TParameter.h"

// user includes
#include "wgConst.hpp"
#include "wgFileSystemTools.hpp"
#include "wgExceptions.hpp"
#include "wgGetCalibData.hpp"
#include "wgChannelMap.hpp"
#include "wgRawData.hpp"
#include "wgDecoder.hpp"
#include "wgDecoderSeeker.hpp"
#include "wgDecoderReader.hpp"
#include "wgDecoderUtils.hpp"
#include "wgLogger.hpp"

using namespace wagasci_tools;

int wgDecoder(const char * x_input_raw_file,
              const char * x_calibration_dir,
              const char * x_output_dir,
              const bool overwrite,
              const bool compatibility_mode,
              unsigned dif,
              unsigned n_chips) {

  std::string input_raw_file(x_input_raw_file);
  std::string calibration_dir(x_calibration_dir);
  std::string output_dir(x_output_dir);
  std::string output_file_name = GetName(input_raw_file) + "_tree.root";

  // ===================================================================== //
  //                         Arguments sanity check                        //
  // ===================================================================== //

  // ======== input_raw_file ========= //
  
  if (input_raw_file.empty() || !check_exist::RawFile(input_raw_file)) { 
    Log.eWrite("[wgDecoder] Input file doesn't exist : " + input_raw_file);
    return ERR_INPUT_FILE_NOT_FOUND;
  }

  // ======== calibration_dir ========= //

  if (calibration_dir.empty()) {
    wgEnvironment env;
    calibration_dir = env.CONF_DIRECTORY;
  }

  // ======== dif ========= //

  if (dif > NDIFS) {
    Log.eWrite("[wgDecoder] The DIF number must be {1-" + std::to_string(NDIFS) + "}");
    exit(1);
  }
  
  // ======== n_chips ========= //

  if (n_chips == 0) {
    n_chips = wagasci_decoder_utils::GetNumChips(input_raw_file);
  }
  if (n_chips == 0 || n_chips > NCHIPS) {
    Log.eWrite("[wgDecoder] The number of chips per DIF must be {1-"
               + std::to_string(NCHIPS) + "}");
    exit(1);
  }

  // ============ pyrame_log_file ============ //

  // This is not the output log file but the log file that should be already
  // present in the input folder and was created together with the .raw file
  std::string pyrame_log_file  = GetName(input_raw_file);
  size_t pos  = pyrame_log_file.rfind("_ecal_dif_") ;
  pyrame_log_file = GetPath(input_raw_file) + pyrame_log_file.substr(0, pos) + ".log";

  // ============ Create output_dir ============ //
  
  try { MakeDir(output_dir); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[wgDecoder] " + std::string(e.what()));
    return ERR_FAILED_CREATE_DIRECTORY;
  }

  Log.Write("[wgDecoder] READING FILE     : " + input_raw_file      );
  Log.Write("[wgDecoder] OUTPUT TREE FILE : " + output_file_name );
  Log.Write("[wgDecoder] OUTPUT DIRECTORY : " + output_dir      );

  // ===================================================================== //
  //                        DIF number detection                           //
  // ===================================================================== //

  // If the number of DIFs is not provided as an argument, try to infer it from
  // the file name
  if (dif == 0) {
    std::string input_raw_file_name = wagasci_tools::GetName(input_raw_file);
    if ((pos = input_raw_file_name.find("dif_1_1_")) != std::string::npos) {
      try {
        dif = stoi(input_raw_file_name.substr(pos + 8, pos + 9));
      } catch (const std::invalid_argument& e) {
        Log.eWrite("[wgDecoder] failed to read the DIF number from the file name : " + std::string(e.what()));
      }
    } else if ((pos = input_raw_file_name.find("dif_")) != std::string::npos) {
      try {
        dif = stoi(input_raw_file_name.substr(pos + 4, pos + 5));
      } catch (const std::invalid_argument& e) {
        Log.eWrite("[wgDecoder] failed to read the DIF number from the file name : " + std::string(e.what()));
      }
    } else {
      Log.eWrite("[wgDecoder] Error: DIF ID number not given nor found");
      return ERR_WRONG_DIF_VALUE;
    }
  }

  // ===================================================================== //
  //                  Allocate the raw data Raw_t class                    //
  // ===================================================================== //
  
  Raw_t rd(n_chips);
  
  // ===================================================================== //
  //                        Get calibration data                           //
  // ===================================================================== //
  
  // Read the value of pedestal, TDC ramp and gain from the calibration files
  bool pedestal_is_calibrated = false;
  bool gain_is_calibrated = false;
  bool adc_is_calibrated = false;
  bool tdc_is_calibrated = false;
  try {
    wgGetCalibData calib(calibration_dir, dif);
    try {
      if ((pedestal_is_calibrated = calib.isPedestalCalibrated()))
        calib.GetPedestal(dif, rd.pedestal);
    } catch (const exception & e ) {
      pedestal_is_calibrated = false;
      Log.Write("[wgDecoder] pedestal is not calibrated yet : " + std::string(e.what()));
    }
    try {
      if ((gain_is_calibrated = calib.isGainCalibrated()))
        calib.GetGain(dif, rd.gain);
    } catch (const exception & e ) {
      adc_is_calibrated = false;
      Log.Write("[wgDecoder] ADC is not calibrated yet : " + std::string(e.what()));
    }
    try {
      if ((tdc_is_calibrated = calib.isTDCCalibrated()))
        calib.GetTDC(dif, rd.tdc_slope, rd.tdc_intcpt);
    } catch (const exception & e ) {
      tdc_is_calibrated = false;
      Log.Write("[wgDecoder] TDC is not calibrated yet : " + std::string(e.what()));
    }  
  } catch (const exception & e ) {
    Log.Write("[wgDecoder] detector is not calibrated yet : " + std::string(e.what())); 
  }
  adc_is_calibrated = pedestal_is_calibrated && gain_is_calibrated;

  // ===================================================================== //
  //                             Get mapping                               //
  // ===================================================================== //
  
  // Get the geometrical information (position in space) for each channel
  wgChannelMap Map;
  if (adc_is_calibrated) {
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
  //                         Create the output file                        //
  // ===================================================================== //

  TFile * output_file;
  TString output_file_path(output_dir + "/" + output_file_name);
  if (!overwrite) {
    if (check_exist::RootFile(output_file_path)) {
      Log.eWrite("[wgDecoder] Error:" + std::string(output_file_path.Data()) +
                 " already exists!");
      return ERR_OVERWRITE_FLAG_NOT_SET;
    }
    output_file = new TFile(output_file_path, "create");
  } else {
    output_file = new TFile(output_file_path, "recreate");
  }


  // ===================================================================== //
  //                      Create TTree branches                            //
  // ===================================================================== //

  TString tree_name("tree_dif_" + std::to_string(dif));
  TString tree_title("ROOT tree containing decoded data : DIF " + std::to_string(dif));
  TTree * tree = new TTree(tree_name, tree_title);
  tree->SetDirectory(output_file);
  tree->Branch("spill_number",&rd.spill_number     ,"spill_number/I"                                               );
  tree->Branch("spill_mode"  ,&rd.spill_mode       ,"spill_mode/I"                                                 );
  tree->Branch("spill_count" ,&rd.spill_count      ,"spill_count/I"                                                );
  tree->Branch("bcid"        ,rd.bcid.data()       ,Form("bcid[%d][%d]/I"           ,n_chips,            MEMDEPTH ));
  tree->Branch("charge"      ,rd.charge.data()     ,Form("charge[%d][%d][%d]/I"     ,n_chips, NCHANNELS, MEMDEPTH ));
  tree->Branch("time"        ,rd.time.data()       ,Form("time[%d][%d][%d]/I"       ,n_chips, NCHANNELS, MEMDEPTH ));
  tree->Branch("gs"          ,rd.gs.data()         ,Form("gs[%d][%d][%d]/I"         ,n_chips, NCHANNELS, MEMDEPTH ));
  tree->Branch("hit"         ,rd.hit.data()        ,Form("hit[%d][%d][%d]/I"        ,n_chips, NCHANNELS, MEMDEPTH ));
  tree->Branch("chipid"      ,rd.chipid.data()     ,Form("chipid[%d]/I"             ,n_chips                      ));
  tree->Branch("col"         ,rd.col.data()        ,Form("col[%d]/I"                ,                    MEMDEPTH ));
  tree->Branch("chan"        ,rd.chan.data()       ,Form("chan[%d]/I"               ,         NCHANNELS           ));
  tree->Branch("chip"        ,rd.chip.data()       ,Form("chip[%d]/I"               ,n_chips                      ));
  tree->Branch("debug_chip"  ,rd.debug_chip.data() ,Form("debug_chip[%d][%d]/I"     ,n_chips, N_DEBUG_CHIP        ));
  tree->Branch("debug_spill" ,rd.debug_spill.data(),Form("debug_spill[%d]/I"        ,N_DEBUG_SPILL                ));
  tree->Branch("view"        ,&rd.view             ,"view/I"                                                       );
  tree->Branch("pln"         ,rd.pln.data()        ,Form("pln[%d][%d]/I"            ,n_chips, NCHANNELS           ));
  tree->Branch("ch"          ,rd.ch.data()         ,Form("ch[%d][%d]/I"             ,n_chips, NCHANNELS           ));
  tree->Branch("grid"        ,rd.grid.data()       ,Form("grid[%d][%d]/I"           ,n_chips, NCHANNELS           ));
  tree->Branch("x"           ,rd.x.data()          ,Form("x[%d][%d]/D"              ,n_chips, NCHANNELS           ));
  tree->Branch("y"           ,rd.y.data()          ,Form("y[%d][%d]/D"              ,n_chips, NCHANNELS           ));
  tree->Branch("z"           ,rd.z.data()          ,Form("z[%d][%d]/D"              ,n_chips, NCHANNELS           ));
  if (adc_is_calibrated) {
    tree->Branch("pedestal"  ,rd.pedestal.data()   ,Form("pedestal[%d][%d][%d]/D"   ,n_chips, NCHANNELS, MEMDEPTH ));
    tree->Branch("pe"        ,rd.pe.data()         ,Form("pe[%d][%d][%d]/D"         ,n_chips, NCHANNELS, MEMDEPTH ));
    tree->Branch("gain"      ,rd.gain.data()       ,Form("gain[%d][%d][%d]/D"       ,n_chips, NCHANNELS, MEMDEPTH ));
  }
  if (tdc_is_calibrated) {
    tree->Branch("time_ns"   ,rd.time_ns.data()    ,Form("time_ns[%d][%d][%d]/D"    ,n_chips, NCHANNELS, MEMDEPTH ));
    tree->Branch("tdc_slope" ,rd.tdc_slope.data()  ,Form("tdc_slope[%d][%d][%d]/D"  ,n_chips, NCHANNELS, 2        ));
    tree->Branch("tdc_intcpt",rd.tdc_intcpt.data() ,Form("tdc_intcpt[%d][%d][%d]/D" ,n_chips, NCHANNELS, 2        ));
  }

  // ===================================================================== //
  //                        Read Pyrame log file                           //
  // ===================================================================== //

  if( check_exist::LogFile(pyrame_log_file) ) {
    // Will be filled with  v[0]: start_time, v[1]: stop_time, v[2]: nb_data_pkts, v[3]: nb_lost_pkts
    std::vector<std::string> v_log;
    wgEditXML edit;
    edit.GetLog(pyrame_log_file, v_log);
    // Start time of the acquisition that produced the .raw file
    tree->GetUserInfo()->Add(new TParameter<Int_t>("start_time", datetime::DatetimeToSeconds(v_log[0])));
    // Stop time of the acquisition that produced the .raw file
    tree->GetUserInfo()->Add(new TParameter<Int_t>("stop_time", datetime::DatetimeToSeconds(v_log[1])));
    // Number of data packets acquired as reported by Pyrame in the acquisition log file
    tree->GetUserInfo()->Add(new TParameter<Int_t>("nb_data_pkts", std::stoi(v_log[2])));
    // Number of lost packets acquired as reported by Pyrame in the acquisition log file
    tree->GetUserInfo()->Add(new TParameter<Int_t>("nb_lost_pkts", std::stoi(v_log[3])));
  }
  else Log.eWrite("[wgDecoder]  Pyrame log file : " + pyrame_log_file + " doesn't exist!");

  // ===================================================================== //
  //                Allocate the RawDataConfig class object                //
  // ===================================================================== //

  unsigned n_chip_id;
  if (compatibility_mode) n_chip_id = 1;
  else n_chip_id = wagasci_decoder_utils::GetNumChipID(input_raw_file);
  RawDataConfig config(n_chips,
                       NCHANNELS,
                       n_chip_id,
                       wagasci_decoder_utils::HasSpillNumber(input_raw_file),
                       wagasci_decoder_utils::HasPhantomMenace(input_raw_file),
                       adc_is_calibrated,
                       tdc_is_calibrated);

  // ===================================================================== //
  //     ============================================================      //
  //                          READ THE RAW FILE                            //
  //     ============================================================      //
  // ===================================================================== //

  ifstream ifs;
  ifs.open(input_raw_file.c_str(), ios_base::in | ios_base::binary);
  if (!ifs.is_open()) {
    Log.eWrite("[wgDecoder] Failed to open raw file: " + std::string(strerror(errno)));
    return ERR_FAILED_OPEN_RAW_FILE;
  }

  unsigned n_good_spills = 0, n_bad_spills = 0;
  int result;
  try {
    SectionSeeker seeker(config);
    SectionReader reader(config, tree, rd);
    int last_spill_count = -1;
    unsigned last_section_type = 0;
    while (true) {

      // ============ Seek and read next section ============ //
      
      SectionSeeker::Section current_section = seeker.SeekNextSection(ifs);
      reader.ReadNextSection(ifs, current_section);

      // ============ If the raw data was correctly read  ============ //
      // ============ this is a good spill otherwise this ============ //
      // ============ is a bad spill.                     ============ //
      
      if (current_section.type == SectionSeeker::SectionType::RawData) {
        if (current_section.ispill == (unsigned) last_spill_count + 1) {
          ++n_good_spills;
        } else {
          n_bad_spills += current_section.ispill - last_spill_count;
        }
        last_spill_count = current_section.ispill;
      } else if (current_section.type == SectionSeeker::SectionType::ChipTrailer &&
                 last_section_type != SectionSeeker::SectionType::RawData) {
        n_bad_spills += current_section.ispill - last_spill_count;
        last_spill_count = current_section.ispill;
      }

      // ============ Print the progress every 1000 spills ============ //
      
      if (current_section.type == SectionSeeker::SectionType::SpillTrailer) {
        rd.clear();
        if ((n_good_spills + n_bad_spills) % 1000 == 0) {
          Log.Write("[wgDecoder] Decoded " + std::to_string(n_good_spills + n_bad_spills) + " spills");
        }
      }
      
      last_section_type = current_section.type;
    }
  } catch (const wgEOF& e) {
    result = WG_SUCCESS;
  } catch (const std::exception& e) {
    Log.eWrite("[wgDecoder] Error while reading raw data : " + std::string(e.what()));
    result = ERR_WG_DECODER;
  }

  // ===================================================================== //
  //                           Close everything                            //
  // ===================================================================== //

  ifs.close();
  tree->Write();
  output_file->Close();
  delete output_file;

  Log.Write("[wgDecoder] *****  GOOD spills : " + std::to_string(n_good_spills) + " spills *****");
  Log.Write("[wgDecoder] *****  BAD  spills : " + std::to_string(n_bad_spills) + " spills *****");
  
  return result;
}

