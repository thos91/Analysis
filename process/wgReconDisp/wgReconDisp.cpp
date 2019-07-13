/* ***********************************************************************
 * To execute wagasci display program 
 * Program : wagasci_disp_exe.cpp
 * Name: Naruhiro Chikuma
 * Date: 2017-05-08 23:33:49
 * ********************************************************************** */

#include <TStyle.h>
#include <TLine.h>
#include <TArc.h>
#include <TPad.h>
#include <TROOT.h>
#include <TApplication.h>
#include <TGraph.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TBox.h>
#include <TH2D.h>

#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgChannelMap.h"
#include "DetectorwgConst.hpp"
#include "wgGetTree.h"
#include "wgConst.hpp"
#include "wgReconClass.h"

using namespace std;

bool DrawScintillator(double x, double y, bool grid);
void DrawModule(int dif_id);
void DrawHit(int i_bcid,Hit_t t_hit,Recon_t t_recon, Track_t t_track);
void DrawRecon(int i_bcid,Hit_t t_hit,Recon_t t_recon, Track_t t_track);
void DrawTrack(int current_bcid,Hit_t t_hit,Recon_t t_recon, Track_t t_track);
void clear();

int n_trackid=0;
bool bn_trackid=false;
bool b_mode=false;
//int num_hit; 
//int num_recon; 
#define F_CIRCLE 5.0 //3.0
const double margin_t = 0.00;
const double margin_b = 0.10;
const double margin_l = 0.05;
const double margin_r = 0.02;
const double line_width = 2.5;
TPad   *pad[2];
TArc   *hit[2][MAX_NUM_HIT];
TArc   *recon_hit[2][MAX_NUM_HIT];
TLine  *recon[2][MAX_NUM_TRACK];
TLine  *track[2][MAX_NUM_TRACK];
TLegend *leg[2];
Map_t    mapping;
MapInv_t mapping_inv;
TCanvas *canvas;
TCanvas *canvas_time;
TH2D    *h_time[3];
TH2D    *h_hitbcid;
bool veto_cut = false;

vector<int> v_recon_hit(0);

