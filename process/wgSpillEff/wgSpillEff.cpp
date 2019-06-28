#include <string>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <ctime>
#include <fstream>

#include <TCanvas.h>
#include <TLegend.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TGraph.h>
#include <TLegend.h>
#include <TBox.h>
#include <TPaveText.h>
#include <TApplication.h>
#include <TROOT.h>
#include <TArrow.h>
#include <TFrame.h>
#include <TGaxis.h>
#include <TStyle.h>

#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgEditXML.h"
#include "wgColor.h"
#include "wgGetTree.h"
#include "wgGetCalibData.h"
#include "wgChannelMap.h"

//#define MARKERSTYLE
#define LINESTYLE

#define MARKER_COLOR_BSD 1
#define MARKER_SIZE_BSD 0.5
#define MARKER_STYLE_BSD 1
#define LINE_COLOR_BSD 1
#define LINE_WIDTH_BSD 5
#define LINE_STYLE_BSD 1

#define MARKER_COLOR_WG 3
#define MARKER_SIZE_WG 0.5
#define MARKER_STYLE_WG 1
#define LINE_COLOR_WG 3
#define LINE_WIDTH_WG 5
#define LINE_STYLE_WG 2

#define MARKER_COLOR_DIFF 2
#define MARKER_SIZE_DIFF 0.5
#define MARKER_STYLE_DIFF 1
#define LINE_COLOR_DIFF 2
#define LINE_WIDTH_DIFF 5
#define LINE_STYLE_DIFF 3

using namespace std;

void get_strtime(int time, string* str){
  struct tm tm;
  char buf[50];
  time_t t = time;
  localtime_r(&t,&tm);
  string tmp;
  tmp = asctime_r(&tm,buf);
  istringstream sstr(tmp);
  getline(sstr,*str);
}

void SpillEff(
    string inputDirName, string outputDirName, 
    int t2krun, int mrrun, string version);

int main(int argc, char** argv){
  int opt;
  int t2krun = -1;
  int mrrun = -1;
  string version = "";
  wgConst *con = new wgConst;
  con->GetENV();
  string inputDirName  = con->SPILL_DIRECTORY;
  string outputDirName = con->SPILL_DIRECTORY;
  string logoutputDir  = con->LOG_DIRECTORY;
  delete con;

  OperateString *OpStr = new OperateString;
  Logger *Log = new Logger;
  CheckExist *check = new CheckExist;

  Log->Initialize();
  Log->Write("start calibration...");

  while((opt = getopt(argc,argv, "f:s:e:t:m:v:o:h")) !=-1 ){
    switch(opt){
      case 'f':
        inputDirName = optarg;
        if(!check->Dir(inputDirName)){ 
          cout<<"!!Error!! "<<inputDirName.c_str()<<"doesn't exist!!";
          Log->eWrite(Form("Error!!target:%s doesn't exist",inputDirName.c_str()));
          return 1;
        }
        Log->Write(Form("target:%s",inputDirName.c_str()));
        break;
      case 't':
        t2krun=atoi(optarg); 
        break;
      case 'm':
        mrrun =atoi(optarg);
        break;
      case 'v':
        version = optarg;
        break;
      case 'o':
        outputDirName = optarg; 
        break;
      case 'h':
        cout <<"This program is for data quality check_exist:: "<<endl;
        cout <<"You can take several option..."<<endl;
        cout <<"  -h               : help"<<endl;
        cout <<"  -f <inputDir>    : Input Directory (default: DQHISTORY_DIRECTORY)" << endl;
        cout <<"  -o <outputDir>   : Output Directory (defualt: DQHISTORY_DIRECTORY)" << endl;
        cout <<"  -t <t2krun>      : T2K Run ID" << endl;
        cout <<"  -m <mrrun>       : MR Run ID" << endl;
        cout <<"  -v <bsd version> : BSD version" << endl;
        exit(0);
    }   
  }

  if(inputDirName.empty()||outputDirName.empty()||t2krun==-1||mrrun==-1){
    cout << "See the usage: " << argv[0] << " -h" << endl;
    exit(0);
  }

  cout << " *****  READ   DIRECTORY     :" << inputDirName  << "  *****" << endl;
  cout << " *****  OUTPUT DIRECTORY     :" << outputDirName << "  *****" << endl;
  cout << " *****  T2K RUN" << t2krun << ", MRRUN" << mrrun << endl;

  delete check;
  delete OpStr;

  vector<int> runid(0);
  SpillEff(inputDirName,outputDirName,t2krun,mrrun,version);
  delete Log;  
}

