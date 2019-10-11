#ifndef CONST_H_INCLUDE
#define CONST_H_INCLUDE

// system includes
#include <string>
#include <vector>
#include <memory>
#include <climits>

// ROOT includes
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TNamed.h>

// user includes
#include "wgContiguousVectors.hpp"
#include "wgErrorCodes.hpp"
#include "wgEnvironment.hpp"

// Only the ROOT Minuit2 minimizer is thread-safe. All the others are
// not. So if ROOT has not support for the Minuit2 minimizer than we
// have to restrict access to the fitting sections only to one thread
// at a time
#ifdef ROOT_HAS_NOT_MINUIT2
extern std::mutex MUTEX;
#endif

// ================================================================ //
//                                                                  //
//                         SPIROC2D MACROS                          //
//                                                                  //
// ================================================================ //

const unsigned NDIFS      = 8;
const unsigned NCHIPS     = 20;
const unsigned NCHANNELS  = 36;
const unsigned MEMDEPTH   = 16;

const unsigned CHIP_HEADER_LENGTH    = 5;
const unsigned CHIP_TRAILER_LENGTH   = 4;
const unsigned SPILL_HEADER_LENGTH   = 6;
const unsigned SPILL_TRAILER_LENGTH  = 7;
const unsigned PHANTOM_MENACE_LENGTH = 3;
const unsigned SPILL_NUMBER_LENGTH   = 3;

// One column length in 16 bits lines (but the BCID)
// 1 BCID              +
// n_channels times    +
// n_channels charges
const unsigned ONE_COLUMN_LENGTH = 1 + 2 * NCHANNELS;

const unsigned MAX_EVENT = UINT_MAX;
const unsigned N_DEBUG_SPILL = 7;
const unsigned N_DEBUG_CHIP = 6;

enum SPILL_TYPE {
  NON_BEAM_SPILL = 0,  // non beam spill bit (spill_flag)
  BEAM_SPILL     = 1   // beam spill bit (spill_flag)
};

const unsigned BCIDwidth    = 580; //ns
const unsigned NumReconAxis = 2;

const unsigned HIT_BIT        = 1;    // there was a hit (over threshold)
const unsigned NO_HIT_BIT     = 0;    // there was no hit (under threshold)
const unsigned HIGH_GAIN_BIT  = 1;    // high gain bit (gs)
const unsigned LOW_GAIN_BIT   = 0;    // low gain bit (gs)
const double HIGH_GAIN_NORM   = 1.08; // Normalization for the high gain
const double LOW_GAIN_NORM    = 10.8; // Normalization for the low gain

const unsigned MAX_VALUE_16BITS = 65535;
const unsigned MAX_VALUE_12BITS = 4095;
const unsigned MAX_VALUE_10BITS = 1023;
const unsigned MAX_VALUE_8BITS  = 255;
const unsigned MAX_VALUE_6BITS  = 63;
const unsigned MAX_VALUE_4BITS  = 15;

// The SPIROC2D bitstream is 1192 bits long. It contains all the parameters that
// can be set on the SPIROC2D chip. These parameters are listed in the
// spiroc2d.csv file.
//  - The _INDEX macros are just for sorting the parameters
//  - The _START macros are the start bit of the parameter(s) they refer to
//  - the _LENGTH macros are the length in bits of the parameter
//  - the _OFFSET macros are the length in bits of a single channel parameter
//    (when the parameter can be set for each channel individually)

const unsigned BITSTREAM_HEX_STRING_LENGTH = 300;  // length of the bitstream hex string
const unsigned BITSTREAM_BIN_STRING_LENGTH = 1192; // length of the bitstream bin string
const unsigned VALUE_OFFSET_IN_BITS = 6;           // offset in bits before a parameter start

// global 10-bit threshold
const unsigned GLOBAL_THRESHOLD_INDEX      = 0;
const unsigned GLOBAL_THRESHOLD_START      = 931;
const unsigned GLOBAL_THRESHOLD_LENGTH     = 10; // 4 bits (most significant bits) +
                                                 // 4 bits (least significant bits) +
                                                 // 2 bits (??)

// global 10-bit gain selection threshold
const unsigned GLOBAL_GS_INDEX             = 1;
const unsigned GLOBAL_GS_THRESHOLD_START   = 941;
const unsigned GLOBAL_GS_THRESHOLD_LENGTH  = 10; // 4 bits (most significant bits) +
                                                 // 4 bits (least significant bits) +
                                                 // 2 bits (??)

