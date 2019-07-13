// system includes
#include <string>

// ROOT includes
#include "TROOT.h"

// user includes
#include "wgConst.hpp"

//***************************************
void Hit_t::Clear()
{
  num_hits = 0;
  num_bcid_cluster=0;
  for(int i=0;i<MAX_NUM_BCID_CLUSTER;i++){
    num_bcid_hits [i] = 0;
    clustered_bcid[i] = 0;
    for(int m=0;m<MAX_NUM_HIT;m++){
      clustered_hitid[i][m] = 0;
    }
  }
  for(int i=0;i<MAX_NUM_HIT;i++){
    hit_bcid        [i] = -1 ;
    hit_dif         [i] = -1 ;
    hit_chip        [i] = -1 ;
    hit_chipch      [i] = -1 ;
    hit_sca         [i] = -1 ;
    hit_adc         [i] = -1 ;
    hit_gs          [i] = -1 ;
    hit_tdc         [i] = -1 ;
    hit_pe          [i] = -1.;
    hit_time        [i] = -1.;
    hit_cos         [i] = -10.;
    hit_pe_permm    [i] = -1.;
    hit_pathlength  [i] = -1.;
    hit_ontrack     [i] = false;
    hit_grid        [i] = false;
    hit_numtrack    [i] = 0;
    hit_cluster_pe   [i] = 0;
  }
}

//***************************************
void Recon_t::Clear()
{
  num_recon = 0;
  for(int i=0;i<MAX_NUM_TRACK;i++){
    num_recon_hits  [i] = 0;
    recon_start_z   [i] = 0.;
    recon_stop_z    [i] = 0.;
    recon_start_pln [i] = -1;  
    recon_stop_pln  [i] = -1;  
    recon_start_ch  [i] = -1;  
    recon_stop_ch   [i] = -1;  
    recon_start_xy  [i] = 0.;
    recon_stop_xy   [i] = 0.;
    recon_slope     [i] = 0.;
    recon_intercept [i] = 0.;
    recon_pathlength[i] = 0.;
    recon_total_pe  [i] = 0.;
    recon_mean_dedx [i] = 0.;
    recon_mean_time [i] = 0.;
    recon_view      [i] = -1;
    recon_bcid      [i] = -1;
    recon_bcid_id   [i] = -1;
    recon_veto      [i] = -1;
    recon_sideescape[i] = -1;
    recon_cos       [i] = -10.0;
    recon_len       [i] = -1.;
    recon_chi2      [i] = -1.;

    for(int j=0;j<MAX_NUM_TRACKHIT;j++){
      recon_hits_hitid[i][j]=0;
    }
  }

  bcid_cluster_hitid      .clear();
  time_cluster_hitid      .clear();
  neighborhits_hitid      .clear();
  cell_clusterid          .clear();
  neighborcell_down_cellid.clear();
  neighborcell_up_cellid  .clear();
  bcid_cluster_hitid      .shrink_to_fit();
  time_cluster_hitid      .shrink_to_fit();
  neighborhits_hitid      .shrink_to_fit();
  cell_clusterid          .shrink_to_fit();
  neighborcell_down_cellid.shrink_to_fit();
  neighborcell_up_cellid  .shrink_to_fit();
}

//***************************************
void Recon_t::Clear_ReconVector()
{
  neighborhits_hitid      .clear();
  cell_clusterid          .clear();
  neighborcell_down_cellid.clear();
  neighborcell_up_cellid  .clear();
  neighborhits_hitid      .shrink_to_fit();
  cell_clusterid          .shrink_to_fit();
  neighborcell_down_cellid.shrink_to_fit();
  neighborcell_up_cellid  .shrink_to_fit();
}

