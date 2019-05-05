#include <iostream>
#include "wgGetTree.h"
#include "wgTools.h"
#include "wgErrorCode.h"
#include "Const.h"
#include "TH1F.h"

using namespace std;

//************************************************************************

wgGetTree::wgGetTree(const string& root_file_name, Raw_t& rd) {
  this->Open(root_file_name);
  this->SetTreeFile(rd);
}

wgGetTree::wgGetTree(const string& root_file_name, Hit_t& hit) {
  this->Open(root_file_name);
  this->SetTreeFile(hit);
}
wgGetTree::wgGetTree(const string& root_file_name, Recon_t& recon) {
  this->Open(root_file_name);
  this->SetTreeFile(recon);
}
wgGetTree::wgGetTree(const string& root_file_name, Track_t& track) {
  this->Open(root_file_name);
  this->SetTreeFile(track);
}

wgGetTree::wgGetTree(const string& root_file_name, IngRecon_t& ingrecon) {
  this->Open(root_file_name);
  this->SetTreeFile(ingrecon);
}

//************************************************************************
void wgGetTree::Open(const string& root_file_name) {
  // Check if the ROOT file exists
  CheckExist Check;
  wgGetTree::finputname = root_file_name;
  if(!Check.RootFile(root_file_name))
    throw wgInvalidFile( "[wgGetTree::Open] failed to open " + root_file_name );
  // Check if the ROOT file is already opened
  try {
	if ( wgGetTree::finput != NULL ) {
	  if ( wgGetTree::finput->IsOpen() == kTRUE ) {
		Log.Write( "[wgGetTree::Open] " + root_file_name + "is already opened" );
		this->Close();
	  }
	}
  } catch (...) {}
  // Open the ROOT file and get the TTree called "tree"
  // Store the TFile in wgGetTree::finput and TTree in wgGetTree::tree
  wgGetTree::finput = new TFile(root_file_name.c_str(), "read");
  wgGetTree::tree_in = (TTree*) wgGetTree::finput->Get("tree");
}

//************************************************************************
void wgGetTree::Close() {
  if(wgGetTree::finput != NULL) {
	try {
	  wgGetTree::finput->Close();
	}
	catch (const exception &e) {
	  Log.eWrite( "[wgGetTree::Open] failed to close " + wgGetTree::finputname +
				  " : " + string(e.what()));
	}
    delete wgGetTree::finput;
  }
}

//************************************************************************
wgGetTree::~wgGetTree(){
  this->Close();
}

bool wgGetTree::BranchExists(const string& branch_name) {
   size_t n = tree_in->GetListOfBranches()->GetEntries();
   for (size_t i = 0; i < n; i++) {
	TBranch *br = dynamic_cast<TBranch*>(tree_in->GetListOfBranches()->At(i));
	if( br && br->GetName() == branch_name ) return true;
   }
   OperateString OptStr;
   Log.Write("[" + OptStr.GetName(finputname) + "][SetTreeFile] Branch " + branch_name + " not found");
  return false;
}

