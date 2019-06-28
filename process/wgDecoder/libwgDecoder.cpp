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
    struct tm y;
    strptime(this_year.c_str(), "%Y-%m-%d-%H-%M", &y);
    int DATETIME_STR = (int) mktime(&y);
    strptime(next_year.c_str(), "%Y-%m-%d-%H-%M", &y);
    int DATETIME_END = (int)mktime(&y);
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
  else Log.eWrite("PYRAME_LOG_FILE:" + pyrame_log_file + " doesn't exist!");

  // ===================================================================== //
  //                      Create TTree branches                            //
  // ===================================================================== //

  TTree * tree = new TTree("tree", "ROOT tree containing decoded data");
  tree->Branch("spill"       ,&rd.spill            ,Form("spill/I"                                                 ));
  tree->Branch("spill_mode"  ,&rd.spill_mode       ,Form("spill_mode/I"                                            ));
  tree->Branch("spill_flag"  ,&rd.spill_flag       ,Form("spill_flag/I"                                            ));
  tree->Branch("spill_count" ,&rd.spill_count      ,Form("spill_count/I"                                           ));
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
  tree->Branch("view"        ,&rd.view             ,Form("view/I"                                                  ));
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

  unsigned ievent = 0;
  unsigned bad_data = 0;

  int n_found_chips = 0;
  int chip_raw_data_line_count = 0;

  // 2 bytes (16 bits) are the smallest data unit
  bitset<M> rawData;
  vector<bitset<M>> rawDataStack;
  // The last four rawData values are kept in the lastFourLines array
  bitset<M> lastFourLines[4];

  uint16_t SPILL_NUMBER      = 0;
  uint16_t SPILL_MODE        = 0;
  uint32_t SPILL_COUNT       = 0;
  uint32_t LAST_SPILL_COUNT  = 0;
  uint16_t LAST_SPILL_NUMBER = 0;
  bool FILL_FLAG        = false;

  bool spillInsertTag   = false;
  bool endOfChipTag     = false;
  bool endOfSpillTag    = false;
  bool First_Event      = false;

  unsigned offset = setOffset(input_raw_file);
  
  // Pretend that we just reached the end of a previous spill so that the
  // first spill header is correctly recognized
  endOfSpillTag = true;
  lastFourLines[0] = x2020;

  ifstream ifs(input_raw_file, ios_base::in | ios_base::binary);
  if (!ifs.is_open()) {
    Log.eWrite("[wgDecoder] Failed to open raw file: " + string(strerror(errno)));
    return ERR_FAILED_OPEN_RAW_FILE;
  }
  
  while (ifs.read((char*) &rawData, M / 8) && ievent < maxEvt) {
    rawDataStack.push_back(rawData);
    chip_raw_data_line_count++;

    // IMPORTANT :
    // SPILL insert  --> start with 0xFFFB
    // SPILL header  --> start with 0xFFFC
    // CHIP  header  --> start with 0xFFFD
    // CHIP  trailer --> start with 0xFFFE
    // SPILL trailer --> start with 0xFFFF

    // After the spill insert 0xFFFB sequence there are 2 couples of 2 bytes
    // (2 x bitset<M>) , the first one is the spill number (SPILL_NUMBER) and
    // the second one is the spill flag (SPILL_MODE)
	  
    // *********   CHIP TRAILER  ********* //
    if (lastFourLines[1] == xFFFE && rawData == x2020) {
      endOfChipTag = true;
    }

    // *********   SPILL TRAILER  ********* //
    if (lastFourLines[2] == x2020 && lastFourLines[1] == xFFFF) {
      endOfSpillTag = true;
    }

    // **************************************************************//
    // ***********************  SPILL NUMBER  ********************** //
    // **************************************************************//
	  
    if (endOfSpillTag && lastFourLines[2] == xFFFB) {
      // If the most significative bit in the spill flag is set to BEAM_SPILL
      // the spill mode is set accordingly
      if ( lastFourLines[0][15] == BEAM_SPILL )
        SPILL_MODE = BEAM_SPILL;
      else
        SPILL_MODE = NON_BEAM_SPILL;
      if (SPILL_MODE == 1 ) LAST_SPILL_NUMBER = SPILL_NUMBER;

      // Spill number
      SPILL_NUMBER = lastFourLines[1].to_ulong();
      bitset<M> SPILL_GAP((int) LAST_SPILL_NUMBER + 1 - SPILL_NUMBER);

      // Sometimes a bit in the spill number can flip creating a spill gap
      // (the cause can be the ZedBoard misbehaving and in that case the spill
      // gap is not that bad, if the cause is something else the problem is
      // more serious)
      if( SPILL_GAP.to_ulong() != 0 ){
        Log.eWrite("[Decoder] spill gap warning: last = " + to_string(LAST_SPILL_NUMBER) + ", current = "
                   + to_string(SPILL_NUMBER));

        bool good_spill_gap = false;
        for (size_t i = 0; i < SPILL_GAP.size(); ++i)
          // for example in the case of i = 8 and size = 16 the following reads:
          //    if (SPILL_GAP == 0000000100000000) bad_spill_gap = true;
          if( SPILL_GAP == bitset<M>().set(i) ) good_spill_gap = true;
		  
        if ( good_spill_gap ) {
          for(unsigned ichip = 0; ichip < n_chips; ichip++) {
            rd.debug[ichip] += DEBUG_GOOD_SPILLGAP;
          }
        }else{
          for(unsigned ichip = 0; ichip < n_chips; ichip++){
            rd.debug[ichip] += DEBUG_BAD_SPILLGAP;
          }
        }
      }

      if( rawData == xFFFC )
        spillInsertTag = true; 
      else
        Log.eWrite("[Decoder] spill insert tag was inserted in wrong position, last_spill = "
                   + to_string(LAST_SPILL_COUNT) + ", current_spill = " + to_string(SPILL_COUNT)); 
    }

    // **************************************************************//
    // ***********************  SPILL HEADER  ********************** //
    // **************************************************************//

    if( lastFourLines[1] == xFFFC && endOfSpillTag ){
      if( !First_Event ) First_Event = true;
      if( endOfSpillTag ) {
        //Read SPILL information
        n_found_chips            = 0;
        LAST_SPILL_COUNT  = SPILL_COUNT;
        // Spill count most significant byte
        bitset<2*M> spill_count_msb = lastFourLines[0].to_ulong();
        spill_count_msb <<= M;
        // Spill count least significant byte
        bitset<2*M> spill_count_lsb = rawData.to_ulong();
        SPILL_COUNT = (spill_count_msb | spill_count_lsb).to_ullong();

        rd.spill_count   = SPILL_COUNT;
        rd.spill_flag    = n_chips;
        FILL_FLAG        = true;
         
        if( spillInsertTag ){
          rd.spill       = SPILL_NUMBER;
          rd.spill_mode  = SPILL_MODE;
          spillInsertTag = false;
        }else{
          rd.spill       = -1;
          rd.spill_mode  = -1;
        }

        uint32_t SPILL_COUNT_GAP = LAST_SPILL_COUNT + 1 - SPILL_COUNT;
        if( SPILL_COUNT_GAP != 0 )
          Log.eWrite("[Decoder] spill count gap warning: last_spill = " + to_string(LAST_SPILL_COUNT) +
                     ", current_spill = " + to_string(SPILL_COUNT)); 
          
        endOfSpillTag=false;
      }
      else {
        Log.eWrite("[Decoder] Warning : spill trailer not found: current_spill = " +
                   to_string(SPILL_COUNT));
      }
    }

    // **************************************************************//
    // ************************  CHIP HEADER  ********************** //
    // **************************************************************//
	  
    if (lastFourLines[2] == x2020 && lastFourLines[1] == xFFFD){
      chip_raw_data_line_count = 0;
    }

    // **************************************************************//
    // ***********************  CHIP TRAILER  ********************** //
    // **************************************************************//

    if (lastFourLines[1] == xFFFE && rawData == x2020){
      n_found_chips++;
      if (chip_raw_data_line_count >= 74) {
        // when 1 column or more is filled
        // if only 1 column 
        // => 36 charges + 36 time + 1 BCID + 1 chipID = 74
        // +2 +3 (header/trailer) = 79
        // let's replace this magic number 
      }else{          
        Log.eWrite("[Decoder] Warning : chip_raw_data_line_count is too small (chip_raw_data_line_count is " + to_string(chip_raw_data_line_count) + " words), acq id = " +
                   to_string(SPILL_COUNT) + ", chip number = " + to_string(n_found_chips + 1));
        rd.spill_flag--;
      }
    }

    // **************************************************************//
    // **********************   SPILL TRAILER  ********************* //
    // **************************************************************//

    if (First_Event && lastFourLines[3] == x2020 && lastFourLines[2] == xFFFF) {

      // Read the number of chips from the spill trailer
      uint16_t nbchip = ( rawData & x00FF ).to_ulong();
      if( n_found_chips != nbchip ) {
        // maybe one word was skipped
        nbchip = ( lastFourLines[0] & x00FF ).to_ulong();
        if( n_found_chips != nbchip ){
          Log.eWrite("[Decoder] Warning : number of chips mismatch : (chips found) n_found_chips = " + to_string(n_found_chips) +
                     ", (spill trailer) nbchip = " + to_string(nbchip) + ", acq id = " + to_string(SPILL_COUNT));
        }
      }
      if (n_found_chips > 0) {
        //=================  read Event ====================  //

        vector<bitset<M>>& eventData  = rawDataStack;

        ivector v_chipid(n_chips, -1);

        bitset<M> last          = 0;
        uint16_t lastChipID[3]  = {0,0,0};
        uint16_t currentChipID  = 0;
        uint16_t chip_count     = 0;
        bool chipid_exist       = false;
        int Missing_Header      = 0;
        uint32_t chipStartIndex = 0;
        uint32_t rawDataSize    = 0;
        uint32_t nColumns       = 0;
        bool isValidChip        = false;

        // ============================================
        //             Start reading event
        // ============================================

        for (size_t i = 0; i < eventData.size(); i++) {
          // CHIP START MARKER
          if (eventData[i] == xFFFD) {
			  
            chip_count++;
            // Sanity check of the chip header
            currentChipID = check_ChipHeader(n_chips, eventData, i+1, chipid_exist, Missing_Header);

            if(chipid_exist) {
              currentChipID--;
            }
            else {
              currentChipID = chip_count - 1;
#ifdef DEBUG_DECODE
              Log.eWrite("[Decoder] Warning : one chipid_tag is missing "
                         "(chipid_count: " + to_string(chip_count) + ", acq id: " + to_string(SPILL_COUNT) + ")");
#endif
              rd.debug[chip_count - 1] += DEBUG_MISSING_CHIPID_TAG;
            }

            lastChipID[2] = lastChipID[1];
            lastChipID[1] = lastChipID[0];
            lastChipID[0] = currentChipID;

            if( Missing_Header > 1 ) {
              Log.eWrite("[Decoder] Warning : part of chip header is missing (missing words: " + to_string(Missing_Header)  +
                         ", chipid_tag: " + to_string(chip_count) + ", acq id: " + to_string(SPILL_COUNT) + ")");;
              rd.debug[chip_count - 1] += DEBUG_MISSING_CHIP_HEADER;
            }
            chipStartIndex = i + CHIPHEAD - Missing_Header;
          }

          // CHIP END MARKER
          if (last == xFFFE) { 
            if ( (eventData[i] & x00FF) == currentChipID + 1) {
              isValidChip = true;
            }
            else{                    
              Log.eWrite("[Decoder] Warning : chip ID not found or invalid : current ChipID = " + to_string(currentChipID) +
                         ", read ChipID = " + to_string((eventData[i] & x00FF).to_ulong()) + ", acq id = " + to_string(SPILL_COUNT)); 
              bool matchChipID = false;
              if( lastChipID[0] + 1 < n_found_chips && lastChipID[0] > 0 ) {
                if( currentChipID == lastChipID[1] + 1) {
                  for(unsigned int i_int = i; i_int < i + CHIPHEAD; i_int++) {
                    if(eventData[i_int] == xFFFD) {
                      if( currentChipID + 1 == (uint16_t) ((eventData[i_int+1] & x00FF)).to_ulong() - 1 ) {
                        matchChipID = true;
                        rd.debug[currentChipID] += DEBUG_MISSING_CHIP_TRAILER;
                      }
                      break;
                    }
                  }
                }
              }
              else if( lastChipID[0] == 0 ) {
                if(n_found_chips==1) {
                  rd.debug[currentChipID] += DEBUG_MISSING_CHIP_TRAILER_ONLY_ONE_CHIP;
                }
                else {
                  for(unsigned int i_int = i; i_int< i + CHIPHEAD; i_int++) {
                    if(eventData[i_int] == xFFFD) {
                      if( currentChipID+1 == (uint16_t) ((eventData[i_int+1] & x00FF)).to_ulong() - 1 ) {
                        matchChipID=true;
                        rd.debug[currentChipID]+=DEBUG_MISSING_CHIP_TRAILER;
                      }
                      break;
                    }
                  }
                }
              }
              else if( lastChipID[0] == n_found_chips - 1 ) {
                if(currentChipID == lastChipID[1] + 1) {
                  rd.debug[currentChipID] += DEBUG_MISSING_CHIP_TRAILER;
                  matchChipID = true;
                }
              } 

              if(matchChipID) {
#ifdef DEBUG_DECODE
                Log.Write("[Decoder] Debug : the order of CHIPID is OK!");  
#endif
                isValidChip = true;
                rd.debug[currentChipID] += DEBUG_MISSING_CHIP_TRAILER;

              }
              else {
                isValidChip = false;
                rd.debug[currentChipID] += DEBUG_BAD_CHIPNUM;
                Log.eWrite("[Decoder] Warning : HEAD ChipID and END ChipID is wrong! (HEAD: " +
                           to_string(currentChipID) + ", END: " + to_string(eventData[i].to_ulong()) + ") spill: " + to_string(rd.spill));
                rd.spill_flag--;
              }
            }

            /* ============= Decode the SPIROC2D raw data =============== */
			  
            if (isValidChip) {
              // Check the rawDataSize
              rawDataSize = i - chipStartIndex - CHIPENDTAG - offset;
              nColumns    = (rawDataSize - CHIPIDSIZE) / (1 + NCHANNELS * 2);
              if ( (rawDataSize - CHIPIDSIZE) % (1 + NCHANNELS * 2) != 0) {
                Log.eWrite("[Decoder] Warning : BAD DATA SIZE! "
                           "spill: " + to_string(rd.spill) + ", chip: " + to_string(currentChipID) + ", nColumns: "  + to_string(nColumns));
                rd.spill_flag--;
                rd.debug[currentChipID] += DEBUG_BAD_CHIPDATA_SIZE;
                last = eventData[i].to_ulong();
                isValidChip = false;
                bad_data++;
                continue;
              }
              else if(nColumns > MEMDEPTH) {
                rd.debug[currentChipID] += DEBUG_BAD_CHIPDATA_SIZE;
                Log.eWrite("[Decoder] Warning : BAD COLUMN SIZE! "
                           "DataSize: " + to_string(rd.spill) + ", column: " + to_string(nColumns));
                rd.spill_flag--;
                last = eventData[i].to_ulong();
                isValidChip=false;
                continue;
              }

              // extract chipid
              v_chipid[currentChipID] = (eventData[i - CHIPENDTAG] & x00FF).to_ulong();

              if ( !check_ChipID(v_chipid[currentChipID], n_chips) ) {
                Log.eWrite("[Decoder] Warning : BAD CHIP ID! spill: " + to_string(rd.spill) +
                           " chipid: " + to_string(v_chipid[currentChipID]));
                rd.spill_flag--;
                rd.debug[currentChipID] += DEBUG_BAD_CHIPNUM;
              }

              rd.chipid[currentChipID] = v_chipid[currentChipID];
              rd.chipid_tag[currentChipID] = currentChipID;

              for (unsigned int ibc = 0; ibc < nColumns; ibc++) {

                /********************** BCID **********************/

                rd.bcid[currentChipID][ibc] = -1;
                rd.bcid[currentChipID][ibc] = ( eventData[i - CHIPENDTAG - CHIPIDSIZE - ibc] ).to_ulong();

                /********************** Charge and Time **********************/

                unsigned int begin = i - CHIPENDTAG - CHIPIDSIZE - nColumns - ibc * NCHANNELS * 2;
                unsigned int end   = begin - NCHANNELS;
                unsigned int ichan = 0;

                for (unsigned j = begin; j > end; j--) {
                  if (ibc < MEMDEPTH && ichan < n_channels) {
                    //extract charge
                    rd.charge [currentChipID][ichan][ibc] = (eventData[j] & x0FFF).to_ulong();
                    //extract hit (0: no hit, 1: hit)
                    rd.hit    [currentChipID][ichan][ibc] = eventData[j][12];
                    //extract gain (0: low gain, 1: high gain)
                    rd.gs    [currentChipID][ichan][ibc] = eventData[j][13];
                    // Only if the detector is already calibrated fill the histograms
                    if (pedestal_is_calibrated && adc_is_calibrated) {
                      //extract pe
                      if( rd.chipid[currentChipID] >= 0 && rd.chipid[currentChipID] < (int) n_chips ) {
                        if( rd.gs[currentChipID][ichan][ibc] == HIGH_GAIN_BIT ) { // High Gain
                          rd.pe[currentChipID][ichan][ibc] = HIGH_GAIN_NORM * ( rd.charge[currentChipID][ichan][ibc] - rd.pedestal[rd.chipid[currentChipID]][ichan][ibc] ) /
                                                             rd.gain[rd.chipid[currentChipID]][ichan][ibc];
                        }
                        else if( rd.gs[currentChipID][ichan][ibc] == LOW_GAIN_BIT ) { // Low Gain
                          rd.pe[currentChipID][ichan][ibc] = LOW_GAIN_NORM * ( rd.charge[currentChipID][ichan][ibc] - rd.pedestal[rd.chipid[currentChipID]][ichan][ibc] ) /
                                                             rd.gain[rd.chipid[currentChipID]][ichan][ibc];
                        }
                        else {
                          stringstream ss;
                          ss << "[Decoder] Warning : BAD GAIN BIT! " <<
                              "gs[" << currentChipID << "][" << ichan << "][" << ibc << "] = " << rd.gs[currentChipID][ichan][ibc];
                          Log.eWrite(ss.str());
                          rd.pe[currentChipID][ichan][ibc] = -100.;
                        }
                      }
                    }
                  }
                  else if (ibc >= MEMDEPTH || ichan >= NCHANNELS) {
                    stringstream ss;
                    ss << "[Decoder] Warning : BAD CHANNEL OR COLUMN NUMBER! " <<
                        "chip = " << currentChipID << ", channel = " << ichan << ", column = " << ibc;
                    Log.eWrite(ss.str());
                  }
                  ichan++;  
                } // loop on j

                begin = end;
                end   = begin - NCHANNELS;
                ichan = 0;

                for (unsigned j = begin ; j > end ; j--) {
                  if (ibc < MEMDEPTH && ichan < n_channels) {
                    //extract time
                    rd.time[currentChipID][ichan][ibc] = (eventData[j] & x0FFF).to_ulong();
                  }
                  ichan++;
                } // loop on j
              } // for iColumn
              isValidChip = false;
            } // isValidChip
          } // if last = 0xFFFE
          last = eventData[i].to_ulong();;
        } // loop on all the elements of eventData

        // ============================================
        //           Finished reading event
        // ============================================

        if (tdc_is_calibrated) {
          tdc2time(rd.time_ns, rd.time, rd.bcid, rd.tdc_slope, rd.tdc_intcpt);
        }
        
        // ***** Fill TTree ***** //

        tree->Fill();
        FILL_FLAG = false;
        for(unsigned ichip = 0; ichip < n_chips; ichip++) v_chipid[ichip] = -1;
        rd_clear(rd);

        if( ievent % 1000 == 0 ) Log.Write("Entry number ... " + to_string(ievent));
        ievent++;
      }
      else {
        Log.eWrite("[Decoder] Warning : number of chips less than zero : spill count = " + to_string(rd.spill_count));
      }
    }
    // SPILL trailer check
    if ( lastFourLines[3] == xFFFF ) { 
      // Clear everything before we start again with the next event
      rawDataStack.clear();
      endOfChipTag = false;
      rd_clear(rd);

      // If the tree was not filled at the end of the event, fill it now with
      // the "empty" values.
      if (FILL_FLAG) {
        Log.eWrite("[Decoder] Warning : no data recorded : spill count = " + to_string(rd.spill_count));
        for(unsigned ichip = 0 ; ichip < n_chips ; ichip++) {
          rd.debug[ichip] += DEBUG_NODATA;
        }
        tree->Fill(); 
        for(unsigned int ichip = 0; ichip < n_chips; ichip++) {
          rd.debug[ichip] = 0;
        }
      }
      FILL_FLAG = false;
    }
    // ************************ End of SPILL TRAILER ************************** //

    // Unexpected spill header
    if (lastFourLines[1] == x5053 && lastFourLines[0] == x4C49 &&  rawData == x2020) { 
      if (endOfChipTag) {
        Log.eWrite("[Decoder] Warning : spill trailer is missing : spill number = " + to_string(rd.spill));
      }
      rawDataStack.clear();
      endOfChipTag = false;
    }

    lastFourLines[3] = lastFourLines[2];
    lastFourLines[2] = lastFourLines[1];
    lastFourLines[1] = lastFourLines[0];
    lastFourLines[0] = rawData;

  } // while (ifs.read(...

  Log.Write("[Decoder] *****  Finished reading file  *****");
  Log.Write("[Decoder] *****  with " + to_string(ievent) + " entries  *****");
  Log.Write("[Decoder] *****  BAD data : " + to_string(bad_data) + " *****");

  outputTFile->cd();
  tree->Write();
  outputTFile->Close();
  Log.Write("[Decoder] Decode is finished");
  return DE_SUCCESS;
}