// Input 8-bit DAC Data from channel 0 to 35 
const unsigned ADJ_INPUTDAC_INDEX          = 2;
const unsigned ADJ_INPUTDAC_START          = 37;
const unsigned ADJ_INPUTDAC_LENGTH         = 8;
const unsigned ADJ_INPUTDAC_OFFSET         = 9; // 8 bits (DAC7...DAC0) + 1 bit (DAC ON)

// adjustable 6-bit high gain (HG) preamp
const unsigned ADJ_AMPDAC_INDEX            = 3;
const unsigned ADJ_AMPDAC_START            = 367;  
const unsigned ADJ_AMPDAC_LENGTH           = 6;
const unsigned ADJ_AMPDAC_OFFSET           = 15; // 6 bits (HG) + 6 bits (LG) + 3 bits '000'

// adjustable 4-bit threshold
const unsigned ADJ_THRESHOLD_INDEX         = 4;
const unsigned ADJ_THRESHOLD_START         = 1006;
const unsigned ADJ_THRESHOLD_LENGTH        = 4;
const unsigned ADJ_THRESHOLD_OFFSET        = 4; // 4 bits

// CHIP ID 8-bit
const unsigned CHIPID_INDEX         = 5;
const unsigned CHIPID_START         = 18;
const unsigned CHIPID_LENGTH        = 8;

// 1-bit input DAC Voltage Reference (1 = internal 4.5V   0 = internal 2.5V)
const unsigned GLOBAL_INPUT_DAC_REF_INDEX  = 6;
const unsigned GLOBAL_INPUT_DAC_REF_START  = 36;



// ============ wgEditXML MACROS ============== //

const bool NO_CREATE_NEW_MODE = false;
const bool CREATE_NEW_MODE    = true;

// ============ wgPedestalCalib and wgGainCalib MACROS ============== //

#define N_PE_PEDESTAL_CALIB 2
#define N_PE_GAIN_CALIB     2
#define N_IDAC 5
const unsigned ONE_PE = 0; // threshold at 0.5 p.e.
const unsigned TWO_PE = 1; // threshold at 1.5 p.e.

// ===================================================================== //
//                                                                       //
//                                Typedef                                //
//                                                                       //
// ===================================================================== //

typedef std::vector<std::vector<std::vector<std::vector<std::vector<double>>>>> d5vector;
typedef std::vector<std::vector<std::vector<std::vector<double>>>> d4vector;
typedef std::vector<std::vector<std::vector<double>>> d3vector;
typedef std::vector<std::vector<double>> d2vector;
typedef std::vector<double> d1vector;

typedef Contiguous3Vector<double> d3CCvector;
typedef Contiguous2Vector<double> d2CCvector;

typedef std::vector<std::vector<std::vector<std::vector<int>>>> i4vector;
typedef std::vector<std::vector<std::vector<int>>> i3vector;
typedef std::vector<std::vector<int>> i2vector;
typedef std::vector<int> i1vector;

typedef Contiguous3Vector<int> i3CCvector;
typedef Contiguous2Vector<int> i2CCvector;

typedef std::vector<std::vector<std::vector<std::vector<unsigned>>>>u4vector;
typedef std::vector<std::vector<std::vector<unsigned>>> u3vector;
typedef std::vector<std::vector<unsigned>> u2vector;
typedef std::vector<unsigned> u1vector;

typedef Contiguous3Vector<unsigned> u3CCvector;
typedef Contiguous2Vector<unsigned> u2CCvector;



// ===================================================================== //
//                                                                       //
//                                Hit Class                              //
//                                                                       //
// ===================================================================== //

#define MAX_NUM_BCID_CLUSTER 10
#define MAX_NUM_HIT          1000
#define MAX_NUM_TRACK        50
#define MAX_NUM_TRACKHIT     250

#define MAX_NUMCH_WG  1280 //8 tracking planes
#define MAX_NUMCH_ING 616 //11 tracking planes, 4 veto planes
#define MAX_NUMCH_PM  1204 //18 tracking planes, 4 veto planes

#define MAX_NUM_INGHIT   1000 //within a spill
#define MAX_NUM_INGRECON 100 // within a spill
#define NUM_CYC          8

