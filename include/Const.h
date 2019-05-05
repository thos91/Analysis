#ifndef CONST_H_INCLUDE
#define CONST_H_INCLUDE

#include <string>
#include "ContiguousVectors.h"
#include <memory>
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TNamed.h>

#define NumDif        2
#define NumChip       20
#define NumChipSMRD   3
#define NumChipChFull 36
#define NumChipCh     32
#define NumSca        16
#define BCIDwidth     580 //ns
#define NumReconAxis 2

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

#define NON_BEAM_SPILL 0 // non beam spill bit (spill_flag)
#define BEAM_SPILL     1 // beam spill bit (spill_flag)

// ============ wgEditXML MACROS ============== //

#define CREATE_NEW_MODE 1

// ============ SPIROC2D MACROS ============== //

#define HIT_BIT        1    // there was a hit (over threshold)
#define NO_HIT_BIT     0    // there was no hit (under threshold)
#define HIGH_GAIN_BIT  1    // high gain bit (gs)
#define LOW_GAIN_BIT   0    // low gain bit (gs)
#define HIGH_GAIN_NORM 1.08 // Normalization for the high gain
#define LOW_GAIN_NORM  10.8 // Normalization for the low gain

#define BITSTREAM_HEX_STRING_LENGTH 300  // length of the bitstream hex string
#define BITSTREAM_BIN_STRING_LENGTH 1192 // length of the bitstream bin string
#define VALUE_OFFSET_IN_BITS 6           // offset in bits before a parameter start

#define GLOBAL_THRESHOLD_INDEX      0
#define GLOBAL_THRESHOLD_START      931  // global 10-bit threshold
#define GLOBAL_THRESHOLD_LENGTH     10   // global 10-bit threshold

#define GLOBAL_GS_INDEX             1
#define GLOBAL_GS_THRESHOLD_START   941  // global 10-bit gain selection threshold
#define GLOBAL_GS_THRESHOLD_LENGTH  10   // global 10-bit gain selection threshold

#define ADJ_INPUTDAC_INDEX          2
#define ADJ_INPUTDAC_START          37   // adjustable 8-bit input DAC
#define ADJ_INPUTDAC_LENGTH         8    // adjustable 8-bit input DAC
#define ADJ_INPUTDAC_OFFSET         9    // adjustable 8-bit input DAC

#define ADJ_AMPDAC_INDEX            3
#define ADJ_AMPDAC_START            367  // adjustable 6-bit high gain (HG) preamp
#define ADJ_AMPDAC_LENGTH           6    // adjustable 6-bit high gain (HG) preamp
#define ADJ_AMPDAC_OFFSET           15   // adjustable 6-bit high gain (HG) preamp

#define ADJ_THRESHOLD_INDEX         4
#define ADJ_THRESHOLD_START         1006 // adjustable 4-bit threshold
#define ADJ_THRESHOLD_LENGTH        4    // adjustable 4-bit threshold
#define ADJ_THRESHOLD_OFFSET        4    // adjustable 4-bit threshold

using namespace std;

//#define DEBUG_DECODE

//row data fomat
const uint16_t CHIPHEAD   = 4;
const uint16_t CHIPENDTAG = 2;
const uint16_t CHIPIDSIZE = 1;
const uint16_t NCHANNELS  = NumChipChFull;
const uint16_t MEMDEPTH   = NumSca;
const uint16_t NCHIPS     = NumChip;
const uint16_t NCHIPSSMRD = NumChipSMRD;

//define data fomat
typedef vector<vector<vector<vector<float>>>> f4vector;
typedef Contiguous3Vector<float> f3vector;
typedef Contiguous2Vector<float> f2vector;
typedef vector<float> fvector;
typedef vector<vector<vector<vector<int>>>> i4vector;
typedef Contiguous3Vector<int> i3vector;
typedef Contiguous2Vector<int> i2vector;
typedef vector<int> ivector;

