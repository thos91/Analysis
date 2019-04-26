#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <stdio.h>

#include "TDirectory.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TTree.h"
#include "unistd.h"

#include "Const.h"
#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgGetCalibData.h"
#include "wgChannelMap.h"

#define DEBUG_NODATA 1
#define DEBUG_GOOD_SPILLGAP 2
#define DEBUG_BAD_SPILLGAP 4
#define DEBUG_BAD_CHIPNUM 8
#define DEBUG_BAD_CHIPDATA_SIZE 16
#define DEBUG_MISSING_CHIPID_TAG 32
#define DEBUG_MISSING_CHIP_HEADER 64
#define DEBUG_MISSING_CHIP_TRAILER 128
#define DEBUG_MISSING_CHIP_TRAILER_ONLY_ONE_CHIP 256
#define DEBUG_DECODE
#ifndef DEBUG
#define DEBUG
#endif

using namespace std;

void ReadFile(string inputFileName, string calibFileName, bool overwrite, unsigned int maxEvt,string outputDir, string logoutputDir, int n_difs = 0, int n_chips = 0, int n_channels = 0);
unsigned short int check_StartChipIndex(unsigned short int head1,unsigned short int head2,unsigned short int head3,unsigned short  int head4, bool* checkid_exist, int* Missing_Header);
bool check_ChipID(int v_chipid);
void tdc2time(int time[20][36][16],int bcid[20][16],double time_ns[20][36][16],double slope[2][20][36],double intcpt[2][20][36]);

OperateString *OptStr;
CheckExist    *check;
Logger        *Log;

int main(int argc, char** argv) {
  OptStr =  new OperateString;
  check  =  new CheckExist;
  Log    =  new Logger;
  wgConst *con = new wgConst();
  con->GetENV();
  int opt;
  string inputFileName("");
  string calibFileName("");
  string outputFile("");
  string outputDir    = con->DECODE_DIRECTORY;
  string logoutputDir = con->LOG_DIRECTORY;
  bool overwrite = false; 
  delete con;
  Log->Initialize();

  while((opt = getopt(argc,argv, "f:i:o:rh")) !=-1 ){
    switch(opt){
      case 'f':
        inputFileName=optarg;
        if(!check->RawFile(inputFileName)){ 
          cout<<"!!Error!! "<<inputFileName.c_str()<<"is wrong!!"<<endl;
          Log->eWrite(Form("[%s][Decoder]!!target is wrong!!",inputFileName.c_str()));
          return 1;
        }
        delete check;
        cout << "== readfile :" << inputFileName.c_str() << " ==" << endl;
        Log->Write(Form("[%s][Decoder] start decodeing",inputFileName.c_str()));
        break;
      case 'i':
        calibFileName=optarg;
        if(!check->XmlFile(calibFileName)){ 
          cout<<"!!Error!! "<<calibFileName.c_str()<<"is wrong!!"<<endl;
          Log->eWrite(Form("[%s][Decoder]!!calibration file is wrong!!",calibFileName.c_str()));
          return 1;
        }
        cout << "== calibation file :" << calibFileName.c_str() << " ==" << endl;
        break;
      case 'o':
        outputDir = optarg; 
        if(!check->Dir(outputDir)){
          cout << "!!Error!! output directory:" << outputDir<< " don't exist!" << endl;
          Log->eWrite(Form("[%s][Decoder]!!output directory is wrong!!",outputDir.c_str()));
          return 1;
        }
        break;
      case 'r':
        overwrite = true;
        cout << "== mode : overwrite  ==" << endl;
        Log->Write(Form("[%s][Decoder] overwrite mode",inputFileName.c_str()));
        break;
      case 'h':
        cout <<"this program is for decodeing .raw file to .root file "<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose inputfile you wanna read(must)"<<endl;
        cout <<"  -i (char*) : choose calibration file."<<endl;
        cout <<"  -o (char*) : choose output directory. (default=WAGASCI_DECODEDIR)"<<endl;
        cout <<"  -r         : overwrite mode "<<endl;
        exit(0);
      default:
        cout <<"this program is for decodeing .raw file to .root file "<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose inputfile you wanna read(must)"<<endl;
        cout <<"  -i (char*) : choose calibration file."<<endl;
        cout <<"  -o (char*) : choose output directory. (default=WAGASCI_DECODEDIR)"<<endl;
        cout <<"  -r         : overwrite mode "<<endl;
        exit(0);
    }
  }

  if(inputFileName==""){
    cout << "!!ERROR!! please input filename." <<endl;
#ifdef DEBUG_DECODE
    cout << "if you don't know how to input, please see help."<<endl;
    cout << "help : ./Decoder -h" <<endl;
#endif
    Log->eWrite(Form("[%s][Decoder]!!No input file!!",inputFileName.c_str()));
    exit(1);
  }

  const unsigned int maxEvt =99999999;
  cout << "Maximum number of events treated = " << maxEvt << endl;

  ReadFile(inputFileName, calibFileName, overwrite, maxEvt, outputDir, logoutputDir);
  return 0;
}

//******************************************************************************