//***************************************
void Track_t::Clear()
{
  num_track = 0;
  num_trackid  = 0;
  for(int i=0;i<MAX_NUM_TRACK;i++){
    num_track_hits  [i] = 0;
    num_track_recon [i] = 0;
    track_start_z   [i] = 0.;
    track_stop_z    [i] = 0.;
    track_start_xy  [i] = 0.;
    track_stop_xy   [i] = 0.;
    track_start_pln [i] = -1;  
    track_stop_pln  [i] = -1;  
    track_start_ch  [i] = -1;  
    track_stop_ch   [i] = -1;  
    track_slope     [i] = 0.;
    track_intercept [i] = 0.;
    track_pathlength[i] = 0.;
    track_total_pe  [i] = 0.;
    track_mean_dedx [i] = 0.;
    track_mean_time [i] = 0.;
    track_view      [i] = -1;
    track_bcid      [i] = -1;
    track_bcid_id   [i] = -1;
    track_veto      [i] = -1;
    track_sideescape[i] = -1;
    track_cos_zen   [i] = -10;
    track_cos_azi   [i] = -10;
    track_len       [i] = -1;
    track_id        [i] = -1;

    for(int j=0;j<MAX_NUM_TRACK;j++){
      track_recon_id[i][j]=-1;
    }
    for(int j=0;j<MAX_NUM_TRACKHIT;j++){
      track_hits_hitid[i][j]=0;
    }
  }
  pair_track.clear();
  connected_track.clear();
  num_pass_reconpln.clear();
  num_pass_pair_reconpln.clear();
  recon_pair_start_reconpln.clear();
  recon_pair_stop_reconpln.clear();
  /*
	num_pass_reconpln.shrink_to_fit();
	num_pass_pair_reconpln.shrink_to_fit();
	recon_pair_start_reconpln.shrink_to_fit();
	recon_pair_stop_reconpln.shrink_to_fit();
  */
}

//***************************************
void Track_t::Clear_TrackVector()
{
  num_pass_reconpln.clear();
  num_pass_pair_reconpln.clear();
  recon_pair_start_reconpln.clear();
  recon_pair_stop_reconpln.clear();
  /*
	num_pass_reconpln.shrink_to_fit();
	num_pass_pair_reconpln.shrink_to_fit();
	recon_pair_start_reconpln.shrink_to_fit();
	recon_pair_stop_reconpln.shrink_to_fit();
  */
}


