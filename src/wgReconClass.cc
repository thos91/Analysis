/************************************************************************
 * WAGASCI reconstruction class for wagasci 
 * Program : wgRecon.cc
 * Name: Naruhiro Chikuma
 * Date: 2017-09-05
 * ********************************************************************** */

#include <iostream>

#include <TROOT.h>
#include <TFile.h>
#include <TF1.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TGraphAsymmErrors.h>
#include <TCanvas.h>

#include "wgReconClass.h"
#include "wgChannelMap.h"
#include "DetectorConst.h"


using namespace std;

//********************************************************************
wgRecon::wgRecon()
{
  wgChannelMap *chmap = new wgChannelMap();  
  type_map     = chmap->load_mapping();
  type_map_inv = chmap->load_mapping_inv();
  type_remap   = chmap->load_reconmap();
#ifdef DEBUG_RECON
/*
  for(int i=0;i<NumDif;i++){
    for(int j=0;j<NumChip;j++){
      for(int k=0;k<NumChipCh;k++){
        int view =type_map.view[i][j][k];
        int pln  =type_map.pln [i][j][k];
        int ch   =type_map.ch  [i][j][k];
        int recon_axis = 0;
        cout 
          << " dif="    << i
          << " chip="   << j
          << " chipch=" << k
          << " view="   << view
          << " pln="    << pln 
          << " ch="     << ch  
          << " x="      << type_map.x[i][j][k]
          << " y="      << type_map.y[i][j][k]
          << " z="      << type_map.z[i][j][k]
          << " recon_view=" << type_remap.recon_view[recon_axis][view][pln][ch]
          << " recon_pln="  << type_remap.recon_pln [recon_axis][view][pln][ch]
          << " recon_ch="   << type_remap.recon_ch  [recon_axis][view][pln][ch]
          << endl;
      }
    }
  }
*/
#endif
};

//********************************************************************
wgRecon::~wgRecon()
{};

//********************************************************************
void wgRecon::clear()
{
  type_hit.num_bcid_cluster = 0;
  num_hit         = 0;
  num_hit_view[0]=num_hit_view[1] = 0;
  for(int i=0;i<MAX_NUM_BCID_CLUSTER;i++){
    num_cell    [i] = 0;
  }
  recon_hit .clear();
  type_hit  .Clear();
  type_recon.Clear();
  type_track.Clear();
};

//********************************************************************
int wgRecon::get_num_hit()
{
  return num_hit;
};

//********************************************************************
int wgRecon::get_hitbcid(int hitid)
{
  if(hitid>=num_hit){
    cerr << "[BCID]This hitid is out of the range, hitid=" << hitid << endl;
    return -1;
  }
  else{
    try{
      return (int)recon_hit.at(hitid).at(0);
    } catch (const out_of_range& oor){
      cout << "Error! (size:" << hitid << ") (get_hitbcid)" << oor.what() << endl;
      return -1;
    }
  }
};

//********************************************************************
int wgRecon::get_hitview(int hitid)
{
  if(hitid>=num_hit){
    cerr << "[View]This hitid is out of the range, hitid=" << hitid <<", numhit:"<< num_hit << endl;
    return -1;
  }
  else{
    try{
      return (int)recon_hit.at(hitid).at(1);
    } catch (const out_of_range& oor){
      cout << "Error! (size:" << hitid << ") (get_hitview) " << oor.what() << endl;
      return -1;
    }
  }
};

//********************************************************************
int wgRecon::get_hitpln(int hitid)
{
  if(hitid>=num_hit){
    cerr << "This hitid is out of the range, hitid=" << hitid << endl;
    return -1;
  }
  else{
    try{
      return (int)recon_hit.at(hitid).at(2);
    } catch (const out_of_range& oor){
      cout << "Error! (size:" << hitid << ") (get_hitpln) " << oor.what() << endl;
      return -1;
    }
  }
};

//********************************************************************
int wgRecon::get_hitch(int hitid)
{
  if(hitid>=num_hit){
    cerr << "This hitid is out of the range, hitid=" << hitid << endl;
    return -1;
  }
  else{
    try{
      return (int)recon_hit.at(hitid).at(3);
    } catch (const out_of_range& oor){
      cout << "Error! (size:" << hitid << ") (get_hitch) " << oor.what() << endl;
      return -1;
    }
  }
};

//********************************************************************
int wgRecon::get_hitdif(int hitid)
{
  if(hitid>=num_hit){
    cerr << "This hitid is out of the range, hitid=" << hitid << endl;
    return -1;
  }
  else{
    try{
      return (int)recon_hit.at(hitid).at(4);
    } catch (const out_of_range& oor){
      cout << "Error! (size:" << hitid << ") (get_hitdif) " << oor.what() << endl;
      return -1;
    }
  }
};

//********************************************************************
int wgRecon::get_hitchip(int hitid)
{
  if(hitid>=num_hit){
    cerr << "This hitid is out of the range, hitid=" << hitid << endl;
    return -1;
  }
  else{
    try{
      return (int)recon_hit.at(hitid).at(5);
    } catch (const out_of_range& oor){
      cout << "Error! (size:" << hitid << ") (get_hitchip) " << oor.what() << endl;
      return -1;
    }
  }
};

//********************************************************************
int wgRecon::get_hitchipch(int hitid)
{
  if(hitid>=num_hit){
    cerr << "This hitid is out of the range, hitid=" << hitid << endl;
    return -1;
  }
  else{
    try{
      return (int)recon_hit.at(hitid).at(6);
    } catch (const out_of_range& oor){
      cout << "Error! (size:" << hitid << ") (get_hitchipch) " << oor.what() << endl;
      return -1;
    }
  }
};

//********************************************************************
int wgRecon::get_hitsca(int hitid)
{
  if(hitid>=num_hit){
    cerr << "This hitid is out of the range, hitid=" << hitid << endl;
    return -1;
  }
  else{
    try{
      return (int)recon_hit.at(hitid).at(7);
    } catch (const out_of_range& oor){
      cout << "Error! (size:" << hitid << ") (get_hitsca) " << oor.what() << endl;
      return -1;
    }
  }
};

//********************************************************************
int wgRecon::get_hitadc(int hitid)
{
  if(hitid>=num_hit){
    cerr << "This hitid is out of the range, hitid=" << hitid << endl;
    return -1;
  }
  else{
    try{
      return (int)recon_hit.at(hitid).at(8);
    } catch (const out_of_range& oor){
      cout << "Error! (size:" << hitid << ") (get_hitadc) " << oor.what() << endl;
      return -1;
    }
  }
};

//********************************************************************
int wgRecon::get_hitgs(int hitid)
{
  if(hitid>=num_hit){
    cerr << "This hitid is out of the range, hitid=" << hitid << endl;
    return -1;
  }
  else{
    try{
      return (int)recon_hit.at(hitid).at(9);
    } catch (const out_of_range& oor){
      cout << "Error! (size:" << hitid << ") (get_hitgs) " << oor.what() << endl;
      return -1;
    }
  }
};

//********************************************************************
int wgRecon::get_hittdc(int hitid)
{
  if(hitid>=num_hit){
    cerr << "This hitid is out of the range, hitid=" << hitid << endl;
    return -1;
  }
  else{
    try{
      return (int)recon_hit.at(hitid).at(10);
    } catch (const out_of_range& oor){
      cout << "Error! (size:" << hitid << ") (get_hittdc) " << oor.what() << endl;
      return -1;
    }
  }
};

//********************************************************************
double wgRecon::get_hitpe(int hitid)
{
  if(hitid>=num_hit){
    cerr << "This hitid is out of the range, hitid=" << hitid << endl;
    return -1;
  }
  else{
    try{
      return (double)recon_hit.at(hitid).at(11);
    } catch (const out_of_range& oor){
      cout << "Error! (size:" << hitid << ") (get_hitpe) " << oor.what() << endl;
      return -1;
    }
  }
};

//********************************************************************
double wgRecon::get_hittime(int hitid)
{
  if(hitid>=num_hit){
    cerr << "This hitid is out of the range, hitid=" << hitid << endl;
    return -1;
  }
  else{
    try{
      return (double)recon_hit.at(hitid).at(12);
    } catch (const out_of_range& oor){
      cout << "Error! (size:" << hitid << ") (get_hittime) " << oor.what() << endl;
      return -1;
    }
  }
};

//********************************************************************
void wgRecon::pushHitInfo(
    int bcid,
    int view,int pln,int ch,
    int dif,int chip,int chipch,int sca,
    int adc,int gs,int tdc,
    double pe,double time){

  if(pe<0.5) return;

  vector<double> tmp;
  tmp.resize(NumHitPara);
 
  int mod_bcid=bcid;
  if(chip%5>=2) mod_bcid++;

  tmp[ 0] = (double)mod_bcid;
  tmp[ 1] = (double)view;
  tmp[ 2] = (double)pln;
  tmp[ 3] = (double)ch;
  tmp[ 4] = (double)dif;
  tmp[ 5] = (double)chip;
  tmp[ 6] = (double)chipch;
  tmp[ 7] = (double)sca;
  tmp[ 8] = (double)adc;
  tmp[ 9] = (double)gs;
  tmp[10] = (double)tdc;
  tmp[11] = (double)pe;
  tmp[12] = (double)time;

  recon_hit.push_back(tmp);
  num_hit++;
  num_hit_view[view]++;
};

//********************************************************************
bool wgRecon::pushHitStruct(
    int bcid,
    int view, int pln, int ch,
    int dif,int chip,int chipch,
    int sca,
    int adc,int gs, int tdc,
    double pe,double time)
{
  if(type_hit.num_hits>=MAX_NUM_HIT){
    cerr << "Hit ID is out of the range,"
      << " num_hits=" << type_hit.num_hits << " MAX_NUM_HIT="<< MAX_NUM_HIT << endl;
    return false;
  }
  type_hit.hit_bcid  [type_hit.num_hits]=bcid  ;
  type_hit.hit_view  [type_hit.num_hits]=view  ;
  type_hit.hit_pln   [type_hit.num_hits]=pln   ;
  type_hit.hit_ch    [type_hit.num_hits]=ch    ;
  type_hit.hit_dif   [type_hit.num_hits]=dif   ;
  type_hit.hit_chip  [type_hit.num_hits]=chip  ;
  type_hit.hit_chipch[type_hit.num_hits]=chipch;
  type_hit.hit_sca   [type_hit.num_hits]=sca   ;
  type_hit.hit_adc   [type_hit.num_hits]=adc   ;
  type_hit.hit_gs    [type_hit.num_hits]=gs    ;
  type_hit.hit_tdc   [type_hit.num_hits]=tdc   ;
  type_hit.hit_pe    [type_hit.num_hits]=pe    ;
  type_hit.hit_time  [type_hit.num_hits]=time  ;

  if(ch<C_WMNumXYLayerCh) type_hit.hit_grid[type_hit.num_hits]=false;
  else type_hit.hit_grid[type_hit.num_hits]=true;
  type_hit.num_hits++;
  return true;
};

//********************************************************************
bool comp_bcid_map(const vector<double> &v1, const vector<double> &v2){
  if(v1.size()<4||v2.size()<4){
    cerr << "The vector does not have enough length to sort. (bcid,view,pln,ch)" << endl;
    return false;
  }
  else{
    if(v1[0] < v2[0]){ // bcid
      return true;
    }else if(v1[0]==v2[0]){
      if(v1[1] < v2[1]){ //view
        return true;
      }else if(v1[1]==v2[1]){
        if(v1[2] < v2[2]){ //pln
          return true;
        }else if(v1[2]==v2[2]){
          if(v1[3] < v2[3]){ //ch
            return true;
          }else{
            return false;
          }
        } else return false;
      } else return false;
    } else return false;
  }
};

//********************************************************************
void wgRecon::sort_byBCIDnMAP(){
  sort(recon_hit.begin(),recon_hit.end(),comp_bcid_map);
};

//********************************************************************
bool wgRecon::findBCIDCluster(){
  if(num_hit>=MAX_NUM_HIT){
    cout << "This event has too many hits, to be ignored." << endl;
    return false;
  }

  if(num_hit_view[0]<THRES_NHITS_VIEW && num_hit_view[1]<THRES_NHITS_VIEW){
#ifdef DEBUG_RECON
    cout << "This spill has not enough number of hits"
      <<" : nhits=" << num_hit
      <<" : THRES_NHITS=" << THRES_NHITS << endl;
#endif
    return false;
  }

  int i_bcid_cluster = 0;
  int current_bcid=-1;
  int current_view=0;
  for(int hitid=0;hitid<type_hit.num_hits;hitid++){
    if(current_bcid==get_hitbcid(hitid)){
      continue;
    }

    int nhit_view[3]={0,0,0};
    current_bcid = get_hitbcid(hitid);
    current_view = get_hitview(hitid);
    vector<unsigned int> id_bcid_cluster;
    vector<unsigned int> id_bcid_cluster2[2];
    id_bcid_cluster.clear();
    id_bcid_cluster.push_back(hitid);

    for(int hitid2=hitid+1;hitid2<type_hit.num_hits;hitid2++){
      int bcid = get_hitbcid(hitid2);
      int view = get_hitview(hitid2);
  
#ifdef DEBUG_RECON
      if(bcid<current_bcid){
        cout << "!WARNING! recon hit is not in order of BCID (currnet:" << current_bcid << "," << bcid <<" ) " << endl;
      }
#endif

      if(current_view!=view && fabs(bcid-current_bcid)<=MAX_DIFF_BCID_OPTVIEW){
        if(abs(bcid-current_bcid)<=MAX_DIFF_BCID){
          nhit_view[1]++;
          id_bcid_cluster2[0].push_back(hitid2);
        }else{
          nhit_view[2]++;
          id_bcid_cluster2[1].push_back(hitid2);
        }
      }else if(current_view==view && fabs(bcid-current_bcid)<=MAX_DIFF_BCID){
        id_bcid_cluster.push_back(hitid2);
        nhit_view[0]++;
      }else{
        // check the candidate of bcid cluster.
        if(id_bcid_cluster.size()>=THRES_NHITS){
          if(nhit_view[0]<3 || (nhit_view[1]<3 && nhit_view[2]<3)) break;
          
          if(nhit_view[1]>=3){
            for(int ihit=0;ihit<(int)id_bcid_cluster2[0].size();ihit++){
              id_bcid_cluster.push_back(id_bcid_cluster2[0][ihit]);
            }
          }

          if(nhit_view[2]>=3){
            for(int ihit=0;ihit<(int)id_bcid_cluster2[1].size();ihit++){
              id_bcid_cluster.push_back(id_bcid_cluster2[1][ihit]);
            }
          }

          if(id_bcid_cluster.size()>= MAX_NUM_HIT){
            cout << "!WARNING!: Too many hits are clustered. This cluster is aborted!" << endl;
            break;
          }

          type_recon.bcid_cluster_hitid.push_back(id_bcid_cluster);
          type_hit.num_bcid_hits[i_bcid_cluster] = (int)id_bcid_cluster.size();
          type_hit.clustered_bcid[i_bcid_cluster] = current_bcid;
          for(int m=0;m<(int)id_bcid_cluster.size();m++){
            type_hit.clustered_hitid[i_bcid_cluster][m] = id_bcid_cluster[m];
          }
          i_bcid_cluster++;
        }
        break;
      }
    }//hitid2
  }//hitid

  type_hit.num_bcid_cluster = (int)type_recon.bcid_cluster_hitid.size();

#ifdef DEBUG_RECON
  cout << "=========================================" << endl;
  cout << "findBCIDCluster is done ..." << endl;
#endif
  if(type_hit.num_bcid_cluster>0){
#ifdef DEBUG_RECON
    for(int i=0;i<type_hit.num_bcid_cluster;i++){
      cout << "[";
      for(int j=0;j<type_hit.num_bcid_hits[i];j++){
        int hitid = type_recon.bcid_cluster_hitid[i][j];
        cout << "{"
          << "hitid:" << hitid
          << ",bcid:" << get_hitbcid(hitid)
          << "}"; 
      }
      cout << "]" << endl;
      //}
    }
    cout << endl;
#endif
  }else{
#ifdef DEBUG_RECON
    cout << "no BCID Cluster is found." << endl;
#endif
    return false;
  }
  return true;
};

