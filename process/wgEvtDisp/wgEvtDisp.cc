#include "TApplication.h"
#include "TFile.h"
#include "TH1D.h"
#include "TF1.h"
#include "TGraph.h"
#include "TGraphAsymmErrors.h"
#include "TH2D.h"
#include "TStyle.h"
#include "TString.h"
#include "TSystem.h"
#include "TSpectrum.h"
#include "TTree.h"
#include "TArc.h"
#include "TBox.h"
#include "TPolyLine.h"
#include "TLine.h"
#include "TCanvas.h"
#include "TText.h"

#include <iostream>
#include <fstream>
#include "math.h"
#include <sstream>
#include <complex>

#include "Const.h"
#include "DetectorConst.h"
#include "wgReconClass.h"
#include "wgDetectorDimension.h"
#include "wgChannelMap.h"
#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgDisp.h"

#define F_CIRCLE 0.3
using namespace std;
TPad   *pad[2];
TArc   *hits[2][32];
MapInv_t    mapping;
void DrawHits(int pad,int ihit,int view,int pln,int ch,double pe);
void clear();
int main(int argc, char** argv){

  //*****arguments*******************//
  int c = -1;
  string inputfilename("");
  int mode      = -1;
  //int type      = -1;
  double threshold = 3.;
  int bundle[2]      = {0,0};
  int view_bundle[2] = {0,0};
  int evt_start = 0;
  int num_hit_th = 0;
  //bool selecttype = false;
  bool SCANNING   = false;
  bool SAVE = false;
  bool start_run = true;
  //int hitview = 0;

  CheckExist    *check = new CheckExist;

  while ((c = getopt(argc, argv, "f:t:n:s:e:c:v:hi:j:")) != -1){
    switch(c){
      case 'f':
        inputfilename = optarg;
        if(!check->RootFile(inputfilename)){ 
          cout<<"!!Error!! "<<inputfilename.c_str()<<"is wrong!!";
          start_run = false;
        }
        break;
      case 't':
        threshold = atof(optarg);
        break;
      case 'n':
        num_hit_th = atoi(optarg);
        break;
      case 'e':
        evt_start = atoi(optarg);
        break;
      case 'c':
        SCANNING = true;
        if(atoi(optarg)!=0) SAVE = true;
        break;
      case 'i':
        bundle[0] = atoi(optarg);
        break;
      case 'j':
        bundle[1] = atoi(optarg);
        break;
      case 'h':
        cout << "-f <char*> : Input ROOT file." << endl;
        cout << "-t <doubel>: Threshold " << endl;
        cout << "-n <int>   : The number of hits threshold " << endl;
        cout << "-e <int>   : Start event number" << endl;
        cout << "-c <int>   : Auto mode (0:only display 1:print)" << endl;
        cout << "-i <int>   : 1st bundle number(ch0~31)" << endl;
        cout << "-j <int>   : 2nd bundle number(ch32~63)" << endl;
        cout << "       side : 1(down)~5(up) , top : 6(down)~10(up)"<<endl;
        exit(0);
    }
  }

  if(!start_run){cout << " aaaa" << endl; return 0;}

  for(int i=0;i<2;i++){
    if(bundle[i]>=1 && bundle[i]<=5)view_bundle[i]=1;
    else if(bundle[i]>=6 && bundle[i]<=10)view_bundle[i]=0;
    else {cout << "Bad Bundle Number!!! : " << bundle[i] << endl; return 0;}
  }

  //*****open root file and get tree*******************//
  wgChannelMap *wgmap = new wgChannelMap();
  mapping = wgmap->load_mapping_inv();
  TROOT root("GUI","GUI");
  TApplication theApp("App",0,0);

  cout << "=== Open file : " << inputfilename.c_str() << "===" << endl;
  TFile *inputfile = new TFile(inputfilename.c_str(),"read"); 
  TTree* tree = (TTree*) inputfile ->Get("tree");

  int adcHigh[64];
  double pe[64];
  int hit[64];
  tree->SetBranchAddress("adcHigh",adcHigh);
  tree->SetBranchAddress("pe",pe);
  tree->SetBranchAddress("hit",hit);

  int nevt = (int)tree -> GetEntries();
  cout << "Total # of events = " << nevt << endl; 

  // *****event loop start************ ///
  bool newcanv = true;
  TCanvas *c1;

  gROOT->SetStyle("Plain");
  if (newcanv){
    double canvas_norm = 0.8;
    c1 = new TCanvas("c1","c1",800*canvas_norm,600*canvas_norm);
    newcanv = false;
  }

  double pad_div[2][4]= {
    {0.01,0.01,0.49,0.99},
    {0.51,0.01,0.99,0.99}
  };

  for(int i=0;i<2;i++){
    string name = Form("pad%d",i);
    pad[i] = new TPad(name.c_str(),name.c_str(),
        pad_div[i][0],
        pad_div[i][1],
        pad_div[i][2],
        pad_div[i][3]
        );
    pad[i]->Draw();
  }

  for(int i=0;i<2;i++){
    pad[i]->cd();
    Double_t FrameMargin = 10.;
    TH1D *frame = gPad->DrawFrame(
        -WATERTANK_X/2.-FrameMargin, 
        -WATERTANK_Y/2.-FrameMargin+120,
        -WATERTANK_X/2. +FrameMargin+120,
        -WATERTANK_Y/2. +FrameMargin+120+220);
    frame->GetXaxis()->SetTickLength(0.);
    frame->GetXaxis()->SetLabelSize (0.);
    frame->GetYaxis()->SetTickLength(0.);
    frame->GetYaxis()->SetLabelSize (0.);
    TBox *watertank = new TBox(
        -WATERTANK_X/2.,
        -WATERTANK_Y/2.+120,
        -WATERTANK_X/2.+120,
        -WATERTANK_Y/2.+120+220);
    watertank->SetFillColor(kCyan-10);
    watertank->Draw("same");
  }

  //*****make pads for sideview and topveiw****************//
  wgDisp *disp = new wgDisp();
  for(int i=0;i<2;i++){  
    pad[i]->cd();
    if(view_bundle[i]==0){
      disp->drawpartx(true); //draw module in sideview//
      disp->drawtext(Form("bundle_%d",i),0.1,0.95,0.04);
    }else{
      disp->drawparty(true); //draw module in topview//
      disp->drawtext(Form("bundle_%d",i),0.1,0.95,0.04);
    }
  }

  for(int i=0;i<2;i++){
    pad[i]->cd();
    for(int j=0;j<32;j++){
      hits[i][j] = new TArc(-WATERTANK_X,-WATERTANK_Y,0);
      hits[i][j]->SetFillColor(kRed);
    }
  }

  wgChannelMap * cvt = new wgChannelMap;


  //********** Read event ************
  for(int ievt = evt_start;ievt<nevt;ievt++){
    tree->GetEntry(ievt);
    // hit threshold
    int num_hit[2]={0,0};
    for(int ichan=0;ichan<32;ichan++){
      if(pe[ichan]>threshold) num_hit[0]++;
      if(pe[ichan+32]>threshold) num_hit[1]++;
    }
    if(num_hit[0]<num_hit_th && num_hit[1] <num_hit_th) continue;   
    clear();
    //pad[0]->Draw();
    //pad[1]->Draw();
    int hitpln=0;
    int hitch=0;
    int nhit[2]={0,0};
    for(int ichan=0; ichan<64 ; ichan ++){
      double hitpe = pe[ichan]; 
      if(pe[ichan]>threshold){
        if(ichan<=31){
          if(view_bundle[0]==1){cvt->EASIROCtoSIDE(ichan,hitpln,hitch);}
          else if(view_bundle[0]==0){cvt->EASIROCtoTOP(ichan,hitpln,hitch);}
          pad[0]->cd();
          DrawHits(0,nhit[0],view_bundle[0],hitpln,hitch,hitpe);
          cout << "bundle : 0 " << "view:" << view_bundle[0] << " pln:" << hitpln << " ch:"<< hitch <<" (easiroc ch:" << ichan << ")"<< endl;
          nhit[0]++;
        }else{
          if(view_bundle[1]==1){cvt->EASIROCtoSIDE(ichan-32,hitpln,hitch);}
          else if(view_bundle[1]==0){cvt->EASIROCtoTOP(ichan-32,hitpln,hitch);}
          pad[1]->cd();
          DrawHits(1,nhit[1],view_bundle[1],hitpln,hitch,hitpe);
          cout << "bundle : 1 " << "view:" << view_bundle[1] << " pln:" << hitpln << " ch:"<< hitch <<" (easiroc ch:" << ichan << ")"<< endl;
          nhit[1]++;
        }
      }
    }

    c1->cd(0);
    c1->Update();
    cout << "Event # is " << ievt << " , number of hit : (" << nhit[0] << "," << nhit[1] << ")"  <<endl;
    if(SCANNING){
      cout << " =========== Next. ========== \n" << endl;
      if(!SAVE){
        sleep(1);
      }else{
        stringstream filename;
        c1->Print(filename.str().c_str());
      }

    }else{
      printf("  Type \' n\' to ve to next event.\n");
      printf("  Type \' s\' to save the event display.\n");
      printf("  Type \' q\' to quit.\n");
      printf("  Type \' e\' to get a event you want.\n");
      printf("  Type \' m\' to change mode.\n");
      printf("  Type any other key to go to the next event.\n");

      while(1){
        char ans[8];
        cin >> ans;
        if(*ans == 'n'){
          cout << ">> Next." << endl;
          break;
        }
        else if(*ans == 's'){
          string filename = Form("./event%d.png",ievt);
          c1->Print(filename.c_str());
          //break;
        }
        else if(*ans == 'q') exit(0);
        else if(*ans == 'e'){
          int eventnumber;
          cout << "  Type the number of event :";
          cin >> eventnumber;
          ievt = eventnumber - 1;
          break;
        }
        else if(*ans == 'm'){
          cout << "  Type the mode :";
          cin >> mode;
          ievt = ievt - 1;
          break;
        }
        //else{
        printf("  Type \' n\' to move to next event.\n");
        printf("  Type \' s\' to save the event display.\n");
        printf("  Type \' q\' to quit.\n");
        printf("  Type \' e\' to get a event you want.\n");
        printf("  Type \' m\' to change mode.\n");
        printf("  Type any other key to go to the next event.\n");
        //}
      }
    }
  }
  return 0;
}

// ==================================================================
void clear(){
  for(int i=0;i<2;i++){
    pad[i]->cd();
    for(int j=0;j<32;j++){
      hits[i][j]->SetX1(-WATERTANK_X);
      hits[i][j]->SetY1(-WATERTANK_Y);
      hits[i][j]->SetR1(0.);
      hits[i][j]->SetR2(0.);
    }
    pad[i]->Draw();
  }
}
//**************************************************************
void DrawHits(int ipad,int ihit,int view, int pln ,int ch,double pe){
  double XX, YY;
  XX = mapping.z[view][pln][ch];
  if(view==0)YY = mapping.x[view][pln][ch];
  else YY = mapping.y[view][pln][ch];
  pad[ipad]->cd();
  hits[ipad][ihit]->SetX1(XX);
  hits[ipad][ihit]->SetY1(YY);
  hits[ipad][ihit]->SetR1(pe*F_CIRCLE);
  hits[ipad][ihit]->SetR2(pe*F_CIRCLE);
  hits[ipad][ihit]->Draw("");
}