//******************************************************************************

uint16_t check_ChipHeader(unsigned n_chips, vector<bitset<M>>& head, size_t offset, bool& checkid_exist, int& Missing_Header){
  int miss = 0;
  uint16_t ret;
  if (head.size() < offset + CHIPHEAD)
    return 0xFFFF;
  if( (head[offset+0] & x00FF).to_ulong() > 0 && (head[offset+0] & x00FF).to_ulong() <= n_chips ){
    ret = head[offset+0].to_ulong();
    checkid_exist = true;
  }
  else {
#ifdef DEBUG_DECODE
    string s(Form("%#04x - %#04x - %#04x - %#04x", (int) head[offset].to_ulong(), (int) head[offset+1].to_ulong(), (int) head[offset+2].to_ulong(), (int) head[offset+3].to_ulong()));
    Log.Write("[check_ChipHeader] Debug: chip header = " + s); 
#endif
    ret = 0xFFFF;
    for (size_t i = offset + CHIPHEAD - 1; i > offset; i--) head[i] = head[i-1];
    miss++;
    checkid_exist = false;
  }

  if( head[offset+1] != x4843 ){
    miss++;
    if( head[offset+1] != x5049){
      miss++;
      if( head[offset+1] !=x2020){
        miss++;
      }
    }else{
      if(head[offset+2] != x2020) miss++;
    }
  }else{
    if( head[offset+2]!= x5049){
      miss++;
      if( head[offset+2] !=x2020){
        miss++;
      }
    }else{
      if( head[offset+3]!= x2020) miss++;
    }
  }
  Missing_Header = miss;
  return (ret & 0x00FF);
}