int main(int argc, char **argv) 
{

  int opt;
  int mode=0;
  int start_evt=0;
  wgEnvironment env;
  con->GetENV();
  string inputFileName("");
  string logoutputDir=con->LOG_DIRECTORY;

  CheckExist *check = new CheckExist;

  while((opt = getopt(argc,argv, "f:hi:t:vm:")) !=-1 ){
    switch(opt){
      case 'f':
        inputFileName=optarg;
        if(!check->RootFile(inputFileName)){ 
          cout<<"!!Error!! "<<inputFileName.c_str()<<"doesn't exist!!";
          return 1;
        }
        break;

      case 't':
        n_trackid = atoi(optarg); 
        bn_trackid=true;
        break;

      case 'i':
        start_evt = atoi(optarg); 
        break;

      case 'v':
        veto_cut=true; 
        break;

      case 'm':
        b_mode = true;
        mode = atoi(optarg); 
        break;

      case 'h':
        cout <<"This program is for event display after reconstruction. "<<endl;
        cout <<"You can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose input file you wanna read(must)"<<endl;
        cout <<"  -t (int)   : choose number of track"<<endl;
        cout <<"  -i (int)   : choose start # event  "<<endl;
        cout <<"  -m (int)   : choose spill mode (default:both)  "<<endl;
        cout <<"  -v         : veto cut (default:false)  "<<endl;
        exit(0);
      default:
        cout <<"This program is for event display after reconstruction. "<<endl;
        cout <<"You can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose input file you wanna read(must)"<<endl;
        cout <<"  -t (int)   : choose number of track"<<endl;
        cout <<"  -i (int)   : choose start # event  "<<endl;
        cout <<"  -m (int)   : choose spill mode (default:both)  "<<endl;
        cout <<"  -v         : veto cut (default:false)  "<<endl;
        exit(0);
    }   
  }

  if(inputFileName==""){
    cout << "!!ERROR!! please input filename." <<endl;
    cout << "if you don't know how to input, please see help."<<endl;
    cout << "help : ./wgReconDisp -h" <<endl;
    exit(1);
  }

  // =============================================== //
  // ================= INITIALIZE ================== //
  // =============================================== //
  wgChannelMap *wgmap = new wgChannelMap();
  mapping     = wgmap->load_mapping();
  mapping_inv = wgmap->load_mapping_inv();

  TROOT root("GUI","GUI");
  TApplication App("App",0,0);
  gStyle->SetOptStat(0);
  gStyle->SetPalette(1);
  canvas = new TCanvas("canvas","canvas",600,800);

  double pad_div[2][4]= {
    {0.01,0.01,0.49,0.99},
    {0.51,0.01,0.99,0.99}
  };

  for(int i=0;i<2;i++){
    string name = Form("pad%d",i);
    pad[i]=new TPad(name.c_str(),name.c_str(),
        pad_div[i][0],
        pad_div[i][1],
        pad_div[i][2],
        pad_div[i][3]
        );
    pad[i]->Draw();
  }

  TGraph *g_evtdisp2D[2];
  for(int i=0;i<2;i++){
    pad[i]->cd();
    g_evtdisp2D[i] = new TGraph(4);
    int ip=0;
    for(int j=-1;j<=1;j=j+2){
      for(int k=-1;k<=1;k=k+2){
        g_evtdisp2D[i]->SetPoint(ip,j*WATERTANK_X/2.,k*WATERTANK_Y/2.);
        ip++;
      }
    }
    g_evtdisp2D[i] ->SetMarkerSize(0.1);
    g_evtdisp2D[i] ->SetMarkerColor(kBlack);
    g_evtdisp2D[i] ->Draw("ap");
  }


  for(int i=0;i<NumDif;i++){
    pad[i]->cd();
    for(int j=0;j<MAX_NUM_HIT;j++){
      hit[i][j] = new TArc(-WATERTANK_X,-WATERTANK_Y,0);
      hit[i][j]->SetFillColor(kRed);
      recon_hit[i][j] = new TArc(-WATERTANK_X/2.,-WATERTANK_Y/2.,0);
      recon_hit[i][j]->SetFillColor(kBlue);
    }
    for(int j=0;j<MAX_NUM_TRACK;j++){      
      recon[i][j] = new TLine(-WATERTANK_X/2.,-WATERTANK_X/2.,-WATERTANK_X/2.,-WATERTANK_Y/2.);
      recon[i][j]->SetLineStyle(2);
      recon[i][j]->SetLineColor(kBlack);
      recon[i][j]->SetLineWidth(5);
      recon[i][j]->Draw("same");
      track[i][j] = new TLine(-WATERTANK_X/2.,-WATERTANK_Y/2.,-WATERTANK_X/2.,-WATERTANK_Y/2.);
      track[i][j]->SetLineWidth(2.5);
      track[i][j]->SetLineColor(kViolet);
      track[i][j]->Draw("same");
    }
    DrawModule(i);
  }

  canvas_time = new TCanvas("canvas_time","canvas_time",1500,0,300,800);
  canvas_time->Divide(1,2);
  h_hitbcid      = new TH2D(Form("bcid"),"bcid",41,0,41,6,-1,5);
  for(int i=0;i<3;i++){
    h_time[i]      = new TH2D(Form("tdc%d",i),"tdc",41,0,41,43,-100,4200);
  }

  Hit_t t_hit;
  Recon_t t_recon;
  Track_t t_track;
  // =================
  // set TTree
  wgGetTree *gettree = new wgGetTree(inputFileName);
  gettree->SetTreeFile(t_hit);
  gettree->SetTreeFile(t_recon);
  gettree->SetTreeFile(t_track);

  int spill,spill_mode,debug[2][20];
  gettree->tree->SetBranchAddress("spill",&spill);
  gettree->tree->SetBranchAddress("spill_mode",&spill_mode);
  gettree->tree->SetBranchAddress("debug",debug);

  int nevt = wgGetTree::tree->GetEntries();
  int count_nevt = 0;
  for(int ievt=start_evt;ievt<nevt;ievt++){
    gettree->GetEntry(ievt);
    v_recon_hit.clear();
    if(b_mode  && mode!=spill_mode)continue;
    std::cout << "==========================" << std::endl;
    std::cout << " event=" << ievt << ", spill=" << spill <<std::endl;

    if(t_hit.num_hits<THRES_NHITS){cout << "num_hits is too small.("<<t_hit.num_hits<<")" << endl; continue;}
    for(int i_bcid=0;i_bcid<t_hit.num_bcid_cluster;i_bcid++){
      if(t_hit.num_bcid_hits[i_bcid] < THRES_NHITS){cout << "num_bcid_hits is too samll." << endl; continue;}
      std::cout << "num_bcid_hits=" << t_hit.num_bcid_hits[i_bcid] << std::endl;
      std::cout << "clustered_bcid=" << t_hit.clustered_bcid[i_bcid] << std::endl;
      std::cout << "num_recon=" << t_recon.num_recon << std::endl;
      std::cout << "num_track=" << t_track.num_trackid << std::endl;
      if(bn_trackid){
        if(n_trackid!=t_track.num_trackid) continue;
      }

      int current_bcid = t_hit.clustered_bcid[i_bcid];
      bool veto_event = false;
      if(veto_cut){
        for(int i_track=0;i_track<t_track.num_track;i_track++){
          if(fabs(current_bcid-t_track.track_bcid[i_track])>MAX_DIFF_BCID)continue;
          if(t_track.track_veto[i_track]==1)veto_event=true;
        }
        if(!veto_event) continue;
      }
      std::cout << "current_bcid=" << current_bcid << std::endl;
      clear();
      DrawHit(i_bcid,t_hit,t_recon,t_track );
      DrawRecon(current_bcid,t_hit,t_recon,t_track );
      //DrawTrack(current_bcid,t_hit,t_recon,t_track );
      DrawTrack(i_bcid,t_hit,t_recon,t_track );

      count_nevt++;
      cout <<"debug ..." << endl;
      
      for(int i1=0;i1<2;i1++){
        cout << "dif:" << i1;
        for(int i2=0;i2<20;i2++){
          cout << "  chip[" << i2 <<"]"<< debug[i1][i2] << endl;
        }
        cout << endl;
      }

      std::cout << "Type q/quit for stop this program." << std::endl;
      std::cout << "Type p/print for print event display." << std::endl;
      std::string str;
      std::cin >> str;
      if(str=="q"||str=="quit"){ return false; }
      if(str=="p"||str=="print"){
        canvas->Print(Form("png/eventdisp_evt%d_bcid%d.png",
              ievt,
              current_bcid));
      }
    }//i_bcid
  }//ievt

  std::cout << "Number of selected events = " << count_nevt << std::endl;
  std::cout << "Number of spill = " << nevt << std::endl;

  return 0;
} //main


