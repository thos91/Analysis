/* ***********************************************************************
 * For calculation of hit efficiency
 * Program : wgHitEff.cpp
 * Name: Naruhiro Chikuma
 * Date: 2017-11-17 17:05:24 
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
#include "DetectorConst.hpppp"
#include "wgGetTree.h"
#include "Const.hpp"
#include "wgReconClass.h"

using namespace std;


int main(int argc, char **argv) 
{

  int opt;
  int start_evt=0;
  wgConst *con = new wgConst;
  con->GetENV();
  string inputFileName("");
  string inputDirName("");
  string outputDirName("");
  string logoutputDir=con->LOG_DIRECTORY;
  int runid  = -1;
  int acqid  = -1;
  int method = -1;

  while((opt = getopt(argc,argv, "r:s:m:i:o:h")) !=-1 ){
    switch(opt){
      case 'r':
        runid = atoi(optarg);
        break;
      case 's':
        acqid = atoi(optarg);
        break;
      case 'm':
        method = atoi(optarg);
        break;
      case 'i':
        inputDirName=optarg;
        break;
      case 'o':
        outputDirName=optarg;
        break;
      case 'h':
        cout <<"This program is for event display after reconstruction. "<<endl;
        cout <<"You can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -r (int)   : run id" << endl;
        cout <<"  -s (int)   : acq id" << endl;
        cout <<"  -m (int)   : hiteff calcu. method" << endl;
        cout <<"  -i (char*) : input directory"  << endl;
        cout <<"  -o (char*) : output directory" << endl;
        exit(0);
      default:
        cout << "To see help page:" << endl;
        cout << argv[0] << " -h"<<endl;
        exit(0);
    }   
  }

  if(inputDirName==""||outputDirName==""||inputDirName==NULL||runid==-1||acqid==-1){
    cout << "To see help page:" << endl;
    cout << argv[0] << " -h" <<endl;
    exit(0);
  }

  inputFileName = Form("%s/run_%05d_%03d_recon.root",inputDirName.c_str(),runid,acqid);
  
  // =============================================== //
  // ================= INITIALIZE ================== //
  // =============================================== //
  TH1D *h_track_side_cosmic_angleZ  = new TH1D("Cosmic_Track_Side_AngleZ" ,"Cosmic_Track_Side_AngleZ" ,90,0,90);
  TH1D *h_track_side_cosmic_angleXY = new TH1D("Cosmic_Track_Side_AngleXY","Cosmic_Track_Side_AngleXY",90,0,90);
  TH1D *h_track_side_sandmu_angleZ  = new TH1D("Sandmu_Track_Side_AngleZ" ,"Sandmu_Track_Side_AngleZ" ,90,0,90);
  TH1D *h_track_side_sandmu_angleXY = new TH1D("Sandmu_Track_Side_AngleXY","Sandmu_Track_Side_AngleXY",90,0,90);
  TH1D *h_track_top_cosmic_angleZ   = new TH1D("Cosmic_Track_Top_AngleZ"  ,"Cosmic_Track_Top_AngleZ"  ,90,0,90);
  TH1D *h_track_top_cosmic_angleXY  = new TH1D("Cosmic_Track_Top_AngleXY" ,"Cosmic_Track_Top_AngleXY" ,90,0,90);
  TH1D *h_track_top_sandmu_angleZ   = new TH1D("Sandmu_Track_Top_AngleZ"  ,"Sandmu_Track_Top_AngleZ"  ,90,0,90);
  TH1D *h_track_top_sandmu_angleXY  = new TH1D("Sandmu_Track_Top_AngleXY" ,"Sandmu_Track_Top_AngleXY" ,90,0,90);

  TH1D *h_hit_expected_cosmic_angleZ  = new TH1D("Cosmic_Hit_Expected_AngleZ" ,"Cosmic_Hit_Expected_AngleZ" ,90,0,90);
  TH1D *h_hit_expected_cosmic_angleXY = new TH1D("Cosmic_Hit_Expected_AngleXY","Cosmic_Hit_Expected_AngleXY",90,0,90);
  TH1D *h_hit_expected_sandmu_angleZ  = new TH1D("Sandmu_Hit_Expected_AngleZ" ,"Sandmu_Hit_Expected_AngleZ" ,90,0,90);
  TH1D *h_hit_expected_sandmu_angleXY = new TH1D("Sandmu_Hit_Expected_AngleXY","Sandmu_Hit_Expected_AngleXY",90,0,90);
  TH1D *h_hit_measured_cosmic_angleZ  = new TH1D("Cosmic_Hit_Measured_AngleZ" ,"Cosmic_Hit_Measured_AngleZ" ,90,0,90);
  TH1D *h_hit_measured_cosmic_angleXY = new TH1D("Cosmic_Hit_Measured_AngleXY","Cosmic_Hit_Measured_AngleXY",90,0,90);
  TH1D *h_hit_measured_sandmu_angleZ  = new TH1D("Sandmu_Hit_Measured_AngleZ" ,"Sandmu_Hit_Measured_AngleZ" ,90,0,90);
  TH1D *h_hit_measured_sandmu_angleXY = new TH1D("Sandmu_Hit_Measured_AngleXY","Sandmu_Hit_Measured_AngleXY",90,0,90);

  Hit_t t_hit;
  Recon_t t_recon;
  Track_t t_track;
  // =================
  // set TTree
  wgGetTree *gettree = new wgGetTree(inputFileName);
  gettree->SetTreeFile(t_hit);
  gettree->SetTreeFile(t_recon);
  gettree->SetTreeFile(t_track);

  int spill,spill_mode;
  gettree->tree->SetBranchAddress("spill",&spill);
  gettree->tree->SetBranchAddress("spill_mode",&spill_mode);

  int nevt = wgGetTree::tree->GetEntries();
  int count_nevt = 0;
  for(int ievt=start_evt;ievt<nevt;ievt++){
    gettree->GetEntry(ievt);
    if(ievt%1000==0){
      std::cout << " event=" << ievt << ", spill=" << spill <<std::endl;
    }
    bool xy_layer_hit  [ 8]; memset(xy_layer_hit  ,false, 8);
    bool grid_layer_hit[20]; memset(grid_layer_hit,false,20);
    int max_xy_layer = -1;
    int min_xy_layer =  8;
    int max_grid_layer = -1;
    int min_grid_layer = 20;
    if(t_track.num_trackid==1){
      for(int itrack=0;itrack<2;itrack++){
        double angleZ  = fabs(atan(   t_track.track_slope[itrack])*180./3.1416);
        double angleXY = fabs(atan(1./t_track.track_slope[itrack])*180./3.1416);
        int track_view    = t_track.track_view[itrack];
        int num_track_hit = t_track.num_track_hits[itrack];
        for(int ihit1=0;ihit1<num_track_hit-1;ihit1++){
          int hitid1 = t_track.track_hits_hitid[itrack][ihit1];
          int view1  = t_hit.hit_view [hitid1];
          int pln1   = t_hit.hit_pln  [hitid1];
          int ch1    = t_hit.hit_ch   [hitid1];
          bool grid1 = t_hit.hit_grid [hitid1];
          double pe1 = t_hit.hit_pe   [hitid1];
          if(pe1<0.5) continue;
          if(view1!=track_view) continue;

          if(method==0){
            if(grid1) grid_layer_hit[ch1%20] = true;
            else      xy_layer_hit  [pln1  ] = true;
          }
          else if(method==1){
            if(grid1){
              grid_layer_hit[ch1%2] = true;
              xy_layer_hit  [pln1 ] = true;
            }
            else{
              grid_layer_hit[ch1/2] = true;
              xy_layer_hit  [pln1 ] = true;
            }
          }
          else if(method==2){
            int xy_layer_d,xy_layer_u,grid_layer_d,grid_layer_u;
            if(grid1){
              grid_layer_d = ch1%20;
              grid_layer_u = ch1%20;
              xy_layer_u = pln1;
              xy_layer_d = pln1;
              if     (view1==0&&ch1>=60){xy_layer_u--;}
              else if(view1==0&&ch1>=40){xy_layer_d++;}
              else if(view1==1){xy_layer_d++;}
            }
            else{
              grid_layer_d = (ch1+1)/2;
              grid_layer_u = (ch1-1)/2;
              xy_layer_u = pln1;
              xy_layer_d = pln1;
            }
            if(max_grid_layer < grid_layer_d) max_grid_layer = grid_layer_d;
            if(max_xy_layer   < xy_layer_d  ) max_xy_layer   = xy_layer_d  ;
            if(min_grid_layer > grid_layer_u) min_grid_layer = grid_layer_u;
            if(min_xy_layer   > xy_layer_u  ) min_xy_layer   = xy_layer_u  ;
            if(grid1) grid_layer_hit[ch1%20] = true;
            else      xy_layer_hit  [pln1  ] = true;
          }
        } //hit1

        int hitnum_grid_expected = 0;
        int hitnum_grid_measured = 0;
        int hitnum_xy_expected   = 0;
        int hitnum_xy_measured   = 0;
        if(method==0||method==1){
          for(int igrid=1;igrid<19;igrid++){
            if(grid_layer_hit[igrid-1]&&grid_layer_hit[igrid+1]){
              hitnum_grid_expected++;
              if(grid_layer_hit[igrid]){hitnum_grid_measured++;}
            }
          }
          for(int ixy=1;ixy<7;ixy++){
            if(xy_layer_hit[ixy-1]&&xy_layer_hit[ixy+1]){
              hitnum_xy_expected++;
              if(xy_layer_hit[ixy]){hitnum_xy_measured++;}
            }
          }
        }
        else if(method==2){
          for(int igrid=min_grid_layer+1;igrid<max_grid_layer;igrid++){
            hitnum_grid_expected++;
            if(grid_layer_hit[igrid]){hitnum_grid_measured++;}
          }
          for(int ixy=min_xy_layer+1;ixy<max_xy_layer;ixy++){
            hitnum_xy_expected++;
            if(xy_layer_hit[ixy]){hitnum_xy_measured++;}
          }
        }

        if(spill_mode==0){
          h_hit_expected_cosmic_angleZ  ->Fill(angleZ ,hitnum_xy_expected);
          h_hit_measured_cosmic_angleZ  ->Fill(angleZ ,hitnum_xy_measured);
          h_hit_expected_cosmic_angleXY ->Fill(angleXY,hitnum_grid_expected);
          h_hit_measured_cosmic_angleXY ->Fill(angleXY,hitnum_grid_measured);
        }
        else{
          h_hit_expected_sandmu_angleZ  ->Fill(angleZ ,hitnum_xy_expected);
          h_hit_measured_sandmu_angleZ  ->Fill(angleZ ,hitnum_xy_measured);
          h_hit_expected_sandmu_angleXY ->Fill(angleXY,hitnum_grid_expected);
          h_hit_measured_sandmu_angleXY ->Fill(angleXY,hitnum_grid_measured);
        }

        // ===================
        // Fill track angles
        //
        if(spill_mode==0){
          if(track_view==0){ 
            h_track_top_cosmic_angleZ ->Fill(angleZ );
            h_track_top_cosmic_angleXY->Fill(angleXY);
          }
          else if(track_view==1){ 
            h_track_side_cosmic_angleZ ->Fill(angleZ );
            h_track_side_cosmic_angleXY->Fill(angleXY);
          }
        }
        else{
          if(track_view==0){
            h_track_top_sandmu_angleZ ->Fill(angleZ );
            h_track_top_sandmu_angleXY->Fill(angleXY);
          }
          else if(track_view==1){
            h_track_side_sandmu_angleZ ->Fill(angleZ );
            h_track_side_sandmu_angleXY->Fill(angleXY);
          }
        }
      }
    }
  }//ievt
  std::cout << "Number of selected events = " << count_nevt << std::endl;
  std::cout << "Number of spill = " << nevt << std::endl;

  string filename = Form("%s/run_%05d_%03d_hiteff_method%d.root",
      outputDirName.c_str(),runid,acqid,method);

  TFile *fout = new TFile(filename.c_str(),"recreate");
  h_track_side_cosmic_angleZ  ->Write();
  h_track_side_cosmic_angleXY ->Write();
  h_track_side_sandmu_angleZ  ->Write();
  h_track_side_sandmu_angleXY ->Write();
  h_track_top_cosmic_angleZ   ->Write();
  h_track_top_cosmic_angleXY  ->Write();
  h_track_top_sandmu_angleZ   ->Write();
  h_track_top_sandmu_angleXY  ->Write();
  h_hit_expected_cosmic_angleZ  ->Write();
  h_hit_measured_cosmic_angleZ  ->Write();
  h_hit_expected_cosmic_angleXY ->Write();
  h_hit_measured_cosmic_angleXY ->Write();
  h_hit_expected_sandmu_angleZ  ->Write();
  h_hit_measured_sandmu_angleZ  ->Write();
  h_hit_expected_sandmu_angleXY ->Write();
  h_hit_measured_sandmu_angleXY ->Write();

  fout->Write();
  fout->Close();

  return 0;
} //main