//******************************************************************************

bool check_ChipID(int16_t v_chipid, uint16_t n_chips) {
  if ( v_chipid > n_chips ) return false;
  else return true;
}

//******************************************************************************

void tdc2time(d3vector &time_ns, i3vector &time, i2vector &bcid, d3vector &slope, d3vector &intcpt) {
  int Parity;
  for(size_t ichip = 0; ichip < time.size(); ichip++){
    for(size_t ich = 0; ich < time[ichip].size(); ich++){
      for(size_t icol = 0; icol < time[ichip][ich].size(); icol++) {
        if( bcid[ichip][icol]%2 == 0 ) { Parity = TDC_RAMP_EVEN; }
        else                           { Parity = TDC_RAMP_ODD;  }
        time_ns[ichip][ich][icol] = (time[ichip][ich][icol] - intcpt[ichip][ich][Parity]) / slope[ichip][ich][Parity] + (bcid[ichip][icol] - Parity) * BCIDwidth;
      }
    }
  }
}

void rd_clear(Raw_t &rd) {
  // Scalars
  rd.spill            = -1;
  rd.spill_mode       = -1;
  rd.spill_count      = -1;
  rd.spill_flag       =  1;
  // Standard vectors
  unsigned n_chips = rd.chipid.size();
  for(unsigned ichip = 0 ; ichip < n_chips ; ichip++) {
    rd.chipid[ichip] = -1;
    rd.debug[ichip]  =  0;
  }
  // 2D contiguous vectors
  rd.bcid.fill(-1);
  rd.gain.fill(-100);
  // 3D contiguous vectors
  rd.charge.fill(-1);
  rd.time.fill(-1);
  rd.gs.fill(-1);
  rd.hit.fill(-1);
  rd.pedestal.fill(-100);
  rd.pe.fill(-100);
  rd.time_ns.fill(-100);
  rd.tdc_slope.fill(0);
  rd.tdc_intcpt.fill(-100);
}

