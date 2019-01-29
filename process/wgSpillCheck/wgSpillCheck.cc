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

void SpillCheck(string inputDir,string& outputDir,int runid,int acqid);

int main(int argc, char** argv){
  int opt;
  int runid = -1;
  int acqid = -1;
  wgConst *con = new wgConst;
  con->GetENV();
  string inputDirName  = con->RECON_DIRECTORY;
  string outputDirName = con->SPILL_DIRECTORY;
  string logoutputDir  = con->LOG_DIRECTORY;
  delete con;

  OperateString *OpStr = new OperateString;
  Logger *Log = new Logger;
  CheckExist *check = new CheckExist;

  Log->Initialize();
  Log->Write("start calibration...");

  while((opt = getopt(argc,argv, "f:r:s:o:h")) !=-1 ){
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
      case 'r':
        runid=atoi(optarg); 
        break;
      case 's':
        acqid=atoi(optarg); 
        break;
      case 'o':
        outputDirName = optarg; 
        break;
      case 'h':
        cout <<"This program is for data quality check. "<<endl;
        cout <<"You can take several option..."<<endl;
        cout <<"  -h               : help"<<endl;
        cout <<"  -f <inputDir>    : Input Directory (default: RECON_DIRECTORY)" << endl;
        cout <<"  -r <runid>       : Run ID" << endl;
        cout <<"  -s <acqid>       : Acq ID" << endl;
        cout <<"  -o <outputDir>   : Output Directory (defualt: SPILL_DIRECTORY)" << endl;
        exit(0);
    }   
  }

  if(outputDirName.empty()||runid==-1||acqid==-1){
    cout << "See the usage: " << argv[0] << " -h" << endl;
    exit(0);
  }

  cout << " *****  READ   DIRECTORY     :" << inputDirName  << "  *****" << endl;
  cout << " *****  OUTPUT DIRECTORY     :" << outputDirName << "  *****" << endl;
  cout << " *****  RUN ID = " << runid << " ACQ ID = " << acqid << endl;

  delete check;
  delete OpStr;

  SpillCheck(inputDirName,outputDirName,runid,acqid);

  Log->Write("end summarizeing ... " );
  delete Log;  
}