// ==================================================================
void DrawHit(int i_bcid,Hit_t t_hit, Recon_t t_recon, Track_t t_track){

  h_time[0] = new TH2D("time_hit","time_hit",41,0,41,4300,-100,4200);
  h_time[0] -> SetFillColor(kBlack);
  h_hitbcid = new TH2D("bcid","bcid",41,0,41,11,-1,10);
  //h_hitbcid = new TH2D("bcid","bcid",41,0,41,410,0,4100);
  double x_min=0.,x_max=0;
  //for(int i=0;i<t_hit.num_bcid_hits[i_bcid];i++){
  for(int i=0;i<t_hit.num_hits;i++){
    //int hitid  = t_hit.clustered_hitid[i_bcid][i];
    int hitid  = i;
    int view   = t_hit.hit_view  [hitid];
    int dif    = t_hit.hit_dif   [hitid];
    int chip   = t_hit.hit_chip  [hitid];
    int chipch = t_hit.hit_chipch[hitid];
    double pe  = t_hit.hit_pe    [hitid];
    double time= t_hit.hit_time  [hitid];
    int    tdc = t_hit.hit_tdc   [hitid];
    int    bcid= t_hit.hit_bcid  [hitid];
    int    pln = t_hit.hit_pln   [hitid];
    int    ch  = t_hit.hit_ch    [hitid];
    int    current_bcid= t_hit.clustered_bcid  [i_bcid];
    double x  = mapping.x[dif][chip][chipch];
    double y  = mapping.y[dif][chip][chipch];
    double z  = mapping.z[dif][chip][chipch];

    double XX, YY;
    XX = z;
    if     (view==SideView){ YY = y; }
    else if(view==TopView ){ YY = x; }
    pad[view]->cd();
    hit[view][i]->SetX1(XX);
    hit[view][i]->SetY1(YY);
    hit[view][i]->SetR1(F_CIRCLE*pow(pe,0.33));
    hit[view][i]->SetR2(F_CIRCLE*pow(pe,0.33));
    hit[view][i]->Draw("");
    h_hitbcid->Fill(dif*21+chip,bcid-current_bcid);
    //h_hitbcid->Fill(dif*21+chip,bcid);
    if(tdc<1.0) tdc=4096.;
    h_time[0]->Fill(dif*21+chip,tdc);
    if(i==0){ x_max=tdc; x_min=tdc; }
    else if(tdc>x_max)x_max=tdc;
    else if(tdc<x_min)x_min=tdc;

    cout << "bcid="<< bcid
      << ",view=" << view 
      << ",pln=" << pln
      << ",ch="  << ch
      << ",pe=" << pe; 
    cout << endl;
  }//hits
  canvas->cd(0)->Modified();
  canvas->Update();

  canvas_time->cd(1);
  //h_time[0]->GetYaxis()->SetRange((x_min+80)/10,(x_max+120)/10);
  //h_time[0]->GetYaxis()->SetRange(0,500);
  //h_time[0]->GetYaxis()->SetRange(3800,4300);
  h_time[0]->Draw("colz");
  TBox *box = new TBox();
  box -> SetFillColor(kBlack); 
  box -> DrawBox(20,-100,21,4200);
  TBox *box_sub[8];
  for(int i=0;i<4;i++){
    box_sub[i]= new TBox();
    box_sub[i+4]= new TBox();
    box_sub[i] -> SetFillColor(kRed); 
    box_sub[i+4] -> SetFillColor(kRed); 
    box_sub[i] -> SetFillStyle(3004); 
    box_sub[i+4] -> SetFillStyle(3004); 
    box_sub[i] -> DrawBox(0+i*5,-100,2+i*5,4200);
    box_sub[i+4] -> DrawBox(21+i*5,-100,23+i*5,4200);
  }

  canvas_time->cd(2);
  h_hitbcid->Draw("colz");
  TBox *box2 = new TBox();
  box2 -> SetFillColor(kBlack); 
  box2 -> DrawBox(20,-1,21,5);
  TBox *box_sub2[8];
  for(int i=0;i<4;i++){
    box_sub2[i]= new TBox();
    box_sub2[i+4]= new TBox();
    box_sub2[i] -> SetFillColor(kRed); 
    box_sub2[i+4] -> SetFillColor(kRed); 
    box_sub2[i] -> SetFillStyle(3004); 
    box_sub2[i+4] -> SetFillStyle(3004); 
    box_sub2[i] -> DrawBox(0+i*5,-1,2+i*5,5);
    box_sub2[i+4] -> DrawBox(21+i*5,-1,23+i*5,5);
  }
  canvas_time->cd(1)->Modified();
  canvas_time->cd(2)->Modified();
  canvas_time->Update();
}