//********************************************************************
bool wgRecon::findTimeCluster()
{

  type_recon.time_cluster_hitid.clear();
  for(int i=0;i<type_hit.num_bcid_cluster;i++){
    vector<unsigned int> hitid;
    hitid.resize(type_hit.num_bcid_hits[i]);
    for(int j=0;j<type_hit.num_bcid_hits[i];j++){
      hitid[j] = type_recon.bcid_cluster_hitid[i][j];
      //double time = get_hittime(hitid[j]);
      //if(j==0) current_time = time;
      //if(j==type_hit.num_bcid_hits[i]-1) cout <<"a" << endl; //TODO
      //if( abs(current_bcid-time
    }
    type_recon.time_cluster_hitid.push_back(hitid);
  }
#ifdef DEBUG_RECON
  //if(type_hit.num_bcid_cluster>0){
  //  cout << "=========================================" << endl;
  //  cout << "findTimeCluster is done ..." << endl;
  //  for(int i=0;i<type_hit.num_bcid_cluster;i++){
  //    int num_timecluster = (int)type_recon.time_cluster_hitid[i].size();
  //    if(num_timecluster>0){
  //      cout << "{";
  //      for(int j=0;j<num_timecluster;j++){
  //        cout << type_recon.time_cluster_hitid[i][j];
  //        if(j!=(int)type_recon.time_cluster_hitid[i].size()-1)cout << ",";
  //      }
  //      cout << "}";
  //    }
  //  }
  //  cout << endl;
  //}
#endif
  return true;
};

//********************************************************************
bool wgRecon::findNeighborHits(int axis)
{
  type_recon.neighborhits_hitid.resize(type_recon.time_cluster_hitid.size());

  for(int i=0;i<(int)type_recon.time_cluster_hitid.size();i++){
    int timecluster_size = (int)type_recon.time_cluster_hitid[i].size();
    if(axis==0){
      vector<int> view, pln, ch, hitid;
      hitid.resize(timecluster_size);
      view .resize(timecluster_size);
      pln  .resize(timecluster_size);
      ch   .resize(timecluster_size);
      vector<unsigned int> tmp;
      for(int j=0;j<timecluster_size;j++){
        hitid[j] = type_recon.time_cluster_hitid[i][j];
        view [j] = get_hitview(hitid[j]); 
        pln  [j] = get_hitpln (hitid[j]); 
        ch   [j] = get_hitch  (hitid[j]); 
        if(j==0){ continue; }
        else{
          tmp.push_back((unsigned int)hitid[j-1]);
          if(!((ch[j]<C_WMNumXYLayerCh)&&(pln[j]==pln[j-1])&&(ch[j]==ch[j-1]+1))){
            type_recon.neighborhits_hitid[i].push_back(tmp);
            tmp.clear();
          }
          if(j==timecluster_size-1){
            tmp.push_back((unsigned int)hitid[j]);
            type_recon.neighborhits_hitid[i].push_back(tmp);
            tmp.clear();
          }
        }
      }
    }
    else if(axis==1){
      for(int j=0;j<timecluster_size;j++){
        unsigned int hitid = type_recon.time_cluster_hitid[i][j];
        vector<unsigned int> tmp;
        tmp.push_back(hitid);
        type_recon.neighborhits_hitid[i].push_back(tmp);
        tmp.clear();
      }
    }
    else{
      cerr << "Wrong axis value: "
        << " 0=> reconstruction along Z axis,"
        << " 1=> reconstruction along XY axis"
        << endl;
      return false;
    }
  }

#ifdef DEBUG_RECON
  if(type_recon.time_cluster_hitid.size()>0){
    cout << "=========================================" << endl;
    cout << "findNeighborHits is done..." << endl;
    for(int i=0;i<(int)type_recon.time_cluster_hitid.size();i++){
      int num_neighorhitscluster = (int)type_recon.neighborhits_hitid[i].size();
      if(num_neighorhitscluster>0){
        for(int j=0;j<num_neighorhitscluster;j++){
          int num_neighborhits = (int)type_recon.neighborhits_hitid[i][j].size();
          if(num_neighborhits>0){
            for(int k=0;k<num_neighborhits;k++){
              int hitid = type_recon.neighborhits_hitid[i][j][k];
              cout << "{"
                << "hitid:" << hitid
                << ",bcid:" << get_hitbcid(hitid)
                << ",view:" << get_hitview(hitid)
                << ",pln:"  << get_hitpln (hitid)
                << ",ch:"   << get_hitch  (hitid)
                << "}"; 
            }
            cout << endl;
          }
        }
      }
    }
  }
#endif
  if(type_recon.time_cluster_hitid.size()==0){
    return false;
  }
  return true;
};

//********************************************************************
bool wgRecon::findClusterPair(int axis)
{
  type_recon.cell_clusterid.resize(type_recon.time_cluster_hitid.size());
  for(int i=0;i<(int)type_recon.time_cluster_hitid.size();i++){
#ifdef DEBUG_RECON
    int tmpid = type_recon.neighborhits_hitid[i][0][0];
    cout << "bcid=" << get_hitbcid(tmpid)<<endl;
#endif
    int num_cluster = (int)type_recon.neighborhits_hitid[i].size();
    int j[2];
    double cluster_xy[2] = {0.0,0.0}, cluster_z[2] = {0.0,0.0}, cluster_pe[2] = {0.0,0.0};
    bool grid[2] = {false,false};
    for(j[0]=0;j[0]<num_cluster-1;j[0]++){
      for(j[1]=j[0]+1;j[1]<num_cluster;j[1]++){
        int cluster_size[2];
        int view[2],pln[2],ch[2],reconpln[2];
        vector<unsigned int> tmp;
        tmp.resize(2);
        for(int k=0;k<2;k++){
          if(k==0&&j[1]!=j[0]+1){continue;}
          else{
            cluster_size[k] = type_recon.neighborhits_hitid[i][j[k]].size();
            cluster_xy[k] = 0.0;
            cluster_z [k] = 0.0;
            cluster_pe[k] = 0.0;
            grid[k]=false;
            for(int l=0;l<cluster_size[k];l++){
              int hitid  = type_recon.neighborhits_hitid[i][j[k]][l];
              int dif    = get_hitdif   (hitid);
              int chip   = get_hitchip  (hitid);
              int chipch = get_hitchipch(hitid);
              double pe  = get_hitpe    (hitid);
              double x   = type_map.x[dif][chip][chipch];
              double y   = type_map.y[dif][chip][chipch];
              double z   = type_map.z[dif][chip][chipch];
              if(l==0){
                view[k] = get_hitview(hitid);
                pln [k] = get_hitpln (hitid);
                ch  [k] = get_hitch  (hitid);
                reconpln[k] = type_remap.recon_pln[axis][view[k]][pln[k]][ch[k]];
                if(ch[k]>=C_WMNumXYLayerCh) grid[k]=true;
              }
              if(view[k]==SideView){ cluster_xy[k] += pe*y; cluster_z[k] += pe*z; }
              if(view[k]==TopView ){ cluster_xy[k] += pe*x; cluster_z[k] += pe*z; }
              cluster_pe[k] += pe;
            }
            cluster_xy[k] /= cluster_pe[k];
            cluster_z [k] /= cluster_pe[k];
          }
        } //k
        int    diff_reconpln = reconpln[1]-reconpln[0];
        double diff_cluster_pos;
        if     (axis==0){ diff_cluster_pos = cluster_xy[1]-cluster_xy[0]; }
        else if(axis==1){ diff_cluster_pos = cluster_z [1]-cluster_z [0]; }
        bool GRIDtoGRID = false;
        if(axis==1 && fabs(diff_cluster_pos) >5.0 && fabs(diff_cluster_pos) <C_WMScintiWidth +5.0 && grid[0] && grid[1]) GRIDtoGRID = true; 

        if(view[0] == view[1]){
          if( (GRIDtoGRID && abs(diff_reconpln)<=MAX_DIFF_RECONPLN_CELL_GRIDGRID) 
              || (diff_reconpln!=0 && abs(diff_reconpln)<=MAX_DIFF_RECONPLN_CELL) ){
            if( GRIDtoGRID ){
              if(diff_reconpln>0) diff_reconpln=1;
              else if(diff_reconpln<0) diff_reconpln=-1;
            }
            if(fabs(diff_cluster_pos) < (float)MAX_DIFF_CLUSTER_POS[abs(diff_reconpln)-1]){
              //TODO
              //Another criteria should be incremented? 
              //1. ignored if there is a hit ch on the cell
              //2. ignored if there is two nohit ch on the cell or more
              //3. Two cluster in the same reconpln could not be paried. To be paired?
              if(diff_reconpln>0){
                tmp[0] = j[0]; tmp[1] = j[1]; 
              }else{       
                tmp[0] = j[1]; tmp[1] = j[0]; 
              }
              type_recon.cell_clusterid[i].push_back(tmp);
            }
          }
        }
        tmp.clear();
      } //j[1]
    } //j[0]
    num_cell[i] = (int) type_recon.cell_clusterid[i].size();
  } //i

#ifdef DEBUG_RECON
  if(type_recon.time_cluster_hitid.size()>0){
    cout << "=========================================" << endl;
    cout << "findClusterPair is done..." << endl;
    for(int i=0;i<(int)type_recon.time_cluster_hitid.size();i++){
      for(int j=0;j<num_cell[i];j++){
        int cellsize = (int)type_recon.cell_clusterid[i][j].size();
        if(cellsize!=2){
          cerr << "Cell size is strange : cellsize=" << cellsize << endl;
        }else{
          int clusterid1 = type_recon.cell_clusterid[i][j][0];
          int clusterid2 = type_recon.cell_clusterid[i][j][1];
          int hitid1 = type_recon.neighborhits_hitid[i][clusterid1][0];
          int hitid2 = type_recon.neighborhits_hitid[i][clusterid2][0];
          cout << "{"
            << "bcid:"  << get_hitbcid(hitid1)
            << "/view:" << get_hitview(hitid1)
            << ",pln:"  << get_hitpln (hitid1)
            << ",ch:"   << get_hitch  (hitid1)
            << "/"
            << "view:"  << get_hitview(hitid2)
            << ",pln:"  << get_hitpln (hitid2)
            << ",ch:"   << get_hitch  (hitid2)
            << "}";
          cout << endl;
        }
      }
      cout << endl;
    }
  }
#endif 
  return true;
};

