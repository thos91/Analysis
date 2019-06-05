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

using namespace std;

vector<time_t> GetTimeList(string& inputName, time_t current_time);
void MakeDir(string& str);
void AnaDQ(string inputDir, vector<time_t> &TimeList, string& outputDir);
double cal_mean(vector<double>);

int days=7;
const double time_norm = 3600.*3.; //sec every 3 hours = 3600.*3
const double bcid_onbeam_start   =27;
const double bcid_onbeam_end     =35;
const double bcid_onbeam_bin     =bcid_onbeam_end-bcid_onbeam_start;
const double bcid_offbeam_start  =0;
const double bcid_offbeam_end    =8200;
const double bcid_offbeam_bin    =bcid_offbeam_end-bcid_offbeam_start;
const double time_onbeam_start   =580*bcid_onbeam_start;
const double time_onbeam_end     =580*bcid_onbeam_end;
const double time_onbeam_bin     =time_onbeam_end-time_onbeam_start;
const double time_offbeam_start  =580*bcid_offbeam_start;
const double time_offbeam_end    =580*bcid_offbeam_end;
const double time_offbeam_bin    =time_offbeam_end-time_offbeam_start;

int main(int argc, char** argv){
  int opt;
  time_t current_time=0;
  string inputtime("");
  wgConst *con = new wgConst;
  con->GetENV();
  string inputDirName  =  con->DQ_DIRECTORY;
  string outputDir     =  con->DQHISTORY_DIRECTORY;
  string logoutputDir=con->LOG_DIRECTORY;

  OperateString *OpStr = new OperateString;
  Logger *Log = new Logger;
  CheckExist *check = new CheckExist;

  Log->Initialize();
  Log->Write("start calibration...");

  while((opt = getopt(argc,argv, "f:t:o:d:h")) !=-1 ){
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
        inputtime=optarg; 
        break;

      case 'd':
        days=atoi(optarg); 
        break;

      case 'o':
        outputDir = optarg; 
        break;

      case 'h':
        cout <<"This program is for data quality check. "<<endl;
        cout <<"You can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose DQ directory (default : WAGASCI_DQDIR)"<<endl;
        cout <<"  -t (char*) : choose time.        (default : current time)" <<endl;
        cout <<"      (ex.) 2017/9/11 10:11 -> 2017-09-11-10-11" << endl;
        cout <<"  -d (int)   : choose days.        (default : w7)" <<endl;
        cout <<"  -o (char*) : choose output directory (default: dq directory) "<<endl;
        exit(0);
      default:
        cout <<"This program is for data quality check. "<<endl;
        cout <<"You can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose DQ directory (default : WAGASCI_DQDIR)"<<endl;
        cout <<"  -t (char*) : choose time.        (default : current time)" <<endl;
        cout <<"      (ex.) 2017/9/11 10:11 -> 2017-09-11-10-11" << endl;
        cout <<"  -d (int)   : choose days.        (default : w7)" <<endl;
        cout <<"  -o (char*) : choose output directory (default: dq directory) "<<endl;
        exit(0);
    }   
  }

  if(inputtime==""){
    current_time = time(0);
  }else{
    struct tm x;
    strptime(inputtime.c_str(),"%Y-%m-%d-%H-%M", &x);
    current_time = mktime(&x);
  }

  cout << " *****  READ   DIRECTORY     :" << inputDirName << "  *****" << endl;
  cout << " *****  OUTPUT DIRECTORY     :" << outputDir    << "  *****" << endl;
  cout << " *****  CURRENT TIME         :" << current_time << "  *****" << endl;

  delete check;
  delete OpStr;

  vector<time_t> TimeList = GetTimeList(inputDirName,current_time);
  sort(TimeList.begin(),TimeList.end());

  AnaDQ(inputDirName,TimeList,outputDir);

  Log->Write("end summarizeing ... " );
  delete Log;  
}

//******************************************************************
vector<time_t> GetTimeList(string& inputDirName,time_t current_time){
  DIR *dp;
  struct dirent *entry;
  vector<time_t> openfile;
  dp = opendir(inputDirName.c_str());
  if(dp==NULL){
    cout << " !! WARNING !! no data is in "<< inputDirName << endl;
    return openfile;
  }

  time_t time = current_time - days*86400; //one week ago;

  while( (entry = readdir(dp))!=NULL ){
    if((entry->d_name[0])=='.') continue;
    struct tm y;
    memset(&y,0,sizeof(struct tm));
    string str_time = entry->d_name;
    strptime(str_time.c_str(),"%Y-%m-%d-%H-%M", &y);
    time_t file_time = mktime(&y);
    if( file_time > time && file_time < current_time ){  
      openfile.push_back(file_time);
      cout << "ReadFile : " << inputDirName << "/" << str_time << endl;
    }
  }
  closedir(dp);
  return openfile;
} 