class Raw_t
{
public:
  int spill;
  int spill_mode;
  int spill_count;
  int spill_flag;
  ivector chipid;            // [NumChip];    //ASU 
  ivector chipid_tag;        // [NumChip];    //DIF
  ivector chip;              // [NumChip];
  ivector chipch;            //          [NumChipChFull];
  ivector col;               //                         [NumSca];
  i3vector charge;           // [NumChip][NumChipChFull][NumSca];
  i3vector time;             // [NumChip][NumChipChFull][NumSca];
  i2vector bcid;             // [NumChip]               [NumSca];
  i3vector hit;              // [NumChip][NumChipChFull][NumSca];
  i3vector gs;               // [NumChip][NumChipChFull][NumSca];
  ivector debug;             // [NumChip];
  int view;
  i2vector pln;              // [NumChip][NumChipChFull];
  i2vector ch;               // [NumChip][NumChipChFull];
  i2vector grid;             // [NumChip][NumChipChFull];
  f2vector x;                // [NumChip][NumChipChFull];
  f2vector y;                // [NumChip][NumChipChFull];
  f2vector z;                // [NumChip][NumChipChFull];
  f3vector pedestal;         // [NumChip][NumChipChFull][NumSca];
  f3vector ped_nohit;        // [NumChip][NumChipChFull][NumSca];
  f3vector pe;               // [NumChip][NumChipChFull][NumSca];
  f3vector time_ns;          // [NumChip][NumChipChFull][NumSca];
  f2vector gain;             // [NumChip][NumChipChFull];
  f3vector tdc_slope;        // [NumChip][NumChipChFull][2];
  f3vector tdc_intcpt;       // [NumChip][NumChipChFull][2];

  int n_chips;
  int n_chans;
  int n_cols;

  Raw_t();
  Raw_t(size_t n_chips, size_t n_chans);
  ~Raw_t();
  void clear();
};

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
  float hit_pe          [MAX_NUM_HIT];
  float hit_time        [MAX_NUM_HIT];
  float hit_pe_permm    [MAX_NUM_HIT];
  float hit_cos         [MAX_NUM_HIT];
  float hit_pathlength  [MAX_NUM_HIT];
  bool   hit_ontrack     [MAX_NUM_HIT];
  bool   hit_grid        [MAX_NUM_HIT];
  int    hit_numtrack    [MAX_NUM_HIT];
  float hit_cluster_pe  [MAX_NUM_HIT];

  void Clear();
};


class Recon_t
{
public:

  void Clear();
  void Clear_ReconVector();

  vector<vector<uint16_t> > bcid_cluster_hitid;
  vector<vector<uint16_t> > time_cluster_hitid;

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
  vector<vector<vector<uint16_t> > > neighborhits_hitid;
  vector<vector<vector<uint16_t> > > cell_clusterid;
  vector<vector<vector<uint16_t> > > neighborcell_down_cellid;
  vector<vector<vector<uint16_t> > > neighborcell_up_cellid;
  vector<vector<uint16_t> > cell_state;
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
  vector<short int> num_pass_reconpln;
  vector<short int> num_pass_pair_reconpln;
  vector<vector<short int>> pair_track;
  vector<vector<vector<short int>>> connected_track;
  vector<vector<int>> recon_pair_start_reconpln;
  vector<vector<int>> recon_pair_stop_reconpln;
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
  bool OpenBsdFile(string);
  void GetEntry(int);
  void MakeTree(string);

  string version;
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

class wgConst{
public:
  wgConst();
  ~wgConst();
  void GetENV();
  string RAWDATA_DIRECTORY;
  string DECODE_DIRECTORY;
  string HIST_DIRECTORY;
  string RECON_DIRECTORY;
  string XMLDATA_DIRECTORY;
  string IMGDATA_DIRECTORY;
  string LOG_DIRECTORY;
  string MAIN_DIRECTORY;
  string CALICOES_DIRECTORY;
  string CALIBDATA_DIRECTORY;
  string BSD_DIRECTORY;
  string DQ_DIRECTORY;
  string DQHISTORY_DIRECTORY;
  string CONF_DIRECTORY;
};

#endif
