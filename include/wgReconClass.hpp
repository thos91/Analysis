#ifndef ___WAGASCI_RECON___
#define ___WAGASCI_RECON___

/* ***********************************************************************
 * WAGASCI reconstruction for wagasci 
 * Program : wgRecon.h
 * Name: Naruhiro Chikuma
 * Date: 2017-09-05
 * ********************************************************************** */

// user includes
#include "Const.hpp"
#include "wgChannelMap.hpp"

#define NumHitPara 13
#define HGtoLG 3000.
#define TimeOffsetEven 400.
#define TimeOffsetOdd  4000.
#define TimeCoeffEven 1.
#define TimeCoeffOdd -1.

#define MAX_DIFF_BCID 1 //allowed diff. in bcids to be clustered
#define MAX_DIFF_BCID_OPTVIEW 2 //allowed diff. in bcids to be clustered
#define THRES_NHITS  4 //minimum number of hits required to start reconstruction
#define THRES_NHITS_VIEW  4 //minimum number of hits required to start reconstruction

#define THRES_PE 0 //TODO
#define MAX_DIFF_RECONPLN_CELL 6  //allowed diff. in pln to be paired as a cell
#define MAX_DIFF_RECONPLN_CELL_GRIDGRID 9 //if GRID to GRID,  allowed diff. in pln to be paired as a cell

const double MAX_DIFF_CLUSTER_POS[MAX_DIFF_RECONPLN_CELL]
  = {155.,155.,155.,155.,155.,155.}; //mm
           //allowed diff. in center position of neighbor hits clusters to be paired as a cell
#define MAX_WIDTH_RECONPLN_NEIGHBORCELLS 6 //allowed diff. in reconpln to find a neighbor-cell pair
const short MAX_WIDTH_RECONPLN_NEIGHBORCELLS2[2]={6,9}; //allowed diff. in reconpln to find a neighbor-cell pair

//#define MAX_ANGLE_NEIGHBORCELLS   0.866025  //minimum cos = 30deg //allowed angle in reconpln to find a neighbor-cell pair
//#define MAX_ANGLE_NEIGHBORCELLS   0.707107  //if neighboring pln and grid makes cell, minimum cos = 45deg //allowed angle in reconpln to find a neighbor-cell pair
#define MAX_ANGLE_NEIGHBORCELLS   0.65        //if neighboring pln and grid makes cell, minimum cos = 45deg //allowed angle in reconpln to find a neighbor-cell pair
#define MAX_ANGLE_NEIGHBORCELLS2  0.707107  //if neighboring pln and grid makes cell, minimum cos = 45deg //allowed angle in reconpln to find a neighbor-cell pair

//#define LIMIT_CHI2_NEIGHBORCELL 3.  //allowed chi2 by linear fit to find a pair of  neighbor cells

#define THRES_TRACK_HITS  3 //minimum number of hits consisting of track,
#define LIMIT_CHI2_FIND_TRACK1  15.0 //allowed chi2 by linear fit to add a cluster into a track
#define LIMIT_CHI2_FIND_TRACK2  0.1 //fine adjustment to chi2 limit according to num of cluster
#define MAX_SHARED_CLUSTER 7  //max allowed num of shared clusters between tracks
#define NUM_UP_VETO_PLN_SIDE 0
#define NUM_UP_VETO_PLN_TOP  0
#define NUM_DOWN_VETO_PLN_SIDE 7
#define NUM_DOWN_VETO_PLN_TOP  7
#define NUM_TOP_VETO_CH_SIDE 38
#define NUM_TOP_VETO_CH_TOP  38
#define NUM_BOTTOM_VETO_CH_SIDE 1 
#define NUM_BOTTOM_VETO_CH_TOP  1

//#define MAX_DEGREE_RECON_TRACK_MATCH 30.  //max allowd degree to find two tracks with same slope

//#define DEBUG_RECON

class wgRecon
{
  //bcid view pln ch dif chip chipch sca adc gs tdc pe time
  int num_cell    [MAX_NUM_BCID_CLUSTER];
  private:  
  int num_hit        ;
  int num_hit_view[2];

  public:

  std::vector< std::vector<double> > recon_hit;

  Raw_t      type_raw[2];
  Hit_t      type_hit;
  Recon_t    type_recon;
  Track_t    type_track;

  Map_t      type_map;
  MapInv_t   type_map_inv;
  ReconMap_t type_remap;

  wgRecon();
  ~wgRecon();
  void clear();

  int get_num_hit  (); 

  int get_hitbcid  (int hitid); //0
  int get_hitview  (int hitid); //1
  int get_hitpln   (int hitid); //2
  int get_hitch    (int hitid); //3
  int get_hitdif   (int hitid); //4
  int get_hitchip  (int hitid); //5
  int get_hitchipch(int hitid); //6
  int get_hitsca   (int hitid); //7
  int get_hitadc   (int hitid); //8
  int get_hitgs    (int hitid); //9
  int get_hittdc   (int hitid); //10
  double get_hitpe  (int hitid); //11
  double get_hittime(int hitid); //12

  void pushHitInfo(int bcid,int view,int pln,int ch,int dif,int chip,int chipch,int sca,int adc,int gs,int tdc,double pe,double time);
  bool pushHitStruct(int bcid,int view,int pln,int ch,int dif,int chip,int chipch,int sca,int adc,int gs,int tdc,double pe,double time);
  void sort_byBCIDnMAP(); // Sort all hits by BCID-view-pln-ch in this order

  bool findBCIDCluster();
  bool findTimeCluster();

  bool Tracking_alongZ();
  bool Tracking_alongXY();

  bool findNeighborHits (int axis); // Define "Cluster"
  bool findClusterPair  (int axis);  // Define "Cell"
  bool findNeighborCells(int axis);
  bool defineCellState  (int axis);
  bool findTrack        (int axis);
  bool rankTracks_byEdep(int axis); 
                          // If two tracks share a hit (//TODO should be cluster, instead of hit),
                          // one with the larger edep is ranked higher
  bool eraceDuplicateTrack();
  bool findTrackPair();
  bool addNearHits();
  void fillTrackHit();

  bool selectTrueTrack      ();  //TODO: to be implemented.
  bool check_Veto_SideEscape(int axis);

  double GetDispersion(double,double,vector<double>,vector<double>);
  double cal_pathlength(int view, double slope1, double slope2, bool grid);

  //******
  bool Hough();
};


#endif