//************************************************************************
void wgGetTree::SetTreeFile(Raw_t& rdin){

  if(finput == NULL || finput->IsOpen() == kFALSE)
    throw wgInvalidFile("TFile " + finputname + " is not open!");
  try {
	if (BranchExists("spill"))
	  tree_in->SetBranchAddress("spill",       &rdin.spill);
	if (BranchExists("spill_flag"))
	  tree_in->SetBranchAddress("spill_flag",  &rdin.spill_flag);
	if (BranchExists("spill_mode"))
	  tree_in->SetBranchAddress("spill_mode",  &rdin.spill_mode);
	if (BranchExists("spill_count"))
	  tree_in->SetBranchAddress("spill_count", &rdin.spill_count);
	if (BranchExists("bcid"))
	  tree_in->SetBranchAddress("bcid",         rdin.bcid.data());
	if (BranchExists("charge"))
	  tree_in->SetBranchAddress("charge",       rdin.charge.data());
	if (BranchExists("time"))
	  tree_in->SetBranchAddress("time",         rdin.time.data());
	if (BranchExists("gs"))
	  tree_in->SetBranchAddress("gs",           rdin.gs.data());
	if (BranchExists("hit"))
	  tree_in->SetBranchAddress("hit",          rdin.hit.data());
	if (BranchExists("chipid"))
	  tree_in->SetBranchAddress("chipid",       rdin.chipid.data());
	if (BranchExists("col"))
	  tree_in->SetBranchAddress("col",          rdin.col.data());
	if (BranchExists("chipch"))
	  tree_in->SetBranchAddress("chipch",       rdin.chipch.data());
	if (BranchExists("chip"))
	  tree_in->SetBranchAddress("chip",         rdin.chip.data());
	if (BranchExists("debug"))
	  tree_in->SetBranchAddress("debug",        rdin.debug.data());
	if (BranchExists("view"))
	  tree_in->SetBranchAddress("view",        &rdin.view);
	if (BranchExists("pln"))
	  tree_in->SetBranchAddress("pln",          rdin.pln.data());
	if (BranchExists("ch"))
	  tree_in->SetBranchAddress("ch",           rdin.ch.data());
	if (BranchExists("grid"))
	  tree_in->SetBranchAddress("grid",         rdin.grid.data());
	if (BranchExists("x"))
	  tree_in->SetBranchAddress("x",            rdin.x.data());
	if (BranchExists("y"))
	  tree_in->SetBranchAddress("y",            rdin.y.data());
	if (BranchExists("z"))
	  tree_in->SetBranchAddress("z",            rdin.z.data());
	if (BranchExists("time_ns"))
	  tree_in->SetBranchAddress("time_ns",      rdin.time_ns.data());
	if (BranchExists("pe"))
	  tree_in->SetBranchAddress("pe",           rdin.pe.data());
	if (BranchExists("gain"))
	  tree_in->SetBranchAddress("gain",         rdin.gain.data());
	if (BranchExists("pedestal"))
	  tree_in->SetBranchAddress("pedestal",     rdin.pedestal.data());
	if (BranchExists("ped_nohit"))
	  tree_in->SetBranchAddress("ped_nohit",    rdin.ped_nohit.data());
	if (BranchExists("tdc_slope"))
	  tree_in->SetBranchAddress("tdc_slope",    rdin.tdc_slope.data());
	if (BranchExists("tdc_intcpt"))
	  tree_in->SetBranchAddress("tdc_intcpt",   rdin.tdc_intcpt.data());
  }	catch (const exception &e) {
    throw wgElementNotFound( "[wgGetTree::Open] failed to get the TTree from"
							 + wgGetTree::finputname + " : " + string(e.what()));
  }
}

//************************************************************************
void wgGetTree::SetTreeFile(Hit_t& hit){
  
  if(wgGetTree::finput == NULL || wgGetTree::finput->IsOpen() == kFALSE)
    throw wgInvalidFile("TFile " + wgGetTree::finputname + " is not open!");
  try {
	tree_in->SetBranchAddress("num_hits"         ,&hit.num_hits          );
	tree_in->SetBranchAddress("num_bcid_cluster" ,&hit.num_bcid_cluster  );
	tree_in->SetBranchAddress("num_bcid_hits"    , hit.num_bcid_hits     );
	tree_in->SetBranchAddress("clustered_bcid"   , hit.clustered_bcid    );
	tree_in->SetBranchAddress("clustered_hitid"  , hit.clustered_hitid   );
	tree_in->SetBranchAddress("hit_bcid"         , hit.hit_bcid          );
	tree_in->SetBranchAddress("hit_view"         , hit.hit_view          );
	tree_in->SetBranchAddress("hit_pln"          , hit.hit_pln           );
	tree_in->SetBranchAddress("hit_ch"           , hit.hit_ch            );
	tree_in->SetBranchAddress("hit_dif"          , hit.hit_dif           );
	tree_in->SetBranchAddress("hit_chip"         , hit.hit_chip          );
	tree_in->SetBranchAddress("hit_chipch"       , hit.hit_chipch        );
	tree_in->SetBranchAddress("hit_sca"          , hit.hit_sca           );
	tree_in->SetBranchAddress("hit_adc"          , hit.hit_adc           );
	tree_in->SetBranchAddress("hit_gs"           , hit.hit_gs            );
	tree_in->SetBranchAddress("hit_tdc"          , hit.hit_tdc           );
	tree_in->SetBranchAddress("hit_pe"           , hit.hit_pe            );
	tree_in->SetBranchAddress("hit_time"         , hit.hit_time          );
	tree_in->SetBranchAddress("hit_pe_permm"     , hit.hit_pe_permm      );
	tree_in->SetBranchAddress("hit_pathlength"   , hit.hit_pathlength    );
	tree_in->SetBranchAddress("hit_ontrack"      , hit.hit_ontrack       );
	tree_in->SetBranchAddress("hit_grid"         , hit.hit_grid          );
  }	catch (const exception &e) {
    throw wgElementNotFound( "[wgGetTree::Open] failed to get the TTree from"
							 + wgGetTree::finputname + " : " + string(e.what()));
  }
}