//********************************************************************
bool wgRecon::findNeighborCells(int axis)
{
#ifdef DEBUG_RECON
  cout << "findNeighborCells starts..... " << endl;
  cout << "=========================================" << endl;
#endif
  type_recon.neighborcell_down_cellid.resize(type_recon.time_cluster_hitid.size());
  type_recon.neighborcell_up_cellid  .resize(type_recon.time_cluster_hitid.size());
  for(int i=0;i<(int)type_recon.time_cluster_hitid.size();i++){
    type_recon.neighborcell_down_cellid[i].resize(num_cell[i]);
    type_recon.neighborcell_up_cellid  [i].resize(num_cell[i]);
    int j[2],cluster_id[2][2],cluster_size[2][2];
    int cluster_reconpln[3];
    double cluster_posz[3],cluster_posxy[3],cluster_errz[3],cluster_errxy[3]; 
    double cluster_pe[3];
    bool grid[3]={false,false,false};
    for(j[0]=0;j[0]<num_cell[i]-1;j[0]++){
      for(j[1]=j[0]+1;j[1]<num_cell[i];j[1]++){
        bool isPaired = true;
        int view_Paired = 0;
        bool updown = false;
        for(int k=0;k<2;k++){
          int cellsize=type_recon.cell_clusterid[i][j[k]].size();
          if(cellsize!=2){
            cerr << "Cell size is strange : cellsize=" << cellsize << endl;
            isPaired = false;
            continue;
          }
          cluster_id  [k][0] = type_recon.cell_clusterid[i][j[k]][0];
          cluster_id  [k][1] = type_recon.cell_clusterid[i][j[k]][1];
          if(k==1){ 
            if((cluster_id[0][1]!=cluster_id[1][0])&&
                (cluster_id[0][0]!=cluster_id[1][1]))
            {
              isPaired = false;
              continue;
            } 
          }
          for(int l=0;l<2;l++){
            int tmp_id;
            if(k==0){ tmp_id=l; }
            else if(k==1){
              if(cluster_id[0][1]==cluster_id[1][0]){//{k,l}={1,0} identical to {0,1}
                if(l==0){continue;}
                else{ tmp_id=2; }
              }
              else if(cluster_id[0][0]==cluster_id[1][1]){
                if(l==1){continue;} 
                else{
                  cluster_posz [2] = cluster_posz [1];
                  cluster_posxy[2] = cluster_posxy[1];
                  cluster_errz [2] = cluster_errz [1];
                  cluster_errxy[2] = cluster_errxy[1];
                  cluster_pe   [2] = cluster_pe   [1];
                  cluster_reconpln[2] = cluster_reconpln[1];
                  grid[2] = grid[1];
                  cluster_posz [1] = cluster_posz [0];
                  cluster_posxy[1] = cluster_posxy[0];
                  cluster_errz [1] = cluster_errz [0];
                  cluster_errxy[1] = cluster_errxy[0];
                  cluster_reconpln[1] = cluster_reconpln[0];
                  grid[1] = grid[0];
                  tmp_id=0;
                  updown = true;
                }
              }else{
                cerr <<"!!! ERROR !!!! BAD PAIR!!!" << endl;
              }
            } 
            cluster_posz [tmp_id] = 0.;
            cluster_posxy[tmp_id] = 0.;
            cluster_errz [tmp_id] = 0.;
            cluster_errxy[tmp_id] = 0.;
            cluster_pe   [tmp_id] = 0.;
            cluster_size[k][l] 
              = type_recon.neighborhits_hitid[i][cluster_id[k][l]].size();
            int reconpln;
            for(int m=0;m<cluster_size[k][l];m++){
              int hitid  = type_recon.neighborhits_hitid[i][cluster_id[k][l]][m];
              int dif    = get_hitdif   (hitid);
              int chip   = get_hitchip  (hitid);
              int chipch = get_hitchipch(hitid);
              int view   = get_hitview  (hitid);
              int pln    = get_hitpln   (hitid);
              int ch     = get_hitch    (hitid);
              double pe  = get_hitpe    (hitid);
              double x   = type_map.x[dif][chip][chipch];
              double y   = type_map.y[dif][chip][chipch];
              double z   = type_map.z[dif][chip][chipch];       
              reconpln   = type_remap.recon_pln[axis][view][pln][ch];
              view_Paired = view;        
              
              if(ch>=C_WMNumXYLayerCh) grid[tmp_id] = true;
              if(view==SideView) cluster_posxy[tmp_id] += y*pe;
              if(view==TopView ) cluster_posxy[tmp_id] += x*pe;
              cluster_posz[tmp_id] += z*pe;
              cluster_pe[tmp_id]   += pe;
              if(ch<C_WMNumXYLayerCh) cluster_errxy[tmp_id] += C_WMScintiWidth/2.;
              else            cluster_errxy[tmp_id]  = C_WMScintiThick/2.;
              if(ch<C_WMNumXYLayerCh) cluster_errz [tmp_id]  = C_WMScintiThick/2.;
              else            cluster_errz [tmp_id]  = C_WMScintiWidth/2.;
            } //m
            cluster_reconpln[tmp_id] = reconpln;
            cluster_posz [tmp_id] /= cluster_pe[tmp_id];
            cluster_posxy[tmp_id] /= cluster_pe[tmp_id];
          }//l
        } //k

        
        int width_reconpln[3];
        width_reconpln[0] = abs(cluster_reconpln[2] - cluster_reconpln[0]);
        width_reconpln[1] = abs(cluster_reconpln[1] - cluster_reconpln[0]);
        width_reconpln[2] = abs(cluster_reconpln[2] - cluster_reconpln[1]);
        short int GRIDtoGRID[2]={0,0};
        for(int k=0;k<2;k++){
          if(axis!=1)break; 
          if(!grid[k] || !grid[k+1]) continue;
          if( fabs(cluster_posz[k]-cluster_posz[k+1])>5.0 && fabs(cluster_posz[k]-cluster_posz[k+1])<C_WMScintiWidth+5.0) GRIDtoGRID[k]=1;
        }

        if(isPaired && 
            ( width_reconpln[0]<=MAX_WIDTH_RECONPLN_NEIGHBORCELLS 
              || (width_reconpln[1]<=MAX_WIDTH_RECONPLN_NEIGHBORCELLS2[GRIDtoGRID[0]] && width_reconpln[2]<=MAX_WIDTH_RECONPLN_NEIGHBORCELLS2[GRIDtoGRID[1]] )
            ) 
          ){
          vector<double> cell_vector[2];
          cell_vector[0].resize(2);
          cell_vector[1].resize(2);
          if(axis==0){
            cell_vector[0][0] = cluster_posz [1]-cluster_posz [0];
            cell_vector[0][1] = cluster_posxy[1]-cluster_posxy[0];
            cell_vector[1][0] = cluster_posz [2]-cluster_posz [1];
            cell_vector[1][1] = cluster_posxy[2]-cluster_posxy[1];
          }
          else if(axis==1){
            cell_vector[0][0] = cluster_posxy[1]-cluster_posxy[0];
            cell_vector[0][1] = cluster_posz [1]-cluster_posz [0];
            cell_vector[1][0] = cluster_posxy[2]-cluster_posxy[1];
            cell_vector[1][1] = cluster_posz [2]-cluster_posz [1];
          }

          double product=0.,norm1=0.,norm2=0.;
          for(int k=0;k<2;k++){
            product += cell_vector[0][k]*cell_vector[1][k];
            norm1   += cell_vector[0][k]*cell_vector[0][k];
            norm2   += cell_vector[1][k]*cell_vector[1][k];
          }
          double cos = product/sqrt(norm1*norm2);
          bool isNeighborPlnGrid = false;
          //for(int k=0;k<2;k++){
          //  if( fabs(cell_vector[k][0]) < C_WMScintiWidth + 0.1 && fabs(cell_vector[k][1]) < C_WMScintiWidth + 0.1 )
          //  {
          //    isNeighborPlnGrid = true;
          //  }
          //}
#ifdef DEBUG_RECON
          cout 
            << "view=" << view_Paired << " cluster_reconpln={" 
            << cluster_reconpln[0] << ","
            << cluster_reconpln[1] << ","
            << cluster_reconpln[2] << "}"
            << "/cos=" << cos
            << " (norm1=" << norm1 << " norm2=" << norm2 << ")"
            << " (LIMIT=" << MAX_ANGLE_NEIGHBORCELLS
            << ") GRID=(" <<GRIDtoGRID[0] << "," << GRIDtoGRID[1] << ")"
            << endl;
#endif
          if((isNeighborPlnGrid && cos>MAX_ANGLE_NEIGHBORCELLS2) || cos>MAX_ANGLE_NEIGHBORCELLS){
            if(!updown){
              type_recon.neighborcell_down_cellid[i][j[0]].push_back(j[1]);
              type_recon.neighborcell_up_cellid  [i][j[1]].push_back(j[0]);
            }
            else{
              type_recon.neighborcell_down_cellid[i][j[1]].push_back(j[0]);
              type_recon.neighborcell_up_cellid  [i][j[0]].push_back(j[1]);
            }
          }
        }
      }//j[1]
    }//j1[0]
  }//i

#ifdef DEBUG_RECON
  if(type_recon.time_cluster_hitid.size()>0){
    cout << "-----------------------------------------" << endl;
    cout << "findNeighborCells is done..." << endl;
    cout << " >>>> neighbor downstream >>>>> " << endl;
    for(int i=0;i<(int)type_recon.time_cluster_hitid.size();i++){
      int clusterid[2];
      for(int j=0;j<num_cell[i];j++){
        clusterid[0] = type_recon.cell_clusterid[i][j][0];
        clusterid[1] = type_recon.cell_clusterid[i][j][1];
        int hitid1 = type_recon.neighborhits_hitid[i][clusterid[0]][0];
        int hitid2 = type_recon.neighborhits_hitid[i][clusterid[1]][0];
        cout << "["
          << "bcid:"  << get_hitbcid(hitid1)
          << ",view:" << get_hitview(hitid1)
          << "|"
          << "pln:"   << get_hitpln(hitid1)
          << ",ch:"   << get_hitch (hitid1)
          << "|"
          << "pln:"   << get_hitpln(hitid2)
          << ",ch:"   << get_hitch (hitid2)
          << "]";
        int num_neighborcell = type_recon.neighborcell_down_cellid[i][j].size();
        for(int k=0;k<num_neighborcell;k++){
          int neighbor_cellid = type_recon.neighborcell_down_cellid[i][j][k];
          clusterid[0] = type_recon.cell_clusterid[i][neighbor_cellid][0];
          clusterid[1] = type_recon.cell_clusterid[i][neighbor_cellid][1];
          int hitid1 = type_recon.neighborhits_hitid[i][clusterid[0]][0];
          int hitid2 = type_recon.neighborhits_hitid[i][clusterid[1]][0];
          cout << "{"
            << "pln:"   << get_hitpln(hitid1)
            << ",ch:"   << get_hitch (hitid1)
            << "|"
            << "pln:"   << get_hitpln(hitid2)
            << ",ch:"   << get_hitch (hitid2)
            << "}";
        }
        cout << endl;
      }
      cout << endl;
    }
    cout << " >>>> neighbor upstream >>>>> " << endl;
    for(int i=0;i<(int)type_recon.time_cluster_hitid.size();i++){
      int clusterid[2];
      for(int j=0;j<num_cell[i];j++){
        clusterid[0] = type_recon.cell_clusterid[i][j][0];
        clusterid[1] = type_recon.cell_clusterid[i][j][1];
        int hitid1 = type_recon.neighborhits_hitid[i][clusterid[0]][0];
        int hitid2 = type_recon.neighborhits_hitid[i][clusterid[1]][0];
        cout << "["
          << "bcid:"  << get_hitbcid(hitid1)
          << ",view:" << get_hitview(hitid1)
          << "|"
          << "pln:"   << get_hitpln(hitid1)
          << ",ch:"   << get_hitch (hitid1)
          << "|"
          << "pln:"   << get_hitpln(hitid2)
          << ",ch:"   << get_hitch (hitid2)
          << "]";
        int num_neighborcell = type_recon.neighborcell_up_cellid[i][j].size();
        for(int k=0;k<num_neighborcell;k++){
          int neighbor_cellid = type_recon.neighborcell_up_cellid[i][j][k];
          clusterid[0] = type_recon.cell_clusterid[i][neighbor_cellid][0];
          clusterid[1] = type_recon.cell_clusterid[i][neighbor_cellid][1];
          int hitid1 = type_recon.neighborhits_hitid[i][clusterid[0]][0];
          int hitid2 = type_recon.neighborhits_hitid[i][clusterid[1]][0];
          cout << "{"
            << "pln:"   << get_hitpln(hitid1)
            << ",ch:"   << get_hitch (hitid1)
            << "|"
            << "pln:"   << get_hitpln(hitid2)
            << ",ch:"   << get_hitch (hitid2)
            << "}";
        }
        cout << endl;
      }
      cout << endl;
    }
  }
#endif
  return true;
};

//********************************************************************
bool wgRecon::defineCellState(int axis)
{
  type_recon.cell_state.resize(type_recon.time_cluster_hitid.size());
  for(int i=0;i<(int)type_recon.time_cluster_hitid.size();i++){
    vector<int> cell_state,cell_state_next;
    cell_state     .resize(num_cell[i]);
    cell_state_next.resize(num_cell[i]);
    for(int j=0;j<num_cell[i];j++){ cell_state_next[j]=0; }
    for(int step=0;step<C_WMNumPln*3;step++){
      for(int j=0;j<num_cell[i];j++){ cell_state[j]=cell_state_next[j]; }
      for(int j=0;j<num_cell[i];j++){
        int num_neighborcell = type_recon.neighborcell_down_cellid[i][j].size();
        for(int k=0;k<num_neighborcell;k++){
          int neighbor_cellid = type_recon.neighborcell_down_cellid[i][j][k];
          if(cell_state[j]==cell_state[neighbor_cellid]){
            cell_state_next[neighbor_cellid] = cell_state[neighbor_cellid]+1;
          }
        }
      }
    }
    type_recon.cell_state[i].resize(num_cell[i]);
    for(int j=0;j<num_cell[i];j++){
      type_recon.cell_state[i][j] = cell_state[j];
    }
#ifdef DEBUG_RECON
    cout << "=========================================" << endl;
    cout << "defineCellState is done..." << endl;
    int clusterid[2];
    for(int j=0;j<num_cell[i];j++){
      clusterid[0] = type_recon.cell_clusterid[i][j][0];
      clusterid[1] = type_recon.cell_clusterid[i][j][1];
      int hitid1 = type_recon.neighborhits_hitid[i][clusterid[0]][0];
      int hitid2 = type_recon.neighborhits_hitid[i][clusterid[1]][0];
      cout << "["
        << "bcid:"  << get_hitbcid(hitid1)
        << "/view:" << get_hitview(hitid1)
        << ",pln:"  << get_hitpln (hitid1)
        << ",ch:"   << get_hitch  (hitid1)
        << "/"
        << "view:"  << get_hitview(hitid2)
        << ",pln:"  << get_hitpln (hitid2)
        << ",ch:"   << get_hitch  (hitid2)
        << "]";
      cout << "cell_state=" << type_recon.cell_state[i][j] << endl;
    }
    cout << endl;
#endif
  }//i
  return true;
};

//********************************************************************
bool comp_cellstate(const vector<int> &v1, const vector<int> &v2){
  if(v1.size()<1||v2.size()<1){
    cerr << "The vector does not have enough length to sort." << endl;
    return false;
  }
  else{
    if(v1[0] > v2[0]) return true;
    else return false;
  }
}