// ==================================================================
void DrawRecon(int current_bcid,Hit_t t_hit, Recon_t t_recon, Track_t t_track){
  for(int i=0;i<t_recon.num_recon;i++){
    if(current_bcid!=t_recon.recon_bcid[i]) continue;
    double start_z   = t_recon.recon_start_z   [i];
    double stop_z    = t_recon.recon_stop_z    [i];
    double start_xy  = t_recon.recon_start_xy  [i];
    double stop_xy   = t_recon.recon_stop_xy   [i];
    double slope     = t_recon.recon_slope     [i];
    double intercept = t_recon.recon_intercept [i];
    int    numhit    = t_recon.num_recon_hits  [i];
    int    view      = t_recon.recon_view      [i];
    int    bcid      = t_recon.recon_bcid      [i];
    int    start_pln = t_recon.recon_start_pln [i];
    int    stop_pln  = t_recon.recon_stop_pln  [i];
    int    veto      = t_recon.recon_veto      [i];
    int    sideescape= t_recon.recon_sideescape[i];
    std::cout
      << " === recon " << i << " ==" << std::endl
      << " view="      << view     
      << " bcid="      << bcid  
      << " start_z="   << start_z  
      << " stop_z="    << stop_z   
      << " start_pln="   << start_pln  
      << " stop_pln="    << stop_pln 
      << " slope="     << slope    
      << " intercept=" << intercept
      << " #hits=" << numhit
      << std::endl
      << " start_pln=" << start_pln
      << " stop_pln=" << stop_pln
      << " veto=" << veto
      << " sideescape=" << sideescape
      << std::endl;
    pad[view]->cd();
    recon[view][i]->SetX1(start_z);
    recon[view][i]->SetX2(stop_z);
    recon[view][i]->SetY1(start_xy);
    recon[view][i]->SetY2(stop_xy);
    recon[view][i]->Draw();
    for(int j=0;j<numhit;j++){
      int hitid = t_recon.recon_hits_hitid[i][j];
      int dif     = t_hit.hit_dif   [hitid];
      int chip    = t_hit.hit_chip  [hitid];
      int chipch  = t_hit.hit_chipch[hitid];
      int tdc     = t_hit.hit_tdc   [hitid];
      int hitview = t_hit.hit_view  [hitid];
      int pln     = t_hit.hit_pln   [hitid];
      int ch      = t_hit.hit_ch    [hitid];
      double pe   = t_hit.hit_pe    [hitid];
      double time = t_hit.hit_time  [hitid];
      double x  = mapping.x[dif][chip][chipch];
      double y  = mapping.y[dif][chip][chipch];
      double z  = mapping.z[dif][chip][chipch];

      double XX, YY;
      XX = z;
      if     (view==SideView){ YY = y; }
      else if(view==TopView ){ YY = x; }
      recon_hit[view][hitid]->SetFillColor(kBlue);
      recon_hit[view][hitid]->SetX1(XX);
      recon_hit[view][hitid]->SetY1(YY);
      recon_hit[view][hitid]->SetR1(F_CIRCLE*pow(pe,0.33));
      recon_hit[view][hitid]->SetR2(F_CIRCLE*pow(pe,0.33));
      recon_hit[view][hitid]->Draw("same");

      std::cout
        << "["
        << "hitid:" << hitid
        << ",view:" << hitview
        << ",pln:"  << pln
        << ",ch:"   << ch
        << ",pe:"   << pe
        <<"]";

      v_recon_hit.push_back(hitid);
    }
    std::cout << std::endl;
  }
  canvas->cd(0)->Modified();
  canvas->Update();
}