class Hit_t
{
public:
  int    num_hits;
  int    num_bcid_cluster;
  int    num_bcid_hits   [MAX_NUM_BCID_CLUSTER];
  int    clustered_bcid  [MAX_NUM_BCID_CLUSTER];
  int    clustered_hitid [MAX_NUM_BCID_CLUSTER][MAX_NUM_HIT];
  int    hit_bcid        [MAX_NUM_HIT];
  int    hit_view        [MAX_NUM_HIT];
  int    hit_pln         [MAX_NUM_HIT];
  int    hit_ch          [MAX_NUM_HIT];
  int    hit_dif         [MAX_NUM_HIT];
  int    hit_chip        [MAX_NUM_HIT];
  int    hit_chipch      [MAX_NUM_HIT];
  int    hit_sca         [MAX_NUM_HIT];
  int    hit_adc         [MAX_NUM_HIT];
  int    hit_gs          [MAX_NUM_HIT];
  int    hit_tdc         [MAX_NUM_HIT];
  double hit_pe          [MAX_NUM_HIT];
  double hit_time        [MAX_NUM_HIT];
  double hit_pe_permm    [MAX_NUM_HIT];
  double hit_cos         [MAX_NUM_HIT];
  double hit_pathlength  [MAX_NUM_HIT];
  bool   hit_ontrack     [MAX_NUM_HIT];
  bool   hit_grid        [MAX_NUM_HIT];
  int    hit_numtrack    [MAX_NUM_HIT];
  double hit_cluster_pe  [MAX_NUM_HIT];

  void Clear();
};


class Recon_t
{
public:

  void Clear();
  void Clear_ReconVector();

  std::vector<std::vector<uint16_t> > bcid_cluster_hitid;
  std::vector<std::vector<uint16_t> > time_cluster_hitid;

  // ==== Filled data for tree ==== //
  int    num_recon;
  int    num_recon_hits  [MAX_NUM_TRACK];
  int    recon_hits_hitid[MAX_NUM_TRACK][MAX_NUM_TRACKHIT];
  double recon_start_z   [MAX_NUM_TRACK];
  double recon_stop_z    [MAX_NUM_TRACK];
  int    recon_start_pln [MAX_NUM_TRACK];
  int    recon_stop_pln  [MAX_NUM_TRACK];
  int    recon_start_ch  [MAX_NUM_TRACK];
  int    recon_stop_ch   [MAX_NUM_TRACK];
  double recon_start_xy  [MAX_NUM_TRACK];
  double recon_stop_xy   [MAX_NUM_TRACK];
  double recon_slope     [MAX_NUM_TRACK];
  double recon_intercept [MAX_NUM_TRACK];
  double recon_pathlength[MAX_NUM_TRACK];
  double recon_total_pe  [MAX_NUM_TRACK];
  double recon_mean_dedx [MAX_NUM_TRACK];
  double recon_mean_time [MAX_NUM_TRACK];
  int    recon_view      [MAX_NUM_TRACK];
  int    recon_bcid      [MAX_NUM_TRACK];
  int    recon_bcid_id   [MAX_NUM_TRACK];
  int    recon_veto      [MAX_NUM_TRACK];
  int    recon_sideescape[MAX_NUM_TRACK];
  double recon_cos       [MAX_NUM_TRACK];
  double recon_len       [MAX_NUM_TRACK];
  double recon_chi2      [MAX_NUM_TRACK];
    
  // ==== Vectors for reconstruction ==== ///
  std::vector<std::vector<std::vector<uint16_t> > > neighborhits_hitid;
  std::vector<std::vector<std::vector<uint16_t> > > cell_clusterid;
  std::vector<std::vector<std::vector<uint16_t> > > neighborcell_down_cellid;
  std::vector<std::vector<std::vector<uint16_t> > > neighborcell_up_cellid;
  std::vector<std::vector<uint16_t> > cell_state;
};

class Track_t
{
public:

  void Clear();
  void Clear_TrackVector();

  // ==== Filled data for tree ==== //
  int    num_track;
  int    num_trackid;
  int    num_track_hits  [MAX_NUM_TRACK];
  int    num_track_recon [MAX_NUM_TRACK];
  int    track_hits_hitid[MAX_NUM_TRACK][MAX_NUM_TRACKHIT];
  int    track_recon_id  [MAX_NUM_TRACK][MAX_NUM_TRACK];
  double track_start_z   [MAX_NUM_TRACK];
  double track_stop_z    [MAX_NUM_TRACK];
  double track_start_xy  [MAX_NUM_TRACK];
  double track_stop_xy   [MAX_NUM_TRACK];
  int    track_start_pln [MAX_NUM_TRACK];
  int    track_stop_pln  [MAX_NUM_TRACK];
  int    track_start_ch  [MAX_NUM_TRACK];
  int    track_stop_ch   [MAX_NUM_TRACK];
  double track_slope     [MAX_NUM_TRACK];
  double track_intercept [MAX_NUM_TRACK];
  double track_pathlength[MAX_NUM_TRACK];
  double track_total_pe  [MAX_NUM_TRACK];
  double track_mean_dedx [MAX_NUM_TRACK];
  double track_mean_time [MAX_NUM_TRACK];
  int    track_view      [MAX_NUM_TRACK];
  int    track_bcid      [MAX_NUM_TRACK];
  int    track_bcid_id   [MAX_NUM_TRACK];
  int    track_veto      [MAX_NUM_TRACK];
  int    track_sideescape[MAX_NUM_TRACK];
  double track_cos_zen   [MAX_NUM_TRACK];
  double track_cos_azi   [MAX_NUM_TRACK];
  double track_len       [MAX_NUM_TRACK];
  int    track_id        [MAX_NUM_TRACK];
    
