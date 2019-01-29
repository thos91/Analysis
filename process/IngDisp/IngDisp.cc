#include <TROOT.h>
#include <TApplication.h>
#include <TFile.h>
#include <TH1F.h>
#include <TF1.h>
#include <TGraph.h>
#include <TGraphAsymmErrors.h>
#include <TH2F.h>
#include <TStyle.h>
#include <TString.h>
#include <TSystem.h>
#include <TSpectrum.h>
#include <TTree.h>
#include <TArc.h>
#include <TBox.h>
#include <TLegend.h>
#include <TLine.h>
#include <TCanvas.h>
#include <TText.h>
#include <Const.h>

#include <iostream>
#include <fstream>
#include <math.h>
#include <sstream>

#include "Const.h"
#include "DetectorConst.h"
#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgChannelMap.h"
#include "wgGetTree.h"
#include "wgDisp.h"
#include "wgDetectorDimension.h"

#define NUM_VIEWCH_WG  640
#define NUM_VIEWCH_ING 308
#define NUM_VIEWCH_PM  602

#define INITIAL_RADIUS 0.
#define F_CIRCLE 10.
#define LINE_STYLE 7

void clear_ing();
void clear_wg();
void DrawModule();
void DrawWgHit(int i_bcid,Hit_t type_hit);
void DrawWgRecon(int current_bcid, Recon_t t_recon);
void DrawIngRecon(int cyc,IngRecon_t type_ing);


TPad   *pad   [2];
TArc   *wghit [2][NUM_VIEWCH_WG ];
TArc   *inghit[2][NUM_VIEWCH_ING];
TArc   *pmhit [2][NUM_VIEWCH_PM ];
TLine  *wgrecon [2][MAX_NUM_TRACK];
TLine  *ingrecon[2][MAX_NUM_INGRECON];
TLine  *pmrecon [2][MAX_NUM_INGRECON];
TLegend *leg[2];
Map_t    mapping;
MapInv_t mapping_inv;
TCanvas *canvas;
const double margin_t = 0.00;
const double margin_b = 0.00;
const double margin_l = 0.00;
const double margin_r = 0.00;
const double line_width = 5;