//******************************************************************
void SpillEff(string inputDirName, string outputDirName,
    int t2krun, int mrrun, string version)
{
  string bsdtxtfilename = Form("%s/spill_bsd_t2krun%d_mrrun%03d.txt",
      inputDirName.c_str(),t2krun,mrrun);  
  string wgtxtfilename  = Form("%s/spill_wg_t2krun%d_mrrun%03d.txt",
      inputDirName.c_str(),t2krun,mrrun);
  string difftxtfilename  = Form("%s/spill_diff_t2krun%d_mrrun%03d.txt",
      inputDirName.c_str(),t2krun,mrrun);

  ifstream bsd (bsdtxtfilename  .c_str());
  ifstream wg  ( wgtxtfilename  .c_str());
  ifstream diff(difftxtfilename .c_str());
  if(!bsd||!wg||!diff){
    cout << "There is no BSD/WAGASCI spill txt files: " 
      << bsdtxtfilename << ", "
      << wgtxtfilename << ", "
      << difftxtfilename << endl;
    return;
  }

  vector<double> v_trgsec_bsd (0),v_spillnum_bsd (0),v_pot_bsd (0),v_isrhc_bsd (0);
  vector<double> v_trgsec_wg  (0),v_spillnum_wg  (0),v_pot_wg  (0),v_isrhc_wg  (0);
  vector<double> v_trgsec_diff(0),v_spillnum_diff(0),v_pot_diff(0),v_isrhc_diff(0);
  string str;
  int num_bsd = 0;
  int num_bsd_fhc = 0;
  int num_bsd_rhc = 0;
  int tmp = 0;
  while(bsd>>str){
    if     (tmp==0)v_spillnum_bsd.push_back(atof(str.c_str()));
    else if(tmp==1)v_isrhc_bsd   .push_back(atoi(str.c_str()));
    else if(tmp==2)v_trgsec_bsd  .push_back(atof(str.c_str()));
    else if(tmp==3)v_pot_bsd     .push_back(atof(str.c_str()));
    if(tmp==3){
      tmp=0;
      num_bsd++;
      if(v_isrhc_bsd.back()==0)num_bsd_fhc++;
      if(v_isrhc_bsd.back()==1)num_bsd_rhc++;
    }
    else      tmp++;
  }
  int num_wg = 0;
  int num_wg_fhc = 0;
  int num_wg_rhc = 0;
  tmp=0;
  while(wg>>str){
    if     (tmp==0)v_spillnum_wg.push_back(atof(str.c_str()));
    else if(tmp==1)v_isrhc_wg   .push_back(atoi(str.c_str()));
    else if(tmp==2)v_trgsec_wg  .push_back(atof(str.c_str()));
    else if(tmp==3)v_pot_wg     .push_back(atof(str.c_str()));
    if(tmp==3){
      tmp=0;
      num_wg++;
      if(v_isrhc_wg.back()==0)num_wg_fhc++;
      if(v_isrhc_wg.back()==1)num_wg_rhc++;
    }
    else      tmp++;
  }
  int num_diff = 0;
  int num_diff_fhc = 0;
  int num_diff_rhc = 0;
  tmp=0;
  while(diff>>str){
    if     (tmp==0)v_spillnum_diff.push_back(atof(str.c_str()));
    else if(tmp==1)v_isrhc_diff   .push_back(atoi(str.c_str()));
    else if(tmp==2)v_trgsec_diff  .push_back(atof(str.c_str()));
    else if(tmp==3)v_pot_diff     .push_back(atof(str.c_str()));
    if(tmp==3){
      tmp=0;
      num_diff++;
      if(v_isrhc_diff.back()==0)num_diff_fhc++;
      if(v_isrhc_diff.back()==1)num_diff_rhc++;
    }
    else      tmp++;
  }

  TCanvas *c1 = new TCanvas("c1","c1");
  gStyle->SetTimeOffset(-788918400);
  gStyle->SetOptStat(0);
  int id;
  TGraph *g_bsd;
  TGraph *g_bsd_fhc;
  TGraph *g_bsd_rhc;
  TGraph *g_wg;
  TGraph *g_wg_fhc;
  TGraph *g_wg_rhc;
  TGraph *g_diff;
  TGraph *g_diff_fhc;
  TGraph *g_diff_rhc;

  // ======================================================
  // BSD
  double total_pot_bsd     = 0.;
  int start_time = -1, stop_time = -1;
  int start_time_fhc = -1, stop_time_fhc = -1;
  int start_time_rhc = -1, stop_time_rhc = -1;
  if(num_bsd>0){
    g_bsd = new TGraph(num_bsd);
    g_bsd->SetTitle("POT History (FHC/RHC)");
#ifdef LINESTYLE
    g_bsd ->SetLineColor(LINE_COLOR_BSD);
    g_bsd ->SetLineStyle(LINE_STYLE_BSD);
    g_bsd ->SetLineWidth(LINE_WIDTH_BSD);
#else
#ifdef MARKERSTYLE
    g_bsd ->SetMarkerColor(MARKER_COLOR_BSD);
    g_bsd ->SetMarkerStyle(MARKER_STYLE_BSD);
    g_bsd ->SetMarkerSize (MARKER_SIZE_BSD );
#endif
#endif
    for(int i=0;i<num_bsd;i++){
      total_pot_bsd += v_pot_bsd[i];
      g_bsd->SetPoint(i,v_trgsec_bsd[i],total_pot_bsd);
      if(start_time==-1||start_time>v_trgsec_bsd[i]) start_time=v_trgsec_bsd[i];
      if(stop_time ==-1||stop_time <v_trgsec_bsd[i]) stop_time =v_trgsec_bsd[i];
    }
  }
  double total_pot_bsd_fhc = 0.;
  id = 0;
  if(num_bsd_fhc>0){
    g_bsd_fhc = new TGraph(num_bsd_fhc);
    g_bsd_fhc->SetTitle("POT History (FHC)");
#ifdef LINESTYLE
    g_bsd_fhc ->SetLineColor(LINE_COLOR_BSD);
    g_bsd_fhc ->SetLineStyle(LINE_STYLE_BSD);
    g_bsd_fhc ->SetLineWidth(LINE_WIDTH_BSD);
#else
#ifdef MARKERSTYLE
    g_bsd_fhc ->SetMarkerColor(MARKER_COLOR_BSD);
    g_bsd_fhc ->SetMarkerStyle(MARKER_STYLE_BSD);
    g_bsd_fhc ->SetMarkerSize (MARKER_SIZE_BSD );
#endif
#endif
    for(int i=0;i<num_bsd;i++){
      if(v_isrhc_bsd[i]==0){
        total_pot_bsd_fhc += v_pot_bsd[i];
        g_bsd_fhc->SetPoint(id,v_trgsec_bsd[i],total_pot_bsd_fhc);
        if(start_time_fhc==-1||start_time_fhc>v_trgsec_bsd[i]) start_time_fhc=v_trgsec_bsd[i];
        if(stop_time_fhc ==-1||stop_time_fhc <v_trgsec_bsd[i]) stop_time_fhc =v_trgsec_bsd[i];
        id++;
      }
    }
  }
  double total_pot_bsd_rhc = 0.;
  id = 0;
  if(num_bsd_rhc>0){
    g_bsd_rhc = new TGraph(num_bsd_rhc);
    g_bsd_rhc->SetTitle("POT History (RHC)");
#ifdef LINESTYLE
    g_bsd_rhc ->SetLineColor(LINE_COLOR_BSD);
    g_bsd_rhc ->SetLineStyle(LINE_STYLE_BSD);
    g_bsd_rhc ->SetLineWidth(LINE_WIDTH_BSD);
#else
#ifdef MARKERSTYLE
    g_bsd_rhc ->SetMarkerColor(MARKER_COLOR_BSD);
    g_bsd_rhc ->SetMarkerStyle(MARKER_STYLE_BSD);
    g_bsd_rhc ->SetMarkerSize (MARKER_SIZE_BSD );
#endif
#endif
    for(int i=0;i<num_bsd;i++){
      if(v_isrhc_bsd[i]==1){
        total_pot_bsd_rhc += v_pot_bsd[i];
        g_bsd_rhc->SetPoint(id,v_trgsec_bsd[i],total_pot_bsd_rhc);
        if(start_time_rhc==-1||start_time_rhc>v_trgsec_bsd[i]) start_time_rhc=v_trgsec_bsd[i];
        if(stop_time_rhc ==-1||stop_time_rhc <v_trgsec_bsd[i]) stop_time_rhc =v_trgsec_bsd[i];
        id++;
      }
    }
  }

  // ======================================================
  // WAGASCI
  double total_pot_wg = 0.;
  if(num_wg>0){
    g_wg = new TGraph(num_wg);
#ifdef LINESTYLE
    g_wg ->SetLineColor(LINE_COLOR_WG);
    g_wg ->SetLineStyle(LINE_STYLE_WG);
    g_wg ->SetLineWidth(LINE_WIDTH_WG);
#else
#ifdef MARKERSTYLE
    g_wg ->SetMarkerColor(MARKER_COLOR_WG);
    g_wg ->SetMarkerStyle(MARKER_STYLE_WG);
    g_wg ->SetMarkerSize (MARKER_SIZE_WG );
#endif
#endif

    for(int i=0;i<num_wg;i++){
      total_pot_wg += v_pot_wg[i];
      g_wg->SetPoint(i,v_trgsec_wg[i],total_pot_wg);
    }
  }
  double total_pot_wg_fhc = 0.;
  id = 0;
  if(num_wg_fhc>0){
    g_wg_fhc = new TGraph(num_wg_fhc);
#ifdef LINESTYLE
    g_wg_fhc ->SetLineColor(LINE_COLOR_WG);
    g_wg_fhc ->SetLineStyle(LINE_STYLE_WG);
    g_wg_fhc ->SetLineWidth(LINE_WIDTH_WG);
#else
#ifdef MARKERSTYLE
    g_wg_fhc ->SetMarkerColor(MARKER_COLOR_WG);
    g_wg_fhc ->SetMarkerStyle(MARKER_STYLE_WG);
    g_wg_fhc ->SetMarkerSize (MARKER_SIZE_WG );
#endif
#endif
    for(int i=0;i<num_wg;i++){
      if(v_isrhc_wg[i]==0){
        total_pot_wg_fhc += v_pot_wg[i];
        g_wg_fhc->SetPoint(id,v_trgsec_wg[i],total_pot_wg_fhc);
        id++;
      }
    }
  }
  double total_pot_wg_rhc = 0.;
  id = 0;
  if(num_wg_rhc>0){
    g_wg_rhc = new TGraph(num_wg_rhc);
#ifdef LINESTYLE
    g_wg_rhc ->SetLineColor(LINE_COLOR_WG);
    g_wg_rhc ->SetLineStyle(LINE_STYLE_WG);
    g_wg_rhc ->SetLineWidth(LINE_WIDTH_WG);
#else
#ifdef MARKERSTYLE
    g_wg_fhc ->SetMarkerColor(MARKER_COLOR_WG);
    g_wg_fhc ->SetMarkerStyle(MARKER_STYLE_WG);
    g_wg_fhc ->SetMarkerSize (MARKER_SIZE_WG );
#endif
#endif
    for(int i=0;i<num_wg;i++){
      if(v_isrhc_wg[i]==1){
        total_pot_wg_rhc += v_pot_wg[i];
        g_wg_rhc->SetPoint(id,v_trgsec_wg[i],total_pot_wg_rhc);
        id++;
      }
    }
  }

  // ======================================================
  // DIFFERENCE
  double total_pot_diff = 0.;
  if(num_diff>0){
    g_diff = new TGraph(num_diff);
#ifdef LINESTYLE
    g_diff ->SetLineColor(LINE_COLOR_DIFF);
    g_diff ->SetLineStyle(LINE_STYLE_DIFF);
    g_diff ->SetLineWidth(LINE_WIDTH_DIFF);
#else
#ifdef MARKERSTYLE
    g_diff ->SetMarkerColor(MARKER_COLOR_DIFF);
    g_diff ->SetMarkerStyle(MARKER_STYLE_DIFF);
    g_diff ->SetMarkerSize (MARKER_SIZE_DIFF );
#endif
#endif

    for(int i=0;i<num_diff;i++){
      total_pot_diff += v_pot_diff[i];
      g_diff->SetPoint(i,v_trgsec_diff[i],total_pot_diff);
    }
  }
  double total_pot_diff_fhc = 0;
  id = 0;
  if(num_diff_fhc>0){
    g_diff_fhc = new TGraph(num_diff_fhc);
#ifdef LINESTYLE
    g_diff_fhc ->SetLineColor(LINE_COLOR_DIFF);
    g_diff_fhc ->SetLineStyle(LINE_STYLE_DIFF);
    g_diff_fhc ->SetLineWidth(LINE_WIDTH_DIFF);
#else
#ifdef MARKERSTYLE
    g_diff_fhc ->SetMarkerColor(MARKER_COLOR_DIFF);
    g_diff_fhc ->SetMarkerStyle(MARKER_STYLE_DIFF);
    g_diff_fhc ->SetMarkerSize (MARKER_SIZE_DIFF );
#endif
#endif
    for(int i=0;i<num_diff;i++){
      if(v_isrhc_diff[i]==0){
        total_pot_diff_fhc += v_pot_diff[i];
        g_diff_fhc->SetPoint(id,v_trgsec_diff[i],total_pot_diff_fhc);
        id++;
      }
    }
  }
  double total_pot_diff_rhc = 0;
  id = 0;
  if(num_diff_rhc>0){
    g_diff_rhc = new TGraph(num_diff_rhc);
#ifdef LINESTYLE
    g_diff_rhc ->SetLineColor(LINE_COLOR_DIFF);
    g_diff_rhc ->SetLineStyle(LINE_STYLE_DIFF);
    g_diff_rhc ->SetLineWidth(LINE_WIDTH_DIFF);
#else
#ifdef MARKERSTYLE
    g_diff_fhc ->SetMarkerColor(MARKER_COLOR_DIFF);
    g_diff_fhc ->SetMarkerStyle(MARKER_STYLE_DIFF);
    g_diff_fhc ->SetMarkerSize (MARKER_SIZE_DIFF );
#endif
#endif
    for(int i=0;i<num_diff;i++){
      if(v_isrhc_diff[i]==1){
        total_pot_diff_rhc += v_pot_diff[i];
        g_diff_rhc->SetPoint(id,v_trgsec_diff[i],total_pot_diff_rhc);
        id++;
      }
    }
  }


  //================================
  // PLOT 
  string outfilename;

  // ====
  outfilename = Form("%s/pot_history.png",outputDirName.c_str());
#ifdef LINESTYLE
  if(num_bsd>0){
    g_bsd->Draw("AL");
    g_bsd->GetXaxis()->SetTimeDisplay(1);
    g_bsd->GetXaxis()->SetTimeFormat("%b/%d");
    g_bsd->GetXaxis()->SetTimeOffset(0,"jst");
    g_bsd->Draw("AL");
    //g_bsd->Draw("AL same");
  }
  if(num_wg   >0)g_wg  ->Draw("L same");
  if(num_bsd  >0)g_bsd ->Draw("L same");
  if(num_diff >0)g_diff->Draw("L same");
#else
#ifdef MARKERSTYLE
  if(num_bsd>0){
    g_bsd->Draw("AP");
    g_bsd->GetXaxis()->SetTimeDisplay(1);
    g_bsd->GetXaxis()->SetTimeFormat("%b/%d");
    g_bsd->GetXaxis()->SetTimeOffset(0,"jst");
    g_bsd->Draw("AP");
  }
  if(num_wg  >0)g_wg  ->Draw("P same");
  if(num_diff>0)g_diff->Draw("P same");
#endif
#endif
  string starttime1,stoptime1;
  get_strtime(start_time,&starttime1);
  get_strtime(stop_time ,&stoptime1 );
  TLegend *leg1  = new TLegend(0.15,0.5,0.5,0.8);
  TLegend *leg11 = new TLegend(0.12,0.8,0.8,0.9);
  leg1->SetFillStyle(0);
  leg1->SetBorderSize(0);
  leg11->SetTextSize(0.045);
  leg11->SetFillStyle(0);
  leg11->SetBorderSize(0);
  double eff1 = total_pot_wg/total_pot_bsd*100.;
  leg11->SetHeader(Form("%s - %s",starttime1.c_str(),stoptime1.c_str()));
  leg11->Draw();
  leg1->SetHeader(Form("Efficiency = %4.2f %%",eff1));
  leg1->AddEntry(g_bsd ,Form("Delivered POT = %5.4e",total_pot_bsd ),"l");
  leg1->AddEntry(g_wg  ,Form("Acquired POT  = %5.4e",total_pot_wg  ),"l");
  leg1->AddEntry(g_diff,Form("Missed POT    = %5.4e",total_pot_diff),"l");
  leg1->Draw();
  c1->Print(outfilename.c_str());

  // ====
  if(num_bsd_fhc>0){
    outfilename = Form("%s/pot_history_fhc.png",outputDirName.c_str());
#ifdef LINESTYLE
    if(num_bsd_fhc>0){
      g_bsd_fhc->Draw("AL");
      g_bsd_fhc->GetXaxis()->SetTimeDisplay(1);
      g_bsd_fhc->GetXaxis()->SetTimeFormat("%b/%d");
      g_bsd_fhc->GetXaxis()->SetTimeOffset(0,"jst");
      g_bsd_fhc->Draw("AL");
    }
    if(num_wg_fhc  >0)g_wg_fhc  ->Draw("L same");
    if(num_diff_fhc>0)g_diff_fhc->Draw("L same");
#else
#ifdef MARKERSTYLE
    if(num_bsd_fhc>0){
      g_bsd_fhc->Draw("AP");
      g_bsd_fhc->GetXaxis()->SetTimeDisplay(1);
      g_bsd_fhc->GetXaxis()->SetTimeFormat("%b/%d");
      g_bsd_fhc->GetXaxis()->SetTimeOffset(0,"jst");
      g_bsd_fhc->Draw("AP");
    }
    if(num_wg_fhc  >0)g_wg_fhc  ->Draw("P same");
    if(num_diff_fhc>0)g_diff_fhc->Draw("P same");
#endif
#endif
    string starttime2,stoptime2;
    get_strtime(start_time_fhc,&starttime2);
    get_strtime(stop_time_fhc ,&stoptime2 );
    TLegend *leg2  = new TLegend(0.15,0.5,0.5,0.8);
    TLegend *leg22 = new TLegend(0.12,0.8,0.8,0.9);
    leg2->SetFillStyle(0);
    leg2->SetBorderSize(0);
    leg22->SetTextSize(0.045);
    leg22->SetFillStyle(0);
    leg22->SetBorderSize(0);
    leg22->SetHeader(Form("%s - %s",starttime2.c_str(),stoptime2.c_str()));
    leg22->Draw();
    double eff2 = total_pot_wg_fhc/total_pot_bsd_fhc*100.;
    leg2->SetHeader(Form("Efficiency = %4.2f %%",eff2));
    leg2->AddEntry(g_bsd_fhc ,Form("Delivered POT = %5.4e",total_pot_bsd_fhc ),"l");
    leg2->AddEntry(g_wg_fhc  ,Form("Acquired POT  = %5.4e",total_pot_wg_fhc  ),"l");
    leg2->AddEntry(g_diff_fhc,Form("Missed POT    = %5.4e",total_pot_diff_fhc),"l");
    leg2->Draw();
    c1->Print(outfilename.c_str());
  }
  // ===
  if(num_bsd_rhc>0){
    outfilename = Form("%s/pot_history_rhc.png",outputDirName.c_str());
#ifdef LINESTYLE
    if(num_bsd_rhc>0){
      g_bsd_rhc->Draw("AL");
      g_bsd_rhc->GetXaxis()->SetTimeDisplay(1);
      g_bsd_rhc->GetXaxis()->SetTimeFormat("%b/%d");
      g_bsd_rhc->GetXaxis()->SetTimeOffset(0,"jst");
      g_bsd_rhc->Draw("AL");
    }
    if(num_wg_rhc  >0)g_wg_rhc  ->Draw("L same");
    if(num_diff_rhc>0)g_diff_rhc->Draw("L same");
#else
#ifdef MARKERSTYLE
    if(num_bsd_rhc>0){
      g_bsd_rhc->Draw("AP");
      g_bsd_rhc->GetXaxis()->SetTimeDisplay(1);
      g_bsd_rhc->GetXaxis()->SetTimeFormat("%b/%d");
      g_bsd_rhc->GetXaxis()->SetTimeOffset(0,"jst");
      g_bsd_rhc->Draw("AP");
    }
    if(num_wg_rhc  >0)g_wg_rhc  ->Draw("P same");
    if(num_diff_rhc>0)g_diff_rhc->Draw("P same");
#endif
#endif
    string starttime3,stoptime3;
    get_strtime(start_time_rhc,&starttime3);
    get_strtime(stop_time_rhc ,&stoptime3 );
    TLegend *leg3  = new TLegend(0.15,0.5,0.5,0.8);
    TLegend *leg33 = new TLegend(0.12,0.8,0.8,0.9);
    leg3->SetFillStyle(0);
    leg3->SetBorderSize(0);
    leg33->SetTextSize(0.045);
    leg33->SetFillStyle(0);
    leg33->SetBorderSize(0);
    leg33->SetHeader(Form("%s - %s",starttime3.c_str(),stoptime3.c_str()));
    leg33->Draw();
    double eff3 = total_pot_wg_rhc/total_pot_bsd_rhc*100.;
    leg3->SetHeader(Form("Efficiency = %4.2f %%",eff3));
    leg3->AddEntry(g_bsd_rhc ,Form("Delivered POT = %5.4e",total_pot_bsd_rhc ),"l");
    leg3->AddEntry(g_wg_rhc  ,Form("Acquired POT  = %5.4e",total_pot_wg_rhc  ),"l");
    leg3->AddEntry(g_diff_rhc,Form("Missed POT    = %5.4e",total_pot_diff_rhc),"l");
    leg3->Draw();
    c1->Print(outfilename.c_str());
  }
}