//********************************************************************
bool wgRecon::findTrack(int axis)
{
  bool buf_recon_full = false;

  for(int i=0;i<(int)type_recon.time_cluster_hitid.size();i++){
    if(buf_recon_full){ return false;}
    //
    // extract neighbor cell pair with 1 diff. in state
    //
    for(int j=0;j<num_cell[i];j++){
      int cellstate  = type_recon.cell_state[i][j];
      int cellid     = j;
      int num_cellup = type_recon.neighborcell_up_cellid[i][cellid].size();
      for(int k=num_cellup-1;k>=0;k--){  
        int cellup_id    = type_recon.neighborcell_up_cellid[i][cellid][k];
        int cellup_state = type_recon.cell_state[i][cellup_id];
        if(abs(cellstate-cellup_state)!=1){
          type_recon.neighborcell_up_cellid[i][cellid]
            .erase(type_recon.neighborcell_up_cellid[i][cellid].begin()+k);
        }
      }
    }

#ifdef DEBUG_RECON
    cout << "-----------------------" << endl;
    cout << "Extract cell pairs only with one difference in their states." << endl;
    for(int j=0;j<num_cell[i];j++){
      int cellstate  = type_recon.cell_state[i][j];
      int cellid     = j;
      int num_cellup = type_recon.neighborcell_up_cellid[i][cellid].size();
      int clusterid[3];
      for(int k=num_cellup-1;k>=0;k--){  
        int cellup_id    = type_recon.neighborcell_up_cellid[i][cellid][k];
        int cellup_state = type_recon.cell_state[i][cellup_id];
        clusterid[0] = type_recon.cell_clusterid[i][cellid   ][1];
        clusterid[1] = type_recon.cell_clusterid[i][cellid   ][0];
        clusterid[2] = type_recon.cell_clusterid[i][cellup_id][0];
        int hitid1 = type_recon.neighborhits_hitid[i][clusterid[0]][0];
        int hitid2 = type_recon.neighborhits_hitid[i][clusterid[1]][0];
        int hitid3 = type_recon.neighborhits_hitid[i][clusterid[2]][0];
        cout << "["
          << "state={" << cellstate << "," << cellup_state << "}";
        cout << "["
          << "bcid:"  << get_hitbcid(hitid1)
          << "|"
          << "view:"  << get_hitview(hitid1)
          << ",pln:"  << get_hitpln (hitid1)
          << ",ch:"   << get_hitch  (hitid1)
          << "|"
          << "view:"  << get_hitview(hitid2)
          << ",pln:"  << get_hitpln (hitid2)
          << ",ch:"   << get_hitch  (hitid2)
          << "|"
          << "view:"  << get_hitview(hitid3)
          << ",pln:"  << get_hitpln (hitid3)
          << ",ch:"   << get_hitch  (hitid3)
          << "]";
        cout << endl;
      }
    }
#endif

    //sort cells in descending order in its state value
    //
    vector<vector<int> > cell_with_state;
    cell_with_state.resize(num_cell[i]);
    for(int j=0;j<num_cell[i];j++){
      int cellstate = type_recon.cell_state[i][j];
      cell_with_state[j].resize(2);
      cell_with_state[j][0] = cellstate;
      cell_with_state[j][1] = j;
    }
    sort(cell_with_state.begin(),cell_with_state.end(),comp_cellstate);
    //
    // push sequent cells from one with the largest state value to 0.
    //
    vector<vector<int> > sequentcells_cellid;
    vector<int> tmp;
    vector<bool> upstream;
    int n_path = 0;
    for(int j=0;j<num_cell[i];j++){
      int cellid_down = cell_with_state[j][1];
      int num_celldown = type_recon.neighborcell_down_cellid[i][cellid_down].size();
      if(num_celldown!=0){continue;}
      else{
        n_path++;
        upstream.push_back(false);
        sequentcells_cellid.resize(n_path);
        sequentcells_cellid[n_path-1].push_back(cellid_down);
        while(true){
          bool upstream_all = true;
          for(int k=0;k<n_path;k++){upstream_all = upstream_all && upstream[k];}
          if(upstream_all){ break;}

          for(int k=0;k<n_path;k++){
            if(upstream[k]){continue;}
            else{
              cellid_down = (int)sequentcells_cellid[k].back();
              int num_cellup = type_recon.neighborcell_up_cellid[i][cellid_down].size();
              if(num_cellup==0){upstream[k]=true;}
              else if(num_cellup==1){
                int cellid_up = type_recon.neighborcell_up_cellid[i][cellid_down][0];
                sequentcells_cellid[k].push_back(cellid_up);
              }
              else{
                tmp.clear();
                int num_cell_in_path = (int)sequentcells_cellid[k].size();
                tmp.resize(num_cell_in_path);
                for(int m=0;m<num_cell_in_path;m++){
                  tmp[m] = sequentcells_cellid[k][m];
                }
                vector<vector<int> >::iterator it1 = sequentcells_cellid.begin();
                vector<bool>        ::iterator it2 = upstream           .begin();
                for(int l=0;l<k;l++){
                  ++it1;
                  ++it2;
                }
                for(int l=0;l<num_cellup-1;l++){
                  it1 = sequentcells_cellid.insert(it1,tmp);
                  it2 = upstream           .insert(it2,false);
                }
                for(int l=0;l<num_cellup;l++){
                  int cellid_up = type_recon.neighborcell_up_cellid[i][cellid_down][l];
                  sequentcells_cellid[k+l].push_back(cellid_up);
                }
                n_path += num_cellup-1;
                k      += num_cellup-1;
              }
            }
          }
        }
      }
    }
    int num_path = sequentcells_cellid.size();

#ifdef DEBUG_RECON
    cout << "-----------------------" << endl;
    cout << "Pushed sequent cells..." << endl;
    for(int j=0;j<num_path;j++){
      int num_cells_path = sequentcells_cellid[j].size();
      int clusterid[2];
      for(int k=0;k<num_cells_path;k++){
        int cellid = sequentcells_cellid[j][k];
        clusterid[0] = type_recon.cell_clusterid[i][cellid][0];
        clusterid[1] = type_recon.cell_clusterid[i][cellid][1];
        int num_hitid1 = type_recon.neighborhits_hitid[i][clusterid[0]].size();
        int num_hitid2 = type_recon.neighborhits_hitid[i][clusterid[1]].size();
        if(k==0){
          int hitid2 = type_recon.neighborhits_hitid[i][clusterid[1]][0];
          cout << "["
            << "bcid:"  << get_hitbcid(hitid2)
            << ",view:" << get_hitview(hitid2)
            << "]";
        }
        cout << "[";
        for(int l2=0;l2<num_hitid2;l2++){
          if(l2>0) cout << "/";
          int hitid2 = type_recon.neighborhits_hitid[i][clusterid[1]][l2];
          cout
            << "pln:"   << get_hitpln(hitid2)
            << ",ch:"   << get_hitch (hitid2);
        }
        cout << "|";
        for(int l1=0;l1<num_hitid1;l1++){
          if(l1>0) cout << "/";
          int hitid1 = type_recon.neighborhits_hitid[i][clusterid[0]][l1];
          cout 
            << "pln:"   << get_hitpln(hitid1)
            << ",ch:"   << get_hitch (hitid1);
        }
        cout << "]";
      }
      cout << endl;
    }
#endif
    
    // Linear fitting of clusters in the sequent cells
    //
    //vector<vector<int> > recon_hitid;
    //vector<double> recon_slope, recon_intercept;
    vector<unsigned int> tmp_hitid;
    vector<bool> used_clusterid;
    int num_cluster = type_recon.neighborhits_hitid[i].size();
    used_clusterid.resize(num_cluster);
    for(int j=0;j<num_cluster;j++){used_clusterid[j]=false;}

    num_path = sequentcells_cellid.size();
    for(int j=0;j<num_path;j++){
      tmp_hitid.clear();
      if(buf_recon_full){
#ifdef DEBUG_RECON
        cout << "buf_recon_full=" << buf_recon_full << endl;
#endif
        return false;
      }
      int num_cells_path = sequentcells_cellid[j].size();

      int num_not_shared_cluster = num_cells_path+1;
      for(int k=0;k<num_cells_path+1;k++){
        int cellid, clusterid;
        if(k==0){
          cellid    = sequentcells_cellid[j][k];
          clusterid = type_recon.cell_clusterid[i][cellid][1]; //down
        }
        else{
          cellid    = sequentcells_cellid[j][k-1];
          clusterid = type_recon.cell_clusterid[i][cellid][0]; //up
        }
        if(used_clusterid[clusterid]){
          num_not_shared_cluster--;
        }
      }
      if(num_not_shared_cluster<(num_cells_path+1)*0.5){ continue; } //goto next path

     
      vector<double> cluster_posz,cluster_posxy,cluster_errz,cluster_errxy,cluster_pe;
      vector<int> clusterid_path;
      cluster_posz  .resize(num_cells_path+1);
      cluster_posxy .resize(num_cells_path+1);
      cluster_errz  .resize(num_cells_path+1);
      cluster_errxy .resize(num_cells_path+1);
      cluster_pe    .resize(num_cells_path+1);
      clusterid_path.resize(num_cells_path+1);
      
      for(int k=0;k<num_cells_path+1;k++){
        int i_track = type_recon.num_recon;
        if(buf_recon_full){
#ifdef DEBUG_RECON
          cout << "buf_recon_full=" << buf_recon_full << endl;
#endif
          return false;
        }

        int cellid, clusterid;
        if(k==0){
          cellid    = sequentcells_cellid[j][k];
          clusterid = type_recon.cell_clusterid[i][cellid][1]; //down
        }
        else{
          cellid    = sequentcells_cellid[j][k-1];
          clusterid = type_recon.cell_clusterid[i][cellid][0]; //up
        }

        clusterid_path[k] = clusterid;
        int cluster_size = type_recon.neighborhits_hitid[i][clusterid].size();
        for(int l=0;l<cluster_size;l++){
          int hitid  = type_recon.neighborhits_hitid[i][clusterid][l];
          //if(hitid!=0) 
          tmp_hitid.push_back((unsigned int)hitid);
          int dif    = get_hitdif   (hitid);
          int chip   = get_hitchip  (hitid);
          int chipch = get_hitchipch(hitid);
          int view   = get_hitview  (hitid);
          int pln    = get_hitpln   (hitid);
          int ch     = get_hitch    (hitid);
          double pe  = get_hitpe    (hitid);
          double x   = type_map.x[dif][chip][chipch];
          double y   = type_map.y[dif][chip][chipch];
          double z   = type_map.z[dif][chip][chipch];
          if(view==SideView) cluster_posxy[k] += pe*y;
          if(view==TopView ) cluster_posxy[k] += pe*x;
          cluster_posz[k] += pe*z;
          cluster_pe  [k] += pe;
          if(ch<C_WMNumXYLayerCh) cluster_errxy[k] += C_WMScintiWidth/2.;
          else            cluster_errxy[k]  = C_WMScintiThick/2.;
          if(ch<C_WMNumXYLayerCh) cluster_errz [k]  = C_WMScintiThick/2.;
          else            cluster_errz [k]  = C_WMScintiWidth/2.;
        } //l
        cluster_posz [k] /= cluster_pe[k];
        cluster_posxy[k] /= cluster_pe[k];
        if(k>1){
          vector<double> z,xy,errz,errxy;
          z    .resize(k+1);
          xy   .resize(k+1);
          errz .resize(k+1);
          errxy.resize(k+1);
          for(int l=0;l<k+1;l++){
            if(axis==0){
              z    [k-l] = cluster_posz [l];
              xy   [k-l] = cluster_posxy[l];
              errz [k-l] = cluster_errz [l];
              errxy[k-l] = cluster_errxy[l];
            }
            else if(axis==1){
              z    [k-l] = cluster_posxy[l];
              xy   [k-l] = cluster_posz [l];
              errz [k-l] = cluster_errxy[l];
              errxy[k-l] = cluster_errz [l];
            }
          }
          TGraphAsymmErrors *graph = new TGraphAsymmErrors(k+1,
              &z[0],&xy[0],&errz[0],&errz[0],&errxy[0],&errxy[0]);
          TF1 *f = new TF1("f","pol1");          
          double ini_slope,ini_intercept; 
          if(z[k]-z[0]==0.){
            ini_slope = 1000000.;
            ini_intercept = 0.;
          }else{
            ini_slope = (xy[k]-xy[0])/(z[k]-z[0]);
            ini_intercept = xy[0]-z[0]*ini_slope;
          }
          f->SetParameters(ini_intercept,ini_slope);
          graph->Fit("f","Q");
          double chi2 = GetDispersion(f->GetParameter(0),f->GetParameter(1),z,xy);
#ifdef DEBUG_RECON
          cout 
            << "chi2=" << chi2 
            << "/limit=" << LIMIT_CHI2_FIND_TRACK1+LIMIT_CHI2_FIND_TRACK2*(k-2)
            << "/k=" << k
            << "/num_cells_path="<<num_cells_path
            << endl; 
#endif
          if(chi2>LIMIT_CHI2_FIND_TRACK1+LIMIT_CHI2_FIND_TRACK2*(k-2)){
            delete f;
            delete graph;
            break; //go to next path
          }
          else if(k==num_cells_path){ //if find track, fill hit information.
            double intercept = f->GetParameter(0);
            double slope     = f->GetParameter(1);

            int num_recon_hits = tmp_hitid.size();
#ifdef DEBUG_RECON
            cout 
              << "num_recon_hits=" << num_recon_hits
              << "/MAX_NUM_TRACKHIT=" << MAX_NUM_TRACKHIT
              << endl;
#endif
            if(num_recon_hits>MAX_NUM_TRACKHIT){continue;}

#ifdef DEBUG_RECON
            cout << "Filling recon hits.." << endl;
#endif

            type_recon.num_recon_hits[i_track] = num_recon_hits;
            double start_z,stop_z,start_xy,stop_xy,total_pe=0.,mean_dedx=0.,mean_time=0.,pathlength=0.;
            int bcid,view;
            int start_pln, stop_pln, start_ch,stop_ch;
            int tmp_numberhit=0;
            for(int kk=0;kk<num_cells_path+1;kk++){
              if(kk==0){
                cellid    = sequentcells_cellid[j][kk];
                clusterid = type_recon.cell_clusterid[i][cellid][1]; //down
              }else{
                cellid    = sequentcells_cellid[j][kk-1];
                clusterid = type_recon.cell_clusterid[i][cellid][0]; //up
              }

              clusterid_path[kk] = clusterid;
              cluster_size = type_recon.neighborhits_hitid[i][clusterid].size();
              double cluster_z  = 0.0;
              double cluster_xy = 0.0;
              double cluster_pe = 0.0;
              int pln,ch;
              for(int l=0;l<cluster_size;l++){
                int hitid  = type_recon.neighborhits_hitid[i][clusterid][l];
                int dif    = get_hitdif   (hitid);
                int chip   = get_hitchip  (hitid);
                int chipch = get_hitchipch(hitid);
                    pln    = get_hitpln   (hitid);
                    ch     = get_hitch    (hitid);
                double pe  = get_hitpe    (hitid);
                double time= get_hittime  (hitid);
                double x   = type_map.x[dif][chip][chipch];
                double y   = type_map.y[dif][chip][chipch];
                double z   = type_map.z[dif][chip][chipch];
                if(kk==0 && l==0){
                  bcid = type_hit.clustered_bcid[i];
                  view = get_hitview (hitid);
                }
                if(view==SideView)  cluster_xy += pe*y;
                if(view==TopView)  cluster_xy  += pe*x;
                cluster_z  += pe*z;
                cluster_pe += pe;
                total_pe  += pe;
                mean_time += time;
                type_recon.recon_hits_hitid  [i_track][tmp_numberhit] = hitid;
                tmp_numberhit++;
              } //l
          
              for(int l=0;l<cluster_size;l++){
                int hitid  = type_recon.neighborhits_hitid[i][clusterid][l];
                type_hit.hit_cluster_pe  [hitid] = cluster_pe;
              }

              cluster_z  /= cluster_pe;
              cluster_xy /= cluster_pe;

              if(kk==0){
                if(axis==0){
                  start_z   = cluster_z;
                  start_xy  = slope * start_z + intercept;
                }else if(axis==1){
                  start_xy  = cluster_xy;
                  start_z   = slope * start_xy + intercept;
                }
                start_pln = pln;
                start_ch  = ch;
              }

              if(kk==num_cells_path){
                if(axis==0){
                  stop_z   = cluster_z;
                  stop_xy  = slope * stop_z + intercept;
                }else if(axis==1){
                  stop_xy  = cluster_xy;
                  stop_z   = slope * stop_xy + intercept;
                }
                stop_pln = pln;
                stop_ch  = ch;
              }
            }//kk

            if(tmp_numberhit!=type_recon.num_recon_hits[i_track]){
              cout << "ERROR! Fail to count hit!" << endl;
            }

            mean_time /=(double)type_recon.num_recon_hits[i_track];

            for(int l=0;l<type_recon.num_recon_hits[i_track];l++){
              type_recon.recon_hits_hitid[i_track][l] = tmp_hitid[l];
#ifdef DEBUG_RECON
              if(l==0) cout << "[";
              cout 
                << " pln=" << get_hitpln(tmp_hitid[l])
                << " ch="  << get_hitch (tmp_hitid[l]);
              if(l!=type_recon.num_recon_hits[i_track]-1) cout << "|";
              else cout << "]" << endl;
#endif
            }
            if(axis==0){
              slope=slope;
              intercept=intercept;
            }else if(axis==1){
              if(slope!=0.){
                intercept=-intercept/slope;
                slope=1.0/slope;
              }else{
                slope=1000000.;
                intercept=0.0;
              }
            }
            
            if(start_z>stop_z){
              double temp_xyz = start_z;
              start_z = stop_z;
              stop_z  = temp_xyz;
              temp_xyz  = start_xy;
              start_xy  = stop_xy;
              stop_xy   = temp_xyz;
              int temp_pln = start_pln;
              start_pln = stop_pln;
              stop_pln  = temp_pln;
              temp_pln  = start_ch;
              start_ch  = stop_ch;
              stop_ch   = temp_pln;
            }
            double cos_theta = slope/sqrt(1.+slope*slope);            
            double length = sqrt(pow(start_z-stop_z,2.)+pow(start_xy-stop_xy,2.));  

            type_recon.recon_bcid_id    [i_track] = i         ;
            type_recon.recon_start_z    [i_track] = start_z   ;
            type_recon.recon_stop_z     [i_track] = stop_z    ;
            type_recon.recon_start_pln  [i_track] = start_pln ;
            type_recon.recon_stop_pln   [i_track] = stop_pln  ;
            type_recon.recon_start_ch   [i_track] = start_ch  ;
            type_recon.recon_stop_ch    [i_track] = stop_ch   ;
            type_recon.recon_start_xy   [i_track] = start_xy  ;
            type_recon.recon_stop_xy    [i_track] = stop_xy   ;
            type_recon.recon_slope      [i_track] = slope     ;
            type_recon.recon_intercept  [i_track] = intercept ;
            type_recon.recon_pathlength [i_track] = pathlength;
            type_recon.recon_total_pe   [i_track] = total_pe  ;
            type_recon.recon_mean_dedx  [i_track] = mean_dedx ;
            type_recon.recon_mean_time  [i_track] = mean_time ;
            type_recon.recon_view       [i_track] = view      ;
            type_recon.recon_bcid       [i_track] = bcid      ;
            type_recon.recon_cos        [i_track] = cos_theta ;
            type_recon.recon_len        [i_track] = length    ;
            type_recon.recon_chi2       [i_track] = chi2    ;
            type_recon.num_recon++;
#ifdef DEBUG_RECON
            cout 
              << " start_z="    <<  start_z   
              << " stop_z="     <<  stop_z    
              << " start_pln="  <<  start_pln
              << " stop_pln="   <<  stop_pln    
              << " slope="      <<  slope     
              << " intercept="  <<  intercept 
              << " pathlength=" <<  pathlength
              << " total_pe ="  <<  total_pe  
              << " mean_dedx="  <<  mean_dedx 
              << " mean_time="  <<  mean_time 
              << " view="       <<  view      
              << " bcid="       <<  bcid      
              << " cos="        <<  cos_theta     
              << " len="        <<  length
              << endl;
#endif
            for(int l=0;l<(int)clusterid_path.size();l++){
              used_clusterid[clusterid_path[l]] = true;
            }
            if(type_recon.num_recon>=MAX_NUM_TRACK){
              buf_recon_full = true;
            }
          }// end fill track
          delete f;
          delete graph;
        }
      }//k
    }//j
  }//i
#ifdef DEBUG_RECON
  cout << "-----------------------" << endl;
  cout << "-----------------------" << endl;
  cout << "track is found..." << endl;
  for(int j=0;j<type_recon.num_recon;j++){
    for(int k=0;k<type_recon.num_recon_hits[j];k++){
      int hitid = type_recon.recon_hits_hitid[j][k];
      if(k==0) 
        cout << "["
          << "bcid:"  << get_hitbcid(hitid)
          << ",view:" << get_hitview(hitid)
          << "]||";
      cout << "{"
        << "pln:"   << get_hitpln(hitid)
        << ",ch:"   << get_hitch (hitid)
        << "}";
    }
    cout << "||["
      << "slope:"      << type_recon.recon_slope[j]
      << ",intercept:" << type_recon.recon_intercept[j]
      << ",length:" << type_recon.recon_len[j]
      << "]" << endl;
  }
  cout << endl;
#endif 
  return true;
};