//******************************************************************
void AnaDQ(string inputDirName, vector<time_t> &TimeList, string& outputDir){

  int FN=TimeList.size();
  if(FN<1){
    cout << "!! ERROR !! : the number of data is not enough !!" << endl;
    return;
  }

  time_t start_time = TimeList[0];
  time_t stop_time  = TimeList[TimeList.size()-1];
  string s_start_time,s_stop_time;
  tm *tm_start = localtime(&TimeList[0]);
  cout << "Start Time : "
    << tm_start->tm_year + 1900 // year 0 corresponds to 1900
    << "/" << tm_start->tm_mon+1 // month starts from 0
    << "/" << tm_start->tm_mday  // mday means day in a month
    << " " << tm_start->tm_hour 
    << ":" << tm_start->tm_min
    << ":" << tm_start->tm_sec <<endl;
  s_start_time = Form("%04d-%02d-%02d-%02d-%02d",
      tm_start->tm_year+1900,
      tm_start->tm_mon+1,
      tm_start->tm_mday,
      tm_start->tm_hour,
      tm_start->tm_min);
  tm *tm_stop = localtime(&TimeList[TimeList.size()-1]);
  cout << "Stop Time : "
    << tm_stop->tm_year + 1900 // year 0 corresponds to 1900
    << "/" << tm_stop->tm_mon+1 // month stops from 0
    << "/" << tm_stop->tm_mday  // mday means day in a month
    << " " << tm_stop->tm_hour 
    << ":" << tm_stop->tm_min
    << ":" << tm_stop->tm_sec <<endl;
  s_stop_time = Form("%04d-%02d-%02d-%02d-%02d",
      tm_stop->tm_year+1900,
      tm_stop->tm_mon+1,
      tm_stop->tm_mday,
      tm_stop->tm_hour,
      tm_stop->tm_min);

  string ofs_time_name = Form("%s/dq_history_time.js",outputDir.c_str());
  ofstream ofs(ofs_time_name.c_str(),ios::out);
  ofs << "document.write(\"From " 
    << s_start_time << " to  " << s_stop_time 
    << "\");" << endl;
  ofs.close();
  int nbin = ((stop_time-start_time)/time_norm) + 1;
  int step=0;
  if     (nbin<  8){ step =  1;} //<1day
  else if(nbin< 56){ step =  8;} //<1week
  else if(nbin<240){ step = 24;} //<1 month
  else             { step = 56;}

  /*
  if(nbin<80){ //~1day
    step = 3;
  }else if(nbin<520){ //~1week
    step = 3*24;
  }else if(nbin<2400){ //~1 month
    step = 3*24*3;
  }else{
    step = 3*24*7;
  }
  */

  // ================ define histgram ================== //
  TH2D *h_Gain         = new TH2D("gain","gain",nbin,0,nbin,80,20,60);
  h_Gain               ->SetTitle("Gain History;time;Gain");
  TH2D *h_diffGain     = new TH2D("diff_gain","diff",nbin,0,nbin,80,-20,20);
  h_diffGain           ->SetTitle("diff Gain History;time;diff Gain");
  TH2D *h_wPedestal = new TH2D("w_pedestal","w_pedestal",nbin,0,nbin,80,0,20);
  h_wPedestal       ->SetTitle("Width of Pedestal History;time;ADC count");
  TH2D *h_diffPedestal = new TH2D("diff_pedestal","diff_pedestal",nbin,0,nbin,80,-20,20);
  h_diffPedestal       ->SetTitle("diff Pedestal History;time;diff Pedestal");
  TH2D *h_diffGain2     = new TH2D("diff_gain2","diff",nbin,0,nbin,80,-20,20);
  h_diffGain2           ->SetTitle("Gain diff. from one previous;time;diff Gain");
  TH2D *h_diffPedestal2 = new TH2D("diff_pedestal2","diff_pedestal",nbin,0,nbin,80,-20,20);
  h_diffPedestal2       ->SetTitle("diff Pedestal History 2;time;diff Pedestal");
  TH2D *h_Noise        = new TH2D("noise","noise",nbin,0,nbin,100,0,100);
  h_Noise              ->SetTitle("Noise History;time;Noise Rate[Hz]");
  TH2D *h_diffNoise2   = new TH2D("diff_noise2","diff_noise2",nbin,0,nbin,100,-100,100);
  h_diffNoise2         ->SetTitle("Noise diff. from one previous;time;diff Noise Rate[%]");
  TH2D *h_pe_mm_mean   = new TH2D("pe_mm_mean","pe_mm_mean",nbin,0,nbin,100,0.,100);
  h_pe_mm_mean         ->SetTitle("Mean PE/mm ;time;pe");
  TH2D *h_spill        = new TH2D("spillnb_history","spillnb_history",nbin,0,nbin,0x8000,0,0x8000);
  h_spill              ->SetTitle("spillnb;time;SpillNb History");

  TH2D *h_pe   [2][2];
  TH2D *h_pe_mm[2][2];
  double hittiming_min = 10000., hittiming_max = 25000.;
  int hittiming_nbin = 1000;
  TH1D *h_hit_timing1 = new TH1D("hit_timing1","hit_timing1",hittiming_nbin,hittiming_min,hittiming_max);
  TH2D *h_hit_timing2 = new TH2D("hit_timing2","hit_timing2",nbin,0,nbin,hittiming_nbin,hittiming_min,hittiming_max);
  double hitbcid_min = 20., hitbcid_max = 40.;
  int hitbcid_nbin = hitbcid_max - hitbcid_min;
  TH1D *h_hit_bcid = new TH1D("hit_bcid","hit_bcid",hitbcid_nbin,hitbcid_min,hitbcid_max);
  TH2D *h_pe_map = new TH2D("pe_map","pe_map",17,0,17,80,0,80);
  TH2D *h_nhit_map = new TH2D("nhit_map","nhit_map",17,0,17,80,0,80);
  TH2D *h_gain_map = new TH2D("gain_map","gain_map",17,0,17,80,0,80);
 
  TH1D * h_pe_onbeam     =  new TH1D(Form("pe_onbeam") ,"a"      ,100,0,50);
  TH1D * h_pe_offbeam    =  new TH1D(Form("pe_offbeam"),"a"      ,100,0,50);

  TH1D* h_hittime_onbeam  = new TH1D(Form ("hittime_onbeam"),"a", time_onbeam_bin+2000, time_onbeam_start-1000, time_onbeam_end+1000);
  TH1D* h_hittime_offbeam = new TH1D(Form ("hittime_offbeam"),"a",time_offbeam_bin+2000,time_offbeam_start-1000,time_offbeam_end+1000);
  TH1D* h_bcid_onbeam     = new TH1D( Form("bcid_onbeam"),"a"  , bcid_onbeam_bin+10, bcid_onbeam_start-5, bcid_onbeam_end+5);
  TH1D* h_bcid_offbeam    = new TH1D( Form("bcid_offbeam"),"a"  ,bcid_offbeam_bin+10,bcid_offbeam_start-5,bcid_offbeam_end+5);

  h_nhit_map        -> SetTitle(Form("Number of Hits;pln (TopView:0-7, SideView:9-16);ch"));
  h_hit_timing1     -> SetTitle(Form("Hit Timing from Acq Start;Timing [ns];nEntry"));
  h_hit_timing2     -> SetTitle(Form("Hit Timing from Acq Start;time;Timing [ns]"));
  h_pe_map          -> SetTitle(Form("pe map;pln (TopView:0-7, SideView:9-16);ch"));
  h_pe_onbeam       -> SetTitle(Form("pe onbeam;pe;nEntry"));
  h_pe_offbeam      -> SetTitle(Form("pe offbeam;pe;nEntry"));
  h_hittime_onbeam  -> SetTitle(Form("hittime onbeam;time;nEntry"));
  h_hittime_offbeam -> SetTitle(Form("hittime offbeam;time;nEntry"));
  h_bcid_onbeam     -> SetTitle(Form("bcid onbeam;bcid;nEntry"));
  h_bcid_offbeam    -> SetTitle(Form("bcid offbeam;bcid;nEntry"));
  
  TH2D * h_pe_chip_onbeam       ;
  TH2D * h_pe_chip_offbeam      ;
  TH2D * h_bcid_chip_onbeam     ;
  TH2D * h_bcid_chip_offbeam    ;
  TH2D * h_hittime_chip_onbeam  ;
  TH2D * h_hittime_chip_offbeam ;
  TH2D * h_diff_hittime_chip_onbeam  ;
  TH2D * h_diff_hittime_chip_offbeam ;
  TH2D * h_diff_hittime_diff_chip_onbeam  ;
  TH2D * h_diff_hittime_diff_chip_offbeam ;
  TH2D * h_diff_hittime_diff_chip_onbeam2  ;
  TH2D * h_diff_hittime_diff_chip_offbeam2 ;
  h_pe_chip_onbeam         = new TH2D(Form("pe_onbeam_chip"),"a",41,0,41,100,0,50);
  h_pe_chip_offbeam        = new TH2D(Form("pe_offbeam_chip"),"a",41,0,41,100,0,50);
  h_hittime_chip_onbeam    = new TH2D(Form("hittime_onbeam_chip"),"a" ,41,0,41, time_onbeam_bin+2000, time_onbeam_start-1000, time_onbeam_end+1000);
  h_hittime_chip_offbeam   = new TH2D(Form("hittime_offbeam_chip"),"a",41,0,41,time_offbeam_bin+2000,time_offbeam_start-1000,time_offbeam_end+1000);
  h_bcid_chip_onbeam       = new TH2D(Form("bcid_onbeam_chip"),"a"    ,41,0,41, bcid_onbeam_bin+10, bcid_onbeam_start-5, bcid_onbeam_end+5);
  h_bcid_chip_offbeam      = new TH2D(Form("bcid_offbeam_chip"),"a"   ,41,0,41,bcid_offbeam_bin+10,bcid_offbeam_start-5,bcid_offbeam_end+5);
  h_diff_hittime_chip_onbeam    = new TH2D(Form("hittime_diff_onbeam_chip"),"a" ,41,0,41, 580*2, -580, 580);
  h_diff_hittime_chip_offbeam   = new TH2D(Form("hittime_diff_offbeam_chip"),"a",41,0,41, 580*2, -580, 580);
  h_diff_hittime_diff_chip_onbeam    = new TH2D(Form("hittime_diff_onbeam_diff_chip"),"a" ,41,0,41, 580*2, -580, 580);
  h_diff_hittime_diff_chip_offbeam   = new TH2D(Form("hittime_diff_offbeam_diff_chip"),"a",41,0,41, 580*2, -580, 580);
  h_diff_hittime_diff_chip_onbeam2    = new TH2D(Form("hittime_diff_onbeam_diff_chip2"),"a" ,41,0,41, 580*2, -580, 580);
  h_diff_hittime_diff_chip_offbeam2   = new TH2D(Form("hittime_diff_offbeam_diff_chip2"),"a",41,0,41, 580*2, -580, 580);

  h_pe_chip_onbeam         ->SetTitle(Form("pe onbeam chip;chip;pe"));
  h_pe_chip_offbeam        ->SetTitle(Form("pe offbeam chip;chip;pe"));
  h_hittime_chip_onbeam    ->SetTitle(Form("hittime onbeam chip;chip;time"));
  h_hittime_chip_offbeam   ->SetTitle(Form("hittime offbeam chip;chip;time"));
  h_bcid_chip_onbeam       ->SetTitle(Form("bcid onbeam chip;chip (TopView:0-19, SideView:21-40);bcid"));
  h_bcid_chip_offbeam      ->SetTitle(Form("bcid offbeam chip;chip (TopView:0-19, SideView:21-40);bcid"));
  h_diff_hittime_chip_onbeam    ->SetTitle(Form("diff hittime onbeam chip;chip;time"));
  h_diff_hittime_chip_offbeam   ->SetTitle(Form("diff hittime offbeam chip;chip;time"));
  h_diff_hittime_diff_chip_onbeam    ->SetTitle(Form("diff hittime onbeam diff chip;chip;time"));
  h_diff_hittime_diff_chip_offbeam   ->SetTitle(Form("diff hittime offbeam diff chip;chip;time"));
  h_diff_hittime_diff_chip_onbeam2    ->SetTitle(Form("diff hittime onbeam diff chip;chip;time"));
  h_diff_hittime_diff_chip_offbeam2   ->SetTitle(Form("diff hittime offbeam diff chip;chip;time"));


  int loop_spill = 0;
  for(int i=0;i<2;i++){
    for(int j=0;j<2;j++){
      h_pe[i][j] = new TH2D(Form("pe_view%d_grid%d",i,j),"pe",nbin,0,nbin,100,0,50);
      h_pe[i][j] -> SetTitle(Form("pe History (view=%d,grid=%d);time;pe",i,j));
      h_pe_mm[i][j] = new TH2D(Form("pe_mm_view%d_grid%d",i,j),"pe_mm",nbin,0,nbin,100,0,50);
      h_pe_mm[i][j] -> SetTitle(Form("pe permm History (view=%d,grid=%d);time;pe",i,j));
    }
  }
  for(int ibin=1;ibin<=nbin;ibin=ibin+step){
    time_t ibin_time = start_time +(ibin-1)*time_norm;
    tm *tm_bin = localtime(&ibin_time);
    string s_time_bin = Form("%04d/%02d/%02d %02d:%02d",
        tm_bin->tm_year+1900,
        tm_bin->tm_mon+1,
        tm_bin->tm_mday,
        tm_bin->tm_hour,
        tm_bin->tm_min);
    h_Gain         -> GetXaxis()-> SetBinLabel(ibin,s_time_bin.c_str());
    h_Noise        -> GetXaxis()-> SetBinLabel(ibin,s_time_bin.c_str());
    h_wPedestal    -> GetXaxis()-> SetBinLabel(ibin,s_time_bin.c_str());
    h_diffGain     -> GetXaxis()-> SetBinLabel(ibin,s_time_bin.c_str());
    h_diffPedestal -> GetXaxis()-> SetBinLabel(ibin,s_time_bin.c_str());
    h_diffGain2    -> GetXaxis()-> SetBinLabel(ibin,s_time_bin.c_str());
    h_diffNoise2   -> GetXaxis()-> SetBinLabel(ibin,s_time_bin.c_str());
    h_pe_mm_mean   -> GetXaxis()-> SetBinLabel(ibin,s_time_bin.c_str());
    h_diffPedestal2-> GetXaxis()-> SetBinLabel(ibin,s_time_bin.c_str());
    h_hit_timing2  -> GetXaxis()-> SetBinLabel(ibin,s_time_bin.c_str());
    h_spill        -> GetXaxis()-> SetBinLabel(ibin,s_time_bin.c_str());
    for(int i=0;i<2;i++){
      for(int j=0;j<2;j++){
        h_pe[i][j]    -> GetXaxis()-> SetBinLabel(ibin,s_time_bin.c_str());
        h_pe_mm[i][j] -> GetXaxis()-> SetBinLabel(ibin,s_time_bin.c_str());
      }
    }
  }

  TH1D * spill;
  TH1D * pe[2][2];
  TH1D * pe_permm[2][2];
  TH1D * hit_timing;
  TH1D * hit_bcid;
  TH2D * pe_map;
  TH2D * nhit_map;
  TH2D * gain_map;

  TH1D * pe_onbeam            ;
  TH1D * pe_offbeam           ;
  TH2D * pe_chip_onbeam       ;
  TH2D * pe_chip_offbeam      ;
  TH1D * bcid_onbeam          ;
  TH1D * bcid_offbeam         ;
  TH2D * bcid_chip_onbeam     ;
  TH2D * bcid_chip_offbeam    ;
  TH1D * hittime_onbeam       ;
  TH1D * hittime_offbeam      ;
  TH2D * hittime_chip_onbeam  ;
  TH2D * hittime_chip_offbeam ;
  TH2D * diff_hittime_chip_onbeam  ;
  TH2D * diff_hittime_chip_offbeam ;
  TH2D * diff_hittime_diff_chip_onbeam  ;
  TH2D * diff_hittime_diff_chip_offbeam ;
  TH2D * diff_hittime_diff_chip_onbeam2  ;
  TH2D * diff_hittime_diff_chip_offbeam2 ;
  /*
  TH1D * pe_permm2[2][2];
  TH1D * pathlength[2][2];
  TH1D * cos_zen   [2][2];
  TH1D * cos_azi   [2][2];
  */

  TFile *fin;
  TTree *tree;
  double t_gain,t_noise,t_pedestal[16],t_pedestal_w[16],tc_gain,tc_pedestal[16],tc_ped_nohit[16];
  vector<double> pt_gain,pt_pedestal,pt_noise;
  int t_view,t_pln,t_ch,t_dif,t_chip,t_chipch,t_col[16];
  // =========== Read Data ============= //
  int last_spill=0; 
  for(int ifn=0;ifn<FN;ifn++){
    tm *tm = localtime(&TimeList[ifn]);
    string s_time = Form("%04d-%02d-%02d-%02d-%02d",
        tm->tm_year+1900,
        tm->tm_mon+1,
        tm->tm_mday,
        tm->tm_hour,
        tm->tm_min);
    string filename=(Form("%s/%s/dq.root",inputDirName.c_str(),s_time.c_str()));
    cout << "Reading .... " << filename << endl;
    fin = new TFile(filename.c_str(),"read");
    tree = (TTree*) fin->Get("tree");
    tree->SetBranchAddress("gain"       ,&t_gain        );
    tree->SetBranchAddress("noise"      ,&t_noise       );
    tree->SetBranchAddress("pedestal"   ,t_pedestal     );
    tree->SetBranchAddress("pedestal_w" ,t_pedestal_w     );
    tree->SetBranchAddress("calib_gain" ,&tc_gain       );
    tree->SetBranchAddress("calib_pedestal" ,tc_pedestal);
    tree->SetBranchAddress("calib_ped_nohit" ,tc_ped_nohit);
    tree->SetBranchAddress("view"       ,&t_view        );
    tree->SetBranchAddress("pln"        ,&t_pln         );
    tree->SetBranchAddress("ch"         ,&t_ch          );
    tree->SetBranchAddress("dif"        ,&t_dif         );
    tree->SetBranchAddress("chip"       ,&t_chip        );
    tree->SetBranchAddress("chipch"     ,&t_chipch      );
    tree->SetBranchAddress("col"        , t_col         );

    spill = (TH1D*) fin->Get("spillnb");
    pe_map = (TH2D*) fin->Get("pe_map");
    nhit_map = (TH2D*) fin->Get("nhit_map");
    gain_map = (TH2D*) fin->Get("gain_map");
    hit_timing = (TH1D*)fin->Get(Form("hit_timing"));
    hit_bcid   = (TH1D*)fin->Get(Form("hit_bcid"));
    pe_onbeam            = (TH1D*) fin->Get("pe_onbeam"); 
    pe_offbeam           = (TH1D*) fin->Get("pe_offbeam");
    bcid_onbeam          = (TH1D*) fin->Get("bcid_onbeam");
    bcid_offbeam         = (TH1D*) fin->Get("bcid_offbeam");
    hittime_onbeam       = (TH1D*) fin->Get("hittime_onbeam");
    hittime_offbeam      = (TH1D*) fin->Get("hittime_offbeam");
    pe_chip_onbeam       = (TH2D*) fin->Get("pe_onbeam_chip");
    pe_chip_offbeam      = (TH2D*) fin->Get("pe_offbeam_chip");
    bcid_chip_onbeam     = (TH2D*) fin->Get("bcid_onbeam_chip");
    bcid_chip_offbeam    = (TH2D*) fin->Get("bcid_offbeam_chip");
    hittime_chip_onbeam  = (TH2D*) fin->Get("hittime_onbeam_chip");
    hittime_chip_offbeam = (TH2D*) fin->Get("hittime_offbeam_chip");
    diff_hittime_chip_onbeam  = (TH2D*) fin->Get("diff_hittime_onbeam_chip");
    diff_hittime_chip_offbeam = (TH2D*) fin->Get("diff_hittime_offbeam_chip");
    diff_hittime_diff_chip_onbeam  = (TH2D*) fin->Get("diff_hittime_onbeam_diff_chip");
    diff_hittime_diff_chip_offbeam = (TH2D*) fin->Get("diff_hittime_offbeam_diff_chip");
    diff_hittime_diff_chip_onbeam2  = (TH2D*) fin->Get("diff_hittime_onbeam_diff_chip2");
    diff_hittime_diff_chip_offbeam2 = (TH2D*) fin->Get("diff_hittime_offbeam_diff_chip2");

    for(int i=0;i<2;i++){
      for(int j=0;j<2;j++){
        pe[i][j]          = (TH1D*)fin->Get(Form("pe_view%d_grid%d",i,j));
        pe_permm[i][j]    = (TH1D*)fin->Get(Form("pe_permm_view%d_grid%d",i,j));
        //pe_permm2[i][j]   = (TH1D*)fin->Get(Form("pe_permm2_%d_%d",i,j));
        //pathlength[i][j]  = (TH1D*)fin->Get(Form("path_view%d_grid%d",i,j));
        //cos_zen[i][j]     = (TH1D*)fin->Get(Form("cos_z_view%d_grid%d",i,j));
        //cos_azi[i][j]     = (TH1D*)fin->Get(Form("cos_azi_view%d_grid%d",i,j));
      }
    }
    
    // ====== Fill  Histgram ====== // 
    pt_gain.clear();
    pt_noise.clear();
    pt_pedestal.clear();
    int neve=tree->GetEntries();
    if(ifn==0){
      pt_gain.resize(neve);
      pt_noise.resize(neve);
      pt_pedestal.resize(neve);
    }
    int ibin = (TimeList[ifn]-start_time)/time_norm;
    for(int ieve=0;ieve<neve;ieve++){
      tree->GetEntry(ieve);
      h_Gain         ->Fill(ibin,  t_gain); 
      h_diffGain     ->Fill(ibin,  t_gain-tc_gain);
      h_diffPedestal ->Fill(ibin,  t_pedestal[0]-tc_ped_nohit[0]);
      h_wPedestal    ->Fill(ibin,     t_pedestal_w[0]);
      h_Noise        ->Fill(ibin,  t_noise);  
      if(ifn!=0){
        h_diffGain2    ->Fill(ibin,  t_gain-pt_gain[ieve]);
        h_diffPedestal2->Fill(ibin,  t_pedestal[0]-pt_pedestal[ieve]); 
        if(pt_noise[ieve]!=0.) h_diffNoise2 ->Fill(ibin,t_noise/pt_noise[ieve]*100.-100.); 
      }
      pt_gain[ieve]  = t_gain;
      pt_noise[ieve] = t_noise;
      pt_pedestal[ieve] = t_pedestal[0];
    }//ieve

    for(int i=0;i<2;i++){
      for(int j=0;j<2;j++){
        for(int k=1;k<=pe[i][j]->GetXaxis()->GetNbins();k++){
          double y,z;
          y = pe[i][j]->GetXaxis()->GetBinCenter(k);
          z = pe[i][j]->GetBinContent(k);
          h_pe[i][j]->Fill(ibin,y,z); 
        }
        for(int k=1;k<=pe_permm[i][j]->GetXaxis()->GetNbins();k++){
          double y,z;
          y = pe_permm[i][j]->GetXaxis()->GetBinCenter(k);
          z = pe_permm[i][j]->GetBinContent(k);
          h_pe_mm[i][j]->Fill(ibin,y,z); 
        }
      }
    }
    for(int i=0;i<17;i++){
      if(i==8){continue;}
      for(int j=0;j<80;j++){
        int nhit = nhit_map->GetBinContent(i,j);
        if(nhit!=0) h_pe_mm_mean->Fill(ibin,pe_map->GetBinContent(i,j));
      }
    }
    
    h_pe_map->Add(pe_map);
    h_nhit_map->Add(nhit_map);
    if(ifn==FN-1) h_gain_map->Add(gain_map);
    for(int i=0x8000;i<=0xffff;i++){
      int content = spill->GetBinContent(i+1);
      if(content==1){ h_spill->SetBinContent(ibin,i+1-0x8000,1); }
      else if(i<0xffff&&content>1&&spill->GetBinContent(i+2)==0){ 
          h_spill->SetBinContent(ibin,i+1-0x8000,1);
          h_spill->SetBinContent(ibin,i+2-0x8000,1);
      }
    }
    for(int k=1;k<=hit_timing->GetXaxis()->GetNbins();k++){
      double y,z;
      y = hit_timing->GetXaxis()->GetBinCenter(k);
      z = hit_timing->GetBinContent(k);
      if(y>hittiming_min&&y<hittiming_max){
        h_hit_timing2->Fill(ibin,y,z); 
        h_hit_timing1->Fill(y,z);
      }
    }
    for(int k=1;k<=hit_bcid->GetXaxis()->GetNbins();k++){
      double y,z;
      y = hit_bcid->GetXaxis()->GetBinCenter(k);
      z = hit_bcid->GetBinContent(k);
      if(y>hitbcid_min&&y<hitbcid_max){
        h_hit_bcid->Fill(y,z);
      }
    }
    h_pe_onbeam           ->Add( pe_onbeam      ); 
    h_pe_offbeam          ->Add( pe_offbeam     );
    h_bcid_onbeam         ->Add( bcid_onbeam    );
    h_bcid_offbeam        ->Add( bcid_offbeam   );
    h_hittime_onbeam      ->Add( hittime_onbeam );
    h_hittime_offbeam     ->Add( hittime_offbeam);
    h_pe_chip_onbeam      ->Add( pe_chip_onbeam      ); 
    h_pe_chip_offbeam     ->Add( pe_chip_offbeam     );
    h_hittime_chip_onbeam ->Add( hittime_chip_onbeam    );
    h_hittime_chip_offbeam->Add( hittime_chip_offbeam   );
    h_bcid_chip_onbeam    ->Add( bcid_chip_onbeam );
    h_bcid_chip_offbeam   ->Add( bcid_chip_offbeam);
    h_diff_hittime_chip_onbeam     ->Add( diff_hittime_chip_onbeam );
    h_diff_hittime_chip_offbeam    ->Add( diff_hittime_chip_offbeam);
    h_diff_hittime_diff_chip_onbeam     ->Add( diff_hittime_diff_chip_onbeam );
    h_diff_hittime_diff_chip_offbeam    ->Add( diff_hittime_diff_chip_offbeam);
    h_diff_hittime_diff_chip_onbeam2     ->Add( diff_hittime_diff_chip_onbeam2 );
    h_diff_hittime_diff_chip_offbeam2    ->Add( diff_hittime_diff_chip_offbeam2);
    fin->Close();
  }//ifn
  h_pe_map->Scale(1./(double)FN);

  TBox *box = new TBox();
  box->SetFillColor(kRed);
  box->SetLineColor(kRed);
  box->SetFillStyle(3004);
  box->IsTransparent();
  // ================== Print Histgram ==================== //
  gStyle->SetOptStat(0);
  TCanvas *c1 = new TCanvas("c1","c1",800,600);
  TPad *pad = new TPad("pad","pad",0.01,0.20,0.99,0.99);
  pad->Draw();
  pad->cd();
  h_Gain->Draw("colz");
  box->DrawBox(0,36,nbin,44);
  c1->Print(Form("%s/History_Gain.png",outputDir.c_str()));
  h_diffGain->Draw("colz");
  box->DrawBox(0,-4,nbin,4);
  c1->Print(Form("%s/History_diffGain.png",outputDir.c_str()));
  h_diffPedestal->Draw("colz");
  c1->Print(Form("%s/History_diffPedestal.png",outputDir.c_str()));
  h_Noise->Draw("colz");
  c1->Print(Form("%s/History_Noise.png",outputDir.c_str()));

  h_diffGain2->Draw("colz");
  box->DrawBox(0,-4,nbin,4);
  c1->Print(Form("%s/History_diffGain2.png",outputDir.c_str()));
  h_diffPedestal2->Draw("colz");
  c1->Print(Form("%s/History_diffPedestal2.png",outputDir.c_str()));
  h_diffNoise2->Draw("colz");
  c1->Print(Form("%s/History_diffNoise2.png",outputDir.c_str()));
  h_pe_mm_mean->Draw("colz");
  c1->Print(Form("%s/History_pe_mm_mean.png",outputDir.c_str()));
  h_spill->Draw("colz");
  c1->Print(Form("%s/History_spillnb.png",outputDir.c_str()));
  h_hit_timing2->Draw("colz");
  c1->Print(Form("%s/History_hit_timing.png",outputDir.c_str()));
  c1->Clear();
  c1->SetLogy();
  h_hit_timing1->SetFillColor(kBlue);
  h_hit_timing1->Draw("HIST");
  c1->Print(Form("%s/hit_timing.png",outputDir.c_str()));
  c1->Clear();
  c1->SetLogy();
  h_hit_bcid->SetFillColor(kBlue);
  h_hit_bcid->Draw("HIST");
  c1->Print(Form("%s/hit_bcid.png",outputDir.c_str()));
  c1->Clear();
  c1->SetLogy(0);
  c1->Update();
  h_pe_map->Draw("colz");
  c1->Print(Form("%s/History_pe_map.png",outputDir.c_str()));
  h_nhit_map->Draw("colz");
  c1->Print(Form("%s/History_nhit_map.png",outputDir.c_str()));
  h_gain_map->Draw("colz");
  c1->Print(Form("%s/History_gain_map.png",outputDir.c_str()));
  for(int i=0;i<2;i++){
    for(int j=0;j<2;j++){
      h_pe[i][j]->Draw("colz");
      c1->Print(Form("%s/History_pe_view%d_grid%d.png",outputDir.c_str(),i,j));
      h_pe_mm[i][j]->Draw("colz");
      c1->Print(Form("%s/History_pemm_view%d_grid%d.png",outputDir.c_str(),i,j));
    }
  }
  c1->SetLogy(1);
  h_pe_onbeam       -> Draw(); 
  c1->Print(Form("%s/History_pe_onbeam.png",outputDir.c_str()));
  h_pe_offbeam      -> Draw();
  c1->Print(Form("%s/History_pe_offbeam.png",outputDir.c_str()));
  h_hittime_onbeam  -> Draw();
  c1->Print(Form("%s/History_hititme_onbeam.png",outputDir.c_str()));
  h_hittime_offbeam -> Draw();
  c1->Print(Form("%s/History_hittime_offbeam.png",outputDir.c_str()));
  h_bcid_onbeam     -> Draw();
  c1->Print(Form("%s/History_bcid_onbeam.png",outputDir.c_str()));
  h_bcid_offbeam    -> Draw();
  c1->Print(Form("%s/History_bcid_offbeam.png",outputDir.c_str()));
  c1->SetLogy(0);
  c1->SetLogz(1);
  h_pe_chip_onbeam      -> Draw("colz"); 
  c1->Print(Form("%s/History_pe_chip_onbeam.png",outputDir.c_str()));
  h_pe_chip_offbeam     -> Draw("colz");
  c1->Print(Form("%s/History_pe_chip_offbeam.png",outputDir.c_str()));
  h_hittime_chip_onbeam -> Draw("colz");
  c1->Print(Form("%s/History_hittime_chip_onbeam.png",outputDir.c_str()));
  h_hittime_chip_offbeam-> Draw("colz");
  c1->Print(Form("%s/History_hittime_chip_offbeam.png",outputDir.c_str()));
  h_bcid_chip_onbeam    -> Draw("colz");
  c1->Print(Form("%s/History_bcid_chip_onbeam.png",outputDir.c_str()));
  h_bcid_chip_offbeam   -> Draw("colz");
  c1->Print(Form("%s/History_bcid_chip_offbeam.png",outputDir.c_str()));
  h_diff_hittime_chip_onbeam -> Draw("colz");
  c1->Print(Form("%s/History_diff_hittime_chip_onbeam.png",outputDir.c_str()));
  h_diff_hittime_chip_offbeam-> Draw("colz");
  c1->Print(Form("%s/History_diff_hittime_chip_offbeam.png",outputDir.c_str()));
  h_diff_hittime_diff_chip_onbeam -> Draw("colz");
  c1->Print(Form("%s/History_diff_hittime_diff_chip_onbeam.png",outputDir.c_str()));
  h_diff_hittime_diff_chip_offbeam-> Draw("colz");
  c1->Print(Form("%s/History_diff_hittime_diff_chip_offbeam.png",outputDir.c_str()));
  h_diff_hittime_diff_chip_onbeam2 -> Draw("colz");
  c1->Print(Form("%s/History_diff_hittime_diff_chip_onbeam2.png",outputDir.c_str()));
  h_diff_hittime_diff_chip_offbeam2-> Draw("colz");
  c1->Print(Form("%s/History_diff_hittime_diff_chip_offbeam2.png",outputDir.c_str()));

  /*
  TFile *fout = new TFile("dq_history.root","recreate");
  h_Gain         ->Write();
  h_diffGain     ->Write();
  h_diffPedestal ->Write();
  h_Noise        ->Write();
  h_diffGain2    ->Write();
  h_diffPedestal2->Write();
  h_Noise        ->Write();
  h_diffNoise2   ->Write();
  h_diffGain2    ->Write();
  h_spill        ->Write();
  h_hit_timing   ->Write();
  h_pe_map       ->Write();
  h_nhit_map     ->Write();
  h_gain_map     ->Write();
  for(int i=0;i<2;i++){
    for(int j=0;j<2;j++){
      h_pe   [i][j]->Write();
      h_pe_mm[i][j]->Write();
    }
  }
  fout->Close();
  */

  delete box;
  delete c1;
  return;
}