//************************************************************************
bool wgGetTree::MakeTreeFile(const string& str, Hit_t& hit){
  if(!wgGetTree::foutput){
    CheckExist *Check = new CheckExist;
    wgGetTree::foutputname = str;
    if(!Check->RootFile(str)){
      Log.eWrite( "ERROR!! FAIL TO SET TREEFILE" );
      return false;
    }
    delete Check;
    wgGetTree::foutput  = new TFile(str.c_str(),"recreate");
    wgGetTree::tree_out = new TTree("tree","tree");
  }
  tree_out->Branch("num_hits"        ,&hit.num_hits  ,"num_hits/I");
  tree_out->Branch("num_bcid_cluster",&hit.num_bcid_cluster,"num_bcid_cluster/I");
  tree_out->Branch("num_bcid_hits"   , hit.num_bcid_hits ,Form("num_bcid_hits[%d]/I"  ,MAX_NUM_BCID_CLUSTER));
  tree_out->Branch("clustered_bcid"  , hit.clustered_bcid,Form("clustered_bcid[%d]/I" ,MAX_NUM_BCID_CLUSTER));
  tree_out->Branch("hit_bcid"        , hit.hit_bcid  ,Form("hit_bcid[%d]/I"  ,MAX_NUM_HIT));
  tree_out->Branch("hit_view"        , hit.hit_view  ,Form("hit_view[%d]/I"  ,MAX_NUM_HIT));
  tree_out->Branch("hit_pln"         , hit.hit_pln   ,Form("hit_pln[%d]/I"   ,MAX_NUM_HIT));
  tree_out->Branch("hit_ch"          , hit.hit_ch    ,Form("hit_ch[%d]/I"    ,MAX_NUM_HIT));
  tree_out->Branch("hit_dif"         , hit.hit_dif   ,Form("hit_dif[%d]/I"   ,MAX_NUM_HIT));
  tree_out->Branch("hit_chip"        , hit.hit_chip  ,Form("hit_chip[%d]/I"  ,MAX_NUM_HIT));
  tree_out->Branch("hit_chipch"      , hit.hit_chipch,Form("hit_chipch[%d]/I",MAX_NUM_HIT));
  tree_out->Branch("hit_sca"         , hit.hit_sca   ,Form("hit_sca[%d]/I"   ,MAX_NUM_HIT));
  tree_out->Branch("hit_adc"         , hit.hit_adc   ,Form("hit_adc[%d]/I"   ,MAX_NUM_HIT));
  tree_out->Branch("hit_gs"          , hit.hit_gs    ,Form("hit_gs[%d]/I"    ,MAX_NUM_HIT));
  tree_out->Branch("hit_tdc"         , hit.hit_tdc   ,Form("hit_tdc[%d]/I"   ,MAX_NUM_HIT));
  tree_out->Branch("hit_pe"          , hit.hit_pe    ,Form("hit_pe[%d]/D"    ,MAX_NUM_HIT));
  tree_out->Branch("hit_time"        , hit.hit_time  ,Form("hit_time[%d]/D"  ,MAX_NUM_HIT));
  return true;
}