//***************************************
bool BSD_t::OpenBsdFile(string filename)
{
  BSD_t::infile = new TFile(filename.c_str(),"read");
  if(BSD_t::infile->IsZombie()){
    cout << "No such a ROOT file: " << filename << endl;
    return false;
  }

  TNamed* version_name = (TNamed*) BSD_t::infile->Get("version");
  if(version_name==NULL){
    cerr << "[BSD] Error! Empty version name! " << endl;
    return false;
  }
  BSD_t::version = (string) version_name->GetTitle();
  if     (BSD_t::version=="p06") BSD_t::is_bsd = 0;
  else if(BSD_t::version=="v01") BSD_t::is_bsd = 1;
  else                           BSD_t::is_bsd = -1;

  BSD_t::bsd = (TTree*)BSD_t::infile->Get("bsd");
  if(BSD_t::bsd==NULL){
    cerr << "[BSD] Error! Empty bsd file! " << endl;
    return false;
  }
  BSD_t::bsd->SetBranchAddress("nurun"          ,&(BSD_t::nurun));
  BSD_t::bsd->SetBranchAddress("midas_event"    ,&(BSD_t::midas_event));
  BSD_t::bsd->SetBranchAddress("mrrun"          ,&(BSD_t::mrrun));
  BSD_t::bsd->SetBranchAddress("mrshot"         ,&(BSD_t::mrshot));
  BSD_t::bsd->SetBranchAddress("spillnum"       ,&(BSD_t::spillnum));
  BSD_t::bsd->SetBranchAddress("trg_sec"        , (BSD_t::trg_sec));
  BSD_t::bsd->SetBranchAddress("trg_nano"       , (BSD_t::trg_nano));
  BSD_t::bsd->SetBranchAddress("gpsstat"        , (BSD_t::gpsstat));
  BSD_t::bsd->SetBranchAddress("ct_np"          , (BSD_t::ct_np));
  BSD_t::bsd->SetBranchAddress("beam_time"      , (BSD_t::beam_time));
  BSD_t::bsd->SetBranchAddress("beam_flag"      , (BSD_t::beam_flag));
  BSD_t::bsd->SetBranchAddress("hct"            , (BSD_t::hct));
  BSD_t::bsd->SetBranchAddress("tpos"           , (BSD_t::tpos));
  BSD_t::bsd->SetBranchAddress("tdir"           , (BSD_t::tdir));
  BSD_t::bsd->SetBranchAddress("tsize"          , (BSD_t::tsize));
  BSD_t::bsd->SetBranchAddress("mumon"          , (BSD_t::mumon));
  BSD_t::bsd->SetBranchAddress("otr"            , (BSD_t::otr));
  BSD_t::bsd->SetBranchAddress("good_gps_flag"  ,&(BSD_t::good_gps_flag));
  BSD_t::bsd->SetBranchAddress("trigger_flag"   ,&(BSD_t::trigger_flag));
  BSD_t::bsd->SetBranchAddress("spill_flag"     ,&(BSD_t::spill_flag));
  BSD_t::bsd->SetBranchAddress("good_spill_flag",&(BSD_t::good_spill_flag));
  BSD_t::bsd->SetBranchAddress("target_eff"     , (BSD_t::target_eff));
  BSD_t::bsd->SetBranchAddress("run_type"       ,&(BSD_t::run_type));
  BSD_t::bsd->SetBranchAddress("magset_id"      ,&(BSD_t::magset_id));
  BSD_t::bsd->SetBranchAddress("hctx"           , (BSD_t::hctx));
  BSD_t::bsd->SetBranchAddress("htrans"         , (BSD_t::htrans));
  BSD_t::bsd->SetBranchAddress("hps"            , (BSD_t::hps));

  return true;
}


//***************************************
void BSD_t::GetEntry(int i)
{
  BSD_t::bsd->GetEntry(i);
}