int main(int argc, char **argv) 
{
  int opt;
  std::string str;
  bool scan = false;
  bool print = false;
  int mode=-1;
  int start_wgevt=0;
  int start_ingevt=0;
  int thres_numwgrecon = -1;
  int thres_numingrecon = -1;
  wgConst *con = new wgConst;
  con->GetENV();
  string wgFileName("");
  string ingFileName("");
  string logoutputDir=con->LOG_DIRECTORY;

  CheckExist *check = new CheckExist;

  while((opt = getopt(argc,argv, "f:i:e:n:m:hr:t:sp")) !=-1 ){
    switch(opt){
      case 'f':
        wgFileName=optarg;
        if(!check->RootFile(wgFileName)){ 
          cout<<"!!Error!! "<<wgFileName.c_str()<<"doesn't exist!!";
          return 1;
        }
        break;
      case 'i':
        ingFileName=optarg;
        if(!check->RootFile(ingFileName)){ 
          cout<<"!!Error!! "<<ingFileName.c_str()<<"doesn't exist!!";
          return 1;
        }
        break;
      case 'e':
        start_wgevt = atoi(optarg); 
        break;
      case 'n':
        start_ingevt = atoi(optarg); 
        break;
      case 'm':
        mode = atoi(optarg); 
        break;
      case 'r':
        thres_numwgrecon = atoi(optarg); 
        break;
      case 't':
        thres_numingrecon = atoi(optarg); 
        break;
      case 's':
        scan = true;
        break;
      case 'p':
        print = true;
        break;

      case 'h':
        cout << argv[0] <<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose input wagasci recon file." <<endl;
        cout <<"  -i (char*) : choose input ingrid recon file."  <<endl;
        cout <<"  -e (int)   : choose start wagasci event # (default: 0)"<<endl;
        cout <<"  -n (int)   : choose start ingri event # (default: 0)"<<endl;
        cout <<"  -m (int)   : choose spill_mode < 0 or 1 > (default: -1: both)"<<endl;
        cout <<"  -r (int)   : set threshold for num wagasci recon (default:"<< thres_numwgrecon 
          <<  ")"<<endl;
        cout <<"  -t (int)   : set threshold for num ingrid  recon (default:"<< thres_numingrecon
          <<  ")"<<endl;
        cout <<"  -s         : scan mode (default: false)" << endl;
        cout <<"  -p         : print all mode (default: false)" << endl;
        exit(0);
      default:
        cout << argv[0] <<endl;
        cout <<"  -h         : help"<<endl;
        exit(0);
    }   
  }
  if(wgFileName==""||ingFileName==""){
    cout << "!!ERROR!! please input filenames." <<endl;
    cout << "help : ./wgReconDisp -h" <<endl;
    exit(1);
  }
  if(mode!=-1&&mode!=0&&mode!=1){
    cout << "!!ERROR!! please input filenames." <<endl;
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
  canvas = new TCanvas("canvas","canvas",800,900);

  double pad_div[2][4]= {
    {0.00,0.00,1.00,0.50},
    {0.00,0.50,1.00,1.00}
  };

  for(int i=0;i<2;i++){
    string name = Form("pad%d",i);
    pad[i]=new TPad(name.c_str(),name.c_str(),
        pad_div[i][0],
        pad_div[i][1],
        pad_div[i][2],
        pad_div[i][3]
        );
    pad[i]->SetMargin(0.03,0.03,0.03,0.1);
    pad[i]->Draw();
  }

  wgDetectorDimension *detdim = new wgDetectorDimension();
  DrawModule();

  for(int view=0;view<NumView;view++){
    double x,y,z,xy;
    for(int j=0;j<NUM_VIEWCH_WG;j++){
      int pln = j/80;
      int ch  = j%80;
      int grid; if(ch<40) grid=0; else if(ch<60) grid=1; else grid=2;
      detdim->GetPosWM(pln,view,ch,grid,&x,&y,&z);
      x+=C_B2WMPosX;
      y+=C_B2WMPosY;
      z+=C_B2WMPosZ;
      if(view==SideView){
        xy = y;
        pad[1]->cd();
      }
      else{
        xy = x;
        pad[0]->cd();
      }
      wghit [view][j] = new TArc(z,xy,INITIAL_RADIUS);
      wghit [view][j]->SetFillColor(kRed);
      wghit [view][j]->Draw("");
    }
    for(int j=0;j<NUM_VIEWCH_ING;j++){
      int pln = j/24;
      int ch  = j%24;
      if(pln>=11){
        pln = (j-24*11)/22+11+2*(view==SideView);
        ch  = (j-24*11)%22;
      }
      detdim->GetPosING(14,pln,view,ch,&x,&y,&z);
      x+=C_B2d1INGPosX;
      y+=C_B2d1INGPosY;
      z+=C_B2d1INGPosZ;
      if(view==SideView){
        xy = y;
        pad[1]->cd();
      }
      else{
        xy = x;
        pad[0]->cd();
      }
      inghit[view][j] = new TArc(z,xy,INITIAL_RADIUS);
      inghit[view][j]->SetFillColor(kRed);
      inghit[view][j]->Draw("");
    }
    for(int j=0;j<NUM_VIEWCH_PM;j++){
      int pln, ch;
      if(j<24){
        pln = 0;
        ch  = j;
      }
      else{
        pln = (j-24)/32+1;
        ch  = (j-24)%32;
        if(pln>=18){
          pln = (j-24-32*17)/17*2+18+1*(view==TopView_i);
          ch  = (j-24-32*17)%17;
        }
      }
      detdim->GetPosPM(pln,view,ch,&x,&y,&z);
      x+=C_B2CHPosX;
      y+=C_B2CHPosY;
      z+=C_B2CHPosZ;
      if(view==SideView_i){
        xy = y; 
        pad[1]->cd();
      }else{
        xy = x;
        pad[0]->cd();
      }
      pmhit [view][j] = new TArc(z,xy,INITIAL_RADIUS);
      pmhit [view][j]->SetFillColor(kRed);
      pmhit [view][j]->Draw("");
    }
    for(int j=0;j<MAX_NUM_TRACK;j++){      
      if(view==SideView){pad[1]->cd();}
      else              {pad[0]->cd();}
      wgrecon [view][j] = new TLine(0.,0.,0.,0.);
      wgrecon [view][j]->SetLineStyle(LINE_STYLE);
      wgrecon [view][j]->SetLineColor(kBlack);
      wgrecon [view][j]->SetLineWidth(line_width);
      wgrecon [view][j]->Draw("");
    }
    for(int j=0;j<MAX_NUM_INGRECON;j++){
      if(view==SideView){pad[1]->cd();}
      else              {pad[0]->cd();}
      ingrecon[view][j] = new TLine(0.,0.,0.,0.);
      ingrecon[view][j]->SetLineStyle(LINE_STYLE);
      ingrecon[view][j]->SetLineColor(kBlack);
      ingrecon[view][j]->SetLineWidth(line_width);
      ingrecon[view][j]->Draw("");
      if(view==SideView_i){pad[1]->cd();}
      else                {pad[0]->cd();}
      pmrecon [view][j] = new TLine(0.,0.,0.,0.);
      pmrecon [view][j]->SetLineStyle(LINE_STYLE);
      pmrecon [view][j]->SetLineColor(kBlack);
      pmrecon [view][j]->SetLineWidth(line_width);
      pmrecon [view][j]->Draw("");
    }
  }

  canvas->cd(0)->Modified();
  canvas->Update();


  // =================
  // set TTree
  cout << "Set TTree." << endl;
  IngRecon_t type_ing;
  Track_t    type_track;
  Recon_t    type_recon;
  Hit_t      type_hit;
  int spill,spill_mode;
  wgGetTree *inggettree = new wgGetTree(ingFileName,type_ing);
  wgGetTree *wggettree = new wgGetTree(wgFileName);
  wggettree->SetTreeFile(type_track);
  wggettree->SetTreeFile(type_recon);
  wggettree->SetTreeFile(type_hit  );
  wggettree->tree->SetBranchAddress("spill"     ,&spill     );
  wggettree->tree->SetBranchAddress("spill_mode",&spill_mode);

  int ingnevt = inggettree->ingtree->GetEntries();
  int wgnevt  = wggettree ->tree->GetEntries();

  /*
  // ==========================
  // Get start/stop information
  //
  cout << "Getting time information...." << std::endl;
  TH1F *h_start_time = (TH1F*) wggettree->finput->Get("start_time");
  int wg_start_time = h_start_time->GetMean();
  TH1F *h_stop_time = (TH1F*) wggettree->finput->Get("stop_time");
  int wg_stop_time = h_stop_time->GetMean();
  delete h_start_time;
  delete h_stop_time;
  int ing_start_time = inggettree->ingtree->GetMinimum("unixtime");
  int ing_stop_time  = inggettree->ingtree->GetMaximum("unixtime");
  int wg_start_spill = 0xffff;
  int wg_stop_spill  = 0; 
  for(int i=0;i<wgnevt;i++){
    wggettree->GetEntry(i);
    if(spill_mode==1&&spill>wg_stop_spill ) wg_stop_spill =spill;
    if(spill_mode==1&&spill<wg_start_spill) wg_start_spill=spill;
  }

  struct tm tm;
  char buf[50];
  time_t t;
  std::istringstream sstr;

  std::string str_wg_start_time;
  t = wg_start_time;
  localtime_r(&t,&tm);
  str = asctime_r(&tm,buf);
  sstr.str(str);
  getline(sstr,str_wg_start_time);

  std::string str_wg_stop_time;
  t = wg_stop_time;
  localtime_r(&t,&tm);
  str = asctime_r(&tm,buf);
  sstr.str(str);
  getline(sstr,str_wg_stop_time);

  std::string str_ing_start_time;
  t = ing_start_time;
  localtime_r(&t,&tm);
  str = asctime_r(&tm,buf);
  sstr.str(str);
  getline(sstr,str_ing_start_time);

  std::string str_ing_stop_time;
  t = ing_stop_time;
  localtime_r(&t,&tm);
  str = asctime_r(&tm,buf);
  sstr.str(str);
  getline(sstr,str_ing_stop_time);

  std::cout << "WAGASCI start time : "  << wg_start_time  << " " << str_wg_start_time  << std::endl;
  std::cout << "WAGASCI stop time  : "  << wg_stop_time   << " " << str_wg_stop_time   << std::endl;
  std::cout << "INGRID start time  : "  << ing_start_time << " " << str_ing_start_time << std::endl;
  std::cout << "INGRID stop time   : "  << ing_stop_time  << " " << str_ing_stop_time  << std::endl;
  std::cout << "WAGASCI start spill = " << wg_start_spill << std::endl;
  std::cout << "WAGASCI stop spill  = " << wg_stop_spill  << std::endl;

  cout << "Type a key to start this event display." << endl;
  cout << "Type q/quit to exit from this program." << endl;
  std::cin >> str;
  if(str=="q"||str=="quit"){ return false; }
  */


  // ==========================
  // Start event loop
  //
  int ingevt  = start_ingevt;
  int ingcyc  = 4;
  int wgevt   = start_wgevt;
  int wgbcid  = 0;
  wggettree->GetEntry(wgevt);
  inggettree->ingtree->GetEntry(ingevt);
  clear_ing();
  DrawIngRecon(ingcyc,type_ing);
  clear_wg();
  int current_bcid = type_hit.clustered_bcid[wgbcid];
  DrawWgHit      (wgbcid,type_hit);
  DrawWgRecon(current_bcid,type_recon);
  pad[0]->Update();
  pad[1]->Update();

  while(ingevt<ingnevt){
    inggettree->ingtree->GetEntry(ingevt);
    while(ingcyc<12){
      if(type_ing.num_ingrecon[ingcyc-4]>=thres_numingrecon){
        //int wg_exp_time = wg_start_time + 2.48*(spill-wg_start_spill);
        int diff_spill = type_ing.nd280nspill - spill%0x8000;
        current_bcid = type_hit.clustered_bcid[wgbcid];
        //if(wgevt==0) wg_exp_time = wg_start_time;
        std::cout << "==========================" << std::endl;
        std::cout 
          << " INGRID: event=" << ingevt
          << ", cyc="          << ingcyc
          << ", nd280nspill="  << type_ing.nd280nspill
          << ", unixtime="     << type_ing.unixtime
          << std::endl
          << " WAGASCI: event=" << wgevt 
          << ", bcid="          << current_bcid
          << ", spill="         << spill%0x8000
          //<< ", time="          << wg_exp_time
          << std::endl;
        std::cout
          //<< "time diff. (INGRID - WAGASCI) = " << type_ing.unixtime - wg_exp_time
          << " spill diff. (INGRID - WAGASCI) = " << diff_spill
          << std::endl
          << " bunch diff. (INGRID - WAGASCI) = " << ingcyc + 22 - current_bcid
          << std::endl;

        bool isContinue = false;
        bool display=false;
        if(scan){
          if     (diff_spill>0){wgevt++;}
          else if(diff_spill<0){ingcyc++;isContinue=true;}
          else{display=true;}
        }
        else{display=true;}

        if(display){
          clear_ing();
          DrawIngRecon(ingcyc,type_ing);
          if(scan){
            wggettree->GetEntry(wgevt);
            clear_wg();
            current_bcid = type_hit.clustered_bcid[wgbcid];
            DrawWgHit(wgbcid,type_hit);
            DrawWgRecon(current_bcid,type_recon);
          }
          pad[0]->Update();
          pad[1]->Update();
          scan=false;
          while(!print){
            std::cout << "Type q/quit to exit from this program."              << std::endl;
            std::cout << "Type n/next to go to the next ingrid cyc/event."     << std::endl;
            std::cout << "Type e/event to jump onto a particlar ingrid event." << std::endl;
            std::cout << "Type w/wagasci to start wagasci event loop."         << std::endl;
            std::cout << "Type s/scan to start scanning wagasci events."       << std::endl;
            std::cin >> str;
            if     (str=="q"||str=="quit"   ){return 0;}
            else if(str=="n"||str=="next"   ){ingcyc++;isContinue=true;break;}
            else if(str=="w"||str=="wagasci"){wgevt++;break;}
            else if(str=="s"||str=="scan"   ){scan=true;wgevt++;break;}
            else if(str=="e"||str=="event"  ){
              cout << "Type an INGRID event number : ";
              cin >> str;
              ingevt = atoi(str.c_str());
              ingcyc = 4;
              isContinue=true;
              inggettree->ingtree->GetEntry(ingevt);
              break;
            }
          }//cin while
          if(print){
            scan=true;
            canvas->Print(Form("ingevt%d_cyc%d_wgevt%d_bcid%d.png",
                  ingevt,ingcyc,wgevt,type_hit.clustered_bcid[wgbcid]));
          }
        }
        if(isContinue){continue;}
      }
      else{
        ingcyc++;
        continue;
      }
      while(wgevt<wgnevt){
        wggettree->GetEntry(wgevt);
        if(mode!=-1&&mode!=spill_mode){wgevt++;continue;}
        if(type_recon.num_recon>=thres_numwgrecon){
          bool isBreak=false;
          while(wgbcid<type_hit.num_bcid_cluster){
          //while(wgbcid==0){
            int diff_spill = type_ing.nd280nspill - spill%0x8000;
            current_bcid = type_hit.clustered_bcid[wgbcid];
            //int wg_exp_time = wg_start_time + 2.48*(spill-wg_start_spill);
            std::cout << "==========================" << std::endl;
            std::cout 
              << " INGRID: event=" << ingevt
              << ", cyc="          << ingcyc
              << ", nd280nspill="  << type_ing.nd280nspill
              << ", unixtime="     << type_ing.unixtime
              << std::endl
              << " WAGASCI: event=" << wgevt 
              << ", bcid="          << current_bcid
              << ", spill="         << spill%0x8000
              //<< ", time="          << wg_exp_time
              << std::endl;
            std::cout
              //<< "time diff. (INGRID - WAGASCI) = " << type_ing.unixtime - wg_exp_time
              << " spill diff. (INGRID - WAGASCI) = " << diff_spill
              << std::endl
              << " bunch diff. (INGRID - WAGASCI) = " << ingcyc + 22 - current_bcid
              << std::endl;
            bool display=false;
            bool isContinue=false;
            if(scan){
              if     (diff_spill>0){wgbcid++;isContinue=true;continue;}
              else if(diff_spill<0){ingevt++;isBreak=true;break;}
              else{display=true;}
            }
            else{display=true;}

            if(display){
              clear_wg();
              current_bcid = type_hit.clustered_bcid[wgbcid];
              DrawWgHit(wgbcid,type_hit);
              //DrawWgHit(0,type_hit);
              DrawWgRecon(current_bcid,type_recon);
              if(scan){
                inggettree->ingtree->GetEntry(ingevt);
                clear_ing();
                DrawIngRecon(ingcyc,type_ing);
              }
              pad[0]->Update();
              pad[1]->Update();
              scan=false;
              while(!print){
                std::cout << "--------------" << std::endl;
                std::cout << "  Type q/quit to exit from this program."               << std::endl;
                std::cout << "  Type n/next to go to the next wagasci bcid/event."    << std::endl;
                std::cout << "  Type e/event to jump onto a particlar wagasci event."<< std::endl;
                std::cout << "  Type i/ingrid to start ingrid event loop."           << std::endl;
                std::cout << "  Type s/scan to start scanning ingrid events."        << std::endl;
                std::cin >> str;
                if     (str=="q"||str=="quit"   ){return 0;}
                else if(str=="n"||str=="next"   ){wgbcid++;isContinue=true;break;}
                else if(str=="i"||str=="ingrid" ){ingevt++;isBreak=true;break;}
                else if(str=="s"||str=="scan"   ){scan=true;ingevt=0;isBreak=true;break;}
                else if(str=="e"||str=="event"  ){
                  cout << "Type a WAGASCI event number : ";
                  cin >> str;
                  wgevt = atoi(str.c_str());
                  wgbcid = 0;
                  isContinue=true;
                  wggettree->tree->GetEntry(wgevt);
                  break;
                }
              } //while
              if(print){
                scan=true;
                canvas->Print(Form("ingevt%d_cyc%d_wgevt%d_bcid%d.png",
                      ingevt,ingcyc,wgevt,type_hit.clustered_bcid[wgbcid]));
              }
            }
            if(isContinue){continue;}
            if(isBreak){break;}
            else{wgbcid++;}
          }//wgbcid
          if(isBreak){break;}
        }
        wgbcid = 0;
        wgevt++;
      }//wgevt
      if(wgevt==wgnevt){
        cout << "##### End of WAGASCI events #####" << endl;
        wgevt = 0;
        if(print) return 0;
      }
      ingcyc++;
    }//ingcyc
    ingcyc = 4;
    ingevt++;
  }//ingevt
  cout << "##### End of INGRID events #####" << endl;

  return 0;
} //main


// ==================================================================
void DrawModule()
{
  for(int view=0;view<2;view++){
    pad[view]->cd();
    Double_t FrameX, FrameY;
    FrameX = C_B2MotherSizeZ;
    if(view==SideView) FrameY = C_B2MotherSizeY; else FrameY = C_B2MotherSizeX;
    Double_t FrameMargin1 = -200.;
    Double_t FrameMargin2 = -100.;
    Double_t FrameMargin3 = -400.;
    Double_t FrameMargin4 =    0.;
    TH1F *frame = gPad->DrawFrame(
        -FrameX+FrameMargin1, 
        -FrameY+FrameMargin2,
        FrameX +FrameMargin3,
        FrameY +FrameMargin4);
    frame->GetXaxis()->SetTickLength(0.);
    frame->GetXaxis()->SetLabelSize (0.);
    frame->GetYaxis()->SetTickLength(0.);
    frame->GetYaxis()->SetLabelSize (0.);
    leg[view] = new TLegend(margin_l+0.01,0.9,1.-margin_r,1.);
    leg[view]->SetFillStyle(0);
    leg[view]->SetBorderSize(0);
    leg[view]->SetTextSize(.05);
    if(view==TopView ) leg[view]->SetHeader("Top View","c");
    if(view==SideView) leg[view]->SetHeader("Side View","c");
    leg[view]->Draw();
  }

  wgDisp *disp = new wgDisp();
  for(int view=0;view<2;view++){
    if(view==SideView){pad[1]->cd();}
    else              {pad[0]->cd();}
    disp->DrawWaterModule ( view  , C_B2WMPosX  ,C_B2WMPosY  ,C_B2WMPosZ );   
    disp->DrawINGRID   (14, view, C_B2d1INGPosX  ,C_B2d1INGPosY  ,C_B2d1INGPosZ  ,0.0  );
    if(view==SideView_i){pad[1]->cd();}
    else                {pad[0]->cd();}
    disp->DrawProtonModule( view, C_B2CHPosX  ,C_B2CHPosY  ,C_B2CHPosZ );   
  }

  canvas->cd(0)->Modified();
  canvas->Update();

  return;
};//DrawModule


// ==================================================================
void DrawWgHit(int i_bcid,Hit_t type_hit){

/*
  for(int i=0;i<type_hit.num_hits;i++){
    int bcid  = type_hit.hit_bcid  [i];
    if(bcid>=31&&bcid<=33){
    int view  = type_hit.hit_view  [i];
    int pln   = type_hit.hit_pln   [i];
    int ch    = type_hit.hit_ch    [i];
    double pe = type_hit.hit_pe    [i];
    int id = pln*80+ch;
    if(view==SideView){ pad[1]->cd(); }
    else              { pad[0]->cd(); }
    wghit[view][id]->SetR1(F_CIRCLE*pow(pe,0.33));
    wghit[view][id]->SetR2(F_CIRCLE*pow(pe,0.33));
    wghit[view][id]->Draw("");
    }
  }//hits
*/
  for(int i=0;i<type_hit.num_bcid_hits[i_bcid];i++){
    int hitid = type_hit.clustered_hitid[i_bcid][i];
    int view  = type_hit.hit_view  [hitid];
    int pln   = type_hit.hit_pln   [hitid];
    int ch    = type_hit.hit_ch    [hitid];
    double pe = type_hit.hit_pe    [hitid];
    int id = pln*80+ch;
#ifdef DEBUG_INGDISP
      cout 
        << " wghit=" << hitid
        << " hitid=" << id
        << " view=" << view << " pln=" << pln << " ch=" << ch 
        << " pe=" << pe 
        << endl;
#endif
    if(view==SideView){ pad[1]->cd(); }
    else              { pad[0]->cd(); }
    wghit[view][id]->SetR1(F_CIRCLE*pow(pe,0.33));
    wghit[view][id]->SetR2(F_CIRCLE*pow(pe,0.33));
    wghit[view][id]->Draw("");
  }//hits
}

// ==================================================================
void DrawWgRecon(int current_bcid, Recon_t t_recon){
  for(int i=0;i<t_recon.num_recon;i++){
#ifdef DEBUG_INGDISP
    cout 
      << "------"
      << "num_recon=" << t_recon.num_recon
      << " current_bcid=" << current_bcid
      << " recon_bcid="  <<  t_recon.recon_bcid[i]
      << endl;
#endif

    if(current_bcid!=t_recon.recon_bcid[i]) continue;
    double start_z   = t_recon.recon_start_z  [i];
    double stop_z    = t_recon.recon_stop_z   [i];
    double start_xy  = t_recon.recon_start_xy [i];
    double stop_xy   = t_recon.recon_stop_xy  [i];
    double slope     = t_recon.recon_slope    [i];
    double intercept = t_recon.recon_intercept[i];
    int    view      = t_recon.recon_view     [i];
    int    bcid      = t_recon.recon_bcid     [i];
#ifdef DEBUG_INGDISP
    std::cout
      << " view="      << view     
      << " bcid="      << bcid  
      << " start_z="   << start_z  
      << " stop_z="    << stop_z   
      << " slope="     << slope    
      << " intercept=" << intercept
      << std::endl;
#endif

    if(view==SideView){
      start_xy+=C_B2WMPosY;
      stop_xy +=C_B2WMPosY;
      pad[1]->cd(); 
    }
    else{
      start_xy+=C_B2WMPosX;
      stop_xy +=C_B2WMPosX;
      pad[0]->cd(); 
    }
    start_z +=C_B2WMPosZ;
    stop_z  +=C_B2WMPosZ;
#ifdef DEBUG_INGDISP
    cout
      << " start_xy=" << start_xy
      << " start_z="  << start_z 
      << " stop_xy="  << stop_xy 
      << " stop_z="   << stop_z  
      << endl;
#endif
    wgrecon[view][i]->SetX1(start_z);
    wgrecon[view][i]->SetX2(stop_z);
    wgrecon[view][i]->SetY1(start_xy);
    wgrecon[view][i]->SetY2(stop_xy);
    wgrecon[view][i]->Draw("");
  }
}

// ==================================================================
void DrawIngRecon(int cyc,IngRecon_t type_ing)
{
  for(int i=0;i<MAX_NUM_INGHIT;i++){
    if(type_ing.inghit_cyc[i]==cyc){
      int pln  = type_ing.inghit_pln [i];
      int view = type_ing.inghit_view[i];
      int ch   = type_ing.inghit_ch  [i];
      int hitid;
      if(pln<11){
        hitid = pln*24 + ch;
      }else{
        hitid = 24*11 + (pln-11)%2*22+ch;
      }
      double pe = type_ing.inghit_pe[i];
#ifdef DEBUG_INGDISP
      cout 
        << " inghit=" << i
        << " hitid=" << hitid
        << " view=" << view << " pln=" << pln << " ch=" << ch 
        << " pe=" << pe 
        << endl;
#endif
      if(view==SideView){ pad[1]->cd();}
      else              { pad[0]->cd();}
      inghit[view][hitid]->SetR1(F_CIRCLE*pow(pe,0.33));
      inghit[view][hitid]->SetR2(F_CIRCLE*pow(pe,0.33));
      inghit[view][hitid]->Draw("");
    }
  }
  for(int i=0;i<MAX_NUM_INGHIT;i++){
    if(type_ing.pmhit_cyc[i]==cyc){
      int pln  = type_ing.pmhit_pln [i];
      int view = type_ing.pmhit_view[i];
      int ch   = type_ing.pmhit_ch  [i];
      int hitid;
      if(pln<1){
        hitid = ch;
      }else if(pln<18){
        hitid = 24 +32*(pln-1)+ch;
      }else{
        hitid = 24 +32*17+(pln-18)/2*17+ch;
      }
      double pe = type_ing.pmhit_pe[i];
#ifdef DEBUG_INGDISP
      cout 
        << " pmhit=" << i
        << " hitid=" << hitid
        << " view=" << view << " pln=" << pln << " ch=" << ch 
        << " pe=" << pe 
        << endl;
#endif
      if(view==SideView_i){ pad[1]->cd();}
      else                { pad[0]->cd();}
      pmhit[view][hitid]->SetR1(F_CIRCLE*pow(pe,0.33));
      pmhit[view][hitid]->SetR2(F_CIRCLE*pow(pe,0.33));
      pmhit[view][hitid]->Draw("");
    }
  }
  for(int i=0;i<MAX_NUM_INGRECON;i++){
    double clstime = type_ing.ingrecon_clstime[i];
    double expected_time = 490.+581.*cyc;
    if(fabs(clstime-expected_time)<50.){
      int    view      = type_ing.ingrecon_view    [i];
      int    start_pln = type_ing.ingrecon_startpln[i];
      int    stop_pln  = type_ing.ingrecon_endpln  [i];
      double start_xy  = type_ing.ingrecon_startxy [i];
      double stop_xy   = type_ing.ingrecon_endxy   [i];
      double slope     = type_ing.ingrecon_slope   [i];
      slope *= -1.;
      start_xy = 600.-start_xy;
      stop_xy  = 600.-stop_xy;
      double start_z, stop_z,x,y;
      wgDetectorDimension *detdim = new wgDetectorDimension();
      detdim->GetPosING(14,start_pln,view,0,&x,&y,&start_z);
      detdim->GetPosING(14,stop_pln ,view,0,&x,&y,&stop_z);
      delete detdim;
      stop_xy = start_xy + slope*(stop_z-start_z);
      if(view==SideView){
        start_xy+=C_B2d1INGPosY;
        stop_xy +=C_B2d1INGPosY;
        pad[1]->cd(); 
      }
      else{
        start_xy+=C_B2d1INGPosX;
        stop_xy +=C_B2d1INGPosX;
        pad[0]->cd(); 
      }
      start_z +=C_B2d1INGPosZ;
      stop_z  +=C_B2d1INGPosZ;
#ifdef DEBUG_INGDISP
      cout
        << " start_xy=" << start_xy
        << " start_z="  << start_z 
        << " stop_xy="  << stop_xy 
        << " stop_z="   << stop_z  
        << endl;
#endif
      ingrecon[view][i]->SetX1(start_z);
      ingrecon[view][i]->SetX2(stop_z);
      ingrecon[view][i]->SetY1(start_xy);
      ingrecon[view][i]->SetY2(stop_xy);
      ingrecon[view][i]->Draw("");
    }
  }
  for(int i=0;i<MAX_NUM_INGRECON;i++){
    double clstime = type_ing.pmrecon_clstime[i];
    double expected_time = 490.+581.*cyc;
    if(fabs(clstime-expected_time)<50.){
      int    view      = type_ing.pmrecon_view    [i];
      int    start_pln = type_ing.pmrecon_startpln[i];
      int    stop_pln  = type_ing.pmrecon_endpln  [i];
      double start_xy  = type_ing.pmrecon_startxy [i];
      double stop_xy   = type_ing.pmrecon_endxy   [i];
      double slope     = type_ing.pmrecon_slope   [i];
      start_xy = start_xy-600.; //TODO
      stop_xy  = stop_xy -600.;
      double start_z, stop_z,x,y;
      wgDetectorDimension *detdim = new wgDetectorDimension();
      detdim->GetPosPM(start_pln,view,0,&x,&y,&start_z);
      detdim->GetPosPM(stop_pln ,view,0,&x,&y,&stop_z);
      delete detdim;
      stop_xy = start_xy + slope*(stop_z-start_z);
      if(view==SideView_i){
        start_xy+=C_B2CHPosY;
        stop_xy +=C_B2CHPosY;
        pad[1]->cd(); 
      }
      else{
        start_xy+=C_B2CHPosX;
        stop_xy +=C_B2CHPosX;
        pad[0]->cd(); 
      }
      start_z +=C_B2CHPosZ;
      stop_z  +=C_B2CHPosZ;
#ifdef DEBUG_INGDISP
      cout
        << " start_xy=" << start_xy
        << " start_z="  << start_z 
        << " stop_xy="  << stop_xy 
        << " stop_z="   << stop_z  
        << endl;
#endif
      pmrecon[view][i]->SetX1(start_z);
      pmrecon[view][i]->SetX2(stop_z);
      pmrecon[view][i]->SetY1(start_xy);
      pmrecon[view][i]->SetY2(stop_xy);
      pmrecon[view][i]->Draw("");
    }
  }
}

// ==================================================================
void clear_ing(){
  for(int i=0;i<NumView;i++){
    for(int j=0;j<NUM_VIEWCH_ING;j++){
      if(i==SideView){ pad[1]->cd(); } 
      else           { pad[0]->cd(); }
      inghit[i][j]->SetR1(0.);
      inghit[i][j]->SetR2(0.);
      inghit[i][j]->Draw("");
    }
    for(int j=0;j<NUM_VIEWCH_PM;j++){
      if(i==SideView_i){ pad[1]->cd(); } 
      else             { pad[0]->cd(); }
      pmhit [i][j]->SetR1(0.);
      pmhit [i][j]->SetR2(0.);
      pmhit [i][j]->Draw("");
    }
    for(int j=0;j<MAX_NUM_INGRECON;j++){
      if(i==SideView){ pad[1]->cd(); } 
      else           { pad[0]->cd(); }
      ingrecon[i][j]->SetX1(-2000.);
      ingrecon[i][j]->SetX2(-2000.);
      ingrecon[i][j]->SetY1(-2000.);
      ingrecon[i][j]->SetY2(-2000.);
      ingrecon[i][j]->Draw("");
    }
    for(int j=0;j<MAX_NUM_INGRECON;j++){
      if(i==SideView_i){ pad[1]->cd(); } 
      else             { pad[0]->cd(); }
      pmrecon [i][j]->SetX1(-2000.);
      pmrecon [i][j]->SetX2(-2000.);
      pmrecon [i][j]->SetY1(-2000.);
      pmrecon [i][j]->SetY2(-2000.);
      pmrecon [i][j]->Draw("");
    }
  }
  pad[0]->Update();
  pad[1]->Update();
  canvas->cd(0)->Modified();
  canvas->Update();
}

void clear_wg(){
  for(int i=0;i<NumView;i++){
    if(i==SideView){ pad[1]->cd(); } 
    else           { pad[0]->cd(); }
    for(int j=0;j<NUM_VIEWCH_WG;j++){
      wghit[i][j]->SetR1(0.);
      wghit[i][j]->SetR2(0.);
      wghit[i][j]->Draw("");
    }
    for(int j=0;j<MAX_NUM_TRACK;j++){      
      wgrecon[i][j]->SetX1(-2000.);
      wgrecon[i][j]->SetX2(-2000.);
      wgrecon[i][j]->SetY1(-2000.);
      wgrecon[i][j]->SetY2(-2000.);
      wgrecon[i][j]->Draw("");
    }
  }
  pad[0]->Update();
  pad[1]->Update();
  canvas->cd(0)->Modified();
  canvas->Update();
}