  // ==== Vectors for reconstruction ==== ///
  std::vector<short int> num_pass_reconpln;
  std::vector<short int> num_pass_pair_reconpln;
  std::vector<std::vector<short int>> pair_track;
  std::vector<std::vector<std::vector<short int>>> connected_track;
  std::vector<std::vector<int>> recon_pair_start_reconpln;
  std::vector<std::vector<int>> recon_pair_stop_reconpln;
};


class BSD_t
{

public:

  TFile  *infile;
  TFile  *outfile;
  TNamed *version_name;
  TTree  *bsd;
  TTree  *bsd_out;

  void Clear();
  bool OpenBsdFile(std::string);
  void GetEntry(int);
  void MakeTree(std::string);

  std::string version;
  int    is_bsd;

  int    nurun          ;
  int    midas_event    ;
  int    mrrun          ;
  int    mrshot         ;
  int    spillnum       ;
  int    trg_sec[3]     ;
  int    trg_nano[3]    ;
  int    gpsstat[2]     ;
  double ct_np[5][9]    ;
  double beam_time[5][9];
  int    beam_flag[5][9];
  double hct[3][5]      ;
  double tpos[2]        ;
  double tdir[2]        ;
  double tsize[2]       ;
  double mumon[12]      ;
  double otr[13]        ;
  int    good_gps_flag  ;
  int    trigger_flag   ;
  int    spill_flag     ;
  int    good_spill_flag;
  double target_eff[3]  ;
  int    run_type       ;
  int    magset_id      ;
  double hctx[2][5]     ;
  double htrans[2]      ;
  double hps[2]         ;
};


class IngRecon_t
{
public:
  int     unixtime;
  int     nd280nspill;

  int     num_inghit  [NUM_CYC];
  int     num_pmhit   [NUM_CYC];
  int     num_ingrecon[NUM_CYC];
  int     num_pmrecon [NUM_CYC];

  int     inghit_cyc       [MAX_NUM_INGHIT];
  int     inghit_view      [MAX_NUM_INGHIT];
  int     inghit_pln       [MAX_NUM_INGHIT];
  int     inghit_ch        [MAX_NUM_INGHIT];
  double  inghit_pe        [MAX_NUM_INGHIT];
  double  inghit_time      [MAX_NUM_INGHIT];

  int     pmhit_cyc        [MAX_NUM_INGHIT];
  int     pmhit_view       [MAX_NUM_INGHIT];
  int     pmhit_pln        [MAX_NUM_INGHIT];
  int     pmhit_ch         [MAX_NUM_INGHIT];
  double  pmhit_pe         [MAX_NUM_INGHIT];
  double  pmhit_time       [MAX_NUM_INGHIT];

  double  ingrecon_clstime [MAX_NUM_INGRECON];
  int     ingrecon_view    [MAX_NUM_INGRECON];
  double  ingrecon_slope   [MAX_NUM_INGRECON];
  double  ingrecon_angle   [MAX_NUM_INGRECON];
  double  ingrecon_startz  [MAX_NUM_INGRECON];
  int     ingrecon_startpln[MAX_NUM_INGRECON];
  double  ingrecon_startxy [MAX_NUM_INGRECON];
  double  ingrecon_endz    [MAX_NUM_INGRECON];
  int     ingrecon_endpln  [MAX_NUM_INGRECON];
  double  ingrecon_endxy   [MAX_NUM_INGRECON];
                      
  double  pmrecon_clstime [MAX_NUM_INGRECON];
  int     pmrecon_view    [MAX_NUM_INGRECON];
  double  pmrecon_slope   [MAX_NUM_INGRECON];
  double  pmrecon_angle   [MAX_NUM_INGRECON];
  double  pmrecon_startz  [MAX_NUM_INGRECON];
  int     pmrecon_startpln[MAX_NUM_INGRECON];
  double  pmrecon_startxy [MAX_NUM_INGRECON];
  double  pmrecon_endz    [MAX_NUM_INGRECON];
  int     pmrecon_endpln  [MAX_NUM_INGRECON];
  double  pmrecon_endxy   [MAX_NUM_INGRECON];

  void    Clear();
};

#endif