// ==================================================================
void DrawTrack(int current_bcid,Hit_t t_hit, Recon_t t_recon, Track_t t_track){
  h_time[1] = new TH2D("time_track_lowpe","time_track_lowpe",41,0,41,43,-100,4200);
  h_time[2] = new TH2D("time_track","time_recon",41,0,41,43,-100,4200);
  h_time[1] -> SetFillColor(kGreen);
  h_time[1] -> SetLineColor(kGreen);
  h_time[2] -> SetFillColor(kRed);
  h_time[2] -> SetLineColor(kRed);
  //int bcid = current_bcid;
  int i_bcid = current_bcid;
  int bcid= t_hit.clustered_bcid[i_bcid];
  for(int i=0;i<t_track.num_track;i++){
    if(bcid!=t_track.track_bcid[i]) continue;
    double start_z   = t_track.track_start_z   [i];
    double stop_z    = t_track.track_stop_z    [i];
    double start_xy  = t_track.track_start_xy  [i];
    double stop_xy   = t_track.track_stop_xy   [i];
    int    view      = t_track.track_view      [i];
    double slope     = t_track.track_slope     [i];
    double intcpt    = t_track.track_intercept [i];
    int    start_pln = t_track.track_start_pln [i];
    int    stop_pln  = t_track.track_stop_pln  [i];
    int    veto      = t_track.track_veto      [i];
    int    sideescape = t_track.track_sideescape[i];
    double pathlength = t_track.track_pathlength[i];
    std::cout
      << "=== track ==" << std::endl
      << " view="      << view     
      << " start_z="   << start_z  
      << " start_xy="  << start_xy
      << " stop_z="    << stop_z
      << " stop_xy="   << stop_xy
      << " slope="     << slope
      << " intcpt="    << intcpt
      << std::endl
      << " start_pln=" << start_pln
      << " stop_pln=" << stop_pln
      << " veto=" << veto
      << " sideescape=" << sideescape
      << " pathlength="  << pathlength
      << std::endl;
    pad[view]->cd();
    track[view][i]->SetX1(start_z);
    track[view][i]->SetX2(stop_z);
    track[view][i]->SetY1(start_xy);
    track[view][i]->SetY2(stop_xy);
    track[view][i]->Draw();

    int l=0;
    for(int j=0;j<t_track.num_track_hits[i];j++){
      int hitid  = t_track.track_hits_hitid[i][j];
      int view   = t_hit.hit_view  [hitid];
      int pln    = t_hit.hit_pln   [hitid];
      int ch     = t_hit.hit_ch    [hitid];
      int dif    = t_hit.hit_dif   [hitid];
      int chip   = t_hit.hit_chip  [hitid];
      int chipch = t_hit.hit_chipch[hitid];
      double pe  = t_hit.hit_pe    [hitid];
      double x  = mapping.x[dif][chip][chipch];
      double y  = mapping.y[dif][chip][chipch];
      double z  = mapping.z[dif][chip][chipch];
      double XX, YY;
      XX = z;
      if     (view==SideView){ YY = y; }
      else if(view==TopView ){ YY = x; }
      double dist = fabs(slope*XX-YY+intcpt)/sqrt(slope*slope+1);
      cout 
        << " view=" << view
        << " pln=" << pln
        << " ch=" << ch
        << " dist=" << dist
        << endl;
      bool isReconHit = false;
      int num_v_reconhit = v_recon_hit.size();
      for(int k=0;k<num_v_reconhit;k++){
        if(hitid==v_recon_hit[k]){
          isReconHit = true;
          break;
        }
      }
      if(!isReconHit){
        pad[view]->cd();
        recon_hit[view][hitid]->SetFillColor(kCyan);
        recon_hit[view][hitid]->SetX1(XX);
        recon_hit[view][hitid]->SetY1(YY);
        recon_hit[view][hitid]->SetR1(F_CIRCLE*pow(pe,0.33));
        recon_hit[view][hitid]->SetR2(F_CIRCLE*pow(pe,0.33));
        recon_hit[view][hitid]->Draw("same");
        l++;
      }
    }//hits


    /*
       for(int j=0;j<t_track.num_track_hits[i];j++){
       int hitid = t_track.track_hits_hitid[i][j];
       int dif    = t_hit.hit_dif   [hitid];
       int chip   = t_hit.hit_chip  [hitid];
       int chipch = t_hit.hit_chipch[hitid];
       int tdc    = t_hit.hit_tdc   [hitid];
       double pe  = t_hit.hit_pe    [hitid];
       double time= t_hit.hit_time  [hitid];
       double x  = mapping.x[dif][chip][chipch];
       double y  = mapping.y[dif][chip][chipch];
       double z  = mapping.z[dif][chip][chipch];

       double XX, YY;
       XX = z;
       if     (view==SideView){ YY = y; }
       else if(view==TopView ){ YY = x; }
       recon_hit[view][j]->SetX1(XX);
       recon_hit[view][j]->SetY1(YY);
       recon_hit[view][j]->SetR1(F_CIRCLE*pow(pe,0.33));
       recon_hit[view][j]->SetR2(F_CIRCLE*pow(pe,0.33));
       recon_hit[view][j]->Draw("same");
       if(tdc<1.) tdc=4096;
       if(view==0){
       h_time[2]->Fill(dif*21+chip,tdc);
       }
       if(view==1){
       h_time[1]->Fill(dif*21+chip,tdc);
       }
       }
       */
  }
  canvas->cd(0)->Modified();
  canvas->Update();

  canvas_time->cd(1);
  h_time[2]->Draw("same colz");
  h_time[1]->Draw("same colz");
  canvas_time->cd(1)->Modified();
  canvas_time->cd(2)->Modified();
  canvas_time->Update();
}