//************************************************************************
void wgGetTree::SetTreeFile(Recon_t& recon){
  if(wgGetTree::finput == NULL || wgGetTree::finput->IsOpen() == kFALSE)
    throw wgInvalidFile("TFile " + wgGetTree::finputname + " is not open!");
  try {
	tree_in->SetBranchAddress("num_recon"       ,&recon.num_recon       );
	tree_in->SetBranchAddress("num_recon_hits"  , recon.num_recon_hits  );
	tree_in->SetBranchAddress("recon_hits_hitid", recon.recon_hits_hitid);
	tree_in->SetBranchAddress("recon_bcid_id"   , recon.recon_bcid_id    );
	tree_in->SetBranchAddress("recon_start_z"   , recon.recon_start_z   );
	tree_in->SetBranchAddress("recon_stop_z"    , recon.recon_stop_z    );
	tree_in->SetBranchAddress("recon_start_pln" , recon.recon_start_pln );
	tree_in->SetBranchAddress("recon_stop_pln"  , recon.recon_stop_pln  );
	tree_in->SetBranchAddress("recon_start_ch"  , recon.recon_start_ch  );
	tree_in->SetBranchAddress("recon_stop_ch"   , recon.recon_stop_ch   );
	tree_in->SetBranchAddress("recon_start_xy"  , recon.recon_start_xy  );
	tree_in->SetBranchAddress("recon_stop_xy"   , recon.recon_stop_xy   );
	tree_in->SetBranchAddress("recon_slope"     , recon.recon_slope     );
	tree_in->SetBranchAddress("recon_intercept" , recon.recon_intercept );
	tree_in->SetBranchAddress("recon_pathlength", recon.recon_pathlength);
	tree_in->SetBranchAddress("recon_total_pe"  , recon.recon_total_pe  );
	tree_in->SetBranchAddress("recon_mean_dedx" , recon.recon_mean_dedx );
	tree_in->SetBranchAddress("recon_mean_time" , recon.recon_mean_time );
	tree_in->SetBranchAddress("recon_view"      , recon.recon_view      );
	tree_in->SetBranchAddress("recon_bcid"      , recon.recon_bcid      );
	tree_in->SetBranchAddress("recon_veto"      , recon.recon_veto      );
	tree_in->SetBranchAddress("recon_sideescape", recon.recon_sideescape);
	tree_in->SetBranchAddress("recon_cos"       , recon.recon_cos       );
	tree_in->SetBranchAddress("recon_len"       , recon.recon_len       );
  }	catch (const exception &e) {
    throw wgElementNotFound( "[wgGetTree::Open] failed to get the TTree from"
							 + wgGetTree::finputname + " : " + string(e.what()));
  }
}