void ReadFile(string inputFileName, string calibFileName, bool overwrite, unsigned int maxEvt,
			  string outputDir, string logoutputDir, int n_chips, int n_difs, int n_channels){

  string outputTreeFileName = OptStr->GetName(inputFileName)+"_tree.root";

  vector<int> v_log(4,0);
  string logpath = OptStr->GetPath(inputFileName);
  string logfilename = OptStr->GetName(inputFileName);
  int pos = logfilename.rfind("_ecal_dif_");
  string logfile = logpath + logfilename.substr(0,pos) + ".log";
  if(check->LogFile(logfile)){
    wgEditXML *Edit = new wgEditXML();
    Edit->GetLog(logfile,v_log);
    delete Edit;
  }else{
    cout << " LOGFILE:" << logfile << " doesn't exist!" << endl;
  }

#ifndef DEBUG_DECODE
  cout << " *****  READING FILE     :" << inputFileName      << "  *****" << endl;
  cout << " *****  OUTPUT TREE FILE :" << outputTreeFileName << "  *****" << endl;
  cout << " *****  OUTPUT DIRECTORY :" << outputDir          << "  *****" << endl;
  cout << " *****  LOG DIRECTORY    :" << logoutputDir       << "  *****" << endl;
#endif

  TFile * outputTreeFile;

  if (!overwrite){
    outputTreeFile = new TFile(Form("%s/%s",outputDir.c_str(),
          outputTreeFileName.c_str()),
        "create");
    if ( !outputTreeFile->IsOpen() ) {
      Log->eWrite(Form("[%s][Decoder]Error!!:%s/%s already exists!",
            inputFileName.c_str(),
            outputDir.c_str(),
            outputTreeFileName.c_str()));
      cout << "!! ERROR !!\tFile already created!" << endl;
      return;
    }
  }
  else {
    outputTreeFile = new TFile(Form("%s/%s",outputDir.c_str(),
          outputTreeFileName.c_str()),
        "recreate");
  }

  Log->Write(Form("[%s][Decoder]%s/%s is being created",
        inputFileName.c_str(),
        outputDir.c_str(),
        outputTreeFileName.c_str()));

  Raw_t rd;
  rd.spill=-1;
  rd.spill_flag=n_chips;

  // If the number of DIFs is not provided as an argument, try to infer it from
  // the file name
  if ( n_difs == 0 ) {
	pos = inputFileName.find("dif_1_1_") + 8;
	if(inputFileName[pos] == '1') {
	  n_difs = 1;
	}
	else if(inputFileName[pos] == '2') {
	  n_difs = 2;
	}
	else {
	  n_difs = 0;
	}
  }

  // If the number of chips is not provided as an argument, use the global macro
  // defined in the Const.h header
  if ( n_chips == 0 ) n_chips = NCHIPS;

  // If the number of channels per chip is not provided as an argument, use the
  // global macro defined in the Const.h header
  if ( n_channels == 0 ) n_channels = NCHANNELS;

  // Get the geometrical information (position in space) for each channel
  wgChannelMap *Map = new wgChannelMap();
  for(int ichip = 0; ichip < (int) n_chips; ichip++) {
    Map->GetMap(n_difs - 1, ichip, &rd.view, rd.pln[ichip], rd.ch[ichip], rd.grid[ichip], rd.x[ichip], rd.y[ichip], rd.z[ichip]);
  }
  delete Map;

  // Read the value of pedestal, TDC ramp and gain from the calibration file
  wgGetCalibData *getcalib = new wgGetCalibData();
  getcalib->Get_Pedestal(n_difs,rd.pedestal,rd.ped_nohit); 
  getcalib->Get_TdcCoeff(n_difs,tdc_slope,tdc_intcpt); //TODO 
  getcalib->Get_Gain(calibFileName,n_difs,rd.gain);
  delete getcalib;

  
  unsigned int i,j; //,k;
  unsigned int iEvt=0;
  unsigned int ineff_data=0;

  //int acqNumber = 0;
  int nChips    = 0;
  int nChipData = 0;

  unsigned short int dataResult = 0;
  unsigned short int lastTwo[4] = {0,0,0,0};

  unsigned int SPILL_NUMBER = 0;
  unsigned int SPILL_MODE = 0;
  unsigned int SPILL_COUNT = 0;
  unsigned int LAST_SPILL_COUNT = 0;
  unsigned int LAST_SPILL_NUMBER = 0;
  bool FILL_FLAG = false;

  bool spill_insert_tag=false;
  bool endOfChipTag=false;
  bool endOfSpillTag=false;
  bool First_Event=false;

  vector<unsigned short int> packetData;

  ifstream inputFile;
  inputFile.open(inputFileName.c_str(), ios_base::in | ios_base::binary);

  outputTreeFile->cd();
 
  time_t current_time = time(0);
  tm *tm = localtime(&current_time);
  string this_year = Form("%04d-%02d-%02d-%02d-%02d",
      tm->tm_year+1900,1,1,0,0);
  string next_year = Form("%04d-%02d-%02d-%02d-%02d",
      tm->tm_year+1900+1,1,1,0,0);
  struct tm y;
  strptime(this_year.c_str(),"%Y-%m-%d-%H-%M", &y);
  int DATETIME_STR = (int)mktime(&y);
  strptime(next_year.c_str(),"%Y-%m-%d-%H-%M", &y);
  int DATETIME_END = (int)mktime(&y);
  int DATETIME_BIN = DATETIME_END-DATETIME_STR;

  TH1F * h_start_time = new TH1F("start_time","start_time",DATETIME_BIN,DATETIME_STR,DATETIME_END);
  h_start_time -> Fill(v_log[0],v_log[0]);
  h_start_time -> Fill(v_log[0]+1,v_log[0]-DATETIME_END);
  TH1F * h_stop_time = new TH1F("stop_time","stop_time",DATETIME_BIN,DATETIME_STR,DATETIME_END);
  h_stop_time -> Fill(v_log[1],v_log[1]);
  h_stop_time -> Fill(v_log[1]+1,v_log[1]-DATETIME_END);
  TH1F * h_nb_data_pkts = new TH1F("nb_data_pkts","nb_data_pkts",DATETIME_BIN,DATETIME_STR,DATETIME_END);
  h_nb_data_pkts -> Fill(v_log[2]);
  TH1F * h_nb_lost_pkts = new TH1F("nb_lost_pkts","nb_lost_pkts",DATETIME_BIN,DATETIME_STR,DATETIME_END);
  h_nb_lost_pkts -> Fill(v_log[3]);

  h_start_time   -> Write(); 
  h_stop_time    -> Write(); 
  h_nb_data_pkts -> Write(); 
  h_nb_lost_pkts -> Write(); 

  TTree * tree = new TTree("tree","tree");
  tree->Branch("spill"       ,&rd.spill       ,Form("spill/I"                                                ));
  tree->Branch("spill_mode"  ,&rd.spill_mode  ,Form("spill_mode/I"                                           ));
  tree->Branch("spill_flag"  ,&rd.spill_flag  ,Form("spill_flag/I"                                           ));
  tree->Branch("spill_count" ,&rd.spill_count ,Form("spill_count/I"                                          ));
  tree->Branch("bcid"        ,rd.bcid         ,Form("bcid[%d][%d]/I"          ,n_chips,             MEMDEPTH ));
  tree->Branch("charge"      ,rd.charge       ,Form("charge[%d][%d][%d]/I"    ,n_chips, n_channels, MEMDEPTH ));
  tree->Branch("time"        ,rd.time         ,Form("time[%d][%d][%d]/I"      ,n_chips, n_channels, MEMDEPTH ));
  tree->Branch("gs"          ,rd.gs           ,Form("gs[%d][%d][%d]/I"        ,n_chips, n_channels, MEMDEPTH ));
  tree->Branch("hit"         ,rd.hit          ,Form("hit[%d][%d][%d]/I"       ,n_chips, n_channels, MEMDEPTH ));
  tree->Branch("chipid_tag"  ,rd.chipid_tag   ,Form("chipid_tag[%d]/I"        ,n_chips                       ));
  tree->Branch("chipid"      ,rd.chipid       ,Form("chipid[%d]/I"            ,n_chips                       ));
  tree->Branch("col"         ,rd.col          ,Form("col[%d]/I"               ,                     MEMDEPTH ));
  tree->Branch("chipch"      ,rd.chipch       ,Form("chipch[%d]/I"            ,         n_channels           ));
  tree->Branch("chip"        ,rd.chip         ,Form("chip[%d]/I"              ,n_chips                       ));
  tree->Branch("debug"       ,rd.debug        ,Form("debug[%d]/I"             ,n_chips                       ));
  tree->Branch("view"        ,&rd.view        ,Form("view/I"                                                 ));
  tree->Branch("pln"         ,rd.pln          ,Form("pln[%d][%d]/I"           ,n_chips, n_channels           ));
  tree->Branch("ch"          ,rd.ch           ,Form("ch[%d][%d]/I"            ,n_chips, n_channels           ));
  tree->Branch("grid"        ,rd.grid         ,Form("grid[%d][%d]/I"          ,n_chips, n_channels           ));
  tree->Branch("x"           ,rd.x            ,Form("x[%d][%d]/D"             ,n_chips, n_channels           ));
  tree->Branch("y"           ,rd.y            ,Form("y[%d][%d]/D"             ,n_chips, n_channels           ));
  tree->Branch("z"           ,rd.z            ,Form("z[%d][%d]/D"             ,n_chips, n_channels           ));
  tree->Branch("pe"          ,rd.pe           ,Form("pe[%d][%d][%d]/D"        ,n_chips, n_channels, MEMDEPTH ));
  tree->Branch("time_ns"     ,rd.time_ns      ,Form("time_ns[%d][%d][%d]/D"   ,n_chips, n_channels, MEMDEPTH ));
  tree->Branch("gain"        ,rd.gain         ,Form("gain[%d][%d]/D"          ,n_chips, n_channels           ));
  tree->Branch("pedestal"    ,rd.pedestal     ,Form("pedestal[%d][%d][%d]/D"  ,n_chips, n_channels, MEMDEPTH ));
  tree->Branch("ped_nohit"   ,rd.ped_nohit    ,Form("ped_nohit[%d][%d][%d]/D" ,n_chips, n_channels, MEMDEPTH ));
  
  // =====================================================
  //     ============================================
  //              Start reading binary file
  //     ============================================
  // =====================================================

  if (inputFile.is_open()) {
    cout << "     *****  Start reading file  *****" << endl;
    while (inputFile.read((char*) &dataResult, sizeof dataResult)&& iEvt < maxEvt) {
      packetData.push_back(dataResult);
      nChipData++;

      // IMPORTANT :
      // SPILL insert  --> start with 0xFFFB
      // SPILL header  --> start with 0xFFFC
      // CHIP  header  --> start with 0xFFFD
      // CHIP  trailer --> start with 0xFFFE
      // SPILL trailer --> start with 0xFFFF

      // *********   CHIP TRAILER  ********* //
      if (lastTwo[1] == 0xFFFE && dataResult == 0x2020){
        endOfChipTag = true;
      }

      // *********   SPILL TRAILER  ********* //
      if (lastTwo[2] == 0x2020 && lastTwo[1] == 0xFFFF) {
        endOfSpillTag = true;
      }
     
      // *********   SPILL_INSERT  ********* //
      if (endOfSpillTag && lastTwo[2] == 0xFFFB) {
        SPILL_MODE = ((lastTwo[0]&0x8000)>>15); 
        if(SPILL_MODE==1) LAST_SPILL_NUMBER=SPILL_NUMBER;
        SPILL_NUMBER = lastTwo[1] & 0xFFFF;
        if( LAST_SPILL_NUMBER + 1 != SPILL_NUMBER ){
          Log->eWrite(Form("[%s][Decoder]!! WARNING !! SPILL GAP!! last_spill=%d,current_spill=%d",
                inputFileName.c_str(),
                LAST_SPILL_NUMBER,
                SPILL_NUMBER));
          cout << "!! WARNING !! SPILL GAP!! (last="<< LAST_SPILL_NUMBER << ", current="<<SPILL_NUMBER << ")" << endl;
          
          if(  LAST_SPILL_NUMBER - SPILL_NUMBER+ 1 == 1  
            || LAST_SPILL_NUMBER - SPILL_NUMBER+ 1 == 2 
            || LAST_SPILL_NUMBER - SPILL_NUMBER+ 1 == 4 
            || LAST_SPILL_NUMBER - SPILL_NUMBER+ 1 == 8 
            || LAST_SPILL_NUMBER - SPILL_NUMBER+ 1 == 16 
            || LAST_SPILL_NUMBER - SPILL_NUMBER+ 1 == 32
            || LAST_SPILL_NUMBER - SPILL_NUMBER+ 1 == 64
            || LAST_SPILL_NUMBER - SPILL_NUMBER+ 1 == 128
            || LAST_SPILL_NUMBER - SPILL_NUMBER+ 1 == 256 
            || LAST_SPILL_NUMBER - SPILL_NUMBER+ 1 == 512
            || LAST_SPILL_NUMBER - SPILL_NUMBER+ 1 == 1024
            || LAST_SPILL_NUMBER - SPILL_NUMBER+ 1 == 2048
            || LAST_SPILL_NUMBER - SPILL_NUMBER+ 1 == 4096
            || LAST_SPILL_NUMBER - SPILL_NUMBER+ 1 == 8192
            || LAST_SPILL_NUMBER - SPILL_NUMBER+ 1 == 16384
            || LAST_SPILL_NUMBER - SPILL_NUMBER+ 1 == 32768
            || LAST_SPILL_NUMBER - SPILL_NUMBER+ 1 == 65536
            ){
            for(unsigned int ichip=0;ichip<n_chips;ichip++){
              rd.debug[ichip] +=DEBUG_GOOD_SPILLGAP;
            }
          }else{
            for(unsigned int ichip=0;ichip<n_chips;ichip++){
              rd.debug[ichip] +=DEBUG_BAD_SPILLGAP;
            }
          }
        }

        if( dataResult == 0xFFFC ){
          spill_insert_tag = true; 
        }else{
          Log->eWrite(Form("[%s][Decoder]!! WARNING !! SPILL INSERT TAG is inserted in different position!! last_spill=%d,current_spill=%d",
                inputFileName.c_str(),
                LAST_SPILL_COUNT,
                SPILL_COUNT)); 
          cout << "!! WARNING !! SPILL INSERT TAG is inserted in different position!! (last="<< LAST_SPILL_COUNT << ", current="<<SPILL_COUNT << ")" << endl;     
        }
      }

      // *********   SPILL_HEADER  ********* //
      if( lastTwo[1]==0xFFFC && endOfSpillTag ){
        if(!First_Event) First_Event=true;
        if(endOfSpillTag){
          //Read SPILL information
          nChips            = 0;
          LAST_SPILL_COUNT  = SPILL_COUNT;
          if(dataResult==0x0000){
            SPILL_COUNT       = lastTwo[0]*65534+65535;
          }else{ 
            SPILL_COUNT       = lastTwo[0]*65535+dataResult;
          }

          rd.spill_count    = SPILL_COUNT;
          rd.spill_flag     = n_chips;
          FILL_FLAG         = true;
         
          if( spill_insert_tag ){
            rd.spill          = SPILL_NUMBER;
            rd.spill_mode     = SPILL_MODE;
            spill_insert_tag  = false;
          }else{
            rd.spill          = SPILL_NUMBER;
            rd.spill_mode     = SPILL_MODE;
          }

          if( LAST_SPILL_COUNT == 0 ||
              ( LAST_SPILL_COUNT != 0 && ( SPILL_COUNT==LAST_SPILL_COUNT+1 ))){
          }else{
            Log->eWrite(Form("[%s][Decoder]!! WARNING !! SPILL COUNT GAP!! last_spill=%d,current_spill=%d",
                  inputFileName.c_str(),
                  LAST_SPILL_COUNT,
                  SPILL_COUNT)); 
            cout << "!! WARNING !! SPILL COUNT GAP!! (last="<< LAST_SPILL_COUNT << ", current="<<SPILL_COUNT << ")" << endl; 
          }
          
          endOfSpillTag=false;
        }else{
          Log->eWrite(Form("[%s][Decoder]!! WARNING !! WHILE DECODING, NEXT SPILL IS INSERTED!! ,current_spill=%d",
                inputFileName.c_str(),
                SPILL_COUNT)); 
          cout << "!! WARNING !! WHILE DECODING, NEXT SPILL IS INSERTED, current=)"<<SPILL_COUNT << endl;
          cout << "!! WARNING !! DATA is aborted !! "<< endl;
        }
      }

      // *********   CHIP_HEADER  ********* //
      if (lastTwo[2] == 0x2020 &&lastTwo[1] == 0xFFFD){
        nChipData = 0;
      }


      // *********   CHIP TRAILER  ********* //
      if (lastTwo[1] == 0xFFFE && dataResult == 0x2020){
        nChips++;
        if (nChipData > 78) { // when 1 column or more is filled
          // if only 1 column 
          // => 36 charges + 36 time + 1 BCID + 1 chipID = 74
          // +2 +3 (header/trailer) = 79
          // let's replace this magic number !!!!
        }else{          
#ifdef DEBUG_DECODE
          cout << "!! WARNING !! nChipData = " << nChipData << " is too small!!" <<endl;
#endif
          Log->eWrite(Form("[%s][Decoder]!! WARNING !!nChipData is too small!!.(nChipData is %d) spill=%d, chipid_tag=%d",
                inputFileName.c_str(),
                nChipData,
                SPILL_COUNT,
                nChips+1
                )); 
          rd.spill_flag--;
        }
      }

      // *********   SPILL TRAILER  ********* //
      if (First_Event && lastTwo[3] == 0x2020 && lastTwo[2] == 0xFFFF){
          
        int nbchip = (dataResult&0x00FF);
        if( nChips != nbchip){
#ifdef DEBUG_DECODE
          cout << "!! WARNING !! The number of chips doesn't match the number of Chip Trailer! spill:"<< SPILL_COUNT<< endl;
#endif
          Log->eWrite(Form("[%s][Decoder]!! WARNING !! The number of chips doesn't match the number of Chip Trailer!) acqid=%d, nbchip=%d, #chip_trailer=%d",
                inputFileName.c_str(),
                SPILL_COUNT,
                nbchip,
                nChips
                )); 
        }
        if (nChips > 0){
          //=================  read Event ====================  //

          vector<unsigned short int>& eventData  = packetData;

          Int_t v_chipid   [n_chips];
          Int_t v_bcid     [n_chips][MEMDEPTH];
          Int_t v_time     [n_chips][n_channels][MEMDEPTH];
          Int_t v_charge   [n_chips][n_channels][MEMDEPTH];
          Int_t v_gain_hit [n_chips][n_channels][MEMDEPTH];

          for(unsigned int i0=0;i0<n_chips;i0++){
            v_chipid[i0]=-1;
            for(unsigned int k0=0;k0<MEMDEPTH;k0++){
              v_bcid[i0][k0]=-1;
              for(unsigned int j0=0;j0<MEMDEPTH;j0++){
                v_time    [i0][j0][k0]=-1;
                v_charge  [i0][j0][k0]=-1;
                v_gain_hit[i0][j0][k0]=-1;
              }
            }
          }

          unsigned short int last          = 0;
          unsigned short int lastChipID[3] = {0,0,0};
          unsigned short int currentChipID = 0;
          unsigned short int chip_count=0;
          bool chipid_exist=false;
          int Missing_Header=0;
          unsigned int chipStartIndex = 0;
          unsigned int rawDataSize    = 0;
          unsigned int nColumns       = 0;
          bool isValidChip = false;

          for (i=0; i < eventData.size(); i++){
            if (eventData[i] == 0xFFFD) { // Start CHIP tag
              chip_count++;
              //check CHIP start index ...
              currentChipID = check_StartChipIndex(eventData[i+1],eventData[i+2],eventData[i+3],eventData[i+4],&chipid_exist,&Missing_Header);

              if(chipid_exist){
                currentChipID--;
              }else{
                currentChipID=chip_count-1;
#ifdef DEBUG_DECODE
                cout << " !!WARNING!! 1 chipid_tag is mssed! "<<endl; 
#endif
                rd.debug[chip_count-1] +=DEBUG_MISSING_CHIPID_TAG;
              }

              lastChipID[2] = lastChipID[1];
              lastChipID[1] = lastChipID[0];
              lastChipID[0] = currentChipID;

              if(Missing_Header>1){
                cout << " !! ERROR !! CHIP HEADER INDEX IS TOTTALY BROKEN! (#Missing Header:" << Missing_Header <<", chipid_tag:"<< chip_count << ",acq id:"<< SPILL_COUNT << ")" << endl;
                Missing_Header=0;
              }else if(Missing_Header==1){
#ifdef DEBUG_DECODE
                cout << " !!WARNING!! 1 chip headers are missed! "<<endl; 
#endif
                rd.debug[chip_count-1] +=DEBUG_MISSING_CHIP_HEADER;
              }
              chipStartIndex = i + CHIPHEAD-Missing_Header;
            }

            if (last == 0xFFFE){ // Find end chip tag
              if ( (eventData[i]&0x00FF) == currentChipID+1){
                isValidChip = true;
              }else{                    
#ifdef DEBUG_DECODE
                cout << "!! WARNING !! CHIPID is not found....."<<endl;
                cout << "    Last ChipID is "<< lastChipID[1] << "current is" << currentChipID << endl;
#endif
                bool matchChipID=false;
                if( lastChipID[0]+1 < nChips && lastChipID[0] > 0 ){
                  if(currentChipID==lastChipID[1]+1){
                    for(unsigned int i_int=i; i_int< i+4;i_int++){
                      if(eventData[i_int] == 0xFFFD){
                        if( currentChipID+1 == (eventData[i_int+1]&0x00FF)-1){
                          matchChipID=true;
                          rd.debug[currentChipID]+=DEBUG_MISSING_CHIP_TRAILER;
                        }
                        break;
                      }
                    }
                  }

                }else if( lastChipID[0] == 0 ){
                  if(nChips==1){
                    rd.debug[currentChipID] += DEBUG_MISSING_CHIP_TRAILER_ONLY_ONE_CHIP;
                  }else{
                    for(unsigned int i_int=i; i_int< i+4;i_int++){
                      if(eventData[i_int] == 0xFFFD){
                        if( currentChipID+1 == (eventData[i_int+1]&0x00FF)-1){
                          matchChipID=true;
                          rd.debug[currentChipID]+=DEBUG_MISSING_CHIP_TRAILER;
                        }
                        break;
                      }
                    }
                  }
                }else if( lastChipID[0] == nChips-1 ){
                  if(currentChipID==lastChipID[1]+1){
                    rd.debug[currentChipID]+=DEBUG_MISSING_CHIP_TRAILER;
                    matchChipID=true;
                  }
                } 

                if(matchChipID){
#ifdef DEBUG_DECODE
                  cout << "          ... the order of CHIPID is OK!" <<endl;  
#endif
                  isValidChip = true;
                  rd.debug[currentChipID]+=DEBUG_MISSING_CHIP_TRAILER;

                }else{
                  isValidChip = false;
                  rd.debug[currentChipID]+=DEBUG_BAD_CHIPNUM;
                  cout << "!! ERROR !! CHIP is not valid!\n" << "eventData\tis\t"
                    << (eventData[i]&0x00FF)        << "\ncurrentChipID\tis\t"
                    << currentChipID              << endl;
                  Log->eWrite(Form("[%s][Decoder]WARNING!!:HEAD ChipID and END ChipID is wrong!(HEAD:%d,END:%d) spill:%d",
                        inputFileName.c_str(),
                        currentChipID,
                        eventData[i],
                        rd.spill));
                  rd.spill_flag--;
                }
              }

              if (isValidChip){
                rawDataSize = i - chipStartIndex - CHIPENDTAG;
                nColumns    = (rawDataSize-CHIPIDSIZE)/(1+n_channels*2);
                if ( (rawDataSize-CHIPIDSIZE)%(1+n_channels*2) != 0) {
                  cout << "!! WARNING !! Bad data size! (size : "<< rawDataSize-CHIPIDSIZE << " , spill_count:"<< SPILL_COUNT <<")"  << endl;
                  Log->eWrite(Form("[%s][Decoder]WARNING!!:BAD DATA SIZE! spill:%d ,chip:%d",
                        inputFileName.c_str(),
                        rd.spill,
                        currentChipID));
                  rd.spill_flag--;
                  rd.debug[currentChipID]+=DEBUG_BAD_CHIPDATA_SIZE;
                  last=(eventData[i])&0xFFFF;
                  isValidChip=false;
                  ineff_data++;
                  continue;
                }else{
                  if(nColumns>MEMDEPTH){
                    cout << "!! WARNING !! Bad number of columns!" << endl;
                    rd.debug[currentChipID]+=DEBUG_BAD_CHIPDATA_SIZE;
                    Log->eWrite(Form("[%s][Decoder]WARNING!!:BAD COLUMN SIZE! DataSize:%d column:%d",
                          inputFileName.c_str(),
                          rd.spill,
                          nColumns));
                    rd.spill_flag--;
                    last=(eventData[i])&0xFFFF;
                    isValidChip=false;
                    continue;
                  }
                }

                //extract chipid
                v_chipid[currentChipID] = eventData[i-CHIPENDTAG] & 0x00FF;

                bool isGoodChipNumber=false;
                if (check_ChipID(v_chipid[currentChipID])) isGoodChipNumber = true;
                if (!isGoodChipNumber) {
                  cout << "!! WARNING !! Bad chip ID: " 
                    << v_chipid[currentChipID] << endl;
                  Log->eWrite(Form("[%s][Decoder]WARNING!!:BAD CHIP ID! spill:%d chipid:%d",
                        inputFileName.c_str(),
                        rd.spill,
                        v_chipid[currentChipID]));
                  rd.spill_flag--;
                  rd.debug[currentChipID]+=DEBUG_BAD_CHIPNUM;
                }

                rd.chipid[currentChipID] = v_chipid[currentChipID];
                rd.chipid_tag[currentChipID] = currentChipID;

                int loopBCID = 0;
                for (unsigned int ibc=0; ibc < nColumns; ibc++) {
                  //extract bcid
                  
                  v_bcid [currentChipID][ibc] = eventData[i-CHIPENDTAG-CHIPIDSIZE-ibc] & 0x0FFF;
                  loopBCID                    = (eventData[i-CHIPENDTAG-CHIPIDSIZE-ibc] & 0xF000) >>12;
                   
                  int bcid_slope=1;
                  int bcid_inter=0;
                  if(loopBCID==1)              { bcid_slope= -1;  bcid_inter=2*4096;}
                  else if(loopBCID==3)         { bcid_slope=  1;  bcid_inter=2*4096;}
                  else if(loopBCID==2)         { bcid_slope= -1;  bcid_inter=4*4096;}
                  rd.bcid[currentChipID][ibc] = bcid_inter + v_bcid[currentChipID][ibc] *bcid_slope;
                  
                  // Range for this column
                  unsigned int begin = i - CHIPENDTAG - CHIPIDSIZE - nColumns - ibc*n_channels*2;
                  unsigned int end   = begin - n_channels;
                  unsigned int ichan = 0;
                  //int count_hits     = 0;

                  for (j = begin ; j>end ; j--) {
                    if (ibc<MEMDEPTH && ichan<n_channels) {
                      //extract charge
                      v_charge  [currentChipID][ichan][ibc] = eventData[j] & 0x0FFF;
                      rd.charge [currentChipID][ichan][ibc] = v_charge[currentChipID][ichan][ibc];
                      //extract hit 
                      v_gain_hit[currentChipID][ichan][ibc] = (eventData[j] >> 12) & 0x0003;
                      rd.hit    [currentChipID][ichan][ibc] = v_gain_hit[currentChipID][ichan][ibc]%2 & 0x0001;
                      //extract gain
                      rd.gs    [currentChipID][ichan][ibc] = v_gain_hit[currentChipID][ichan][ibc]/2 & 0x0001;
                      //extract pe
                      if(rd.chipid[currentChipID]>=0 && rd.chipid[currentChipID]<20){
                        if(rd.gs[currentChipID][ichan][ibc] == 1){
                          rd.pe[currentChipID][ichan][ibc] = (v_charge[currentChipID][ichan][ibc]-rd.pedestal[rd.chipid[currentChipID]][ichan][ibc])/rd.gain[rd.chipid[currentChipID]][ichan]*1.08;
                        }else{
                          rd.pe[currentChipID][ichan][ibc] = (v_charge[currentChipID][ichan][ibc]-rd.pedestal[rd.chipid[currentChipID]][ichan][ibc])/rd.gain[rd.chipid[currentChipID]][ichan]*10.8;
                        }
                      }else{
                        rd.pe[currentChipID][ichan][ibc] = -100.;
                      }
                    }
                    ichan++;  
                  } // loop on j

                  begin = end;
                  end   = begin - n_channels;
                  ichan = 0;

                  for (j = begin ; j>end ; j--) {
                    if (ibc<MEMDEPTH && ichan<n_channels) {
                      //extract time
                      v_time [currentChipID][ichan][ibc] = eventData[j] & 0x0FFF;
                      rd.time[currentChipID][ichan][ibc] = v_time[currentChipID][ichan][ibc];
                    }		    
                    ichan++;
                  } // loop on j
                } // for iColumn
                isValidChip = false;
              } // isValidChip
            } // if last = 0xFFFE
            last = eventData[i];
          } // loop on the elements of eventData

          tdc2time(rd.time,rd.bcid,rd.time_ns,tdc_slope,tdc_intcpt);
          // ***** Fill Tree ***** //

          tree->Fill();
          FILL_FLAG = false;
          rd.spill      = -1;
          rd.spill_mode      = -1;
          rd.spill_count      = -1;
          rd.spill_flag =  1;
          for(unsigned int ichip=0 ; ichip<n_chips ; ichip++){
            v_chipid[ichip]  = -1;
            rd.chipid[ichip] = -1;
            rd.debug[ichip]    =  0;
            for(unsigned int icol = 0 ; icol<MEMDEPTH ;icol++){
              v_bcid [ichip][icol] = -1 ;  
              rd.bcid[ichip][icol] = -1;
              for(unsigned int ich=0 ; ich<n_channels; ich++){
                rd.charge  [ichip][ich][icol] = -1;
                rd.time    [ichip][ich][icol] = -1;
                rd.time_ns [ichip][ich][icol] = -100.;
                rd.pe      [ichip][ich][icol] = -100.;
                rd.gs      [ichip][ich][icol] = -1;
                rd.hit     [ichip][ich][icol] = -1;
                v_time     [ichip][ich][icol] =  0;
                v_charge   [ichip][ich][icol] =  0;
                v_gain_hit [ichip][ich][icol] =  0;
              }	
            } 	
          }
          //============= END =================//
          if(iEvt%1000==0) cout << "Entry number ... " << iEvt << endl;
          iEvt++;
        }
      } // END of SPILL trailer

      if (lastTwo[3] == 0xFFFF) { // SPILL trailer check
        packetData.clear();
        endOfChipTag = false;
        for(unsigned int ichip=0 ; ichip<n_chips ; ichip++){
          rd.chipid[ichip] = -1;
          for(unsigned int icol = 0 ; icol<MEMDEPTH ;icol++){ 
            rd.bcid[ichip][icol] = -1;
            for(unsigned int ich=0 ; ich<n_channels; ich++){
              rd.charge[ichip][ich][icol] = -1;
              rd.pe[ichip][ich][icol]     = -100.;
              rd.time  [ichip][ich][icol] = -1;
              rd.gs    [ichip][ich][icol] = -1;
              rd.hit   [ichip][ich][icol] = -1;
            }
          }	
        }
        if(FILL_FLAG){
          for(unsigned int ichip=0 ; ichip<n_chips ; ichip++){
            rd.debug[ichip] += DEBUG_NODATA;
          }
          tree->Fill(); 
          for(unsigned int ichip=0 ; ichip<n_chips ; ichip++){
            rd.debug[ichip] = 0;
          }
        }
        FILL_FLAG=false;
      }

      if (lastTwo[1] == 0x5053 && lastTwo[0] == 0x4C49 &&
          dataResult == 0x2020) { // SPILL header
        // New SPILL: extract SPILL number
        // dataResult is just before CHIP header
        // i.e. at the end of SPILL header
        if (endOfChipTag) {
          cout << "!! WARNING !! New SPILL w/o end flag of the previous SPILL!"
            << " - some CHIPS found!" << endl;
          Log->eWrite(Form("[%s][Decoder]Error!!:SPILL trailer is missed!! spill:%d",
                inputFileName.c_str(),
                rd.spill));
        }
        packetData.clear();
        endOfChipTag = false;
      }

      lastTwo[3] = lastTwo[2];
      lastTwo[2] = lastTwo[1];
      lastTwo[1] = lastTwo[0];
      lastTwo[0] = dataResult;

    } // while (inputFile.read(...
  } // if (fin.is_open())

  cout << endl;
  cout << "     *****  Finished reading file  *****" << endl;
  cout << "     *****  with " << iEvt << " entries  *****" << endl;
  cout << "     *****       BAD data : "<< ineff_data  << " *****" << endl;
  cout << endl;

  outputTreeFile->cd();
  tree->Write();
  outputTreeFile->Close();
  Log->Write(Form("[%s][Decoder]Finish Decode",inputFileName.c_str()));
  delete Log;
}