// ==================================================================
void clear(){
  for(int i=0;i<NumDif;i++){
    pad[i]->cd();
    for(int j=0;j<MAX_NUM_HIT;j++){
      hit[i][j]->SetX1(-WATERTANK_X);
      hit[i][j]->SetY1(-WATERTANK_Y);
      hit[i][j]->SetR1(0.);
      hit[i][j]->SetR2(0.);
      recon_hit[i][j]->SetX1(-WATERTANK_X);
      recon_hit[i][j]->SetY1(-WATERTANK_Y);
      recon_hit[i][j]->SetR1(0.);
      recon_hit[i][j]->SetR2(0.);
    }
    for(int j=0;j<MAX_NUM_TRACK;j++){      
      recon[i][j]->SetX1(-WATERTANK_X);
      recon[i][j]->SetX2(-WATERTANK_X);
      track[i][j]->SetX1(-WATERTANK_X);
      track[i][j]->SetX2(-WATERTANK_X);
    }
    pad[i]->Draw();
  }

  if(h_hitbcid) delete h_hitbcid;
  for(int i=0;i<3;i++){
    if(h_time[i]) delete h_time[i];
  }

  canvas->cd(0)->Modified();
  canvas->Update();
  canvas_time->cd(1)->Modified();
  canvas_time->cd(2)->Modified();
  canvas_time->Update();
}