//************************************************************************
bool wgGetTree::MakeTreeFile(const string& str, Recon_t& recon){
  if(!wgGetTree::foutput){
    CheckExist *Check = new CheckExist;
    wgGetTree::foutputname = str;
    if(!Check->RootFile(str)){
      Log.eWrite( "ERROR!! FAIL TO SET TREEFILE" );
      return false;
    }
    delete Check;
    wgGetTree::foutput  = new TFile(str.c_str(),"recreate");
    wgGetTree::tree_out = new TTree("tree","tree");
  }
  tree_out->Branch("recon_hits_hitid", recon.recon_hits_hitid,Form("recon_hits_hitid[%d][%d]/I",MAX_NUM_TRACK,MAX_NUM_TRACKHIT)); 
  tree_out->Branch("recon_start_z"   , recon.recon_start_z   ,Form("recon_start_z[%d]/D"   ,MAX_NUM_TRACK)); 
  tree_out->Branch("recon_stop_z"    , recon.recon_stop_z    ,Form("recon_stop_z[%d]/D"    ,MAX_NUM_TRACK)); 
  tree_out->Branch("recon_start_pln" , recon.recon_start_pln ,Form("recon_start_pln[%d]/D"   ,MAX_NUM_TRACK)); 
  tree_out->Branch("recon_stop_pln"  , recon.recon_stop_pln  ,Form("recon_stop_pln[%d]/D"    ,MAX_NUM_TRACK)); 
  tree_out->Branch("recon_start_ch"  , recon.recon_start_ch  ,Form("recon_start_ch[%d]/D"   ,MAX_NUM_TRACK)); 
  tree_out->Branch("recon_stop_ch"   , recon.recon_stop_ch   ,Form("recon_stop_ch[%d]/D"    ,MAX_NUM_TRACK)); 
  tree_out->Branch("recon_start_xy"  , recon.recon_start_xy  ,Form("recon_start_xy[%d]/D"  ,MAX_NUM_TRACK)); 
  tree_out->Branch("recon_stop_xy"   , recon.recon_stop_xy   ,Form("recon_stop_xy[%d]/D"   ,MAX_NUM_TRACK)); 
  tree_out->Branch("recon_slope"     , recon.recon_slope     ,Form("recon_slope[%d]/D"     ,MAX_NUM_TRACK));   
  tree_out->Branch("recon_intercept" , recon.recon_intercept ,Form("recon_intercept [%d]/D",MAX_NUM_TRACK)); 
  tree_out->Branch("recon_pathlength", recon.recon_pathlength,Form("recon_pathlength[%d]/D",MAX_NUM_TRACK)); 
  tree_out->Branch("recon_total_pe " , recon.recon_total_pe  ,Form("recon_total_pe[%d]/D"  ,MAX_NUM_TRACK));  
  tree_out->Branch("recon_mean_dedx" , recon.recon_mean_dedx ,Form("recon_mean_dedx[%d]/D" ,MAX_NUM_TRACK));  
  tree_out->Branch("recon_mean_time" , recon.recon_mean_time ,Form("recon_mean_time[%d]/D" ,MAX_NUM_TRACK));  
  tree_out->Branch("recon_view"      , recon.recon_view      ,Form("recon_view[%d]/I"      ,MAX_NUM_TRACK));  
  tree_out->Branch("recon_bcid"      , recon.recon_bcid      ,Form("recon_bcid[%d]/I"      ,MAX_NUM_TRACK)); 
  tree_out->Branch("recon_veto"      , recon.recon_veto      ,Form("recon_veto[%d]/I"      ,MAX_NUM_TRACK)); 
  tree_out->Branch("recon_sideescape", recon.recon_sideescape,Form("recon_sideescape[%d]/I",MAX_NUM_TRACK)); 
  tree_out->Branch("recon_cos"       , recon.recon_cos       ,Form("recon_cos[%d]/D"       ,MAX_NUM_TRACK));   
  tree_out->Branch("recon_len"       , recon.recon_len       ,Form("recon_len[%d]/D"       ,MAX_NUM_TRACK));   

  return true;
}

//************************************************************************
void wgGetTree::SetTreeFile(Track_t& track){
  if(wgGetTree::finput == NULL || wgGetTree::finput->IsOpen() == kFALSE)
    throw wgInvalidFile("TFile " + wgGetTree::finputname + " is not open!");
  try {
	tree_in->SetBranchAddress("num_track"       ,&track.num_track       );
	tree_in->SetBranchAddress("num_trackid"     ,&track.num_trackid     );
	tree_in->SetBranchAddress("num_track_recon" , track.num_track_recon );
	tree_in->SetBranchAddress("num_track_hits"  , track.num_track_hits  );
	tree_in->SetBranchAddress("track_hits_hitid", track.track_hits_hitid);
	tree_in->SetBranchAddress("track_recon_id"  , track.track_recon_id  );
	tree_in->SetBranchAddress("track_start_z"   , track.track_start_z   );
	tree_in->SetBranchAddress("track_stop_z"    , track.track_stop_z    );
	tree_in->SetBranchAddress("track_start_pln" , track.track_start_pln );
	tree_in->SetBranchAddress("track_stop_pln"  , track.track_stop_pln  );
	tree_in->SetBranchAddress("track_start_ch"  , track.track_start_ch  );
	tree_in->SetBranchAddress("track_stop_ch"   , track.track_stop_ch   );
	tree_in->SetBranchAddress("track_start_xy"  , track.track_start_xy  );
	tree_in->SetBranchAddress("track_stop_xy"   , track.track_stop_xy   );
	tree_in->SetBranchAddress("track_slope"     , track.track_slope     );
	tree_in->SetBranchAddress("track_intercept" , track.track_intercept );
	tree_in->SetBranchAddress("track_pathlength", track.track_pathlength);
	tree_in->SetBranchAddress("track_total_pe"  , track.track_total_pe  );
	tree_in->SetBranchAddress("track_mean_dedx" , track.track_mean_dedx );
	tree_in->SetBranchAddress("track_mean_time" , track.track_mean_time );
	tree_in->SetBranchAddress("track_view"      , track.track_view      );
	tree_in->SetBranchAddress("track_bcid"      , track.track_bcid      );
	tree_in->SetBranchAddress("track_veto"      , track.track_veto      );
	tree_in->SetBranchAddress("track_sideescape", track.track_sideescape);
	tree_in->SetBranchAddress("track_cos_zen"   , track.track_cos_zen   );
	tree_in->SetBranchAddress("track_cos_azi"   , track.track_cos_azi   );
	tree_in->SetBranchAddress("track_len"       , track.track_len       );
	tree_in->SetBranchAddress("track_id"        , track.track_id        );
  }	catch (const exception &e) {
    throw wgElementNotFound( "[wgGetTree::Open] failed to get the TTree from"
							 + wgGetTree::finputname + " : " + string(e.what()));
  }
}