//******************************************************************
void SpillCheck(string inputDir, string& outputDir, int runid, int acqid)
{
  int nbin = 0x8000;
  TH1F *h_spill     = new TH1F("spillnb"     ,"spillnb"     ,nbin,0,nbin);
  TH1F *h_spill2    = new TH1F("spillnb_cor" ,"spillnb_cor" ,nbin,0,nbin);
  TH1F *h_dspillcnt = new TH1F("dspillcnt"   ,"dspillcnt"   ,31  ,-1,30);
  int last_spill[2]={-1,-1};
  int last_spillcnt[2]={-1,-1};
  int dspill[2]={-1,-1};
  int dspillcnt[2]={-1,-1};
  int ddspill= -1;
  int ddspillcnt= -1;
  int loop = 0;
  int loop2 = 0;
  int start_spill = 0x8000;
  int stop_spill = -1;
  int num_spill = 0;
  int num_spillgap = 0;
  int num_spillcnt = 0;
  int num_spillcntgap = 0;
  int num_1bitfail = 0;
  vector<int> v_event(0);
  vector<int> v_starttime(0),v_stoptime(0);
  vector<int> v_spillnb(0),v_dspillnb(0),v_ddspillnb(0);
  vector<int> v_spillnb_cor(0),v_dspillnb_cor(0);
  vector<int> v_spillcnt(0),v_dspillcnt(0),v_ddspillcnt(0);
  string filename = Form("%s/run_%05d_%03d_recon.root",inputDir.c_str(),runid,acqid);
  TFile *file = new TFile(filename.c_str(),"read");
  if(file->IsZombie()){
    cout << "No such a file." << filename << endl;
    return;
  }

  cout << "Reading ... runid=" << runid << " acqid=" << acqid << endl;

  TTree* tree = (TTree*)file->Get("tree");
  int spill,spill_mode,spillcnt;
  tree->SetBranchAddress("spill",&spill);
  tree->SetBranchAddress("spill_mode",&spill_mode);
  tree->SetBranchAddress("spill_count",&spillcnt);
  int nevt = tree->GetEntries();
  int ievt=0;
  int iievt=0;
  int starttime = ((TH1F*)file->Get("start_time"))->GetMean();
  int stoptime  = ((TH1F*)file->Get("stop_time")) ->GetMean();

  while(ievt<nevt){
    tree->GetEntry(ievt);
    spill = spill%0x8000;
    if(spill_mode!=1){ievt++;continue;}
    if( (spill<0x0002)&&
        (spill+0x8000*loop<last_spill[0]||spill+0x8000*loop<last_spill[1])&&
        ((last_spill[0]>0x7ffd+0x8000*loop)||(last_spill[1]>0x7ffd+0x8000*loop)))
    {loop++;}
    if( (spillcnt<0x00014)&&
        (spillcnt+0x10000*loop2<last_spillcnt[0]||spill+0x10000*loop2<last_spillcnt[1])&&
        ((last_spillcnt[0]>0xfffd+0x10000*loop2)||(last_spill[1]>0xfffd+0x10000*loop2)))
    {loop2++;}

    spill    = spill   + 0x8000*loop;
    spillcnt = spillcnt+0x10000*loop2;

    dspill[0] = spill-last_spill[0];
    dspill[1] = last_spill[0]-last_spill[1];
    ddspill   = dspill[0]-dspill[1];
    dspillcnt[0] = spillcnt-last_spillcnt[0];
    dspillcnt[1] = last_spillcnt[0]-last_spillcnt[1];
    ddspillcnt   = dspillcnt[0]-dspillcnt[1];

    num_spillcnt++;
    if(last_spillcnt[0]!=-1){
      if(dspillcnt[0]>=0&&dspillcnt[0]<30) h_dspillcnt->Fill(dspillcnt[0]);
      else                                 h_dspillcnt->Fill(-1);
      if(dspillcnt[0]!=7) num_spillcntgap++;
    }

    h_spill->Fill(spill);
    if(last_spill[0]==-1){
      h_spill2->Fill(spill);
      num_spill++;
      v_spillnb_cor .push_back(spill);
      v_dspillnb_cor.push_back(1);
      v_starttime   .push_back(starttime);
      v_stoptime    .push_back(stoptime);
    }
    else if(dspill[0]==1){ 
      h_spill2->Fill(spill); 
      num_spill++;
      v_spillnb_cor .push_back(spill);
      v_dspillnb_cor.push_back(1);
      v_starttime   .push_back(starttime);
      v_stoptime    .push_back(stoptime);
    }
    else if(
        (dspill[0]==0x0001+1&&ddspill==0x0001*2)||
        (dspill[0]==0x0002+1&&ddspill==0x0002*2)||
        (dspill[0]==0x0004+1&&ddspill==0x0004*2)||
        (dspill[0]==0x0008+1&&ddspill==0x0008*2)||
        (dspill[0]==0x0010+1&&ddspill==0x0010*2)||
        (dspill[0]==0x0020+1&&ddspill==0x0020*2)||
        (dspill[0]==0x0040+1&&ddspill==0x0040*2)||
        (dspill[0]==0x0080+1&&ddspill==0x0080*2)||
        (dspill[0]==0x0100+1&&ddspill==0x0100*2)||
        (dspill[0]==0x0200+1&&ddspill==0x0200*2)||
        (dspill[0]==0x0400+1&&ddspill==0x0400*2)||
        (dspill[0]==0x0800+1&&ddspill==0x0800*2)||
        (dspill[0]==0x1000+1&&ddspill==0x1000*2)||
        (dspill[0]==0x2000+1&&ddspill==0x2000*2)||
        (dspill[0]==0x4000+1&&ddspill==0x4000*2))
    {
      h_spill2->Fill(spill-1);
      h_spill2->Fill(spill);
      num_1bitfail++;
      num_spill++;
      v_spillnb_cor .push_back(spill-1);
      v_spillnb_cor .push_back(spill);
      v_dspillnb_cor.push_back(1);
      v_dspillnb_cor.push_back(1);
      v_starttime   .push_back(starttime);
      v_stoptime    .push_back(stoptime);
    }
    else if(dspill[0]>1){
      h_spill2->Fill(spill);
      num_spill++;
      num_spillgap++;
      v_spillnb_cor .push_back(spill);
      v_dspillnb_cor.push_back(dspill[0]);
      v_starttime   .push_back(starttime);
      v_stoptime    .push_back(stoptime);
    }
    v_event     .push_back(iievt);
    v_spillcnt  .push_back(spillcnt);
    v_dspillcnt .push_back(dspillcnt[0]);
    v_ddspillcnt.push_back(ddspillcnt);
    v_spillnb   .push_back(spill);
    v_dspillnb  .push_back(dspill[0]);
    v_ddspillnb .push_back(ddspill);

    last_spill[1]=last_spill[0];
    last_spill[0]=spill;
    last_spillcnt[1]=last_spillcnt[0];
    last_spillcnt[0]=spillcnt;

    if(start_spill>spill) start_spill = spill;
    if(stop_spill <spill) stop_spill  = spill;
    ievt++;
    iievt++;
  }
  if(tree) delete tree;
  if(file) delete file;

  string outtxtfilename  = Form("%s/spill_wg_run%05d_acq%03d.txt" ,outputDir.c_str(),runid,acqid);
  string outrootfilename = Form("%s/spill_wg_run%05d_acq%03d.root",outputDir.c_str(),runid,acqid);
  cout << "Output txt file is : " << outtxtfilename << endl;
  TFile *fout = new TFile(outrootfilename.c_str(),"recreate");
  int nevent     = iievt;
  int nevent_cor = num_spill;
  TGraph *g_spillcnt     = new TGraph(nevent);
  TGraph *g_dspillcnt    = new TGraph(nevent);
  TGraph *g_ddspillcnt   = new TGraph(nevent);
  TGraph *g_spillnb      = new TGraph(nevent);
  TGraph *g_dspillnb     = new TGraph(nevent);
  TGraph *g_ddspillnb    = new TGraph(nevent);
  TGraph *g_spillnb_cor  = new TGraph(nevent_cor);
  TGraph *g_dspillnb_cor = new TGraph(nevent_cor);
  g_spillcnt  ->SetName("g_spillcnt"  );g_spillcnt  ->SetTitle("g_spillcnt"  );
  g_dspillcnt ->SetName("g_dspillcnt" );g_dspillcnt ->SetTitle("g_dspillcnt" );
  g_ddspillcnt->SetName("g_ddspillcnt");g_ddspillcnt->SetTitle("g_ddspillcnt");
  g_spillnb   ->SetName("g_spillnb"   );g_spillnb   ->SetTitle("g_spillnb"   );
  g_dspillnb  ->SetName("g_dspillnb"  );g_dspillnb  ->SetTitle("g_dspillnb"  );
  g_ddspillnb ->SetName("g_ddspillnb" );g_ddspillnb ->SetTitle("g_ddspillnb" );
  for(int i=0;i<nevent;i++){
    g_spillcnt  ->SetPoint(i,v_event[i],v_spillcnt  [i]); 
    g_spillnb   ->SetPoint(i,v_event[i],v_spillnb   [i]); 
    if(i!=0){
      g_dspillcnt ->SetPoint(i,v_event[i],v_dspillcnt [i]); 
      g_dspillnb  ->SetPoint(i,v_event[i],v_dspillnb  [i]); 
    }
    else{
      g_dspillcnt ->SetPoint(i,v_event[i],0); 
      g_dspillnb  ->SetPoint(i,v_event[i],0); 
    }
    if(i>1){
      g_ddspillcnt->SetPoint(i,v_event[i],v_ddspillcnt[i]); 
      g_ddspillnb ->SetPoint(i,v_event[i],v_ddspillnb [i]); 
    }
    else{
      g_ddspillcnt->SetPoint(i,v_event[i],0); 
      g_ddspillnb ->SetPoint(i,v_event[i],0); 
    }
  }
  ofstream ofs(outtxtfilename.c_str());
  for(int i=0;i<nevent_cor;i++){
    g_spillnb_cor->SetPoint(i,i,v_spillnb_cor[i]); 
    if(i!=0){
      g_dspillnb->SetPoint(i,i,v_dspillnb_cor[i]); 
    }
    else{
      g_dspillnb->SetPoint(i,i,0); 
    }
    ofs
      << v_spillnb_cor[i] << " "
      << v_starttime  [i] << " "
      << v_stoptime   [i] << endl;
  }
  ofs.close();
  h_spill       ->Write();
  h_spill2      ->Write();
  h_dspillcnt   ->Write();
  g_spillcnt    ->Write();
  g_dspillcnt   ->Write();
  g_ddspillcnt  ->Write();
  g_spillnb     ->Write();
  g_dspillnb    ->Write();
  g_ddspillnb   ->Write();
  g_spillnb_cor ->Write();
  g_dspillnb_cor->Write();
  fout->Write();
  fout->Close();
  int numspills=stop_spill - start_spill+1;
  cout 
    << " start_spill=" << start_spill
    << " stop_spill="  << stop_spill
    << endl
    << " Total number of spills : " << num_spill 
    << " (stop_spill-start_spill+1)="<<numspills
    << endl
    << " Number of spill gaps : " << num_spillgap
    << endl
    << " Number of 1 bit failure : " << num_1bitfail
    << endl
    << " Data taking efficiency = " << (1.-(double)num_spillgap/((double)num_spill))*100. << "%"
    << endl;
  cout 
    << " Total number of spill_count : " << num_spillcnt
    << " Number of spill_count gap (not equal to 7) : " << num_spillcntgap
    << endl;
}
