#include <fstream>

#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgEditXML.h"
#include "wgColor.h"
#include "wgGetTree.h"
#include "wgGetCalibData.h"
#include "wgChannelMap.h"

bool normal_cond;
bool beamtrg;
bool mucenter;
bool muct;
bool GoodSpillv01();
bool GoodSpill();
bool HornOff();
bool Horn320();
bool v01    ;
bool hornoff;
bool horn320;

void get_time(int t, int& yea, int& mon, int& mday){
  time_t aclock = t;
  struct tm *newtime;
  newtime = localtime(&aclock);
  mday = newtime -> tm_mday;
  mon  = newtime -> tm_mon+1;
  yea  = newtime -> tm_year+1900;
}

using namespace std;


void BSD_INFO(
    string inputDirName,string outputDir,
    int t2krun,int mrrun,int smrrun,int ssmrrun,
    string bsd_version);

int main(int argc, char** argv){
  int opt;
  int t2krun = -1;
  int mrrun  = -1;
  int smrrun = -1;
  int ssmrrun = -1;
  string bsd_version = "";
  wgConst *con = new wgConst;
  con->GetENV();
  string inputDirName  = con->BSD_DIRECTORY;
  string outputDirName = con->SPILL_DIRECTORY;
  string logoutputDir  = con->LOG_DIRECTORY;
  delete con;

  OperateString *OpStr = new OperateString;
  Logger *Log = new Logger;
  CheckExist *check = new CheckExist;

  Log->Initialize();
  Log->Write("Getting Beam Summary Data information...");

  while((opt = getopt(argc,argv, "f:t:m:v:o:s:r:h")) !=-1 ){
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
        mrrun=atoi(optarg); 
        break;
      case 'v':
        bsd_version =optarg;
        break;
      case 'o':
        outputDirName = optarg; 
        break;
      case 's':
        smrrun = atoi(optarg);
        break;
      case 'r':
        ssmrrun = atoi(optarg);
        break;
      case 'h':
        cout <<"This program is for extracting beam summary data information. "<<endl;
        cout <<"You can take several options..."<<endl;
        cout <<"  -h               : help"<<endl;
        cout <<"  -f <inputDir>    : Input Directory (default: BSD_DIRECTORY)" << endl;
        cout <<"  -t <t2krun>      : T2K RunID" << endl;
        cout <<"  -m <mrrun>       : MR RunID" << endl;
        cout <<"  -s <smrrun>      : MR Sub-RunID" << endl;
        cout <<"  -r <ssmrrun>     : MR Sub-Sub-RunID" << endl;
        cout <<"  -v <p06/v01>     : BSD vesrion"  << endl;
        cout <<"  -o <outputDir>   : Output Directory (defualt: SPILL_DIRECTORY)" << endl;
        exit(0);
    }   
  }

  if(inputDirName.empty()||outputDirName.empty()||t2krun==-1||mrrun==-1||bsd_version==""){
    cout << "See the usage: " << argv[0] << " -h" << endl;
    exit(0);
  }

  cout << " *****  READ   DIRECTORY     :" << inputDirName << "  *****" << endl;
  cout << " *****  OUTPUT DIRECTORY     :" << outputDirName   << "  *****" << endl;
  cout << " *****  T2K RUN = " << t2krun << ", MR RUN = " << mrrun << endl;

  delete check;
  delete OpStr;

  BSD_INFO(inputDirName,outputDirName,t2krun,mrrun,smrrun,ssmrrun,bsd_version);

  Log->Write("end BSD data check ... " );
  delete Log;  
}