//************************************************************************
bool wgGetTree::MakeTreeFile(const string& str, Track_t& track){
  if(!wgGetTree::foutput){
    CheckExist *Check = new CheckExist;
    wgGetTree::foutputname = str;
    if(!Check->RootFile(str)){
      Log.eWrite( "ERROR!! FAIL TO SET TREEFILE" );
      return false;
    }
    delete Check;
    wgGetTree::foutput  = new TFile(str.c_str(),"recreate");
    wgGetTree::tree_out = new TTree("tree","tree");
  }
  tree_out->Branch("num_track"       ,&track.num_track       ,"num_track/I"); 
  tree_out->Branch("num_trackid"     ,&track.num_trackid     ,"num_trackid/I"); 
  tree_out->Branch("num_track_recon" , track.num_track_recon ,Form("num_track_recon[%d]/I"  ,MAX_NUM_TRACK)); 
  tree_out->Branch("track_recon_id"  , track.track_recon_id  ,Form("track_recon_id[%d][%d]/I",MAX_NUM_TRACK,MAX_NUM_TRACK)); 
  tree_out->Branch("track_start_z"   , track.track_start_z   ,Form("track_start_z[%d]/D"   ,MAX_NUM_TRACK)); 
  tree_out->Branch("track_stop_z"    , track.track_stop_z    ,Form("track_stop_z[%d]/D"    ,MAX_NUM_TRACK)); 
  tree_out->Branch("track_start_pln" , track.track_start_pln ,Form("track_start_pln[%d]/D" ,MAX_NUM_TRACK)); 
  tree_out->Branch("track_stop_pln"  , track.track_stop_pln  ,Form("track_stop_pln[%d]/D"  ,MAX_NUM_TRACK)); 
  tree_out->Branch("track_start_ch"  , track.track_start_ch  ,Form("track_start_ch[%d]/D"  ,MAX_NUM_TRACK)); 
  tree_out->Branch("track_stop_ch"   , track.track_stop_ch   ,Form("track_stop_ch[%d]/D"   ,MAX_NUM_TRACK)); 
  tree_out->Branch("track_start_xy"  , track.track_start_xy  ,Form("track_start_xy[%d]/D"  ,MAX_NUM_TRACK)); 
  tree_out->Branch("track_stop_xy"   , track.track_stop_xy   ,Form("track_stop_xy[%d]/D"   ,MAX_NUM_TRACK)); 
  tree_out->Branch("track_slope"     , track.track_slope     ,Form("track_slope[%d]/D"     ,MAX_NUM_TRACK));   
  tree_out->Branch("track_intercept" , track.track_intercept ,Form("track_intercept [%d]/D",MAX_NUM_TRACK)); 
  tree_out->Branch("track_pathlength", track.track_pathlength,Form("track_pathlength[%d]/D",MAX_NUM_TRACK)); 
  tree_out->Branch("track_total_pe"  , track.track_total_pe  ,Form("track_total_pe[%d]/D"  ,MAX_NUM_TRACK));  
  tree_out->Branch("track_mean_dedx" , track.track_mean_dedx ,Form("track_mean_dedx[%d]/D" ,MAX_NUM_TRACK));  
  tree_out->Branch("track_mean_time" , track.track_mean_time ,Form("track_mean_time[%d]/D" ,MAX_NUM_TRACK));  
  tree_out->Branch("track_view"      , track.track_view      ,Form("track_view[%d]/I"      ,MAX_NUM_TRACK));  
  tree_out->Branch("track_bcid"      , track.track_bcid      ,Form("track_bcid[%d]/I"      ,MAX_NUM_TRACK)); 
  tree_out->Branch("track_veto"      , track.track_veto      ,Form("track_veto[%d]/I"      ,MAX_NUM_TRACK)); 
  tree_out->Branch("track_sideescape", track.track_sideescape,Form("track_sideescape[%d]/I",MAX_NUM_TRACK)); 
  tree_out->Branch("track_cos_zen"   , track.track_cos_zen   ,Form("track_cos_zen[%d]/D"   ,MAX_NUM_TRACK));   
  tree_out->Branch("track_cos_azi"   , track.track_cos_azi   ,Form("track_cos_azi[%d]/D"   ,MAX_NUM_TRACK));   
  tree_out->Branch("track_len"       , track.track_len       ,Form("track_len[%d]/D"       ,MAX_NUM_TRACK));   
  tree_out->Branch("track_id"        , track.track_id        ,Form("track_id[%d]/I"        ,MAX_NUM_TRACK));   

  return true;
}