//******************************************************************************
unsigned short int check_StartChipIndex(unsigned short int head1,unsigned short int head2,unsigned short  int head3,unsigned short int head4,bool* checkid_exist, int *Missing_Header){
  int miss=0;
  unsigned short int head[4]={head1,head2,head3,head4};
  unsigned short int ret;

  if( (head[0]&0x00FF) > 0 && (head[0]&0x00FF) <=n_chips ){
    ret = head[0];
    *checkid_exist=true;
  }else{
    ret=0xFFFF;
    head[3]=head[2];
    head[2]=head[1];
    head[1]=head[0];
    miss++;
    *checkid_exist=false;
#ifdef DEBUG_DECODE
    cout << "DEBUG : " << (head[1]&0xFFFF) << " / " << (head[2]&0xFFFF) << " / " << (head[3]&0xFFFF) << endl; 
#endif
  }

  if( head[1]!=0x4843 ){
    miss++;
    if( head[1] !=0x5049){
      miss++;
      if( head[1] !=0x2020){
        miss++;
      }
    }else{
      if(head[2] != 0x2020) miss++;
    }
  }else{
    if( head[2]!= 0x5049){
      miss++;
      if( head[2] !=0x2020){
        miss++;
      }
    }else{
      if( head[3]!= 0x2020) miss++;
    }
  }
  *Missing_Header=miss;
  return (ret&0x00FF);
}

