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
#include <TH1F.h>
#include <TH2F.h>
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
void AnaDQ(string inputDirName, vector<time_t> &TimeList, string& outputDir,bool chipmode,int targetchip);
double cal_mean(vector<double>);

double days=7;
const double time_norm = 3600.*3.; //sec every 3 hours = 3600.*3

int main(int argc, char** argv){
  int opt;
  time_t current_time=0;
  string inputtime("");
  wgConst *con = new wgConst;
  con->GetENV();
  string inputDirName  =  con->DQ_DIRECTORY;
  string outputDir     =  con->DQHISTORY_DIRECTORY;
  string logoutputDir=con->LOG_DIRECTORY;

  bool chipmode = false;
  int targetchip = -1;

  OperateString *OpStr = new OperateString;
  Logger *Log = new Logger;
  CheckExist *check = new CheckExist;

  Log->Initialize();
  Log->Write("start calibration...");

  while((opt = getopt(argc,argv, "f:t:o:d:c:h")) !=-1 ){
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
        days=atof(optarg); 
        break;

      case 'o':
        outputDir = optarg; 
        break;

      case 'c':
        chipmode = true;
        targetchip = atoi(optarg);
        break;
      case 'h':
        cout <<"This program is for data quality check. "<<endl;
        cout <<"You can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose DQ directory (default : WAGASCI_DQDIR)"<<endl;
        cout <<"  -t (char*) : choose time.        (default : current time)" <<endl;
        cout <<"      (ex.) 2017/9/11 10:11 -> 2017-09-11-10-11" << endl;
        cout <<"  -d (double): choose days.        (default : w7)" <<endl;
        cout <<"  -o (char*) : choose output directory (currently set:"<<outputDir<<")"<<endl;
        cout <<"  -c (int)   : to focus on a chip, to see gain chipch by chipch" << endl;
        exit(0);
      default:
        cout <<"See the help" << endl;
        cout << argv[0] << "-h"<<endl;
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

  AnaDQ(inputDirName,TimeList,outputDir,chipmode,targetchip);

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
void AnaDQ(string inputDirName, vector<time_t> &TimeList, string& outputDir,bool chipmode,int targetchip){

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

  // ================ define histgram ================== //
  TH2F *h_Gain[40];
  TH2F *h_diffGain[40];
  TH2F *h_diffGain2[40];
  if(!chipmode){
    for(int ichip=0;ichip<40;ichip++){
      string name;
      name = Form("gain_chip%d",ichip);
      h_Gain[ichip]      = new TH2F(name.c_str(),name.c_str(),nbin,0,nbin,80,20,60);
      name = Form("gain_diff_chip%d",ichip);
      h_diffGain[ichip]  = new TH2F(name.c_str(),name.c_str(),nbin,0,nbin,80,-20,20);
      name = Form("gain_diff2_chip%d",ichip);
      h_diffGain2[ichip] = new TH2F(name.c_str(),name.c_str(),nbin,0,nbin,80,-20,20);

      h_Gain     [ichip]->SetTitle(Form("Gain History chip%d;time;Gain"                     ,ichip));
      h_diffGain [ichip]->SetTitle(Form("diff Gain History chip%d;time;diff Gain"           ,ichip));
      h_diffGain2[ichip]->SetTitle(Form("Gain diff. from one previous chip%d;time;diff Gain",ichip));
    }
    for(int ichip=0;ichip<40;ichip++){
      for(int ibin=1;ibin<=nbin;ibin=ibin+step){
        time_t ibin_time = start_time +(ibin-1)*time_norm;
        tm *tm_bin = localtime(&ibin_time);
        string s_time_bin = Form("%04d/%02d/%02d %02d:%02d",
            tm_bin->tm_year+1900,
            tm_bin->tm_mon+1,
            tm_bin->tm_mday,
            tm_bin->tm_hour,
            tm_bin->tm_min);
        h_Gain     [ichip]-> GetXaxis()-> SetBinLabel(ibin,s_time_bin.c_str());
        h_diffGain [ichip]-> GetXaxis()-> SetBinLabel(ibin,s_time_bin.c_str());
        h_diffGain2[ichip]-> GetXaxis()-> SetBinLabel(ibin,s_time_bin.c_str());
      }
    }
  }
  else{
    for(int ichipch=0;ichipch<32;ichipch++){
      string name;
      name = Form("gain_chip%d_chipch%d",targetchip,ichipch);
      h_Gain[ichipch]      = new TH2F(name.c_str(),name.c_str(),nbin,0,nbin,80,20,60);
      name = Form("gain_diff_chip%d_chipch%d",targetchip,ichipch);
      h_diffGain[ichipch]  = new TH2F(name.c_str(),name.c_str(),nbin,0,nbin,80,-20,20);
      name = Form("gain_diff2_chip%d_chipch%d",targetchip,ichipch);
      h_diffGain2[ichipch] = new TH2F(name.c_str(),name.c_str(),nbin,0,nbin,80,-20,20);

      h_Gain     [ichipch]->SetTitle(Form("Gain History chip%d chipch%d;time;Gain"                     ,targetchip,ichipch));
      h_diffGain [ichipch]->SetTitle(Form("diff Gain History chip%d chipch%d;time;diff Gain"           ,targetchip,ichipch));
      h_diffGain2[ichipch]->SetTitle(Form("Gain diff. from one previous chip%d chipch%d;time;diff Gain",targetchip,ichipch));
    }
    for(int ichipch=0;ichipch<32;ichipch++){
      for(int ibin=1;ibin<=nbin;ibin=ibin+step){
        time_t ibin_time = start_time +(ibin-1)*time_norm;
        tm *tm_bin = localtime(&ibin_time);
        string s_time_bin = Form("%04d/%02d/%02d %02d:%02d",
            tm_bin->tm_year+1900,
            tm_bin->tm_mon+1,
            tm_bin->tm_mday,
            tm_bin->tm_hour,
            tm_bin->tm_min);
        h_Gain     [ichipch]-> GetXaxis()-> SetBinLabel(ibin,s_time_bin.c_str());
        h_diffGain [ichipch]-> GetXaxis()-> SetBinLabel(ibin,s_time_bin.c_str());
        h_diffGain2[ichipch]-> GetXaxis()-> SetBinLabel(ibin,s_time_bin.c_str());
      }
    }
  }



  TFile *fin;
  TTree *tree;
  double t_gain,tc_gain;
  vector<double> pt_gain;
  int t_view,t_pln,t_ch,t_dif,t_chip,t_chipch,t_col[16];
  // =========== Read Data ============= //
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
    tree->SetBranchAddress("calib_gain" ,&tc_gain       );
    tree->SetBranchAddress("view"       ,&t_view        );
    tree->SetBranchAddress("pln"        ,&t_pln         );
    tree->SetBranchAddress("ch"         ,&t_ch          );
    tree->SetBranchAddress("dif"        ,&t_dif         );
    tree->SetBranchAddress("chip"       ,&t_chip        );
    tree->SetBranchAddress("chipch"     ,&t_chipch      );
    tree->SetBranchAddress("col"        , t_col         );

   
    // ====== Fill  Histgram ====== // 
    pt_gain.clear();
    int neve=tree->GetEntries();
    if(ifn==0){
      pt_gain.resize(neve);
    }
    int ibin = (TimeList[ifn]-start_time)/time_norm;
    for(int ieve=0;ieve<neve;ieve++){
      tree->GetEntry(ieve);
      if(!chipmode){
        h_Gain    [20*t_view+t_chip]->Fill(ibin,  t_gain); 
        h_diffGain[20*t_view+t_chip]->Fill(ibin,  t_gain-tc_gain);
        if(ifn!=0){
          h_diffGain2[20*t_view+t_chip]->Fill(ibin,  t_gain-pt_gain[ieve]);
        }
      }
      else{
        if(t_chip!=targetchip) continue;
        h_Gain    [t_chipch]->Fill(ibin,  t_gain); 
        h_diffGain[t_chipch]->Fill(ibin,  t_gain-tc_gain);
        if(ifn!=0){
          h_diffGain2[t_chipch]->Fill(ibin,  t_gain-pt_gain[ieve]);
        }
      }
      pt_gain[ieve]  = t_gain;
    }//ieve
    fin->Close();
  }//ifn

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

  if(!chipmode){
    for(int ichip=0;ichip<40;ichip++){
      h_Gain[ichip]->Draw("colz");
      box->DrawBox(0,36,nbin,44);
      c1->Print(Form("%s/History_Gain_chip%d.png",outputDir.c_str(),ichip));
      h_diffGain[ichip]->Draw("colz");
      box->DrawBox(0,-4,nbin,4);
      c1->Print(Form("%s/History_diffGain_chip%d.png",outputDir.c_str(),ichip));
      h_diffGain2[ichip]->Draw("colz");
      box->DrawBox(0,-4,nbin,4);
      c1->Print(Form("%s/History_diffGain2_chip%d.png",outputDir.c_str(),ichip));
    }
  }
  else{
    for(int ichipch=0;ichipch<32;ichipch++){
      h_Gain[ichipch]->Draw("colz");
      box->DrawBox(0,36,nbin,44);
      c1->Print(Form("%s/History_Gain_chip%d_chipch%d.png",outputDir.c_str(),targetchip,ichipch));
      h_diffGain[ichipch]->Draw("colz");
      box->DrawBox(0,-4,nbin,4);
      c1->Print(Form("%s/History_diffGain_chip%d_chipch%d.png",outputDir.c_str(),targetchip,ichipch));
      h_diffGain2[ichipch]->Draw("colz");
      box->DrawBox(0,-4,nbin,4);
      c1->Print(Form("%s/History_diffGain2_chip%d_chipch%d.png",outputDir.c_str(),targetchip,ichipch));
    }
  }
  delete box;
  delete c1;
  return;
}