//************************************************************************
void wgGetTree::SetTreeFile(IngRecon_t& IngRecon){
  if(wgGetTree::finput == NULL || wgGetTree::finput->IsOpen() == kFALSE)
    throw wgInvalidFile("TFile " + wgGetTree::finputname + " is not open!");
  try {
	tree_in-> SetBranchAddress("unixtime"    ,&IngRecon.unixtime   );
	tree_in-> SetBranchAddress("nd280nspill" ,&IngRecon.nd280nspill);

	tree_in-> SetBranchAddress("num_inghit"  ,IngRecon.num_inghit  );
	tree_in-> SetBranchAddress("num_pmhit"   ,IngRecon.num_pmhit   );
	tree_in-> SetBranchAddress("num_ingrecon",IngRecon.num_ingrecon);
	tree_in-> SetBranchAddress("num_pmrecon" ,IngRecon.num_pmrecon );

	tree_in-> SetBranchAddress("inghit_cyc"  , IngRecon.inghit_cyc);
	tree_in-> SetBranchAddress("inghit_view" , IngRecon.inghit_view);
	tree_in-> SetBranchAddress("inghit_pln"  , IngRecon.inghit_pln);
	tree_in-> SetBranchAddress("inghit_ch"   , IngRecon.inghit_ch);
	tree_in-> SetBranchAddress("inghit_pe"   , IngRecon.inghit_pe);
	tree_in-> SetBranchAddress("inghit_time" , IngRecon.inghit_time);
	tree_in-> SetBranchAddress("pmhit_cyc"   , IngRecon.pmhit_cyc);
	tree_in-> SetBranchAddress("pmhit_view"  , IngRecon.pmhit_view);
	tree_in-> SetBranchAddress("pmhit_pln"   , IngRecon.pmhit_pln);
	tree_in-> SetBranchAddress("pmhit_ch"    , IngRecon.pmhit_ch);
	tree_in-> SetBranchAddress("pmhit_pe"    , IngRecon.pmhit_pe);
	tree_in-> SetBranchAddress("pmhit_time"  , IngRecon.pmhit_time);

	tree_in-> SetBranchAddress("ingrecon_clstime"  , IngRecon.ingrecon_clstime);
	tree_in-> SetBranchAddress("ingrecon_view"     , IngRecon.ingrecon_view);
	tree_in-> SetBranchAddress("ingrecon_slope"    , IngRecon.ingrecon_slope);
	tree_in-> SetBranchAddress("ingrecon_angle"    , IngRecon.ingrecon_angle);
	tree_in-> SetBranchAddress("ingrecon_startz"   , IngRecon.ingrecon_startz);
	tree_in-> SetBranchAddress("ingrecon_startpln" , IngRecon.ingrecon_startpln);
	tree_in-> SetBranchAddress("ingrecon_startxy"  , IngRecon.ingrecon_startxy);
	tree_in-> SetBranchAddress("ingrecon_endz"     , IngRecon.ingrecon_endz);
	tree_in-> SetBranchAddress("ingrecon_endpln"   , IngRecon.ingrecon_endpln);
	tree_in-> SetBranchAddress("ingrecon_endxy"    , IngRecon.ingrecon_endxy);

	tree_in-> SetBranchAddress("pmrecon_clstime"   , IngRecon.pmrecon_clstime);
	tree_in-> SetBranchAddress("pmrecon_view"      , IngRecon.pmrecon_view);
	tree_in-> SetBranchAddress("pmrecon_slope"     , IngRecon.pmrecon_slope);
	tree_in-> SetBranchAddress("pmrecon_angle"     , IngRecon.pmrecon_angle);
	tree_in-> SetBranchAddress("pmrecon_startz"    , IngRecon.pmrecon_startz);
	tree_in-> SetBranchAddress("pmrecon_startpln"  , IngRecon.pmrecon_startpln);
	tree_in-> SetBranchAddress("pmrecon_startxy"   , IngRecon.pmrecon_startxy);
	tree_in-> SetBranchAddress("pmrecon_endz"      , IngRecon.pmrecon_endz);
	tree_in-> SetBranchAddress("pmrecon_endpln"    , IngRecon.pmrecon_endpln);
	tree_in-> SetBranchAddress("pmrecon_endxy"     , IngRecon.pmrecon_endxy);

  }	catch (const exception &e) {
    throw wgElementNotFound( "[wgGetTree::Open] failed to get the TTree from"
							 + wgGetTree::finputname + " : " + string(e.what()));
  }
}