//********************************************************************
bool wgRecon::check_Veto_SideEscape(int axis)
{
  for(int i=0;i<type_recon.num_recon;i++){
    type_recon.recon_veto      [i] = 0;
    type_recon.recon_sideescape[i] = 0;
    // is front veto
    double slope = type_recon.recon_slope[i];
    int view,start_pln,start_ch,stop_pln,stop_ch,reconpln,reconch;
    //hitid = type_recon.recon_hits_hitid[i][type_recon.num_recon_hits[i]];
    view =  type_recon.recon_view[i];
    start_pln = type_recon.recon_start_pln[i];
    start_ch  = type_recon.recon_start_ch[i];
    if( view == SideView ){
      if(start_pln<=NUM_UP_VETO_PLN_SIDE || start_pln>=NUM_DOWN_VETO_PLN_SIDE){
        type_recon.recon_veto[i] = 1;
      }

      if(start_ch<C_WMNumXYLayerCh){
        if(start_ch<=NUM_BOTTOM_VETO_CH_SIDE   
            || start_ch>=NUM_TOP_VETO_CH_SIDE){   
          type_recon.recon_veto[i] = 1;
        }
      }else if(start_ch<C_WMNumXYLayerCh+C_WMNumGridCh){
        if(start_ch-C_WMNumXYLayerCh<=(NUM_BOTTOM_VETO_CH_SIDE-1)/2   
            || start_ch-C_WMNumXYLayerCh>=NUM_TOP_VETO_CH_SIDE/2){   
          type_recon.recon_veto[i] = 1;
        }
      }else{
        if(start_ch-C_WMNumXYLayerCh-C_WMNumGridCh<=(NUM_BOTTOM_VETO_CH_SIDE-1)/2   
            || start_ch-C_WMNumXYLayerCh-C_WMNumGridCh>=NUM_TOP_VETO_CH_SIDE/2){   
          type_recon.recon_veto[i] = 1;
        }
      }
    }else if( view == TopView ){
      if(start_pln<=NUM_UP_VETO_PLN_TOP || start_pln>=NUM_DOWN_VETO_PLN_TOP){
        type_recon.recon_veto[i] = 1;
      }
      
      if(start_ch<C_WMNumXYLayerCh){
        if(start_ch<=NUM_BOTTOM_VETO_CH_TOP   
            || start_ch>=NUM_TOP_VETO_CH_TOP){   
          type_recon.recon_veto[i] = 1;
        }
      }else if(start_ch<C_WMNumXYLayerCh+C_WMNumGridCh){
        if(start_ch-C_WMNumXYLayerCh<=(NUM_BOTTOM_VETO_CH_TOP-1)/2   
            || start_ch-C_WMNumXYLayerCh>=NUM_TOP_VETO_CH_TOP/2){   
          type_recon.recon_veto[i] = 1;
        }
      }else{
        if(start_ch-C_WMNumXYLayerCh-C_WMNumGridCh<=(NUM_BOTTOM_VETO_CH_TOP-1)/2   
            || start_ch-C_WMNumXYLayerCh-C_WMNumGridCh>=NUM_TOP_VETO_CH_TOP/2){   
          type_recon.recon_veto[i] = 1;
        }
      }
    }
      
    // is sideescape
    stop_pln = type_recon.recon_stop_pln[i];
    stop_ch  = type_recon.recon_stop_ch[i];
    if( view == SideView ){

      if(stop_ch<C_WMNumXYLayerCh){
        if(stop_ch<=NUM_BOTTOM_VETO_CH_SIDE   
            || stop_ch>=NUM_TOP_VETO_CH_SIDE){   
          type_recon.recon_sideescape[i] = 1;
        }
      }else if(stop_ch<C_WMNumXYLayerCh+C_WMNumGridCh){
        if(stop_ch-C_WMNumXYLayerCh<=(NUM_BOTTOM_VETO_CH_SIDE-1)/2   
            || stop_ch-C_WMNumXYLayerCh>=NUM_TOP_VETO_CH_SIDE/2){   
          type_recon.recon_sideescape[i] = 1;
        }
      }else{
        if(stop_ch-C_WMNumXYLayerCh-C_WMNumGridCh<=(NUM_BOTTOM_VETO_CH_SIDE-1)/2   
            || stop_ch-C_WMNumXYLayerCh-C_WMNumGridCh>=NUM_TOP_VETO_CH_SIDE/2){   
          type_recon.recon_sideescape[i] = 1;
        }
      }
    }else if( view == TopView ){
      
      if(stop_ch<C_WMNumXYLayerCh){
        if(stop_ch<=NUM_BOTTOM_VETO_CH_TOP   
            || stop_ch>=NUM_TOP_VETO_CH_TOP){   
          type_recon.recon_sideescape[i] = 1;
        }
      }else if(stop_ch<C_WMNumXYLayerCh+C_WMNumGridCh){
        if(stop_ch-C_WMNumXYLayerCh<=(NUM_BOTTOM_VETO_CH_TOP-1)/2   
            || stop_ch-C_WMNumXYLayerCh>=NUM_TOP_VETO_CH_TOP/2){   
          type_recon.recon_sideescape[i] = 1;
        }
      }else{
        if(stop_ch-C_WMNumXYLayerCh-C_WMNumGridCh<=(NUM_BOTTOM_VETO_CH_TOP-1)/2   
            || stop_ch-C_WMNumXYLayerCh-C_WMNumGridCh>=NUM_TOP_VETO_CH_TOP/2){   
          type_recon.recon_sideescape[i] = 1;
        }
      }
    }
  }

#ifdef DEBUG_RECON 
    cout <<"====================================" << endl;
    cout <<"check veto sideescape "<< endl;
#endif

  return true;
};

//********************************************************************
bool wgRecon::Tracking_alongZ()
{
#ifdef DEBUG_RECON 
  cout << " ******************************************************" << endl;
  cout << " *********       *********************        *********" << endl;
  cout << " *********       *********************        *********" << endl;
  cout << "                    TRACKING ALONGZ                   " << endl;
  cout << " *********       *********************        *********" << endl;
  cout << " *********       *********************        *********" << endl;
  cout << " ******************************************************" << endl;
#endif
  type_recon.Clear_ReconVector();
  if(!findNeighborHits     (0)) return false;
  if(!findClusterPair      (0)) return false;
  if(!findNeighborCells    (0)) return false;
  if(!defineCellState      (0)) return false;
  if(!findTrack            (0)) return false;
  if(!check_Veto_SideEscape(0)) return false;
  return true;
};

//********************************************************************
bool wgRecon::Tracking_alongXY()
{
#ifdef DEBUG_RECON 
  cout << " ******************************************************" << endl;
  cout << " ****************                     *****************" << endl;
  cout << " ****************                     *****************" << endl;
  cout << "                    TRACKING ALONGXY                   " << endl;
  cout << " ****************                     *****************" << endl;
  cout << " ****************                     *****************" << endl;
  cout << " ******************************************************" << endl;
#endif
  type_recon.Clear_ReconVector();
  if(!findNeighborHits     (1)) return false;
  if(!findClusterPair      (1)) return false;
  if(!findNeighborCells    (1)) return false;
  if(!defineCellState      (1)) return false;
  if(!findTrack            (1)) return false;
  if(!check_Veto_SideEscape(1)) return false;
  return true;
};

//********************************************************************
bool comp_rank(const vector<double> &v1, const vector<double> &v2){
  if(v1.size()<6||v2.size()<6){
    cerr << "The vector does not have enough length to sort." << endl;
    cout << "v1=";
    for(int i=0;i<(int)v1.size();i++){
      cout << " " << v1[i];
    }
    cout << endl;
    cout << "v2=";
    for(int i=0;i<(int)v2.size();i++){
      cout << " " << v2[i];
    }
    cout << endl;
    return false;
  }
  else{
    if(v1[0] < v2[0]) return true;
    else if(v1[0]==v2[0]){
      //rank,id,len,pe,chi2,numhit
      if(v2[4]-v2[4]<5.0)return true;
      if(v1[3]>1.2*v2[3])return true;
      else if(v1[2]>1.2*v2[2]) return true; 
      else if(v1[5]>v2[5]) return true; 
      //else if(v2[4]-v1[4]>2.5) return true; 
      else return false;
    }else return false;
  }
  return false;
}
//********************************************************************
bool wgRecon::eraceDuplicateTrack()
{
#ifdef DEBUG_RECON
  cout << "============================"<<endl;
  cout << " eraceDuplicateTrack " << endl;
#endif
  //erace duplicate tracks.
  Recon_t tmp_recon;
  tmp_recon.Clear();
  vector<int> list_recon_cluster;       //cluster for same view and same bcid_id

  for(int i=0;i<(int)type_recon.time_cluster_hitid.size();i++){
    for(int view=0;view<C_WMNumView;view++){ 
      list_recon_cluster.clear();
      for(int k=0;k<type_recon.num_recon;k++){
        if( type_recon.recon_bcid_id[k] == i && type_recon.recon_view[k]==view ){
          list_recon_cluster.push_back(k);
        }
      }//k
#ifdef DEBUG_RECON
      cout << "-------------"<<endl;
      cout << "view="<<view<<endl;
      for(int k1=0;k1<(int)list_recon_cluster.size();k1++){
        int id1=list_recon_cluster[k1];
        cout <<"{" 
          << type_recon.recon_len[id1]<<","
          << type_recon.num_recon_hits[id1]<<","
          << type_recon.recon_chi2[id1]<<","
          << type_recon.recon_total_pe[id1]<<","
          << id1 <<","
          << "}" 
          <<endl;
      } 
#endif

      if( list_recon_cluster.size() ==0 ){
        continue;
      }else if( list_recon_cluster.size() ==1 ){
        int id1 = list_recon_cluster[0];
        tmp_recon.num_recon_hits   [tmp_recon.num_recon] = type_recon.num_recon_hits   [id1] ;
        tmp_recon.recon_bcid_id    [tmp_recon.num_recon] = type_recon.recon_bcid_id    [id1] ;
        tmp_recon.recon_start_z    [tmp_recon.num_recon] = type_recon.recon_start_z    [id1] ;
        tmp_recon.recon_stop_z     [tmp_recon.num_recon] = type_recon.recon_stop_z     [id1] ;
        tmp_recon.recon_start_pln  [tmp_recon.num_recon] = type_recon.recon_start_pln  [id1] ;
        tmp_recon.recon_stop_pln   [tmp_recon.num_recon] = type_recon.recon_stop_pln   [id1] ;
        tmp_recon.recon_start_ch   [tmp_recon.num_recon] = type_recon.recon_start_ch   [id1] ;
        tmp_recon.recon_stop_ch    [tmp_recon.num_recon] = type_recon.recon_stop_ch    [id1] ;
        tmp_recon.recon_start_xy   [tmp_recon.num_recon] = type_recon.recon_start_xy   [id1] ;
        tmp_recon.recon_stop_xy    [tmp_recon.num_recon] = type_recon.recon_stop_xy    [id1] ;
        tmp_recon.recon_slope      [tmp_recon.num_recon] = type_recon.recon_slope      [id1] ;
        tmp_recon.recon_intercept  [tmp_recon.num_recon] = type_recon.recon_intercept  [id1] ;
        tmp_recon.recon_pathlength [tmp_recon.num_recon] = type_recon.recon_pathlength [id1] ;
        tmp_recon.recon_total_pe   [tmp_recon.num_recon] = type_recon.recon_total_pe   [id1] ;
        tmp_recon.recon_mean_dedx  [tmp_recon.num_recon] = type_recon.recon_mean_dedx  [id1] ;
        tmp_recon.recon_mean_time  [tmp_recon.num_recon] = type_recon.recon_mean_time  [id1] ;
        tmp_recon.recon_view       [tmp_recon.num_recon] = type_recon.recon_view       [id1] ;
        tmp_recon.recon_bcid       [tmp_recon.num_recon] = type_recon.recon_bcid       [id1] ;
        tmp_recon.recon_cos        [tmp_recon.num_recon] = type_recon.recon_cos        [id1] ;
        tmp_recon.recon_len        [tmp_recon.num_recon] = type_recon.recon_len        [id1] ;
        tmp_recon.recon_chi2       [tmp_recon.num_recon] = type_recon.recon_chi2       [id1] ; 
        tmp_recon.recon_veto       [tmp_recon.num_recon] = type_recon.recon_veto       [id1] ;
        tmp_recon.recon_sideescape [tmp_recon.num_recon] = type_recon.recon_sideescape [id1] ; 
        for(int l=0;l<type_recon.num_recon_hits[id1];l++){
          tmp_recon.recon_hits_hitid[tmp_recon.num_recon][l] = type_recon.recon_hits_hitid[id1][l];
        }//l
        tmp_recon.num_recon++;
        continue;
      }

      // i. rank recon.
      //     1st selecton : num_recon_hits
      //       -> if the difference of hits is smaller than 2, then go to 3rd selection. 
      //     2nd selecton : length 
      //       -> if the difference of length is smaller than 50mm(2*scinti width), then go to 2nd selection. 
      //     3rd selecton : chi2
      //       -> if the difference of ch2 is smaller than 2mm2(2*scinti width), then go to 4th selection. 
      //     4th selecton : total_pe
      //vector<vector<double> > rank_recon(0);
      int num_recon_cluster = list_recon_cluster.size();
      vector<vector<int> > rank_recon(num_recon_cluster);
      for(int k1=0;k1<num_recon_cluster;k1++){
        int rank = 0;
        int id1=list_recon_cluster[k1];
        for(int k2=0;k2<num_recon_cluster;k2++){
          int id2=list_recon_cluster[k2];
          if(id1==id2) continue;
          // 1st selection : number of hits
          if((double)type_recon.num_recon_hits[id1]<0.9*(double)type_recon.num_recon_hits[id2]){
            rank++;
          }else if((double)type_recon.num_recon_hits[id1]>1.1*(double)type_recon.num_recon_hits[id2]){
            rank=rank;
          }
          else{
            //2nd selection : length of recon
            if(type_recon.recon_len[id1]<type_recon.recon_len[id2]*0.85){
              rank++;
            }else if(type_recon.recon_len[id1]>type_recon.recon_len[id2]*1.15){
              rank=rank;
            }else{
              //3rd selection : ch2
              if(type_recon.recon_chi2[id1]<type_recon.recon_chi2[id2]-1.5){
                rank++;
              }else if(type_recon.recon_chi2[id1]>type_recon.recon_chi2[id2]+1.5){
                rank=rank;
              }else{
                //4th selection : total pe
                if(type_recon.recon_total_pe[id1]<type_recon.recon_total_pe[id2]){
                  rank++;
                }else{
                  rank=rank;
                }
              }
            }
          }
        }
        rank_recon[k1].resize(2);
        rank_recon[k1][0] = rank;
        rank_recon[k1][1] = id1;
      }//k1

#ifdef DEBUG_RECON
      cout << "-----------------------" <<endl;
      cout << "view="<<view<<endl;
      for(int k1=0;k1<(int)rank_recon.size();k1++){
        int id1=rank_recon[k1][1];
        cout << "rank="<< rank_recon[k1][0] << "(" 
          << type_recon.recon_len     [id1]<<","
          << type_recon.num_recon_hits[id1]<<","
          << type_recon.recon_chi2    [id1]<<","
          << type_recon.recon_total_pe[id1]<<","
          << id1 <<","
          << ")" 
          <<endl;
        cout  <<  "   [" ;
        for(int ihit=0;ihit<type_recon.num_recon_hits[id1];ihit++){
          int hitid = type_recon.recon_hits_hitid[id1][ihit]; 
          int hitview= type_hit.hit_view  [hitid];
          int pln    = type_hit.hit_pln   [hitid];
          int ch     = type_hit.hit_ch    [hitid];
          cout 
            << "view=" << hitview
            << ",pln=" << pln
            << ",ch=" << ch;
            if(ihit!=type_recon.num_recon_hits[id1]-1) cout << "|";
        }
        cout  <<  "]"<<endl;
      }
#endif
      for(int ii=0;ii<num_recon_cluster-1;ii++){
        for(int jj=1;jj<num_recon_cluster;jj++){
          if(rank_recon[ii][0]>rank_recon[jj][0]){
            for(int kk=0;kk<2;kk++){
              int tmp = rank_recon[ii][kk];
              rank_recon[ii][kk] = rank_recon[jj][kk];
              rank_recon[jj][kk] = tmp;
            }
          }
        }
      }
      
      for(int k1=0;k1<num_recon_cluster-1;k1++){
        int rank1 = rank_recon[k1  ][0];
        int rank2 = rank_recon[k1+1][0];
        if(rank1==rank2){
          for(int k2=k1+1;k2<num_recon_cluster-1;k2++){
            rank_recon[k2][0]++;
          }
        }
      }

#ifdef DEBUG_RECON
      cout << "-------------------"<<endl;
      cout << "view="<<view<<endl;
      for(int k1=0;k1<num_recon_cluster;k1++){
        int id1=rank_recon[k1][1];
        cout << "rank="<< rank_recon[k1][0] << "(" 
          << type_recon.recon_len     [id1]<<","
          << type_recon.num_recon_hits[id1]<<","
          << type_recon.recon_chi2    [id1]<<","
          << type_recon.recon_total_pe[id1]<<","
          << id1 <<","
          << ")" 
          <<endl;
        cout  <<  "   [" ;
        for(int ihit=0;ihit<type_recon.num_recon_hits[id1];ihit++){
          cout << type_recon.recon_hits_hitid[id1][ihit]<<","; 
        }
        cout  <<  "]"<<endl;
      } 
#endif
      // ii. select true recon.
      //    Compare the number of not shared hits to lower rank recon.
      //    If it is same or higher than 3, it is true recon. 
      vector<bool> true_track(num_recon_cluster,true);
      for(int k1=0;k1<num_recon_cluster;k1++){
        int id1 = rank_recon[k1][1];
        for(int k2=0;k2<k1;k2++){
          if(k1==0) break;
          if(!true_track[k2]) continue;
          int id2 = rank_recon[k2][1];
          int not_shared_hits=0;
          for(int l1=0;l1<type_recon.num_recon_hits[id1];l1++){
            bool not_shared_hit_flag=true;
            for(int l2=0;l2<type_recon.num_recon_hits[id2];l2++){
              if(type_recon.recon_hits_hitid[id2][l2] == 0) continue;
              if(type_recon.recon_hits_hitid[id1][l1] == type_recon.recon_hits_hitid[id2][l2]){
                not_shared_hit_flag=false;
                break;
              }
            }//l2
            if(not_shared_hit_flag) not_shared_hits++;
          }//l1
          //if(type_recon.num_recon_hits[id1]<THRES_TRACK_HITS) true_track[k1]=false;
#ifdef DEBUG_RECON
          cout 
            << " k1=" << k1 
            << " k2=" << k2 
            << " num_recon_hits="  << type_recon.num_recon_hits[id1]
            << " not_shared_hits=" << not_shared_hits 
            << endl;
#endif
          if( 
              //(type_recon.num_recon_hits[id1]<=4 && not_shared_hits<2)||
              (type_recon.num_recon_hits[id1]<=3 && not_shared_hits<2)||
              (type_recon.num_recon_hits[id1]==4 && not_shared_hits<3)||
              (type_recon.num_recon_hits[id1]>=5 && not_shared_hits<(type_recon.num_recon_hits[id1])/2.))
          {
            true_track[k1]=false;
          }
          if(!true_track[k1]){
            break;
          }
        }//k2
        if(true_track[k1]){
          tmp_recon.num_recon_hits   [tmp_recon.num_recon] = type_recon.num_recon_hits   [id1] ;
          tmp_recon.recon_bcid_id    [tmp_recon.num_recon] = type_recon.recon_bcid_id    [id1] ;
          tmp_recon.recon_start_z    [tmp_recon.num_recon] = type_recon.recon_start_z    [id1] ;
          tmp_recon.recon_stop_z     [tmp_recon.num_recon] = type_recon.recon_stop_z     [id1] ;
          tmp_recon.recon_start_pln  [tmp_recon.num_recon] = type_recon.recon_start_pln  [id1] ;
          tmp_recon.recon_stop_pln   [tmp_recon.num_recon] = type_recon.recon_stop_pln   [id1] ;
          tmp_recon.recon_start_ch   [tmp_recon.num_recon] = type_recon.recon_start_ch   [id1] ;
          tmp_recon.recon_stop_ch    [tmp_recon.num_recon] = type_recon.recon_stop_ch    [id1] ;
          tmp_recon.recon_start_xy   [tmp_recon.num_recon] = type_recon.recon_start_xy   [id1] ;
          tmp_recon.recon_stop_xy    [tmp_recon.num_recon] = type_recon.recon_stop_xy    [id1] ;
          tmp_recon.recon_slope      [tmp_recon.num_recon] = type_recon.recon_slope      [id1] ;
          tmp_recon.recon_intercept  [tmp_recon.num_recon] = type_recon.recon_intercept  [id1] ;
          tmp_recon.recon_pathlength [tmp_recon.num_recon] = type_recon.recon_pathlength [id1] ;
          tmp_recon.recon_total_pe   [tmp_recon.num_recon] = type_recon.recon_total_pe   [id1] ;
          tmp_recon.recon_mean_dedx  [tmp_recon.num_recon] = type_recon.recon_mean_dedx  [id1] ;
          tmp_recon.recon_mean_time  [tmp_recon.num_recon] = type_recon.recon_mean_time  [id1] ;
          tmp_recon.recon_view       [tmp_recon.num_recon] = type_recon.recon_view       [id1] ;
          tmp_recon.recon_bcid       [tmp_recon.num_recon] = type_recon.recon_bcid       [id1] ;
          tmp_recon.recon_cos        [tmp_recon.num_recon] = type_recon.recon_cos        [id1] ;
          tmp_recon.recon_len        [tmp_recon.num_recon] = type_recon.recon_len        [id1] ;
          tmp_recon.recon_chi2       [tmp_recon.num_recon] = type_recon.recon_chi2       [id1] ; 
          tmp_recon.recon_veto       [tmp_recon.num_recon] = type_recon.recon_veto       [id1] ;
          tmp_recon.recon_sideescape [tmp_recon.num_recon] = type_recon.recon_sideescape [id1] ; 
          for(int l=0;l<type_recon.num_recon_hits[id1];l++){
            tmp_recon.recon_hits_hitid[tmp_recon.num_recon][l] = type_recon.recon_hits_hitid[id1][l];
          }//l
          tmp_recon.num_recon++;
        }
      }//k1
      rank_recon.clear();
      rank_recon.shrink_to_fit();
    }//view
  }//i

  type_recon.Clear();
  type_recon = tmp_recon;

#ifdef DEBUG_RECON
  cout << "===========================" << endl;
  cout << "eraceDuplicateTrack is done..." << endl;
  cout << "num_recon=" <<type_recon.num_recon<<endl;
  for(int j=0;j<type_recon.num_recon;j++){
    cout << "track " << j <<  " (view,start_pln,stop_pln)=(" << type_recon.recon_view[j] << "," << type_recon.recon_start_pln[j] << "," << type_recon.recon_stop_pln[j] << ")" << endl;
  }
  cout << endl;
#endif 
  return true;
}