//******************************************************************************
bool check_ChipID(int v_chipid){
  if (v_chipid == 0x0000 ||
      v_chipid == 0x0001 || 
      v_chipid == 0x0002 ||
      v_chipid == 0x0003 ||
      v_chipid == 0x0004 ||
      v_chipid == 0x0005 ||
      v_chipid == 0x0006 ||
      v_chipid == 0x0007 ||
      v_chipid == 0x0008 ||
      v_chipid == 0x0009 ||
      v_chipid == 0x000a ||
      v_chipid == 0x000b ||
      v_chipid == 0x000c ||
      v_chipid == 0x000d ||
      v_chipid == 0x000e ||
      v_chipid == 0x000f ||
      v_chipid == 0x0010 ||
      v_chipid == 0x0011 ||
      v_chipid == 0x0012 ||
      v_chipid == 0x0013 ||
      v_chipid == 0x0014 ||
      v_chipid == 0x0015 ||
      v_chipid == 0x0016 ||
      v_chipid == 0x0017 ||
      v_chipid == 0x0018 ||
      v_chipid == 0x0019 ||
      v_chipid == 0x001a ||
      v_chipid == 0x001b ||
      v_chipid == 0x001c ||
      v_chipid == 0x001d ||
      v_chipid == 0x001e ||
      v_chipid == 0x001f ||
      v_chipid == 0x0020 ||
      v_chipid == 0x0021 ||
      v_chipid == 0x0022 ||
      v_chipid == 0x0023 ||
      v_chipid == 0x0024 ||
      v_chipid == 0x0025 ||
      v_chipid == 0x0026 ||
      v_chipid == 0x0027 ||
      v_chipid == 0x0028 
      ) return true;
  else return false;

}

