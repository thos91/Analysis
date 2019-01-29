#include <iostream>
#include "wgGetTree.h"
#include "wgErrorCode.h"
#include "Const.h"
#include "TH1F.h"

using namespace std;

TFile* wgGetTree::finput;    
string wgGetTree::finputname;
TTree* wgGetTree::tree;

TFile* wgGetTree::foutput;    
string wgGetTree::foutputname;
TTree* wgGetTree::tree_out;

TFile* wgGetTree::ingfinput;    
string wgGetTree::ingfinputname;
TTree* wgGetTree::ingtree;

//************************************************************************
wgGetTree::wgGetTree(){
}

//************************************************************************
wgGetTree::wgGetTree(string& str){
  this->Open(str);
}

wgGetTree::wgGetTree(string& str,IngRecon_t& ingrecon){
  cout << "INGRID tree date to be opened." << endl;
  CheckExist *Check = new CheckExist;
  if(!Check->RootFile(str)){
    cout << "ERROR!! FAIL TO SET TREEFILE" <<endl;
  }
  else{
    wgGetTree::ingfinputname = str;
    if(wgGetTree::ingfinput){
      cout << "WARNING!! TFile is overwrited!" <<endl;
      this->Close();
    }
    wgGetTree::ingfinput = new TFile(str.c_str(),"read");
    wgGetTree::ingtree = (TTree*)wgGetTree::ingfinput->Get("tree");
    this->SetTreeFile(ingrecon);
  }
  delete Check;
}

//************************************************************************
bool wgGetTree::Open(string& str){
  CheckExist *Check = new CheckExist;
  wgGetTree::finputname = str;
  if(!Check->RootFile(str)){
    cout << "ERROR!! FAIL TO SET TREEFILE" <<endl;
    return false;
  }
  delete Check;
  if(wgGetTree::finput){
    cout << "WARNING!! TFile is overwrited!" <<endl;
    this->Close();
  }
  wgGetTree::finput = new TFile(str.c_str(),"read");
  wgGetTree::tree = (TTree*)wgGetTree::finput->Get("tree");
  return true;
}

//************************************************************************
void wgGetTree::Close(){
  if(wgGetTree::finput){
    wgGetTree::finput->Close();
    delete wgGetTree::finput;
  }
}

//************************************************************************
wgGetTree::~wgGetTree(){
  this->Close();
}

//************************************************************************
bool wgGetTree::SetTreeFile(Raw_t& rdin){
  if(!wgGetTree::finput){
    cout << "WARNING!! TFile is not open!" <<endl;
    return false;
  }
  tree->SetBranchAddress("spill",&rdin.spill);
  tree->SetBranchAddress("spill_flag",&rdin.spill_flag);
  tree->SetBranchAddress("spill_mode",&rdin.spill_mode);
  tree->SetBranchAddress("spill_count",&rdin.spill_count);
  tree->SetBranchAddress("bcid",rdin.bcid);
  tree->SetBranchAddress("charge",rdin.charge);
  tree->SetBranchAddress("time",rdin.time);
  tree->SetBranchAddress("gs",rdin.gs);
  tree->SetBranchAddress("hit",rdin.hit);
  tree->SetBranchAddress("chipid",rdin.chipid);
  tree->SetBranchAddress("col",rdin.col);
  tree->SetBranchAddress("chipch",rdin.chipch);
  tree->SetBranchAddress("chip",rdin.chip);
  tree->SetBranchAddress("debug",rdin.debug);
  tree->SetBranchAddress("view",&rdin.view);
  tree->SetBranchAddress("pln",rdin.pln);
  tree->SetBranchAddress("ch",rdin.ch);
  tree->SetBranchAddress("grid",rdin.grid);
  tree->SetBranchAddress("x", rdin.x);
  tree->SetBranchAddress("y", rdin.y);
  tree->SetBranchAddress("z", rdin.z);
  tree->SetBranchAddress("time_ns",rdin.time_ns);
  tree->SetBranchAddress("pe",rdin.pe);
  tree->SetBranchAddress("gain",rdin.gain);
  tree->SetBranchAddress("pedestal",rdin.pedestal);
  tree->SetBranchAddress("ped_nohit",rdin.ped_nohit);
  return true;
}