//***************************************
void BSD_t::MakeTree(string filename)
{
  BSD_t::outfile = new TFile(filename.c_str(),"recreate");
  if(!BSD_t::outfile) return;
  BSD_t::bsd_out = new TTree("bsd","bsd");
  BSD_t::bsd_out->Branch("is_bsd"         ,&(BSD_t::is_bsd),"is_bsd/I");
  BSD_t::bsd_out->Branch("nurun"          ,&(BSD_t::nurun),"nurun/I");
  BSD_t::bsd_out->Branch("midas_event"    ,&(BSD_t::midas_event),"midas_event/I");
  BSD_t::bsd_out->Branch("mrrun"          ,&(BSD_t::mrrun),"mrrun/I");
  BSD_t::bsd_out->Branch("mrshot"         ,&(BSD_t::mrshot),"mrshot/I");
  BSD_t::bsd_out->Branch("spillnum"       ,&(BSD_t::spillnum),"spillnum/I");
  BSD_t::bsd_out->Branch("trg_sec"        , (BSD_t::trg_sec),"trig_sec[3]/I");
  BSD_t::bsd_out->Branch("trg_nano"       , (BSD_t::trg_nano),"trig_nano[3]/I");
  BSD_t::bsd_out->Branch("gpsstat"        , (BSD_t::gpsstat),"gpsstat[2]/I");
  BSD_t::bsd_out->Branch("ct_np"          , (BSD_t::ct_np),"ct_np[5][9]/D");
  BSD_t::bsd_out->Branch("beam_time"      , (BSD_t::beam_time),"beam_time[5][9]/D");
  BSD_t::bsd_out->Branch("beam_flag"      , (BSD_t::beam_flag),"beam_flag[5][9]/I");
  BSD_t::bsd_out->Branch("hct"            , (BSD_t::hct),"hct[3][5]/D");
  BSD_t::bsd_out->Branch("tpos"           , (BSD_t::tpos),"tpos[2]/D");
  BSD_t::bsd_out->Branch("tdir"           , (BSD_t::tdir),"tdir[2]/D");
  BSD_t::bsd_out->Branch("tsize"          , (BSD_t::tsize),"tsize[2]/D");
  BSD_t::bsd_out->Branch("mumon"          , (BSD_t::mumon),"mumon[12]/D");
  BSD_t::bsd_out->Branch("otr"            , (BSD_t::otr),"otr[13]/D");
  BSD_t::bsd_out->Branch("good_gps_flag"  ,&(BSD_t::good_gps_flag),"good_gps_flag/I");
  BSD_t::bsd_out->Branch("trigger_flag"   ,&(BSD_t::trigger_flag),"trigger_flag/I");
  BSD_t::bsd_out->Branch("spill_flag"     ,&(BSD_t::spill_flag),"spill_flag/I");
  BSD_t::bsd_out->Branch("good_spill_flag",&(BSD_t::good_spill_flag),"good_spill_flag/I");
  BSD_t::bsd_out->Branch("target_eff"     , (BSD_t::target_eff),"target_eff[3]/D");
  BSD_t::bsd_out->Branch("run_type"       ,&(BSD_t::run_type),"run_type/I");
  BSD_t::bsd_out->Branch("magset_id"      ,&(BSD_t::magset_id),"magset_id/I");
  BSD_t::bsd_out->Branch("hctx"           , (BSD_t::hctx),"hctx[2][5]/D");
  BSD_t::bsd_out->Branch("htrans"         , (BSD_t::htrans),"htrans[2]/D");
  BSD_t::bsd_out->Branch("hps"            , (BSD_t::hps),"hps[2]/D");
}

void BSD_t::Clear()
{
}

void IngRecon_t::Clear(){
  unixtime     = -1;
  nd280nspill  = -1;
  for(int i=0;i<NUM_CYC;i++){
    num_inghit  [i] = -1;
    num_pmhit   [i] = -1;
    num_ingrecon[i] = -1;
    num_pmrecon [i] = -1;
  }

  for(int i=0;i<MAX_NUM_INGHIT;i++){ 
    inghit_cyc [i] = -1;
    inghit_view[i] = -1;
    inghit_pln [i] = -1;
    inghit_ch  [i] = -1;
    inghit_pe  [i] = -1.;
    inghit_time[i] = -1.;
    pmhit_cyc  [i] = -1;
    pmhit_view [i] = -1;
    pmhit_pln  [i] = -1;
    pmhit_ch   [i] = -1;
    pmhit_pe   [i] = -1.;
    pmhit_time [i] = -1.;
  }

  for(int i=0;i<MAX_NUM_INGRECON;i++){ 
    ingrecon_clstime [i] = -1.;
    ingrecon_view    [i] = -1 ;
    ingrecon_slope   [i] = -1.;
    ingrecon_angle   [i] = -1.;
    ingrecon_startz  [i] = -1.;
    ingrecon_startpln[i] = -1 ;
    ingrecon_startxy [i] = -1.;
    ingrecon_endz    [i] = -1.;
    ingrecon_endpln  [i] = -1 ;
    ingrecon_endxy   [i] = -1.;

    pmrecon_clstime [i] = -1.;
    pmrecon_view    [i] = -1 ;
    pmrecon_slope   [i] = -1.;
    pmrecon_angle   [i] = -1.;
    pmrecon_startz  [i] = -1.;
    pmrecon_startpln[i] = -1 ;
    pmrecon_startxy [i] = -1.;
    pmrecon_endz    [i] = -1.;
    pmrecon_endpln  [i] = -1 ;
    pmrecon_endxy   [i] = -1.;
  }
}