//*******************************************************************
bool wgRecon::findTrackPair(){
  //find track pair whose start_z and stop_z is same as the other
  if(type_recon.num_recon<1) return false;
  type_track.num_pass_reconpln        .resize(type_recon.num_recon);
  type_track.num_pass_pair_reconpln   .resize(type_recon.num_recon);
  type_track.recon_pair_start_reconpln.resize(type_recon.num_recon);
  type_track.recon_pair_stop_reconpln .resize(type_recon.num_recon);
  for(int j=0;j<type_recon.num_recon;j++){
    int bcid_id   = type_recon.recon_bcid_id  [j];
    int view      = type_recon.recon_view     [j];
    int start_pln = type_recon.recon_start_pln[j];
    int start_ch  = type_recon.recon_start_ch [j];
    int stop_pln  = type_recon.recon_stop_pln [j];
    int stop_ch   = type_recon.recon_stop_ch  [j];
    int start_reconpln  = type_remap.recon_pln [0][view][start_pln][start_ch];
    int stop_reconpln   = type_remap.recon_pln [0][view][stop_pln ][stop_ch ];

    int limit_upper_start_reconpln,limit_lower_start_reconpln;
    int limit_upper_stop_reconpln ,limit_lower_stop_reconpln;        

    if(view==TopView){
      limit_upper_start_reconpln = ((int)(start_reconpln/3)+1)*3+1;
      limit_lower_start_reconpln = ((int)(start_reconpln/3)-0)*3-1;
      limit_upper_stop_reconpln  = ((int)(stop_reconpln /3)+1)*3+1;
      limit_lower_stop_reconpln  = ((int)(stop_reconpln /3)-0)*3-1;  
    }else if(view==SideView){
      limit_upper_start_reconpln = ((int)((start_reconpln+1)/3)+0)*3+1+1;
      limit_lower_start_reconpln = ((int)((start_reconpln+1)/3)-1)*3+1-1;
      limit_upper_stop_reconpln  = ((int)((stop_reconpln +1)/3)+0)*3+1+1;
      limit_lower_stop_reconpln  = ((int)((stop_reconpln +1)/3)-1)*3+1-1;
    }else{
      cout << "Wrong view!"<< endl;
      return false;
    }
    if(start_pln==stop_pln){ 
      if(start_reconpln > stop_reconpln){
        int temp_reconpln = start_reconpln;
        start_reconpln    = stop_reconpln;
        stop_reconpln     = temp_reconpln;
      }
    }

    type_track.num_pass_reconpln[j] = stop_reconpln - start_reconpln + 1; 
    if(view==0){
      int num_pair_reconpln1 = (int)(start_reconpln%3+1)/3 + 2 + (int)(start_reconpln/3)*3;
      int num_pair_reconpln2 = (int)(stop_reconpln %3+1)/3 + 2 + (int)(stop_reconpln /3)*3;
      type_track.num_pass_pair_reconpln[j] = num_pair_reconpln2 - num_pair_reconpln1 + 1;
    }else if(view==1){
      int num_pair_reconpln1 = (int)((start_reconpln+1)%3+1)/3 + (int)((start_reconpln+1)/3)*3;
      int num_pair_reconpln2 = (int)((stop_reconpln +1)%3+1)/3 + (int)((stop_reconpln +1)/3)*3;
      type_track.num_pass_pair_reconpln[j] = num_pair_reconpln2 - num_pair_reconpln1 + 1;
    }

    vector<int> start_pair_recon_id;
    vector<int> stop_pair_recon_id;
    for(int j2=0;j2<type_recon.num_recon;j2++){
      int view2 = type_recon.recon_view[j2];
      if( view2 == view || type_recon.recon_bcid_id[j2] != bcid_id ) continue;
      start_pln = type_recon.recon_start_pln[j2];
      start_ch  = type_recon.recon_start_ch [j2];
      stop_pln  = type_recon.recon_stop_pln [j2];
      stop_ch   = type_recon.recon_stop_ch  [j2];
      start_reconpln  = type_remap.recon_pln [0][view2][start_pln][start_ch];
      stop_reconpln   = type_remap.recon_pln [0][view2][stop_pln ][stop_ch ];
      if(start_reconpln <= limit_upper_start_reconpln && start_reconpln >= limit_lower_start_reconpln ){
        start_pair_recon_id.push_back(j2);
      } 
      if(stop_reconpln <= limit_upper_stop_reconpln && stop_reconpln >= limit_lower_stop_reconpln ){
        stop_pair_recon_id.push_back(j2);
      }
    }//j2
    type_track.recon_pair_start_reconpln[j] = start_pair_recon_id;
    type_track.recon_pair_stop_reconpln[j]  = stop_pair_recon_id;
  }//j

#ifdef DEBUG_RECON
  cout << "==================================" << endl;
  cout << "findTrackPair is done ..." << endl;    
  cout << "---------------------------------" << endl;
  cout << " find recon pair is done ...." << endl;
  cout << " num_recon = " << type_recon.num_recon << endl;
  for(int j=0;j<(int)type_track.recon_pair_start_reconpln.size();j++){  
    cout << " num_pass_reconpln = " << type_track.num_pass_reconpln[j] << endl;
    cout << " num_pass_pair_reconpln = " << type_track.num_pass_pair_reconpln[j] << endl;
    cout << " start track id:" << j << " {" ;
    for(int k=0;k<(int)type_track.recon_pair_start_reconpln[j].size();k++){
      cout << type_track.recon_pair_start_reconpln[j][k] <<",";
    }
    cout << "}" << endl;
    cout << " stop  track id:" << j << " {" ;
    for(int k=0;k<(int)type_track.recon_pair_stop_reconpln[j].size();k++){  
      cout << type_track.recon_pair_stop_reconpln[j][k] <<",";
    }
    cout << "}" << endl;
  }
#endif

  // ****************  For Paired Track , Connect Track of counter-part view **************** //
  type_track.pair_track     .resize(type_recon.num_recon);
  type_track.connected_track.resize(type_recon.num_recon);
  vector<vector<short int>> max_stop_z_trackid;
  vector<vector<short int>> max_start_z_trackid;
  max_stop_z_trackid .resize(type_recon.num_recon);
  max_start_z_trackid.resize(type_recon.num_recon);
  for(int i=0;i<type_recon.num_recon;i++){
    type_track.pair_track[i]     .assign(type_recon.num_recon,-1);
    type_track.connected_track[i].resize(type_recon.num_recon);
    max_stop_z_trackid [i].assign(type_recon.num_recon,-1);
    max_start_z_trackid[i].assign(type_recon.num_recon,-1);
  }

  short int track_id=0; 
  for(int j=0;j<(int)type_recon.num_recon;j++){
    // *********** For Start Pair ************ //
    for(int k1=0;k1<(int)type_track.recon_pair_start_reconpln[j].size();k1++){
      int start_id = type_track.recon_pair_start_reconpln[j][k1];
      double slope1   =type_recon.recon_slope  [start_id];
      int    view1    =type_recon.recon_view   [start_id];
      double start_z1 =type_recon.recon_start_z[start_id]; 
      if(j<start_id){ 
        type_track.pair_track[j][start_id] = track_id;
        track_id++;
      }
      //Connect track
      vector<short int> temp_v;
      double temp_z=-999999.;
      int    temp_id=-1;
      for(int k2=0;k2<(int)type_recon.num_recon;k2++){
        double slope2   = type_recon.recon_slope  [k2];
        int    view2    = type_recon.recon_view   [k2];
        double start_z2 = type_recon.recon_start_z[k2];
        if(view1!=view2 || start_z2 < start_z1) continue;
        double cos_diff = (1.+slope1*slope2)/sqrt((1.+slope1*slope1)*(1.+slope2*slope2));
        // if cos>0.9863 (degree<10) and distance < 50 (2pln)
        if( fabs(cos_diff) < 0.9863 ) continue;
        double pos_diff[2];
        double z[2][2],xy[2][2];
        xy[0][0]  = type_recon.recon_start_xy[start_id];
        xy[0][1]  = type_recon.recon_stop_xy [start_id];
        xy[1][0]  = type_recon.recon_start_xy[k2];
        xy[1][1]  = type_recon.recon_stop_xy [k2];
        z[0][0]   = type_recon.recon_start_z[start_id];
        z[0][1]   = type_recon.recon_stop_z [start_id];
        z[1][0]   = type_recon.recon_start_z[k2];
        z[1][1]   = type_recon.recon_stop_z [k2];
        double cos = 1./sqrt(1.+slope1*slope1);
        double sin = slope1/sqrt(1.+slope1*slope1);
        pos_diff[0] =  fabs((z[0][1]-z[1][0])*sin-(xy[0][1]-xy[1][0])*cos);
        pos_diff[1] =  fabs((z[0][1]-z[1][1])*sin-(xy[0][1]-xy[1][1])*cos); 
        if( pos_diff[0] < C_WMScintiWidth*2){
          temp_v.push_back(k2);
          if(temp_z>z[1][0]){
            temp_id = k2; 
          }          
        }          
      }//k2
      type_track.connected_track[j][start_id]=temp_v;
      max_stop_z_trackid[j][start_id]=temp_id;
    }//k1

    // *********** For Stop Pair ************ //
    for(int k1=0;k1<(int)type_track.recon_pair_stop_reconpln[j].size();k1++){
      int    stop_id  = type_track.recon_pair_stop_reconpln[j][k1];
      double slope1   = type_recon.recon_slope [stop_id];
      int    view1    = type_recon.recon_view  [stop_id];
      double stop_z1  = type_recon.recon_stop_z[stop_id]; 
      if(type_track.pair_track[stop_id][j]>=0||type_track.pair_track[j][stop_id]>=0){ continue; }
      if(j<stop_id){ 
        type_track.pair_track[j][stop_id] = track_id;
        track_id++;
      }
      //Connect track
      vector<short int> temp_v;
      double temp_z=-999999.;
      int    temp_id=-1;
      for(int k2=0;k2<(int)type_recon.num_recon;k2++){
        double slope2   = type_recon.recon_slope [k2];
        int    view2    = type_recon.recon_view  [k2];
        double stop_z2  = type_recon.recon_stop_z[k2];
        if(view1!=view2 || stop_z2 > stop_z1) continue;
        double cos_diff = (1.+slope1*slope2)/sqrt((1.+slope1*slope1)*(1.+slope2*slope2));
        // if cos>0.9863 (degree<10) and distance < 50 (2pln)
        if( fabs(cos_diff) < 0.9863 ) continue;
        double pos_diff[2];
        double z[2][2],xy[2][2];
        xy[0][0]  = type_recon.recon_start_xy[stop_id];
        xy[0][1]  = type_recon.recon_stop_xy [stop_id];
        xy[1][0]  = type_recon.recon_start_xy[k2];
        xy[1][1]  = type_recon.recon_stop_xy [k2];
        z[0][0]   = type_recon.recon_start_z[stop_id];
        z[0][1]   = type_recon.recon_stop_z [stop_id];
        z[1][0]   = type_recon.recon_start_z[k2];
        z[1][1]   = type_recon.recon_stop_z [k2];
        double cos = 1./sqrt(1.+slope1*slope1);
        double sin = slope1/sqrt(1.+slope1*slope1);
        pos_diff[0] =  fabs((z[0][0]-z[1][1])*sin-(xy[0][0]-xy[1][1])*cos);
        pos_diff[1] =  fabs((z[0][1]-z[1][1])*sin-(xy[0][1]-xy[1][1])*cos); 
        if( pos_diff[0] < C_WMScintiWidth*2){
          temp_v.push_back(k2);
          if(temp_z<z[1][1]){
            temp_id = k2;
          }          
        }          
      }//k2
      type_track.connected_track[j][stop_id]=temp_v;
      max_start_z_trackid       [j][stop_id]=temp_id;
    }//k1
  }//j

  // **************** Fill Track ********************** //
  type_track.num_track=0;
  type_track.num_trackid=track_id;
  vector<bool> used_hitid;
  int num_track_hit=0;  

  for(int j=0;j<type_recon.num_recon;j++){
    // Fill counterpart view track.
    for(int k=0;k<type_recon.num_recon;k++){
      if(type_track.num_track>=MAX_NUM_TRACK) break;
      if(type_track.pair_track[j][k]<0) continue;
      if(type_track.num_track>=MAX_NUM_TRACK) break;
      used_hitid.clear();
      used_hitid.assign(MAX_NUM_HIT,false);
      num_track_hit=0;
      double cos_zen , cos_azi;
      if(type_recon.recon_view[k]==1){
        cos_zen = type_recon.recon_slope[k]/sqrt(1.+pow(type_recon.recon_slope[k],2)+pow(type_recon.recon_slope[j],2));
        cos_azi = type_recon.recon_slope[j]/sqrt(1.+pow(type_recon.recon_slope[j],2));
      }else{
        cos_zen = type_recon.recon_slope[j]/sqrt(1.+pow(type_recon.recon_slope[k],2)+pow(type_recon.recon_slope[j],2));
        cos_azi = type_recon.recon_slope[k]/sqrt(1.+pow(type_recon.recon_slope[k],2));
      }

      type_track.track_start_z   [type_track.num_track] = type_recon.recon_start_z   [k];
      type_track.track_start_xy  [type_track.num_track] = type_recon.recon_start_xy  [k];
      type_track.track_start_pln [type_track.num_track] = type_recon.recon_start_pln [k];
      type_track.track_start_ch  [type_track.num_track] = type_recon.recon_start_ch  [k];
      type_track.track_stop_z    [type_track.num_track] = type_recon.recon_stop_z    [k];
      type_track.track_stop_xy   [type_track.num_track] = type_recon.recon_stop_xy   [k];
      type_track.track_stop_pln  [type_track.num_track] = type_recon.recon_stop_pln  [k];
      type_track.track_stop_ch   [type_track.num_track] = type_recon.recon_stop_ch   [k];
      type_track.track_slope     [type_track.num_track] = type_recon.recon_slope     [k];
      type_track.track_intercept [type_track.num_track] = type_recon.recon_intercept [k];
      type_track.track_view      [type_track.num_track] = type_recon.recon_view      [k];
      type_track.track_bcid      [type_track.num_track] = type_recon.recon_bcid      [k];
      type_track.track_bcid_id   [type_track.num_track] = type_recon.recon_bcid_id   [k];
      type_track.track_len       [type_track.num_track] = type_recon.recon_len       [k];
      type_track.track_id        [type_track.num_track] = type_track.pair_track      [j][k];
      type_track.track_veto      [type_track.num_track] = type_recon.recon_veto      [k];
      type_track.track_sideescape[type_track.num_track] = type_recon.recon_sideescape[k];
      type_track.track_cos_zen   [type_track.num_track] = cos_zen;
      type_track.track_cos_azi   [type_track.num_track] = cos_azi;
      type_track.track_recon_id  [type_track.num_track][0] = k;
      for(int m=0;m<type_recon.num_recon_hits[k];m++){
        if(used_hitid[type_recon.recon_hits_hitid[k][m]]) continue;
        type_track.track_hits_hitid[type_track.num_track][num_track_hit]=type_recon.recon_hits_hitid[k][m];
        used_hitid[type_recon.recon_hits_hitid[k][m]] = true;
        num_track_hit++;
      }

      type_track.num_track_recon [type_track.num_track] = type_track.connected_track[j][k].size()+1;
      if(max_stop_z_trackid[j][k]>=0){
        double stop_z    = type_recon.recon_stop_z      [max_stop_z_trackid[j][k]];
        double stop_xy   = type_recon.recon_stop_xy     [max_stop_z_trackid[j][k]];
        double len       = sqrt(pow(type_recon.recon_start_z[k]-stop_z,2)+pow(type_recon.recon_start_xy[k]-stop_xy,2));
        type_track.track_stop_z    [type_track.num_track] = stop_z;
        type_track.track_stop_xy   [type_track.num_track] = stop_xy;
        type_track.track_len       [type_track.num_track] = len;
      }else if(max_start_z_trackid[j][k]>=0){
        double start_z    = type_recon.recon_start_z      [max_start_z_trackid[j][k]];
        double start_xy   = type_recon.recon_start_xy     [max_start_z_trackid[j][k]];
        double len       = sqrt(pow(type_recon.recon_stop_z[k]-start_z,2)+pow(type_recon.recon_stop_xy[k]-start_xy,2));
        type_track.track_start_z   [type_track.num_track] = start_z;
        type_track.track_start_xy  [type_track.num_track] = start_xy;
        type_track.track_len       [type_track.num_track] = len;
      }

      double pathlength  = type_recon.recon_pathlength[k];
      double total_pe    = type_recon.recon_total_pe  [k];
      double mean_dedx   = type_recon.recon_mean_dedx [k];
      double mean_time   = type_recon.recon_mean_time [k];
      int    veto        = type_recon.recon_veto      [k];  
      int    sideescape  = type_recon.recon_sideescape[k];
      if(type_recon.recon_veto[j]      ==1)     veto   =1;  
      if(type_recon.recon_sideescape[j]==1) sideescape =1;  

      for(int l=0;l<(int)type_track.connected_track[j][k].size();l++){
        int con_id = type_track.connected_track[j][k][l];
        type_track.track_recon_id  [type_track.num_track][l+1] = con_id;
        for(int m=0;m<type_recon.num_recon_hits[con_id];m++){
          if(num_track_hit>=MAX_NUM_TRACKHIT) break;
          if(used_hitid[type_recon.recon_hits_hitid[con_id][m]]) continue;
          type_track.track_hits_hitid[type_track.num_track][num_track_hit]=type_recon.recon_hits_hitid[con_id][m];
          used_hitid[type_recon.recon_hits_hitid[con_id][m]] = true;
          num_track_hit++;
        }
        //type_track.num_track_hits  [type_track.num_track] += type_recon.num_recon_hits  [con_id];
        pathlength  += type_recon.recon_pathlength[con_id];
        total_pe    += type_recon.recon_total_pe  [con_id];
        mean_dedx   += type_recon.recon_mean_dedx [con_id];
        mean_time   += type_recon.recon_mean_time [con_id];
        if(type_recon.recon_veto      [con_id]==1) veto       = 1;
        if(type_recon.recon_sideescape[con_id]==1) sideescape = 1;
      }
      type_track.num_track_hits  [type_track.num_track] = num_track_hit;

      mean_dedx = mean_dedx/(type_track.connected_track[j][k].size()+1);
      mean_time = mean_time/(type_track.connected_track[j][k].size()+1);

      type_track.track_pathlength[type_track.num_track] = pathlength ;
      type_track.track_total_pe  [type_track.num_track] = total_pe   ;
      type_track.track_mean_dedx [type_track.num_track] = mean_dedx  ;
      type_track.track_mean_time [type_track.num_track] = mean_time  ;
      type_track.track_veto      [type_track.num_track] = veto       ;
      type_track.track_sideescape[type_track.num_track] = sideescape ;
      type_track.num_track++;
      if(type_track.num_track>=MAX_NUM_TRACK) break;

      // Fill same view track.
      if(type_track.num_track>=MAX_NUM_TRACK) break;
      num_track_hit=0;
      type_track.track_start_z   [type_track.num_track] = type_recon.recon_start_z   [j];
      type_track.track_start_xy  [type_track.num_track] = type_recon.recon_start_xy  [j];
      type_track.track_start_pln [type_track.num_track] = type_recon.recon_start_pln [j];
      type_track.track_start_ch  [type_track.num_track] = type_recon.recon_start_ch  [j];
      type_track.track_stop_z    [type_track.num_track] = type_recon.recon_stop_z    [j];
      type_track.track_stop_xy   [type_track.num_track] = type_recon.recon_stop_xy   [j];
      type_track.track_stop_pln  [type_track.num_track] = type_recon.recon_stop_pln  [j];
      type_track.track_stop_ch   [type_track.num_track] = type_recon.recon_stop_ch   [j];
      type_track.track_slope     [type_track.num_track] = type_recon.recon_slope     [j];
      type_track.track_intercept [type_track.num_track] = type_recon.recon_intercept [j];
      type_track.track_view      [type_track.num_track] = type_recon.recon_view      [j];
      type_track.track_bcid      [type_track.num_track] = type_recon.recon_bcid      [j];
      type_track.track_bcid_id   [type_track.num_track] = type_recon.recon_bcid_id   [j];
      type_track.track_len       [type_track.num_track] = type_recon.recon_len       [j];
      type_track.track_id        [type_track.num_track] = type_track.pair_track      [j][k];
      type_track.track_veto      [type_track.num_track] = type_recon.recon_veto      [j];
      type_track.track_sideescape[type_track.num_track] = type_recon.recon_sideescape[j];
      //type_track.num_track_hits  [type_track.num_track] = type_recon.num_recon_hits  [j];
      type_track.track_cos_zen   [type_track.num_track] = cos_zen;
      type_track.track_cos_azi   [type_track.num_track] = cos_azi;
      type_track.track_recon_id  [type_track.num_track][0] = k;
      for(int m=0;m<type_recon.num_recon_hits[j];m++){
        //type_track.track_hits_hitid[type_track.num_track][m]=type_recon.recon_hits_hitid[j][m];
        if(used_hitid[type_recon.recon_hits_hitid[j][m]]) continue;
        type_track.track_hits_hitid[type_track.num_track][num_track_hit]=type_recon.recon_hits_hitid[j][m];
        used_hitid[type_recon.recon_hits_hitid[j][m]] = true;
        num_track_hit++;
      }
      type_track.num_track_recon [type_track.num_track] = type_track.connected_track[k][j].size()+1;
      if(max_stop_z_trackid[k][j]>=0){
        double stop_z    = type_recon.recon_stop_z      [max_stop_z_trackid[k][j]];
        double stop_xy   = type_recon.recon_stop_xy     [max_stop_z_trackid[k][j]];
        double len       = sqrt(pow(type_recon.recon_start_z[j]-stop_z,2)+pow(type_recon.recon_start_xy[j]-stop_xy,2));
        type_track.track_stop_z    [type_track.num_track] = stop_z;
        type_track.track_stop_xy   [type_track.num_track] = stop_xy;
        type_track.track_len       [type_track.num_track] = len;
      }else if(max_start_z_trackid[k][j]>=0){
        double start_z    = type_recon.recon_start_z      [max_start_z_trackid[k][j]];
        double start_xy   = type_recon.recon_start_xy     [max_start_z_trackid[k][j]];
        double len       = sqrt(pow(type_recon.recon_stop_z[j]-start_z,2)+pow(type_recon.recon_stop_xy[j]-start_xy,2));
        type_track.track_start_z    [type_track.num_track] = start_z;
        type_track.track_start_xy   [type_track.num_track] = start_xy;
        type_track.track_len       [type_track.num_track] = len;
      }

      pathlength  = type_recon.recon_pathlength[j];
      total_pe    = type_recon.recon_total_pe  [j];
      mean_dedx   = type_recon.recon_mean_dedx [j];
      mean_time   = type_recon.recon_mean_time [j];
      if(type_recon.recon_veto[j]==1) veto = 1;  
      if(type_recon.recon_sideescape[j]==1) sideescape= 1;  

      for(int l=0;l<(int)type_track.connected_track[k][j].size();l++){
        int con_id = type_track.connected_track[k][j][l];
        type_track.track_recon_id  [type_track.num_track][l+1] = con_id;
        for(int m=0;m<type_recon.num_recon_hits[con_id];m++){
          if(num_track_hit>=MAX_NUM_TRACKHIT) break;
          if(used_hitid[type_recon.recon_hits_hitid[con_id][m]]) continue;
          type_track.track_hits_hitid[type_track.num_track][num_track_hit]=type_recon.recon_hits_hitid[con_id][m];
          used_hitid[type_recon.recon_hits_hitid[con_id][m]] = true;
          num_track_hit++;
          //type_track.track_hits_hitid[type_track.num_track][m+type_track.num_track_hits[type_track.num_track]]=type_recon.recon_hits_hitid[con_id][m];
        }
        //type_track.num_track_hits  [type_track.num_track] += type_recon.num_recon_hits  [con_id];
        pathlength  += type_recon.recon_pathlength[con_id];
        total_pe    += type_recon.recon_total_pe  [con_id];
        mean_dedx   += type_recon.recon_mean_dedx [con_id];
        mean_time   += type_recon.recon_mean_time [con_id];
        if(type_recon.recon_veto[con_id]==1) veto = 1;
        if(type_recon.recon_sideescape[con_id]==1) sideescape = 1;
      }


      type_track.num_track_hits  [type_track.num_track] = num_track_hit;
      mean_dedx = mean_dedx/(type_track.connected_track[k][j].size()+1);
      mean_time = mean_time/(type_track.connected_track[k][j].size()+1);

      type_track.track_pathlength[type_track.num_track] = pathlength ;
      type_track.track_total_pe  [type_track.num_track] = total_pe   ;
      type_track.track_mean_dedx [type_track.num_track] = mean_dedx  ;
      type_track.track_mean_time [type_track.num_track] = mean_time  ;
      type_track.track_sideescape[type_track.num_track] = sideescape ;
      if(veto==1){
        type_track.track_veto      [type_track.num_track-1] = veto       ;
        type_track.track_veto      [type_track.num_track] = veto       ;
      }
      if(sideescape==1){
        type_track.track_sideescape      [type_track.num_track-1] = sideescape       ;
        type_track.track_sideescape      [type_track.num_track] = sideescape       ;
      }
      type_track.num_track++;
      if(type_track.num_track>=MAX_NUM_TRACK) break;
    }//k
  }//j

  
  if(type_track.num_track>=MAX_NUM_TRACK) return false;
#ifdef DEBUG_RECON
  cout << "---------------------------------" << endl;
  cout << " connect track is done .... "<< endl;
  cout << " num_track= " << type_track.num_track << endl;
  for(int i=0;i<type_track.num_track;i++){
    cout << "{" << "bcid:"  << type_track.track_bcid[i]  << ",view:"      << type_track.track_view[i]      << "}";
    cout << "{" << "slope:" << type_track.track_slope[i] << ",intercept:" << type_track.track_intercept[i] << "}";
    cout << "{" << "id:"    << type_track.track_id[i]    << ",length:"    << type_track.track_len[i]       << "}" << endl;
  }
#endif
  return true;
}