//************************************************************************
bool wgGetTree::SetTreeFile(Hit_t& hit){
  if(!wgGetTree::finput){
    cout << "WARNING!! TFile is not open!" <<endl;
    return false;
  }
  tree->SetBranchAddress("num_hits"         ,&hit.num_hits          );
  tree->SetBranchAddress("num_bcid_cluster" ,&hit.num_bcid_cluster  );
  tree->SetBranchAddress("num_bcid_hits"    , hit.num_bcid_hits     );
  tree->SetBranchAddress("clustered_bcid"   , hit.clustered_bcid    );
  tree->SetBranchAddress("clustered_hitid"  , hit.clustered_hitid   );
  tree->SetBranchAddress("hit_bcid"         , hit.hit_bcid          );
  tree->SetBranchAddress("hit_view"         , hit.hit_view          );
  tree->SetBranchAddress("hit_pln"          , hit.hit_pln           );
  tree->SetBranchAddress("hit_ch"           , hit.hit_ch            );
  tree->SetBranchAddress("hit_dif"          , hit.hit_dif           );
  tree->SetBranchAddress("hit_chip"         , hit.hit_chip          );
  tree->SetBranchAddress("hit_chipch"       , hit.hit_chipch        );
  tree->SetBranchAddress("hit_sca"          , hit.hit_sca           );
  tree->SetBranchAddress("hit_adc"          , hit.hit_adc           );
  tree->SetBranchAddress("hit_gs"           , hit.hit_gs            );
  tree->SetBranchAddress("hit_tdc"          , hit.hit_tdc           );
  tree->SetBranchAddress("hit_pe"           , hit.hit_pe            );
  tree->SetBranchAddress("hit_time"         , hit.hit_time          );
  tree->SetBranchAddress("hit_pe_permm"     , hit.hit_pe_permm      );
  tree->SetBranchAddress("hit_pathlength"   , hit.hit_pathlength    );
  tree->SetBranchAddress("hit_ontrack"      , hit.hit_ontrack       );
  tree->SetBranchAddress("hit_grid"         , hit.hit_grid          );
  return true;
}

