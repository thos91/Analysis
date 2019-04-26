#ifndef CONST_H_INCLUDE
#define CONST_H_INCLUDE

#include <string>
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

using namespace std;

//#define DEBUG_DECODE

//row data fomat
const unsigned int CHIPHEAD   = 4;
const unsigned int CHIPENDTAG = 2;
const unsigned int CHIPIDSIZE = 1;
const unsigned int NCHANNELS  = NumChipChFull;
const unsigned int MEMDEPTH   = NumSca;
const unsigned int NCHIPS     = NumChip;
const unsigned int NCHIPSSMRD = NumChipSMRD;

//define data fomat

typedef vector<vector<vector<double>>> d3vector;
typedef vector<vector<double>> d2vector;
typedef vector<double> dvector;
typedef vector<vector<vector<int>>> i3vector;
typedef vector<vector<int>> i2vector;
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
  d2vector x;                // [NumChip][NumChipChFull];
  d2vector y;                // [NumChip][NumChipChFull];
  d2vector z;                // [NumChip][NumChipChFull];
  d3vector pedestal;         // [NumChip][NumChipChFull][NumSca];
  d3vector ped_nohit;        // [NumChip][NumChipChFull][NumSca];
  d3vector pe;               // [NumChip][NumChipChFull][NumSca];
  d3vector time_ns;          // [NumChip][NumChipChFull][NumSca];
  d2vector gain;             // [NumChip][NumChipChFull];
  d2vector tdc_slope;        // [NumChip][NumChipChFull];
  d2vector tdc_intcpt;       // [NumChip][NumChipChFull];

  int n_chips;
  int n_chans;
  int n_cols;

  Raw_t();
  Raw_t(size_t n_chips, size_t n_chans, size_t n_cols);
  ~Raw_t();
  void init(size_t n_chips, size_t n_chans, size_t n_cols);
  void clear(size_t n_chips, size_t n_chans, size_t n_cols);
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

  vector<vector<unsigned int> > bcid_cluster_hitid;
  vector<vector<unsigned int> > time_cluster_hitid;

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
  vector<vector<vector<unsigned int> > > neighborhits_hitid;
  vector<vector<vector<unsigned int> > > cell_clusterid;
  vector<vector<vector<unsigned int> > > neighborcell_down_cellid;
  vector<vector<vector<unsigned int> > > neighborcell_up_cellid;
  vector<vector<unsigned int> > cell_state;  
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
  char* RAWDATA_DIRECTORY;
  char* DECODE_DIRECTORY;
  char* HIST_DIRECTORY;
  char* RECON_DIRECTORY;
  char* XMLDATA_DIRECTORY;
  char* IMGDATA_DIRECTORY;
  char* LOG_DIRECTORY;
  char* MAIN_DIRECTORY;
  char* RUNCOMMAND_DIRECTORY;
  char* CALICOE_DIRECTORY;
  char* CALIBDATA_DIRECTORY;
  char* BSD_DIRECTORY;
  char* DQ_DIRECTORY;
  char* DQHISTORY_DIRECTORY;
  char* SPILL_DIRECTORY;
};

#endif