//*******************************************************************

bool wgRecon::addNearHits(){
#ifdef DEBUG_RECON
  std::cout << "================= addNearHits ==="  << std::endl;
#endif

  for(int i_bcid=0;i_bcid<type_hit.num_bcid_cluster;i_bcid++){
    int current_bcid = type_hit.clustered_bcid[i_bcid];
    for(int i=0;i<type_track.num_track;i++){
      if(current_bcid!=type_track.track_bcid[i]) continue;
      double start_z   = type_track.track_start_z   [i];
      double stop_z    = type_track.track_stop_z    [i];
      double start_xy  = type_track.track_start_xy  [i];
      double stop_xy   = type_track.track_stop_xy   [i];
      int    trk_view  = type_track.track_view      [i];
      double slope     = type_track.track_slope     [i];
      double intcpt    = type_track.track_intercept [i];
#ifdef DEBUG_RECON
      std::cout
        << "=== track ==" << std::endl
        << " view="      << trk_view     
        << " start_z="   << start_z  
        << " start_xy="  << start_xy
        << " stop_z="    << stop_z
        << " stop_xy="   << stop_xy
        << " slope="     << slope
        << " intcpt="    << intcpt
        << std::endl;
#endif
      int num_hits       = type_hit  .num_hits;
      int num_track_hits = type_track.num_track_hits[i];
      vector<bool> trackhits(num_hits);
      trackhits.assign(num_hits,false);
      for(int j=0;j<num_track_hits;j++){
        int hitid = type_track.track_hits_hitid[i][j];
        trackhits[hitid] = true;
      }
      for(int j=0;j<type_hit.num_bcid_hits[i_bcid];j++){
        int hitid  = type_hit.clustered_hitid[i_bcid][j];
        int hitview= type_hit.hit_view  [hitid];
        int dif    = type_hit.hit_dif   [hitid];
        int chip   = type_hit.hit_chip  [hitid];
        int chipch = type_hit.hit_chipch[hitid];
        int pln    = type_hit.hit_pln   [hitid];
        int ch     = type_hit.hit_ch    [hitid];
        double x  = type_map.x[dif][chip][chipch];
        double y  = type_map.y[dif][chip][chipch];
        double z  = type_map.z[dif][chip][chipch];
        double xy;
        if     (hitview==SideView){ xy = y; }
        else if(hitview==TopView ){ xy = x; }
        if(trackhits[hitid] ){ continue; }
        if(hitview!=trk_view){ continue; }
        if(!(
              ((z >=start_z -50.&&z <=stop_z +50.)||(z<=start_z  +50.&&z >=stop_z -50.))&&
              ((xy>=start_xy-50.&&xy<=stop_xy+50.)||(xy<=start_xy+50.&&xy>=stop_xy-50.))
              )){
          continue;
        }
        double dist = fabs(slope*z-xy+intcpt)/sqrt(slope*slope+1);
#ifdef DEBUG_RECON
        cout 
          << " view=" << hitview
          << " pln=" << pln
          << " ch=" << ch
          << " dist=" << dist
          << endl;
#endif
        if(dist<25.){
          type_track.num_track_hits  [i]++;
          type_track.track_hits_hitid[i][type_track.num_track_hits[i]-1] = hitid;
        }
      }
#ifdef DEBUG_RECON
      cout << "track hits...." << endl;
      for(int j=0;j<type_track.num_track_hits[i];j++){
        int hitid  = type_track.track_hits_hitid[i][j];
        int view   = type_hit.hit_view  [hitid];
        int pln    = type_hit.hit_pln   [hitid];
        int ch     = type_hit.hit_ch    [hitid];
        cout 
          << "[hitid=" << hitid
          << ",view=" << view
          << ",pln=" << pln
          << ",ch=" << ch
          << "]";
      }
      cout << endl;
#endif
    }
  }

  return true;
}