//******************************************************************************
void Get_calibData(string inputFileName,string calibFileName,string pedFileName, double gain[20][36], double pedestal[20][36][16], double ped_nohit[20][36][16]){

  int n_difs = 0;
  int pos = inputFileName.find("dif_1_1_")+8;
  if(inputFileName[pos]=='1'){
    n_difs=1;
  }else if(inputFileName[pos]=='2'){
    n_difs=2;
  }
  if(n_difs==0) return; 
  string name("");
  wgEditXML *Edit = new wgEditXML();

  Edit->Open(pedFileName);
  for(unsigned int i=0;i<n_chips;i++){
    int ichip=i;
    for(unsigned int j=0;j<n_channels;j++){
      int ich=j;
      if(ich>31){
        for(unsigned int k=0;k<MEMDEPTH;k++){
          int icol=k;
          ped_nohit[ichip][ich][icol]=1.;
          pedestal[ichip][ich][icol]=1.;
        }
      }else{
        for(unsigned int k=0;k<MEMDEPTH;k++){
          int icol=k;
          name=Form("ped_nohit_%d",icol);
          ped_nohit[ichip][ich][icol]=Edit->Calib_GetValue(name,n_difs,ichip,ich);
          name=Form("ped_%d",icol);
          pedestal[ichip][ich][icol]=Edit->Calib_GetValue(name,n_difs,ichip,ich);
        }
      }
    }
  }
  Edit->Close();

  if(calibFileName==""){
    for(unsigned int i=0;i<n_chips;i++){
      int ichip=i;
      for(unsigned int j=0;j<n_channels;j++){
        int ich=j;
        gain[ichip][ich]=1.;
      }
    }
    return;
  }


  Edit->Open(calibFileName);
  for(unsigned int i=0;i<n_chips;i++){
    int ichip=i;
    for(unsigned int j=0;j<n_channels;j++){
      int ich=j;
      if(ich>31){
        gain[ichip][ich]=1.;
        for(unsigned int k=0;k<MEMDEPTH;k++){
          int icol=k;
          ped_nohit[ichip][ich][icol]=1.;
          pedestal[ichip][ich][icol]=1.;
        }
      }else{
        name=Form("Gain");
        gain[ichip][ich]=Edit->Calib_GetValue(name,n_difs,ichip,ich);
      }
    }
  }
  Edit->Close();
  
}

//******************************************************************************
void tdc2time(int time[20][36][16],int bcid[20][16],double time_ns[20][36][16],double slope[2][20][36],double intcpt[2][20][36])
{
    const int Even=0;
    const int Odd=1;
    int Parity;
    int BCIDwidth=580;//ns

    for(int chip=0; chip<(int)n_chips;chip++){
      for(int col=0; col<(int)MEMDEPTH; col++){
        if(bcid[chip][col]%2 == 0){ Parity = Even; }
        else           { Parity = Odd;  }
        for(int ch=0;ch<(int)n_channels;ch++){
          time_ns[chip][ch][col] 
            = (time[chip][ch][col]-intcpt[Parity][chip][ch])
            /slope[Parity][chip][ch]+(bcid[chip][col]-Parity)*BCIDwidth;
        }
      }
    }
};