/*
   void AnaRecon(string &filename,string& outputDir, string& s_time){
   wgGetTree * gettree = new wgGetTree(filename);
   Hit_t    type_hit;
   Track_t  type_track;

   gettree->SetTreeFile(type_hit);
   gettree->SetTreeFile(type_track);

   TCanvas *c1 = new TCanvas("c1","c1",1200,1000);
   c1->Divide(2,2);

   TH1D * pe[2][2];
   TH1D * pe_permm[2][2];
   TH1D * pe_permm2[2][2];
   TH1D * pathlength[2][2];
   TH1D * cos_zen   [2][2];
   TH1D * cos_azi   [2][2];
   TH2D * cos_p_zen   [2][2];
   TH2D * cos_p_azi   [2][2];
   TH2D * pe_permm_p  [2][2];
   TH2D * pe_permm_cz  [2][2];
   TH2D * pe_permm_ca  [2][2];
   for(int i=0;i<2;i++){
   for(int j=0;j<2;j++){
   pe[i][j] = new TH1D(Form("pe_view%d_grid%d",i,j),"a",100,0,50);
   pe[i][j] -> SetTitle(Form("pe view=%d grid=%d;pe;nEntry",i,j));
   pe_permm[i][j] = new TH1D(Form("pe_permm_view%d_grid%d",i,j),"a",100,0,50);
   pe_permm[i][j] -> SetTitle(Form("pe per3mm view=%d grid=%d;pe;nEntry",i,j));
   pe_permm2[i][j] = new TH1D(Form("pe_permm2_%d_%d",i,j),"a",98,1,50);
   pe_permm2[i][j] -> SetTitle(Form("pe per3mm view=%d grid=%d;pe;nEntry",i,j));
   pathlength[i][j] = new TH1D(Form("path_view%d_grid%d",i,j),"a",100,0,50);
   pathlength[i][j] -> SetTitle(Form("pathlength view=%d grid=%d;pathlength;nEntry",i,j));
   cos_zen[i][j] = new TH1D(Form("cos_z_view%d_grid%d",i,j),"a",100,-1,1);
   cos_zen[i][j] -> SetTitle(Form("cos zenith view=%d grid=%d;cos;nEntry",i,j));
   cos_azi[i][j] = new TH1D(Form("cos_azi_view%d_grid%d",i,j),"a",100,-1,1);
   cos_azi[i][j] -> SetTitle(Form("cos azimuth view=%d grid=%d;cos;nEntry",i,j));
   cos_p_zen[i][j] = new TH2D(Form("cos_path_z_view%d_grid%d",i,j),"a",100,-1,1,100,0,50);
   cos_p_zen[i][j] -> SetTitle(Form("path vs cos zenith view=%d grid=%d;cos;pathlength",i,j));
   cos_p_azi[i][j] = new TH2D(Form("cos_path_azi_view%d_grid%d",i,j),"a",100,-1,1,100,0,50);
   cos_p_azi[i][j] -> SetTitle(Form("path vs cos azimuth view=%d grid=%d;cos;pathlength",i,j));
   pe_permm_p[i][j] = new TH2D(Form("pe_permm_path_view%d_grid%d",i,j),"a",100,0,50,100,0,50);
   pe_permm_p[i][j] -> SetTitle(Form("path vs pe per3mm view=%d grid=%d;pathlength;pe per3mm",i,j));
   pe_permm_cz[i][j] = new TH2D(Form("pe_permm_zen_%d_%d",i,j),"a",100,-1,1,100,0,50);
   pe_permm_cz[i][j] -> SetTitle(Form("cos zenith vs pe per3mm view=%d grid=%d;cos;pe per3mm",i,j));
   pe_permm_ca[i][j] = new TH2D(Form("pe_permm_azi_%d_%d",i,j),"a",100,-1,1,100,0,50);
   pe_permm_ca[i][j] -> SetTitle(Form("cos azimuth vs pe per3mm view=%d grid=%d;cos;pe per3mm",i,j));
   }
   }


   int neve = wgGetTree::tree->GetEntries();
   for(int ieve=0; ieve < neve; ieve++){
   gettree->GetEntry(ieve);
   if(type_track.num_trackid!=1) continue;
   for(int i=0;i<type_track.num_track;i++){
   int view = type_track.track_view[i];
   for(int j=0;j<type_track.num_track_hits[i];j++){
   int grid=0;
   int hitid = type_track.track_hits_hitid[i][j];
   if(type_hit.hit_grid[hitid]) grid=1;
   pe           [view][grid] -> Fill( type_hit.hit_pe[hitid]         );
   pe_permm     [view][grid] -> Fill( type_hit.hit_pe_permm[hitid]*3 );
   pe_permm2    [view][grid] -> Fill( type_hit.hit_pe_permm[hitid]*3 );
   pathlength   [view][grid] -> Fill( type_hit.hit_pathlength[hitid] );
   cos_zen      [view][grid] -> Fill( type_track.track_cos_zen[i]);
   cos_azi      [view][grid] -> Fill( type_track.track_cos_azi[i]);
   cos_p_zen    [view][grid] -> Fill( type_track.track_cos_zen[i],type_hit.hit_pathlength[hitid]);
   cos_p_azi    [view][grid] -> Fill( type_track.track_cos_azi[i],type_hit.hit_pathlength[hitid]);
   pe_permm_p   [view][grid] -> Fill( type_hit.hit_pathlength[hitid] , type_hit.hit_pe_permm[hitid]*3 );
   pe_permm_cz  [view][grid] -> Fill( type_track.track_cos_zen[i] , type_hit.hit_pe_permm[hitid]*3 );
pe_permm_ca  [view][grid] -> Fill( type_track.track_cos_azi[i] , type_hit.hit_pe_permm[hitid]*3 );
}
}
}

TLine *l = new TLine(-1,-1,-1,-1);
l->SetLineColor(kRed);
l->SetLineWidth(1);
TArrow *arrow = new TArrow();
arrow->SetLineColor(kRed);
arrow->SetLineWidth(1);
TText *text = new TText();
text->SetTextSize(0.04);
text->SetTextColor(kRed);

fout->cd(); 
for(int i=0;i<2;i++){
  for(int j=0;j<2;j++){
    c1->cd(i*2+j+1);
    pe           [i][j] -> Write();
    pe           [i][j] -> Draw();
  }
}
c1->Print(Form("%s/%s/pe.png",outputDir.c_str(),s_time.c_str()));

for(int i=0;i<2;i++){
  for(int j=0;j<2;j++){
    c1->cd(i*2+j+1);
    pe_permm    [i][j] -> Write();
    pe_permm    [i][j] -> Draw();
    l->DrawLine(1,0,1,pe_permm[i][j]->GetMaximum());
    arrow->DrawArrow(1,pe_permm[i][j]->GetMaximum()*0.1,5,pe_permm[i][j]->GetMaximum()*0.1,0.01,"->");
    double mean = pe_permm2[i][j]->GetMean();
    text->DrawText(7,pe_permm[i][j]->GetMaximum()*0.1,Form("mean(1-50pe):%2.2f",mean));
  }
}
c1->Print(Form("%s/%s/pe_permm.png",outputDir.c_str(),s_time.c_str()));

for(int i=0;i<2;i++){
  for(int j=0;j<2;j++){
    c1->cd(i*2+j+1);
    pathlength  [i][j] -> Write();
    pathlength  [i][j] -> Draw();
  }
}
c1->Print(Form("%s/%s/pathlengh.png",outputDir.c_str(),s_time.c_str()));

for(int i=0;i<2;i++){
  for(int j=0;j<2;j++){
    c1->cd(i*2+j+1);
    cos_zen    [i][j] -> Write();
    cos_zen    [i][j] -> Draw();
  }
}
c1->Print(Form("%s/%s/cos_zen.png",outputDir.c_str(),s_time.c_str()));

for(int i=0;i<2;i++){
  for(int j=0;j<2;j++){
    c1->cd(i*2+j+1);
    cos_azi    [i][j] -> Write();
    cos_azi    [i][j] -> Draw();
  }
}
c1->Print(Form("%s/%s/cos_azi.png",outputDir.c_str(),s_time.c_str()));

for(int i=0;i<2;i++){
  for(int j=0;j<2;j++){
    c1->cd(i*2+j+1);
    cos_p_zen    [i][j] -> Write();
    cos_p_zen    [i][j] -> Draw("colz");
  }
}
c1->Print(Form("%s/%s/cos_p_zen.png",outputDir.c_str(),s_time.c_str()));

for(int i=0;i<2;i++){
  for(int j=0;j<2;j++){
    c1->cd(i*2+j+1);
    cos_p_azi    [i][j] -> Write();
    cos_p_azi    [i][j] -> Draw("colz");
  }
}
c1->Print(Form("%s/%s/cos_p_azi.png",outputDir.c_str(),s_time.c_str()));

for(int i=0;i<2;i++){
  for(int j=0;j<2;j++){
    c1->cd(i*2+j+1);
    pe_permm_p    [i][j] -> Write();
    pe_permm_p    [i][j] -> Draw("colz");
  }
}
c1->Print(Form("%s/%s/pe_permm_p.png",outputDir.c_str(),s_time.c_str()));

for(int i=0;i<2;i++){
  for(int j=0;j<2;j++){
    c1->cd(i*2+j+1);
    pe_permm_cz    [i][j] -> Write();
    pe_permm_cz    [i][j] -> Draw("colz");
  }
}
c1->Print(Form("%s/%s/pe_permm_zen.png",outputDir.c_str(),s_time.c_str()));

for(int i=0;i<2;i++){
  for(int j=0;j<2;j++){
    c1->cd(i*2+j+1);
    pe_permm_ca    [i][j] -> Write();
    pe_permm_ca    [i][j] -> Draw("colz");
  }
}
c1->Print(Form("%s/%s/pe_permm_azi.png",outputDir.c_str(),s_time.c_str()));

fout->Close();
delete gettree; 
}
void MakeDir(string& str){
  CheckExist *check = new CheckExist;
  if(!check->Dir(str)){
    system(Form("mkdir %s",str.c_str()));
  }
  delete check;
}

//
*/