//*******************************************************************
void wgRecon::fillTrackHit(){
  for(int i=0;i<type_track.num_trackid;i++){
    int view1 = type_track.track_view[i*2];
    int view2 = type_track.track_view[i*2+1];
    double slope1 = type_track.track_slope[i*2];
    double slope2 = type_track.track_slope[i*2+1];
    double pathlength=-1.;
    type_track.track_pathlength[i*2] = 0.;
    type_track.track_total_pe  [i*2] = 0.;
    type_track.track_mean_dedx [i*2] = 0.;
    type_track.track_pathlength[i*2+1] = 0.;
    type_track.track_total_pe  [i*2+1] = 0.;
    type_track.track_mean_dedx [i*2+1] = 0.;
    for(int j=0;j<type_track.num_track_hits[i*2];j++){
      int     hitid = type_track.track_hits_hitid[i*2][j];
      double  pe    = type_hit.hit_pe    [hitid];
      double  cluster_pe  = type_hit.hit_cluster_pe  [hitid];
      if(cluster_pe<0.5) cluster_pe=pe;
      bool    grid  = type_hit.hit_grid  [hitid];
      pathlength = cal_pathlength(view1,slope1,slope2,grid);
      pathlength = pathlength*pe/cluster_pe;
      type_hit.hit_pathlength [hitid] += pathlength;
      type_hit.hit_numtrack   [hitid] ++;
      type_hit.hit_pe_permm   [hitid] = pe/pathlength;
      type_hit.hit_ontrack    [hitid] = true;
      type_track.track_pathlength[i*2] += pathlength;
      type_track.track_total_pe  [i*2] += pe;
      type_track.track_mean_dedx [i*2] += pe/pathlength/type_track.num_track_hits[i*2];
    }//j

    for(int j=0;j<type_track.num_track_hits[i*2+1];j++){
      int     hitid = type_track.track_hits_hitid[i*2+1][j];
      double  pe    = type_hit.hit_pe    [hitid];
      double  cluster_pe  = type_hit.hit_cluster_pe    [hitid];
      if(cluster_pe<0.5) cluster_pe=pe;
      bool    grid  = type_hit.hit_grid  [hitid];
      pathlength = cal_pathlength(view2,slope2,slope1,grid);
      pathlength = pathlength*pe/cluster_pe;
      type_hit.hit_pathlength [hitid] = pathlength;
      type_hit.hit_numtrack   [hitid] ++;
      type_hit.hit_pe_permm   [hitid] = pe/pathlength;
      type_hit.hit_ontrack    [hitid] = true;
      type_track.track_pathlength[i*2+1] += pathlength;
      type_track.track_total_pe  [i*2+1] += pe;
      type_track.track_mean_dedx [i*2+1] += pe/pathlength/type_track.num_track_hits[i*2+1];
    }//j
  }
}

//********************************************************************
double wgRecon::cal_pathlength(int view, double slope1, double slope2, bool grid){
  double path=-1.;
  double x,y,z;
  
  if(!grid && view==TopView)      { x=C_WMTrueScintiThick; y=C_WMTrueScintiWidth; z=C_WMScintiLength; }
  else if(grid && view==TopView)  { y=C_WMTrueScintiThick; x=C_WMTrueScintiWidth; z=C_WMScintiLength; }
  else if(!grid &&view==SideView) { x=C_WMTrueScintiThick; z=C_WMTrueScintiWidth; y=C_WMScintiLength; }
  else if(grid && view==SideView) { z=C_WMTrueScintiThick; x=C_WMTrueScintiWidth; y=C_WMScintiLength; }
  else{
      cout << "Error!" << endl;
  }

  double volume = x*y*z;
  double cos1,sin1,cos2,sin2;
  if(view==SideView){
   cos1 = slope1/sqrt(1.0+slope1*slope1);
   sin1 = 1.0/sqrt(1.0+slope1*slope1);
   cos2 = 1.0/sqrt(1.0+slope2*slope2);
   sin2 = slope2/sqrt(1.0+slope2*slope2);
  }else if(view==TopView){
   cos1 = slope2/sqrt(1.0+slope2*slope2);
   sin1 = 1.0/sqrt(1.0+slope2*slope2);
   cos2 = 1.0/sqrt(1.0+slope1*slope1);
   sin2 = slope1/sqrt(1.0+slope1*slope1);
  }

  double crosssection=fabs(x*y*cos1)+fabs(x*z*sin1*sin2)+fabs(y*z*sin1*cos2);
  path = volume/crosssection;
  return path; 
}

//********************************************************************
bool wgRecon::selectTrueTrack()
{
  /*
     for(int j=0;j<type_recon.num_recon;j++){


  }

}



  //2. check if track exists in two views or not.
  for(unsigned int j=0;j<bcid_id.size();j++){
    if( bcid_id[j][0].size() ==0 || bcid_id[j][1].size()==0 ){
      cout << "all tracks don't have corresponding track on the other view." << endl;
      continue;
    }else{
      //find tracks which has same edge of position z from each view.
      const int num_top_track=(int)bcid_id_[j][0].size()
      const int num_side_track=(int)bcid_id_[j][1].size()
      vector<int> start_pair[num_top_track];
      vector<int> stop_pair[num_top_track];
      for(unsigned int k1=0;k1<num_top_track;k1++){ 
        double top_edge_z[2]={type_recon.recon_start_z[bcid_id[j][0][k1]],type_recon.recon_stop_z[bcid_id[j][0][k1]]};
        if( top_edge_z[0] > top_edge_z[1] ){
          double temp = top_edge_z[0];
          top_edge_z[0] = top_edge_z[1] ;
          top_edge_z[1] = temp;
        }
        for(unsigned int k2=0;k2<num_side_track;k2++){
          double side_edge_z[2]={type_recon.recon_start_z[bcid_id[j][1][k2]],type_recon.recon_stop_z[bcid_id[j][1][k2]]};
          if( side_edge_z[0] > side_edge_z[1] ){
            double temp = side_edge_z[0];
            side_edge_z[0] = side_edge_z[1] ;
            side_edge_z[1] = temp;
          }
          //TODO
          if( fabs( top_edge_z[0]-side_edge_z[0] ) <= C_WMLayerDist + C_WMPlnDist ){
            start_pair[k1].push_back(k2);
          }
          if( fabs( top_edge_z[1]-side_edge_z[1] ) <= C_WMLayerDist + C_WMPlnDist ){
            stop_pair[k1].push_back(k2);
          }        
        }//k2
      }//k1

  
      for(unsigned int k1=0;k1<num_top_track;k1++){ 
        if(start_pair[k1].size>0 && stop_pair[k1].size()>0){
          for(int l1=0;l1<start_pair[k1].size();l1++){
            for(int l2=0;l2<stop_pair[k1].size();l2++){
              double start_degree = 180.*acos(type_recon.recon_cos[bcid_id[j][1][start_pair[k1][l1]]])/PI;
              double stop_degree  = 180.*acos(type_recon.recon_cos[bcid_id[j][1][stop_pair[k1][l2]]])/PI;
              if( fabs(start_degree-stop_degree) < MAX_DEFREE_RECON_TRACK_MATCH ){
              
              }
            }
          }
        }
      }
  }//j
      int view  = type_recon.recon_view[bcid_id[j][k]];
      double start_z = type_recon.recon_start_z[bcid_id[j][k]];
      double stop_z = type_recon.recon_stop_z[bcid_id[j][k]];
      double start_xy = type_recon.recon_start_xy[bcid_id[j][k]]; 
      double stop_xy = type_recon.recon_stop_xy[bcid_id[j][k]];
      if(start_z>stop_z){
        double temp_z = start_z;
        double temp_xy = start_xy;
        start_z = stop_z;
        start_xy = stop_xy;
        stop_z = temp_z;
        stop_xy = temp_xy;
      }

      for(unsigned int k2=k+1;k<bcid_id[j].size();k2++){
        double degree2 = 180.*type_recon.recon_cos[bcid_id[j][k2]]/PI;
        int view2  = type_recon.recon_view[bcid_id[j][k2]];
        if( view!=view2 ) continue;
        if(fabs(degree-degree2) > MAX_DEGREE_RECON_TRACK_MATCH &&
            fabs(180.-(degree-degree2)) > MAX_DEGREE_RECON_TRACK_MATCH) continue;
        double start_z2 = type_recon.recon_start_z[bcid_id[j][k]];
        double stop_z2 = type_recon.recon_stop_z[bcid_id[j][k]];
        double start_xy2 = type_recon.recon_start_xy[bcid_id[j][k]]; 
        double stop_xy2 = type_recon.recon_stop_xy[bcid_id[j][k]];
        if(start_z2>stop_z2){
          double temp_z = start_z;
          double temp_xy = start_xy;
          start_z2 = stop_z2;
          start_xy2 = stop_xy2;
          stop_z2 = temp_z;
          stop_xy2 = temp_xy;
        }
        
        bool b_seperate=false;
      
        if( stop_z < start_z2 || stop_z2 < start_z) b_separate=true;
        if( start_z < start_z2 ) b_z[0] = true;
        if( stop_z < stop_z2 ) b_z[1] = true;
        if( stop_z < start_z2 ||  ) b_z[1] = true;
        
        
      }
    }
  }
*/
  return true;
};


//********************************************************************
bool wgRecon::rankTracks_byEdep(int axis)
{
  //if(int i=0;i<(int)type_recon.time_cluster_hitid.size();i++){
  //  int j[2];
  //  vector<vector<int> > trackid_pair;
  //  for(int j[0]=0;j[0]<type_recon.num_recon-1;j[0]++){
  //    for(int j[1]=j[0]+1;j[1]<type_recon.num_recon;j[1]++){
  //      int num_recon_hit[2];
  //      for(int k=0;k<2;k++){num_recon_hit[k] =type_recon.recon_hits_hitid[j[k]].size();}
  //      int k[2];
  //      for(int k[0]=0;k[0]<num_recon_hit[0];k[0]++){
  //        for(int k[1]=0;k[1]<num_recon_hit[1];k[1]++){
  //          if(recon_hits_hitid[k[0]]==recon_hits_hitid[k[1]]){
  //            if((recon_total_pe[j[0]]-recon_total_pe[j[1]])*(j[0]-j[1])<0){
  //              vector<int> tmp = {j[0],j[1]};
  //              trackid_pair.push_back(tmp);
  //            }
  //          }
  //        }
  //      }
  //    }
  //  }
  //}
  return true;
};

//********************************************************************
double wgRecon::GetDispersion(double inter, double slope, vector<double> x, vector<double> y){
  if(x.size()!=y.size()){
    cout << "[Error!][GetDispersion]Data size of x and y is different!" << endl;
    return 10000.;
  }
  double dis=0;
  for(int i=0;i<(int)x.size();i++){
    dis += fabs(slope*x[i]-y[i]+inter)/sqrt(slope*slope+1.00);   
  }
  dis = dis / (double)x.size();
  return dis;
}