void BSD_INFO(
    string inputDirName,string outputDirName,
    int t2krun,int mrrun,int smrrun,int ssmrrun,
    string bsd_version){

  bool   isRHC     = false;
  int    count_good_spill = 0;
  double count_good_pot   = 0.;
  int    Nspill_nu     = 0;
  int    Nspill_antinu = 0;
  double Npot_nu       = 0.;
  double Npot_antinu   = 0.;

  string bsdfilename;
  if(smrrun==-1){
    bsdfilename = Form("%s/%s/t2krun%d/merge_bsd_run%03d_%s.root",
      inputDirName.c_str(), bsd_version.c_str(), t2krun, mrrun, bsd_version.c_str());
  }
  else{
    bsdfilename = Form("%s/%s/t2krun%d/bsd_run%03d%04d_%02d%s.root",
      inputDirName.c_str(),bsd_version.c_str(),t2krun,mrrun,smrrun,ssmrrun,bsd_version.c_str());
  }


  BSD_t* bsd_t = new BSD_t();
  if(!bsd_t->OpenBsdFile(bsdfilename)) return;
  if(bsd_version!=bsd_t->version) return;

  int nevt = bsd_t->bsd->GetEntries();

  string outtxtfilename  = Form("%s/spill_bsd_t2krun%d_mrrun%03d.txt"    
      ,outputDirName.c_str(),t2krun,mrrun);
  string outtxtfilename1 = Form("%s/spill_bsd_t2krun%d_mrrun%03d_fhc.txt"
      ,outputDirName.c_str(),t2krun,mrrun);
  string outtxtfilename2 = Form("%s/spill_bsd_t2krun%d_mrrun%03d_rhc.txt"
      ,outputDirName.c_str(),t2krun,mrrun);
  ofstream wfile (outtxtfilename .c_str());
  ofstream wfile1(outtxtfilename1.c_str());
  ofstream wfile2(outtxtfilename2.c_str());

  for(int ievt = 0; ievt < nevt; ievt++){  
    if(ievt%100000==0){ cout<<" "<< ievt << flush; }
    bsd_t->bsd->GetEntry(ievt);

    bool ok = false;
    if(bsd_t->version=="v01"){
      if(bsd_t->good_spill_flag==1){
        ok = true; 
        isRHC = false;
      }
      else if(bsd_t->good_spill_flag==-1){
        ok = true; 
        isRHC = true;
      }
      else{ ok = false; }
    }
    else if(bsd_t->version=="p06"){
      double mucenter = sqrt(bsd_t->mumon[2]*bsd_t->mumon[2]+bsd_t->mumon[4]*bsd_t->mumon[4]);
      if     (bsd_t->spill_flag!=1   ) { ok = false; }
      else if(bsd_t->run_type  !=1   ) { ok = false; }
      else if(bsd_t->ct_np[4][0]<1e11) { ok = false; }
      else if(mucenter>10            ) { ok = false; }
      else{
        if(fabs(bsd_t->hct[0][0]-250.)<5.&&
            fabs(bsd_t->hct[1][0]-250.)<5.&&
            fabs(bsd_t->hct[2][0]-250.)<5.)
        {
          ok = true;
          isRHC = false;
        }
        else if(fabs(bsd_t->hct[0][0]+250.)<5.&&
            fabs(bsd_t->hct[1][0]+250.)<5.&&
            fabs(bsd_t->hct[2][0]+250.)<5.)
        {
          ok = true;
          isRHC = true;
        }
        else{
          ok = false;
        }
      }
    }

    if(ok){
      if(isRHC){
        Nspill_antinu++;
        Npot_antinu+=bsd_t->ct_np[4][0];
        wfile2
          << bsd_t->spillnum   << " " 
          << isRHC             << " " 
          << bsd_t->trg_sec[0] << " " 
          << Npot_antinu       << endl;
      }
      else{
        Nspill_nu++;
        Npot_nu+=bsd_t->ct_np[4][0];
        wfile1
          << bsd_t->spillnum   << " " 
          << isRHC             << " " 
          << bsd_t->trg_sec[0] << " " 
          << Npot_nu       << endl;
      }
      count_good_spill ++;
      count_good_pot   += bsd_t->ct_np[4][0];
      wfile 
        << bsd_t->spillnum   << " " 
        << isRHC             << " " 
        << bsd_t->trg_sec[0] << " " 
        //<< count_good_pot    << endl;
        << Form("%6.5e",bsd_t->ct_np[4][0]) << endl;
    }
  }

  wfile .close();
  wfile1.close();
  wfile2.close();

  cout<<endl;
  cout << "=============================================" << endl;
  cout << " Beam Data Summary: T2KRUN" << t2krun << ", MRRUN" << mrrun << endl;
  cout << " Total Spill: "  << count_good_spill 
       << ", Total POT: "   << count_good_pot
       << endl;  
  cout << "  FHC Spill: "  << Nspill_nu 
       << ", POT: "        << Npot_nu
       << endl;  
  cout << "  RHC Spill: "  << Nspill_antinu 
       << ", POT: "        << Npot_antinu
       << endl;  
  cout << "=============================================" << endl;
  cout<<endl;  
}