//************************************************************************
bool wgGetTree::MakeTreeFile(string& str, Hit_t& hit){
  if(!wgGetTree::foutput){
    CheckExist *Check = new CheckExist;
    wgGetTree::foutputname = str;
    if(!Check->RootFile(str)){
      cout << "ERROR!! FAIL TO SET TREEFILE" <<endl;
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
bool wgGetTree::SetTreeFile(Recon_t& recon){
  if(!wgGetTree::finput){
    cout << "WARNING!! TFile is not open!" <<endl;
    return false;
  }
  tree->SetBranchAddress("num_recon"       ,&recon.num_recon       );
  tree->SetBranchAddress("num_recon_hits"  , recon.num_recon_hits  );
  tree->SetBranchAddress("recon_hits_hitid", recon.recon_hits_hitid);
  tree->SetBranchAddress("recon_bcid_id"   , recon.recon_bcid_id    );
  tree->SetBranchAddress("recon_start_z"   , recon.recon_start_z   );
  tree->SetBranchAddress("recon_stop_z"    , recon.recon_stop_z    );
  tree->SetBranchAddress("recon_start_pln" , recon.recon_start_pln );
  tree->SetBranchAddress("recon_stop_pln"  , recon.recon_stop_pln  );
  tree->SetBranchAddress("recon_start_ch"  , recon.recon_start_ch  );
  tree->SetBranchAddress("recon_stop_ch"   , recon.recon_stop_ch   );
  tree->SetBranchAddress("recon_start_xy"  , recon.recon_start_xy  );
  tree->SetBranchAddress("recon_stop_xy"   , recon.recon_stop_xy   );
  tree->SetBranchAddress("recon_slope"     , recon.recon_slope     );
  tree->SetBranchAddress("recon_intercept" , recon.recon_intercept );
  tree->SetBranchAddress("recon_pathlength", recon.recon_pathlength);
  tree->SetBranchAddress("recon_total_pe"  , recon.recon_total_pe  );
  tree->SetBranchAddress("recon_mean_dedx" , recon.recon_mean_dedx );
  tree->SetBranchAddress("recon_mean_time" , recon.recon_mean_time );
  tree->SetBranchAddress("recon_view"      , recon.recon_view      );
  tree->SetBranchAddress("recon_bcid"      , recon.recon_bcid      );
  tree->SetBranchAddress("recon_veto"      , recon.recon_veto      );
  tree->SetBranchAddress("recon_sideescape", recon.recon_sideescape);
  tree->SetBranchAddress("recon_cos"       , recon.recon_cos       );
  tree->SetBranchAddress("recon_len"       , recon.recon_len       );
  return true;
}

//************************************************************************
bool wgGetTree::MakeTreeFile(string& str, Recon_t& recon){
  if(!wgGetTree::foutput){
    CheckExist *Check = new CheckExist;
    wgGetTree::foutputname = str;
    if(!Check->RootFile(str)){
      cout << "ERROR!! FAIL TO SET TREEFILE" <<endl;
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
bool wgGetTree::SetTreeFile(Track_t& track){
  if(!wgGetTree::finput){
    cout << "WARNING!! TFile is not open!" <<endl;
    return false;
  }
  tree->SetBranchAddress("num_track"       ,&track.num_track       );
  tree->SetBranchAddress("num_trackid"     ,&track.num_trackid     );
  tree->SetBranchAddress("num_track_recon" , track.num_track_recon );
  tree->SetBranchAddress("num_track_hits"  , track.num_track_hits  );
  tree->SetBranchAddress("track_hits_hitid", track.track_hits_hitid);
  tree->SetBranchAddress("track_recon_id"  , track.track_recon_id  );
  tree->SetBranchAddress("track_start_z"   , track.track_start_z   );
  tree->SetBranchAddress("track_stop_z"    , track.track_stop_z    );
  tree->SetBranchAddress("track_start_pln" , track.track_start_pln );
  tree->SetBranchAddress("track_stop_pln"  , track.track_stop_pln  );
  tree->SetBranchAddress("track_start_ch"  , track.track_start_ch  );
  tree->SetBranchAddress("track_stop_ch"   , track.track_stop_ch   );
  tree->SetBranchAddress("track_start_xy"  , track.track_start_xy  );
  tree->SetBranchAddress("track_stop_xy"   , track.track_stop_xy   );
  tree->SetBranchAddress("track_slope"     , track.track_slope     );
  tree->SetBranchAddress("track_intercept" , track.track_intercept );
  tree->SetBranchAddress("track_pathlength", track.track_pathlength);
  tree->SetBranchAddress("track_total_pe"  , track.track_total_pe  );
  tree->SetBranchAddress("track_mean_dedx" , track.track_mean_dedx );
  tree->SetBranchAddress("track_mean_time" , track.track_mean_time );
  tree->SetBranchAddress("track_view"      , track.track_view      );
  tree->SetBranchAddress("track_bcid"      , track.track_bcid      );
  tree->SetBranchAddress("track_veto"      , track.track_veto      );
  tree->SetBranchAddress("track_sideescape", track.track_sideescape);
  tree->SetBranchAddress("track_cos_zen"   , track.track_cos_zen   );
  tree->SetBranchAddress("track_cos_azi"   , track.track_cos_azi   );
  tree->SetBranchAddress("track_len"       , track.track_len       );
  tree->SetBranchAddress("track_id"        , track.track_id        );
  return true;
}

//************************************************************************
bool wgGetTree::MakeTreeFile(string& str, Track_t& track){
  if(!wgGetTree::foutput){
    CheckExist *Check = new CheckExist;
    wgGetTree::foutputname = str;
    if(!Check->RootFile(str)){
      cout << "ERROR!! FAIL TO SET TREEFILE" <<endl;
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
bool wgGetTree::SetTreeFile(IngRecon_t& IngRecon){
  if(!wgGetTree::ingfinput){
    cout << "WARNING!! INGRID TFile is not open!" <<endl;
    return false;
  }
  ingtree-> SetBranchAddress("unixtime"    ,&IngRecon.unixtime   );
  ingtree-> SetBranchAddress("nd280nspill" ,&IngRecon.nd280nspill);

  ingtree-> SetBranchAddress("num_inghit"  ,IngRecon.num_inghit  );
  ingtree-> SetBranchAddress("num_pmhit"   ,IngRecon.num_pmhit   );
  ingtree-> SetBranchAddress("num_ingrecon",IngRecon.num_ingrecon);
  ingtree-> SetBranchAddress("num_pmrecon" ,IngRecon.num_pmrecon );

  ingtree-> SetBranchAddress("inghit_cyc"  , IngRecon.inghit_cyc);
  ingtree-> SetBranchAddress("inghit_view" , IngRecon.inghit_view);
  ingtree-> SetBranchAddress("inghit_pln"  , IngRecon.inghit_pln);
  ingtree-> SetBranchAddress("inghit_ch"   , IngRecon.inghit_ch);
  ingtree-> SetBranchAddress("inghit_pe"   , IngRecon.inghit_pe);
  ingtree-> SetBranchAddress("inghit_time" , IngRecon.inghit_time);
  ingtree-> SetBranchAddress("pmhit_cyc"   , IngRecon.pmhit_cyc);
  ingtree-> SetBranchAddress("pmhit_view"  , IngRecon.pmhit_view);
  ingtree-> SetBranchAddress("pmhit_pln"   , IngRecon.pmhit_pln);
  ingtree-> SetBranchAddress("pmhit_ch"    , IngRecon.pmhit_ch);
  ingtree-> SetBranchAddress("pmhit_pe"    , IngRecon.pmhit_pe);
  ingtree-> SetBranchAddress("pmhit_time"  , IngRecon.pmhit_time);

  ingtree-> SetBranchAddress("ingrecon_clstime"  , IngRecon.ingrecon_clstime);
  ingtree-> SetBranchAddress("ingrecon_view"     , IngRecon.ingrecon_view);
  ingtree-> SetBranchAddress("ingrecon_slope"    , IngRecon.ingrecon_slope);
  ingtree-> SetBranchAddress("ingrecon_angle"    , IngRecon.ingrecon_angle);
  ingtree-> SetBranchAddress("ingrecon_startz"   , IngRecon.ingrecon_startz);
  ingtree-> SetBranchAddress("ingrecon_startpln" , IngRecon.ingrecon_startpln);
  ingtree-> SetBranchAddress("ingrecon_startxy"  , IngRecon.ingrecon_startxy);
  ingtree-> SetBranchAddress("ingrecon_endz"     , IngRecon.ingrecon_endz);
  ingtree-> SetBranchAddress("ingrecon_endpln"   , IngRecon.ingrecon_endpln);
  ingtree-> SetBranchAddress("ingrecon_endxy"    , IngRecon.ingrecon_endxy);

  ingtree-> SetBranchAddress("pmrecon_clstime"   , IngRecon.pmrecon_clstime);
  ingtree-> SetBranchAddress("pmrecon_view"      , IngRecon.pmrecon_view);
  ingtree-> SetBranchAddress("pmrecon_slope"     , IngRecon.pmrecon_slope);
  ingtree-> SetBranchAddress("pmrecon_angle"     , IngRecon.pmrecon_angle);
  ingtree-> SetBranchAddress("pmrecon_startz"    , IngRecon.pmrecon_startz);
  ingtree-> SetBranchAddress("pmrecon_startpln"  , IngRecon.pmrecon_startpln);
  ingtree-> SetBranchAddress("pmrecon_startxy"   , IngRecon.pmrecon_startxy);
  ingtree-> SetBranchAddress("pmrecon_endz"      , IngRecon.pmrecon_endz);
  ingtree-> SetBranchAddress("pmrecon_endpln"    , IngRecon.pmrecon_endpln);
  ingtree-> SetBranchAddress("pmrecon_endxy"     , IngRecon.pmrecon_endxy);

  return true;
}

//************************************************************************
void wgGetTree::GetEntry(int i){
  wgGetTree::tree->GetEntry(i);
}

//************************************************************************
double wgGetTree::GetStartTime(){
  TH1F* h;
  h=(TH1F*)wgGetTree::finput->Get("start_time");
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
  TH1F* h;
  h=(TH1F*)wgGetTree::finput->Get("start_time");
  return h;
}

//************************************************************************
TH1F* wgGetTree::GetHist_StopTime(){
  TH1F* h;
  h=(TH1F*)wgGetTree::finput->Get("stop_time");
  return h;
}

//************************************************************************
TH1F* wgGetTree::GetHist_DataPacket(){
  TH1F* h;
  h=(TH1F*)wgGetTree::finput->Get("nb_data_pkts");
  return h;
}

//************************************************************************
TH1F* wgGetTree::GetHist_LostPacket(){
  TH1F* h;
  h=(TH1F*)wgGetTree::finput->Get("nb_lost_pkts");
  return h;
}