unsigned setOffset(string & input_raw_file) {
  ifstream ifs;
  ifs.open(input_raw_file.c_str(), ios_base::in | ios_base::binary);
  if (!ifs.is_open()) {
    Log.eWrite("[wgDecoder] Failed to open raw file: " + string(strerror(errno)));
    return ERR_FAILED_OPEN_RAW_FILE;
  }

  // unsigned chipID = 0;
  
  // 2 bytes (16 bits) are the smallest data unit in all the headers and trailers
  bitset<M> rawData;
  unsigned nRawData = 0;
  // The last four rawData values are kept in the lastFour array
  bitset<M> lastFourLines[4];
  lastFourLines[0] = x2020;

  /////////////////////////////////////////////////////////////////////////////
  //             Check that the number of columns is consistent              //
  /////////////////////////////////////////////////////////////////////////////

  // One column length in 16 bits lines (but the BCID)
  // 1 BCID              +
  // n_channels times    +
  // n_channels charges
  const unsigned oneColumnLength = 1 + 2 * NCHANNELS;

  const unsigned offset = 1;
  unsigned error_count = 0;
  
  // Read M / 8 bytes of raw data and put it into rawData
  while ( ifs.read((char * ) &rawData, M / 8) ) {

    // CHIP HEADER :
    // Reset the counter
    if (lastFourLines[2] == x4843 && lastFourLines[1] == x5049 && lastFourLines[0] == x2020 ) {
      nRawData = 0;
    }

    nRawData++;
    
    // CHIP TRAILER :
    // Count the columns
    if (lastFourLines[3] == xFFFE && lastFourLines[1] == x2020 && lastFourLines[0] == x2020 ) {
      if ( (nRawData - CHIPTRAIL - CHIPIDSIZE - offset) % oneColumnLength != 0 ) {
        error_count++;
      }
    }

    lastFourLines[3] = lastFourLines[2];
    lastFourLines[2] = lastFourLines[1];
    lastFourLines[1] = lastFourLines[0];
    lastFourLines[0] = rawData;

    if (error_count > HOW_MANY_FAILED_TRIES) {
      ifs.close();
      return offset;
    }
  }

  ifs.close();
  return 0;
}
