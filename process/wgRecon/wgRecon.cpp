/* ***********************************************************************
 * Reconstruction program for wagasci 
 * Program : wagasci_recon_exe.cc
 * Name: Naruhiro Chikuma
 * Date: 2017-05-09 02:45:48
 * ********************************************************************** */
#include <iostream>

#include "wgReconClass.h"
#include "wgGetTree.h"
#include "wgErrorCode.h"
#include "wgTools.h"
#include "Const.h"

using namespace std;

int main(int argc, char* argv[]){

  int opt;
  string inputFileName("");
  string outputFile("");
  wgConst *con = new wgConst();
  con->GetENV();
  string outputDir    = con->RECON_DIRECTORY;
  string logoutputDir = con->LOG_DIRECTORY;
  string c_name1,c_name2;
  CheckExist *check = new CheckExist();
  Logger *Log = new Logger();
  
  Log->Initialize();

  while((opt = getopt(argc,argv, "f:o:h")) !=-1 ){
    switch(opt){
      case 'f':
        inputFileName=optarg;
        c_name1=Form("%s_dif_1_1_1_tree.root",inputFileName.c_str());
        c_name2=Form("%s_dif_1_1_2_tree.root",inputFileName.c_str());
        if(!check->RootFile(c_name1)||!check->RootFile(c_name2)){
          cout << "!!Error!! inputFile" << c_name1 << " / " << c_name2 << " don't exist!" << endl;
          Log->eWrite(Form("[%s][wgRecon]!!target is wrong!!",inputFileName.c_str()));
          return 1;
        }
        break;
      case 'o': 
        outputDir=optarg;
        if(!check->Dir(outputDir)){
          cout << "!!Error!! output directory:" << outputDir<< " don't exist!" << endl;
          Log->eWrite(Form("[%s][wgRecon]!!output directory is wrong!!",outputDir.c_str()));
          return 1;
        }
        break;
      case 'h':
        cout <<"this program is for reconstrancting track from tree file."<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose inputfile you wanna read(must)"<<endl;
        cout <<"  -o (char*) : choose output directory (default:" << con->RECON_DIRECTORY << ")"<<endl;
        exit(0);
      default:
        cout <<"this program is for reconstrancting track from tree file."<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose inputfile you wanna read(must)"<<endl;
        cout <<"  -o (char*) : choose output directory (default:" << con->RECON_DIRECTORY << ")"<<endl;
        exit(0);
    }
  }

  delete con;
 
  if(inputFileName==""){
    cout << "!!ERROR!! please input filename." <<endl;
    cout << "if you don't know how to input, please see help."<<endl;
    cout << "help : ./wgRecon -h" <<endl;
    Log->eWrite(Form("[%s][wgRecon]!!No input file!!",inputFileName.c_str()));
    exit(1);
  }

  // =====================
  // initialize reconstruction Class
  wgRecon wg_rec;
  
  // =====================
  // open ROOT Tree files

  std::string filename1 = Form("%s_dif_1_1_1_tree.root",
      inputFileName.c_str());
  std::string filename2 = Form("%s_dif_1_1_2_tree.root",
      inputFileName.c_str());
  
  TFile* datafile1 = new TFile(filename1.c_str(),"read");
  if(!datafile1){
    std::cerr << "Cannot open file : " << filename1.c_str() << std::endl;
    exit(1);
  }
  TFile* datafile2 = new TFile(filename2.c_str(),"read");
  if(!datafile2){
    std::cerr << "Cannot open file : " << filename2.c_str() << std::endl;
    exit(1);
  }

  TTree* tree[NumDif];
  TH1D * start_time;
  TH1D * stop_time;
  TH1D * nb_data_pkts;
  TH1D * nb_lost_pkts;
  for(int i=0;i<NumDif;i++){
    if(i==0){
      datafile1->cd();
      tree[i] = (TTree *)datafile1->Get("tree");
      start_time   = (TH1D*) datafile1->Get("start_time");
      stop_time    = (TH1D*) datafile1->Get("stop_time");
      nb_data_pkts = (TH1D*) datafile1->Get("nb_data_pkts");
      nb_lost_pkts = (TH1D*) datafile1->Get("nb_lost_pkts");
    } 
    if(i==1){
      datafile2->cd();
      tree[i] = (TTree *)datafile2->Get("tree");
    } 
    tree[i]->SetBranchAddress("charge"     , wg_rec.type_raw[i].charge    );
    tree[i]->SetBranchAddress("time"       , wg_rec.type_raw[i].time      );
    tree[i]->SetBranchAddress("gain"       , wg_rec.type_raw[i].gain      );
    tree[i]->SetBranchAddress("hit"        , wg_rec.type_raw[i].hit       );
    tree[i]->SetBranchAddress("bcid"       , wg_rec.type_raw[i].bcid      );
    tree[i]->SetBranchAddress("col"        , wg_rec.type_raw[i].col       );
    tree[i]->SetBranchAddress("ch"         , wg_rec.type_raw[i].ch        );
    tree[i]->SetBranchAddress("chip"       , wg_rec.type_raw[i].chip      );
    tree[i]->SetBranchAddress("chipid"     , wg_rec.type_raw[i].chipid    );
    tree[i]->SetBranchAddress("spill"      ,&wg_rec.type_raw[i].spill     );
    tree[i]->SetBranchAddress("spill_flag" ,&wg_rec.type_raw[i].spill_flag);
    tree[i]->SetBranchAddress("spill_mode" ,&wg_rec.type_raw[i].spill_mode);
    tree[i]->SetBranchAddress("spill_count",&wg_rec.type_raw[i].spill_count);
    tree[i]->SetBranchAddress("debug"      , wg_rec.type_raw[i].debug     );
    tree[i]->SetBranchAddress("view"       ,&wg_rec.type_raw[i].view);
    tree[i]->SetBranchAddress("pln"        , wg_rec.type_raw[i].pln);
    tree[i]->SetBranchAddress("ch"         , wg_rec.type_raw[i].ch);
    tree[i]->SetBranchAddress("grid"       , wg_rec.type_raw[i].grid);
    tree[i]->SetBranchAddress("x"          , wg_rec.type_raw[i].x);
    tree[i]->SetBranchAddress("y"          , wg_rec.type_raw[i].y);
    tree[i]->SetBranchAddress("z"          , wg_rec.type_raw[i].z);
    tree[i]->SetBranchAddress("pe"         , wg_rec.type_raw[i].pe);
    tree[i]->SetBranchAddress("gain"       , wg_rec.type_raw[i].gain);
    tree[i]->SetBranchAddress("pedestal"   , wg_rec.type_raw[i].pedestal);
    tree[i]->SetBranchAddress("ped_nohit"  , wg_rec.type_raw[i].ped_nohit);
    tree[i]->SetBranchAddress("time"       , wg_rec.type_raw[i].time);
    tree[i]->SetBranchAddress("time_ns"    , wg_rec.type_raw[i].time_ns);
  }

  int nevt1 = tree[0]->GetEntries();
  int nevt2 = tree[1]->GetEntries();
  
  // ====================
  // set output ROOT file and TTree
  OperateString *OpStr = new OperateString();
  std::string outputname = Form("%s/%s_recon.root",outputDir.c_str(),OpStr->GetName(inputFileName).c_str());
  TFile* outputfile = new TFile(outputname.c_str(),"recreate");
  TTree* tree_out = new TTree("tree","tree");
  int spill,spill_mode,spill_count;
  int debug[NumDif][NumChip];
  tree_out->Branch("spill"           ,&spill                     ,"spill/I");
  tree_out->Branch("spill_mode"      ,&spill_mode                ,"spill_mode/I");
  tree_out->Branch("spill_count"     ,&spill_count               ,"spill_count/I");
  tree_out->Branch("debug"           , debug                     ,Form("debug[%d][%d]/I",NumDif,NumChip));
  tree_out->Branch("num_hits"        ,&wg_rec.type_hit.num_hits  ,"num_hits/I");
  tree_out->Branch("num_bcid_cluster",&wg_rec.type_hit.num_bcid_cluster,"num_bcid_cluster/I");
  tree_out->Branch("num_bcid_hits"   , wg_rec.type_hit.num_bcid_hits ,Form("num_bcid_hits[%d]/I"  ,MAX_NUM_BCID_CLUSTER));
  tree_out->Branch("clustered_bcid"  , wg_rec.type_hit.clustered_bcid,Form("clustered_bcid[%d]/I" ,MAX_NUM_BCID_CLUSTER));
  tree_out->Branch("clustered_hitid" , wg_rec.type_hit.clustered_hitid,Form("clustered_hitid[%d][%d]/I" ,MAX_NUM_BCID_CLUSTER,MAX_NUM_HIT));
  tree_out->Branch("hit_bcid"        , wg_rec.type_hit.hit_bcid  ,Form("hit_bcid[%d]/I"  ,MAX_NUM_HIT));
  tree_out->Branch("hit_view"        , wg_rec.type_hit.hit_view  ,Form("hit_view[%d]/I"  ,MAX_NUM_HIT));
  tree_out->Branch("hit_pln"         , wg_rec.type_hit.hit_pln   ,Form("hit_pln[%d]/I"   ,MAX_NUM_HIT));
  tree_out->Branch("hit_ch"          , wg_rec.type_hit.hit_ch    ,Form("hit_ch[%d]/I"    ,MAX_NUM_HIT));
  tree_out->Branch("hit_dif"         , wg_rec.type_hit.hit_dif   ,Form("hit_dif[%d]/I"   ,MAX_NUM_HIT));
  tree_out->Branch("hit_chip"        , wg_rec.type_hit.hit_chip  ,Form("hit_chip[%d]/I"  ,MAX_NUM_HIT));
  tree_out->Branch("hit_chipch"      , wg_rec.type_hit.hit_chipch,Form("hit_chipch[%d]/I",MAX_NUM_HIT));
  tree_out->Branch("hit_sca"         , wg_rec.type_hit.hit_sca   ,Form("hit_sca[%d]/I"   ,MAX_NUM_HIT));
  tree_out->Branch("hit_adc"         , wg_rec.type_hit.hit_adc   ,Form("hit_adc[%d]/I"   ,MAX_NUM_HIT));
  tree_out->Branch("hit_gs"          , wg_rec.type_hit.hit_gs    ,Form("hit_gs[%d]/I"    ,MAX_NUM_HIT));
  tree_out->Branch("hit_tdc"         , wg_rec.type_hit.hit_tdc   ,Form("hit_tdc[%d]/I"   ,MAX_NUM_HIT));
  tree_out->Branch("hit_pe"          , wg_rec.type_hit.hit_pe    ,Form("hit_pe[%d]/D"    ,MAX_NUM_HIT));
  tree_out->Branch("hit_time"        , wg_rec.type_hit.hit_time  ,Form("hit_time[%d]/D"  ,MAX_NUM_HIT));
  tree_out->Branch("hit_pe_permm"    , wg_rec.type_hit.hit_pe_permm    ,Form("hit_pe_permm[%d]/D"    ,MAX_NUM_HIT));
  tree_out->Branch("hit_pathlength"  , wg_rec.type_hit.hit_pathlength  ,Form("hit_pathlength[%d]/D"  ,MAX_NUM_HIT));
  tree_out->Branch("hit_ontrack"     , wg_rec.type_hit.hit_ontrack     ,Form("hit_ontrack[%d]/O"    ,MAX_NUM_HIT));
  tree_out->Branch("hit_grid"        , wg_rec.type_hit.hit_grid        ,Form("hit_grid[%d]/O"  ,MAX_NUM_HIT));
  tree_out->Branch("hit_numtrack"    , wg_rec.type_hit.hit_numtrack    ,Form("hit_numtrack[%d]/I"  ,MAX_NUM_HIT));
  tree_out->Branch("hit_cluster_pe"    , wg_rec.type_hit.hit_cluster_pe   ,Form("hit_cluster_pe[%d]/D"  ,MAX_NUM_HIT));

  tree_out->Branch("num_recon"       ,&wg_rec.type_recon.num_recon       ,"num_recon/I"); 
  tree_out->Branch("num_recon_hits"  , wg_rec.type_recon.num_recon_hits  ,Form("num_recon_hits[%d]/I"  ,MAX_NUM_TRACK)); 
  tree_out->Branch("recon_hits_hitid", wg_rec.type_recon.recon_hits_hitid,Form("recon_hits_hitid[%d][%d]/I",MAX_NUM_TRACK,MAX_NUM_TRACKHIT)); 
  tree_out->Branch("recon_bcid_id"   , wg_rec.type_recon.recon_bcid_id   ,Form("recon_bcid_id [%d]/I"   ,MAX_NUM_TRACK)); 
  tree_out->Branch("recon_start_z"   , wg_rec.type_recon.recon_start_z   ,Form("recon_start_z[%d]/D"   ,MAX_NUM_TRACK)); 
  tree_out->Branch("recon_stop_z"    , wg_rec.type_recon.recon_stop_z    ,Form("recon_stop_z[%d]/D"    ,MAX_NUM_TRACK)); 
  tree_out->Branch("recon_start_pln" , wg_rec.type_recon.recon_start_pln ,Form("recon_start_pln[%d]/I" ,MAX_NUM_TRACK)); 
  tree_out->Branch("recon_stop_pln"  , wg_rec.type_recon.recon_stop_pln  ,Form("recon_stop_pln[%d]/I"  ,MAX_NUM_TRACK)); 
  tree_out->Branch("recon_start_ch"  , wg_rec.type_recon.recon_start_ch  ,Form("recon_start_ch[%d]/I"  ,MAX_NUM_TRACK)); 
  tree_out->Branch("recon_stop_ch"   , wg_rec.type_recon.recon_stop_ch   ,Form("recon_stop_ch[%d]/I"   ,MAX_NUM_TRACK)); 
  tree_out->Branch("recon_start_xy"  , wg_rec.type_recon.recon_start_xy  ,Form("recon_start_xy[%d]/D"  ,MAX_NUM_TRACK)); 
  tree_out->Branch("recon_stop_xy"   , wg_rec.type_recon.recon_stop_xy   ,Form("recon_stop_xy[%d]/D"   ,MAX_NUM_TRACK)); 
  tree_out->Branch("recon_slope"     , wg_rec.type_recon.recon_slope     ,Form("recon_slope[%d]/D"     ,MAX_NUM_TRACK));   
  tree_out->Branch("recon_intercept" , wg_rec.type_recon.recon_intercept ,Form("recon_intercept [%d]/D",MAX_NUM_TRACK)); 
  tree_out->Branch("recon_pathlength", wg_rec.type_recon.recon_pathlength,Form("recon_pathlength[%d]/D",MAX_NUM_TRACK)); 
  tree_out->Branch("recon_total_pe"  , wg_rec.type_recon.recon_total_pe  ,Form("recon_total_pe[%d]/D"  ,MAX_NUM_TRACK));  
  tree_out->Branch("recon_mean_dedx" , wg_rec.type_recon.recon_mean_dedx ,Form("recon_mean_dedx[%d]/D" ,MAX_NUM_TRACK));  
  tree_out->Branch("recon_mean_time" , wg_rec.type_recon.recon_mean_time ,Form("recon_mean_time[%d]/D" ,MAX_NUM_TRACK));  
  tree_out->Branch("recon_view"      , wg_rec.type_recon.recon_view      ,Form("recon_view[%d]/I"      ,MAX_NUM_TRACK));  
  tree_out->Branch("recon_bcid"      , wg_rec.type_recon.recon_bcid      ,Form("recon_bcid[%d]/I"      ,MAX_NUM_TRACK)); 
  tree_out->Branch("recon_veto"      , wg_rec.type_recon.recon_veto      ,Form("recon_veto[%d]/I"      ,MAX_NUM_TRACK)); 
  tree_out->Branch("recon_sideescape", wg_rec.type_recon.recon_sideescape,Form("recon_sideescape[%d]/I",MAX_NUM_TRACK)); 
  tree_out->Branch("recon_cos"       , wg_rec.type_recon.recon_cos       ,Form("recon_cos[%d]/D"       ,MAX_NUM_TRACK));   
  tree_out->Branch("recon_len"       , wg_rec.type_recon.recon_len       ,Form("recon_len[%d]/D"       ,MAX_NUM_TRACK));   
  tree_out->Branch("recon_chi2"       , wg_rec.type_recon.recon_chi2      ,Form("recon_chi2[%d]/D"       ,MAX_NUM_TRACK));   

  tree_out->Branch("num_track"       ,&wg_rec.type_track.num_track       ,"num_track/I"); 
  tree_out->Branch("num_trackid"     ,&wg_rec.type_track.num_trackid     ,"num_trackid/I"); 
  tree_out->Branch("num_track_hits"  , wg_rec.type_track.num_track_hits  ,Form("num_track_hits[%d]/I"  ,MAX_NUM_TRACK)); 
  tree_out->Branch("num_track_recon" , wg_rec.type_track.num_track_recon ,Form("num_track_recon[%d]/I"  ,MAX_NUM_TRACK)); 
  tree_out->Branch("track_recon_id"  , wg_rec.type_track.track_recon_id  ,Form("track_recon_id[%d][%d]/I",MAX_NUM_TRACK,MAX_NUM_TRACK)); 
  tree_out->Branch("track_hits_hitid", wg_rec.type_track.track_hits_hitid,Form("track_hits_hitid[%d][%d]/I",MAX_NUM_TRACK,MAX_NUM_TRACKHIT)); 
  tree_out->Branch("track_start_z"   , wg_rec.type_track.track_start_z   ,Form("track_start_z[%d]/D"   ,MAX_NUM_TRACK)); 
  tree_out->Branch("track_stop_z"    , wg_rec.type_track.track_stop_z    ,Form("track_stop_z[%d]/D"    ,MAX_NUM_TRACK)); 
  tree_out->Branch("track_start_pln" , wg_rec.type_track.track_start_pln ,Form("track_start_pln[%d]/I"   ,MAX_NUM_TRACK)); 
  tree_out->Branch("track_stop_pln"  , wg_rec.type_track.track_stop_pln  ,Form("track_stop_pln[%d]/I"    ,MAX_NUM_TRACK)); 
  tree_out->Branch("track_start_ch"  , wg_rec.type_track.track_start_ch  ,Form("track_start_ch[%d]/I"   ,MAX_NUM_TRACK)); 
  tree_out->Branch("track_stop_ch"   , wg_rec.type_track.track_stop_ch   ,Form("track_stop_ch[%d]/I"    ,MAX_NUM_TRACK)); 
  tree_out->Branch("track_start_xy"  , wg_rec.type_track.track_start_xy  ,Form("track_start_xy[%d]/D"  ,MAX_NUM_TRACK)); 
  tree_out->Branch("track_stop_xy"   , wg_rec.type_track.track_stop_xy   ,Form("track_stop_xy[%d]/D"   ,MAX_NUM_TRACK)); 
  tree_out->Branch("track_slope"     , wg_rec.type_track.track_slope     ,Form("track_slope[%d]/D"     ,MAX_NUM_TRACK));   
  tree_out->Branch("track_intercept" , wg_rec.type_track.track_intercept ,Form("track_intercept [%d]/D",MAX_NUM_TRACK)); 
  tree_out->Branch("track_pathlength", wg_rec.type_track.track_pathlength,Form("track_pathlength[%d]/D",MAX_NUM_TRACK)); 
  tree_out->Branch("track_total_pe"  , wg_rec.type_track.track_total_pe  ,Form("track_total_pe[%d]/D"  ,MAX_NUM_TRACK));  
  tree_out->Branch("track_mean_dedx" , wg_rec.type_track.track_mean_dedx ,Form("track_mean_dedx[%d]/D" ,MAX_NUM_TRACK));  
  tree_out->Branch("track_mean_time" , wg_rec.type_track.track_mean_time ,Form("track_mean_time[%d]/D" ,MAX_NUM_TRACK));  
  tree_out->Branch("track_view"      , wg_rec.type_track.track_view      ,Form("track_view[%d]/I"      ,MAX_NUM_TRACK));  
  tree_out->Branch("track_bcid"      , wg_rec.type_track.track_bcid      ,Form("track_bcid[%d]/I"      ,MAX_NUM_TRACK)); 
  tree_out->Branch("track_veto"      , wg_rec.type_track.track_veto      ,Form("track_veto[%d]/I"      ,MAX_NUM_TRACK)); 
  tree_out->Branch("track_sideescape", wg_rec.type_track.track_sideescape,Form("track_sideescape[%d]/I",MAX_NUM_TRACK)); 
  tree_out->Branch("track_cos_zen"   , wg_rec.type_track.track_cos_zen   ,Form("track_cos_zen[%d]/D"   ,MAX_NUM_TRACK));   
  tree_out->Branch("track_cos_azi"   , wg_rec.type_track.track_cos_azi   ,Form("track_cos_azi[%d]/D"   ,MAX_NUM_TRACK));   
  tree_out->Branch("track_len"       , wg_rec.type_track.track_len       ,Form("track_len[%d]/D"       ,MAX_NUM_TRACK));   
  tree_out->Branch("track_id"        , wg_rec.type_track.track_id        ,Form("track_id[%d]/I"        ,MAX_NUM_TRACK));   

  
  // =====================
  // start event loop

  cout << "** inputFile1 : " << filename1 << " **" << endl;
  cout << "** inputFile2 : " << filename2 << " **" << endl;
  std::cout << "Number of event: " << nevt1 << " (dif1), " << nevt2 << " (dif2)" << std::endl;
  //int startevt = 870;
  //int maxevent = startevt+1;
  int startevt = 0;
  int maxevent= 999999;
  int ievt = 0;
  int ievt1=startevt;
  int ievt2=startevt;
  while(ievt1<nevt1&&ievt2<nevt2&&ievt1<maxevent&&ievt2<maxevent){
    wg_rec.clear();
#ifdef DEBUG_RECON
    std::cout << "===================================================="
      << std::endl
      << "event=" << ievt << std::endl;
#else
    if(ievt%1000==0) std::cout << "event: " << ievt1 << "(dif1), "  << ievt2 << "(dif2)" << std::endl;
#endif
    tree[0]->GetEntry(ievt1);
    tree[1]->GetEntry(ievt2);
    int diff_spillcount = wg_rec.type_raw[0].spill_count - wg_rec.type_raw[1].spill_count;
    int diff_spill      = wg_rec.type_raw[0].spill       - wg_rec.type_raw[1].spill;
    int diff_spillmode  = wg_rec.type_raw[0].spill_mode  - wg_rec.type_raw[1].spill_mode;
    int num_rep = 0;
    int repstart1 = ievt1;
    int repstart2 = ievt2;
    while(diff_spillcount!=0||diff_spill!=0||diff_spillmode!=0)
    {
#ifdef DEBUG_RECON
      std::cout 
        << "{spill_count,spill,spill_mode} are different between dif1 and dif2 :" << endl
        << "event: " << ievt1 << "(dif1) " << ievt2 << "(dif2)"
        << " dif1={"<<wg_rec.type_raw[0].spill_count<<","<<wg_rec.type_raw[0].spill<<","<<wg_rec.type_raw[0].spill_mode<<"}"
        << " dif2={"<<wg_rec.type_raw[1].spill_count<<","<<wg_rec.type_raw[1].spill<<","<<wg_rec.type_raw[1].spill_mode<<"}" 
        << std::endl;
#endif
      if(num_rep<3){
        if(diff_spillcount>0){ievt2++;} else{ievt1++;}
      }
      else{
        ievt1=repstart1+1;
        ievt2=repstart2+1;
        num_rep=0;
        repstart1 = ievt1;
        repstart2 = ievt2;
      }
      if(ievt1<nevt1&&ievt2<nevt2&&ievt1<maxevent&&ievt2<maxevent){
        tree[0]->GetEntry(ievt1);
        tree[1]->GetEntry(ievt2);
        diff_spillcount = wg_rec.type_raw[0].spill_count - wg_rec.type_raw[1].spill_count;
        diff_spill      = wg_rec.type_raw[0].spill       - wg_rec.type_raw[1].spill;
        diff_spillmode  = wg_rec.type_raw[0].spill_mode  - wg_rec.type_raw[1].spill_mode;
      }
      else{
        goto ENDOFEVT;
      }
      num_rep++;
    }

    // ===============
    // Push hits into vector
    // Note: This is required to be set with "chipid", not with "chip".

    spill       = wg_rec.type_raw[0].spill;
    spill_mode  = wg_rec.type_raw[0].spill_mode;
    spill_count = wg_rec.type_raw[0].spill_count;

    for(int dif=0;dif<NumDif;dif++){
      for(int chip=0;chip<NumChip;chip++){
        debug[dif][chip] = wg_rec.type_raw[dif].debug[chip];
      }
    }
    
    for(int dif=0;dif<NumDif;dif++){
      for(int chip=0;chip<NumChip;chip++){
        int chipid = wg_rec.type_raw[dif].chipid[chip];
        for(int chipch=0;chipch<NumChipCh;chipch++){
          for(int sca=0;sca<NumSca;sca++){
            if( wg_rec.type_raw[dif].hit[chip][chipch][sca]==1 ){              
              int view = wg_rec.type_map.view[dif][chipid][chipch];
              int pln  = wg_rec.type_map.pln [dif][chipid][chipch];
              int ch   = wg_rec.type_map.ch  [dif][chipid][chipch];
              int charge = wg_rec.type_raw[dif].charge[chip][chipch][sca]; 
              int tdc    = wg_rec.type_raw[dif].time  [chip][chipch][sca]; 
              int gs     = wg_rec.type_raw[dif].gs  [chip][chipch][sca]; 
              int bcid = wg_rec.type_raw[dif].bcid[chip][sca];
              double pe = wg_rec.type_raw[dif].pe[chip][chipch][sca];
              double time = wg_rec.type_raw[dif].time_ns[chip][chipch][sca];
              wg_rec.pushHitInfo(bcid,view,pln,ch,dif,chipid,chipch,sca,charge,gs,tdc,pe,time);
            }
          }
        }
      }
    }

    wg_rec.sort_byBCIDnMAP();
    for(int i=0;i<wg_rec.get_num_hit();i++){
      int    bcid   = wg_rec.get_hitbcid   (i);
      int    view   = wg_rec.get_hitview   (i);
      int    pln    = wg_rec.get_hitpln    (i);
      int    ch     = wg_rec.get_hitch     (i);
      int    dif    = wg_rec.get_hitdif    (i);
      int    chip   = wg_rec.get_hitchip   (i);
      int    chipch = wg_rec.get_hitchipch (i);
      int    sca    = wg_rec.get_hitsca    (i);
      int    adc    = wg_rec.get_hitadc    (i);
      int    gs     = wg_rec.get_hitgs     (i);
      int    tdc    = wg_rec.get_hittdc    (i);
      double pe     = wg_rec.get_hitpe     (i);
      double time   = wg_rec.get_hittime   (i);
      if(!wg_rec.pushHitStruct(bcid,view,pln,ch,dif,chip,chipch,sca,adc,gs,tdc,pe,time))
      {
        goto FILL;
      }
    }

    if(!wg_rec.findBCIDCluster()) goto FILL; 
    if(!wg_rec.findTimeCluster()) goto FILL; 
    if(!wg_rec.Tracking_alongZ()) goto FILL; 
    if(!wg_rec.Tracking_alongXY()) goto FILL;
    if(!wg_rec.eraceDuplicateTrack()) goto FILL;
    if(!wg_rec.findTrackPair()) goto FILL;
    if(!wg_rec.addNearHits()) goto FILL;
    wg_rec.fillTrackHit();
FILL:
    {
      tree_out->Fill();
    }
    ievt1++;
    ievt2++;
    ievt++;
  }// ievt loop end
ENDOFEVT:
  cout << "Number of events filled: " << ievt << endl;
  cout << "Total number of decoded events: "  << nevt1 << "(dif1) " << nevt2 << "(dif2)" << endl;
  
  tree_out    ->Write();
  start_time  ->Write();
  stop_time   ->Write();
  nb_data_pkts->Write();
  nb_lost_pkts->Write();
  outputfile->Close();
    
  cout << "File : " << outputname << " is written!" << endl;

  return 0;
}