// ==================================================================
void DrawModule(int dif_id)
{
  if(dif_id>=NumDif){
    std::cerr << "Wrong dif id: dif=" << dif_id << std::endl;
    return;
  }

  pad[dif_id]->cd();
  Double_t FrameMargin = 10.;
  TH1D *frame = gPad->DrawFrame(
      -WATERTANK_X/2.-FrameMargin, 
      -WATERTANK_Y/2.-FrameMargin,
      WATERTANK_X/2. +FrameMargin,
      WATERTANK_Y/2. +FrameMargin);
  frame->GetXaxis()->SetTickLength(0.);
  frame->GetXaxis()->SetLabelSize (0.);
  frame->GetYaxis()->SetTickLength(0.);
  frame->GetYaxis()->SetLabelSize (0.);

  TBox *watertank = new TBox(
      -WATERTANK_X/2.,
      -WATERTANK_Y/2.,
      WATERTANK_X/2.,
      WATERTANK_Y/2.);
  watertank->SetFillColor(kCyan-10);
  watertank->Draw("same");

  for(int chip=0;chip<NumChip;chip++){
    for(int chipch=0;chipch<NumChipCh;chipch++){
      int view = mapping.view[dif_id][chip][chipch];
      int pln  = mapping.pln [dif_id][chip][chipch];
      int ch   = mapping.ch  [dif_id][chip][chipch];
      double x = mapping.x   [dif_id][chip][chipch];
      double y = mapping.y   [dif_id][chip][chipch];
      double z = mapping.z   [dif_id][chip][chipch];
      if((pln<0)||(pln>7)||(ch<0)||(ch>79)){
        printf("wrong alignment of pln/ch, pln:%d ch:%d\n",pln,ch);
        return;
      }
      bool isGrid;
      if(ch<40){isGrid=false;}else{isGrid=true;}
      Float_t XX,YY;
      XX = z;
      if     (view==SideView){ YY = y; }
      else if(view==TopView ){ YY = x; }
      else{
        printf("wrong dif id\n");
        return;
      }
      if(!DrawScintillator(XX,YY,isGrid)){ return; }
    }
  }
  leg[dif_id] = new TLegend(margin_l+0.01,0.9,1.-margin_r,1.);
  leg[dif_id]->SetFillStyle(0);
  leg[dif_id]->SetBorderSize(0);
  if(dif_id==TopView ) leg[dif_id]->SetHeader("Top View","c");
  if(dif_id==SideView) leg[dif_id]->SetHeader("Side View","c");
  leg[dif_id]->SetTextSize(.1);
  leg[dif_id]->Draw();

  canvas->cd(0)->Modified();
  canvas->Update();

  return;
};//DrawModule

// ==================================================================
bool DrawScintillator(double x, double y, bool grid)
{
  double sci_x, sci_y;
  if(grid){ sci_x=C_WMScintiWidth; sci_y=C_WMScintiThick; }
  else    { sci_x=C_WMScintiThick; sci_y=C_WMScintiWidth; }

  TBox *scinti = new TBox(x-sci_x/2.,y-sci_y/2.,x+sci_x/2.,y+sci_y/2.);
  scinti->SetLineColor(kSpring-1);
  scinti->SetLineWidth(1);
  scinti->Draw("l same");
  return true;
}; //DrawScintillator
