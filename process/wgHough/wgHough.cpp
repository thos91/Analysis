#include <iostream>
#include <TH2D.h>
#include <TCanvas.h>
#include <TROOT.h>
#include <TApplication.h>

#include "wgGetTree.h"
#include "wgErrorCode.h"
#include "wgTools.h"
#include "Const.hpp"
#include "DetectorConst.hpppp"
#include "wgReconClass.h"

#define DEBUG_RECON

using namespace std;

int main(int argc, char* argv[]){
  TROOT root("GUI","GUI");
  TApplication App("App",0,0);
  int opt;
  int startevt=0;
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

  wgRecon wg_rec;

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
  for(int i=0;i<NumDif;i++){
    if(i==0) tree[i] = (TTree *)datafile1->Get("tree"); 
    if(i==1) tree[i] = (TTree *)datafile2->Get("tree"); 
    tree[i]->SetBranchAddress("charge"    , wg_rec.type_raw[i].charge    );
    tree[i]->SetBranchAddress("time"      , wg_rec.type_raw[i].time      );
    tree[i]->SetBranchAddress("gain"      , wg_rec.type_raw[i].gain      );
    tree[i]->SetBranchAddress("hit"       , wg_rec.type_raw[i].hit       );
    tree[i]->SetBranchAddress("bcid"      , wg_rec.type_raw[i].bcid      );
    tree[i]->SetBranchAddress("col"       , wg_rec.type_raw[i].col       );
    tree[i]->SetBranchAddress("ch"        , wg_rec.type_raw[i].ch        );
    tree[i]->SetBranchAddress("chip"      , wg_rec.type_raw[i].chip      );
    tree[i]->SetBranchAddress("chipid"    , wg_rec.type_raw[i].chipid    );
    tree[i]->SetBranchAddress("spill"     ,&wg_rec.type_raw[i].spill     );
    tree[i]->SetBranchAddress("spill_flag",&wg_rec.type_raw[i].spill_flag);
    tree[i]->SetBranchAddress("spill_mode",&wg_rec.type_raw[i].spill_mode);
    tree[i]->SetBranchAddress("spill_count",&wg_rec.type_raw[i].spill_count);
    tree[i]->SetBranchAddress("debug"     ,wg_rec.type_raw[i].debug     );
    tree[i]->SetBranchAddress("view"      ,&wg_rec.type_raw[i].view);
    tree[i]->SetBranchAddress("pln"       ,wg_rec.type_raw[i].pln);
    tree[i]->SetBranchAddress("ch"        ,wg_rec.type_raw[i].ch);
    tree[i]->SetBranchAddress("grid"      ,wg_rec.type_raw[i].grid);
    tree[i]->SetBranchAddress("x"         ,wg_rec.type_raw[i].x);
    tree[i]->SetBranchAddress("y"         ,wg_rec.type_raw[i].y);
    tree[i]->SetBranchAddress("z"         ,wg_rec.type_raw[i].z);
    tree[i]->SetBranchAddress("pe"        ,wg_rec.type_raw[i].pe);
    tree[i]->SetBranchAddress("gain"      ,wg_rec.type_raw[i].gain);
    tree[i]->SetBranchAddress("pedestal"  ,wg_rec.type_raw[i].pedestal);
    tree[i]->SetBranchAddress("ped_nohit" ,wg_rec.type_raw[i].ped_nohit);
  }

  int nevt1 = tree[0]->GetEntries();
  int nevt2 = tree[1]->GetEntries();

  int offset = 0;
  cout << "** inputFile1 : " << filename1 << " **" << endl;
  cout << "** inputFile2 : " << filename2 << " **" << endl;
  std::cout << "Number of event: " << nevt1 << " (dif1), " << nevt2 << " (dif2)" << std::endl;
  for(int ievt=startevt;ievt+offset<nevt2&&ievt<nevt1;ievt++){
    wg_rec.clear();
#ifdef DEBUG_RECON
    std::cout << "===================================================="
      << std::endl
      << "event=" << ievt << std::endl;

#else
    if(ievt%1000==0) std::cout << "event=" << ievt << std::endl;
#endif
    tree[0]->GetEntry(ievt);
    tree[1]->GetEntry(ievt+offset);
    if(wg_rec.type_raw[0].spill_count!=wg_rec.type_raw[1].spill_count){
      std::cout << "ACQ ID is shifted:"
        << "dif1="   << wg_rec.type_raw[0].spill_count
        << ", dif2=" << wg_rec.type_raw[1].spill_count
        << std::endl;
      offset = wg_rec.type_raw[0].spill_count-wg_rec.type_raw[1].spill_count;
      tree[1]->GetEntry(ievt+offset);
    }

    // ===============
    // Push hits into vector
    // Note: This is required to be set with "chipid", not with "chip".

    for(int dif=0;dif<NumDif;dif++){
      for(int chip=0;chip<NumChip;chip++){
        int chipid = wg_rec.type_raw[dif].chipid[chip];
        for(int chipch=0;chipch<NumChipCh;chipch++){
          for(int sca=0;sca<NumSca;sca++){
            if(wg_rec.type_raw[dif].hit[chip][chipch][sca]==1){              
              int view = wg_rec.type_map.view[dif][chipid][chipch];
              int pln  = wg_rec.type_map.pln [dif][chipid][chipch];
              int ch   = wg_rec.type_map.ch  [dif][chipid][chipch];
              int charge = wg_rec.type_raw[dif].charge[chip][chipch][sca]; 
              int tdc    = wg_rec.type_raw[dif].time  [chip][chipch][sca]; 
              int gs     = wg_rec.type_raw[dif].gs  [chip][chipch][sca]; 
              int bcid = wg_rec.type_raw[dif].bcid[chip][sca];
              double pe = wg_rec.type_raw[dif].pe[chip][chipch][sca];
              double time_ns;
              if(bcid%2==0) time_ns = ((double)tdc-TimeOffsetEven)*TimeCoeffEven;
              else          time_ns = ((double)tdc-TimeOffsetOdd )*TimeCoeffOdd ;
              wg_rec.pushHitInfo(bcid,view,pln,ch,dif,chipid,chipch,sca,charge,gs,tdc,pe,time_ns);
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
      wg_rec.pushHitStruct(bcid,view,pln,ch,dif,chip,chipch,sca,adc,gs,tdc,pe,time);
    }

    if(wg_rec.findBCIDCluster()){
      if(wg_rec.findTimeCluster()){
        cout << "===================================" << endl;
        cout << "Hough is done..." << endl;
        const double SCINTI_W2 = SCINTI_W*SCINTI_W/4.;
        const double SCINTI_T2 = SCINTI_T*SCINTI_T/4.;
        TCanvas *c1 = new TCanvas("c1","c1",800,600);
        c1->Divide(2);
        for(int i=0;i<wg_rec.type_hit.num_bcid_cluster;i++){
          int timecluster_size = (int)wg_rec.type_recon.time_cluster_hitid[i].size();
          TH2D *h[2];
          for(int iview=0;iview<2;iview++){
            h[iview] = new TH2D(Form("h%d",iview),"h",314.,0.,3.15,1000.,-500.,500.);
          }
          for(int j=0;j<timecluster_size;j++){
            int hitid  = wg_rec.type_recon.time_cluster_hitid[i][j];
            int dif    = wg_rec.get_hitdif   (hitid);
            int chip   = wg_rec.get_hitchip  (hitid);
            int chipch = wg_rec.get_hitchipch(hitid);
            int view   = wg_rec.get_hitview(hitid);
            double xy,z;
            if(view==0) xy = wg_rec.type_map.x[dif][chip][chipch];
            else if(view==1) xy = wg_rec.type_map.y[dif][chip][chipch];
            else cout << "ERROR!" << endl;
            z = wg_rec.type_map.z[dif][chip][chipch]; 
            int   ch   = wg_rec.get_hitch  (hitid); 
            bool grid=false;
            if(ch>=40) grid=true;

            double theta=0.;
            while(theta<PI){
              double cos0=cos(theta); 
              double sin0=sin(theta);
              double r=z*cos0+xy*sin0;
              double err_r;
              if(grid) err_r = sqrt( SCINTI_W2 + SCINTI_T2 +(SCINTI_W2-SCINTI_T2)*cos0*cos0 );
              else     err_r = sqrt( SCINTI_W2 + SCINTI_T2 -(SCINTI_W2-SCINTI_T2)*cos0*cos0 );
              int size_fill= (int)err_r;
              for(int l=-size_fill;l<=size_fill;l++){
                h[view]->Fill(theta,r+l);
              }
              theta += 3.14/314.;
            }
          }//j
          for(int iview=0;iview<2;iview++){
            c1->cd(iview+1);
            h[iview]->Draw("colz");
          }
          c1->Update();
          std::cout << "Type q/quit for stop this program." << std::endl;
          std::string str;
          std::cin >> str;
          if(str=="q"||str=="quit"){ return 0; }
          delete h[0];
          delete h[1];
        }//i
      }
    }
  }
  return 0;
}