//************************************************************************
void wgGetTree::GetEntry(int i){
  wgGetTree::tree_in->GetEntry(i);
}

//************************************************************************
double wgGetTree::GetStartTime(){
  TH1F* h=(TH1F*)wgGetTree::finput->Get("start_time");
  double ret = h->GetXaxis()->GetBinCenter(h->GetMaximumBin());
  delete h;
  return ret;
}

//************************************************************************
double wgGetTree::GetStopTime(){
  TH1F* h;
  h=(TH1F*)wgGetTree::finput->Get("stop_time");
  double ret = h->GetXaxis()->GetBinCenter(h->GetMaximumBin());
  delete h;
  return ret;
}

//************************************************************************
double wgGetTree::GetDataPacket(){
  TH1F* h;
  h=(TH1F*)wgGetTree::finput->Get("nb_data_pkts");
  double ret = h->GetXaxis()->GetBinCenter(h->GetMaximumBin());
  delete h;
  return ret;
}

//************************************************************************
double wgGetTree::GetLostPacket(){
  TH1F* h;
  h=(TH1F*)wgGetTree::finput->Get("nb_lost_pkts");
  double ret = h->GetXaxis()->GetBinCenter(h->GetMaximumBin());
  delete h;
  return ret;
}

//************************************************************************
TH1F* wgGetTree::GetHist_StartTime(){
  if (BranchExists("start_time"))
	return dynamic_cast<TH1F*>(wgGetTree::finput->Get("start_time"));
  else return NULL;
}

//************************************************************************
TH1F* wgGetTree::GetHist_StopTime(){
  if (BranchExists("stop_time"))
	return dynamic_cast<TH1F*>(wgGetTree::finput->Get("stop_time"));
  else return NULL;
}

//************************************************************************
TH1F* wgGetTree::GetHist_DataPacket(){
  if (BranchExists("nb_data_pkts"))
	return dynamic_cast<TH1F*>(wgGetTree::finput->Get("nb_data_pkts"));
  else return NULL;
}

//************************************************************************
TH1F* wgGetTree::GetHist_LostPacket(){
  if (BranchExists("nb__pktlosts"))
	return dynamic_cast<TH1F*>(wgGetTree::finput->Get("nb_lost_pkts"));
  else return NULL;
}
