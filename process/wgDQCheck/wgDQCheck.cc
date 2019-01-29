#include <string>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <ctime>

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

vector<string> GetIncludeFileName(string& inputXMLName);
void MakeDir(string& str);
void AnaXML(vector<string> &inputXMLName, string& outputDir, string& calibFile);
void AnaRecon(string &inputReconName,string& outputDir, string& s_time);
double cal_mean(vector<double>);

string s_time;
TFile *fout;

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
  wgConst *con = new wgConst;
  con->GetENV();
  string inputName("");
  string xmldir = con->XMLDATA_DIRECTORY;
  string recondir = con->RECON_DIRECTORY;
  string outputDir = con->DQ_DIRECTORY;
  string inputReconName("");
  string inputXMLName1("");
  string inputXMLName2("");
  string calibFile("");
  string logoutputDir=con->LOG_DIRECTORY;

  OperateString *OpStr = new OperateString;
  Logger *Log = new Logger;
  CheckExist *check = new CheckExist;

  Log->Initialize();
  Log->Write("start calibration...");

  while((opt = getopt(argc,argv, "f:c:o:r:x:h:m:")) !=-1 ){
    switch(opt){
      case 'f':
        inputName=optarg;
        Log->Write(Form("target:%s",inputName.c_str()));
        break;

      case 'c':
        calibFile=optarg; 
        break;

      case 'o':
        outputDir = optarg; 
        break;

      case 'r':
        recondir = optarg; 
        break;
      
      case 'x':
        xmldir = optarg; 
        break;
      
      case 'h':
        cout <<"This program is for data quality check. "<<endl;
        cout <<"You can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose run name you wanna read(must)"<<endl;
        cout <<"  -c (char*) : choose calibration file" <<endl;
        cout <<"  -o (char*) : choose output directory (default: dq directory) "<<endl;
        cout <<"  -r (char*) : choose Recon directory  (default: recon directory)" <<endl;
        cout <<"  -x (char*) : choose XML directory  (default: xml directory)" <<endl;
        exit(0);
      default:
        cout <<"This program is for data quality check. "<<endl;
        cout <<"You can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose run name you wanna read(must)"<<endl;
        cout <<"  -c (char*) : choose calibration file" <<endl;
        cout <<"  -o (char*) : choose output directory (default: dq directory) "<<endl;
        cout <<"  -r (char*) : choose Recon directory  (default: recon directory)" <<endl;
        cout <<"  -x (char*) : choose XML directory  (default: xml directory)" <<endl;
        exit(0);
    }   
  }

  inputReconName  =Form("%s/%s_recon.root" ,recondir.c_str(),inputName.c_str());
  inputXMLName1   =Form("%s/%s_dif_1_1_1"  ,xmldir.c_str(),inputName.c_str());         
  inputXMLName2   =Form("%s/%s_dif_1_1_2"  ,xmldir.c_str(),inputName.c_str());        
  if(!check->Dir(inputXMLName1)){ 
    cout<<"!!Error!! "<<inputXMLName1.c_str()<<"doesn't exist!!";
    Log->eWrite(Form("Error!!target:%s doesn't exist",inputXMLName1.c_str()));
    return 1;
  }
  if(!check->Dir(inputXMLName2)){ 
    cout<<"!!Error!! "<<inputXMLName2.c_str()<<"doesn't exist!!";
    Log->eWrite(Form("Error!!target:%s doesn't exist",inputXMLName2.c_str()));
    return 1;
  }
  if(!check->RootFile(inputReconName)){ 
    cout<<"!!Error!! "<<inputReconName.c_str()<<"doesn't exist!!";
    Log->eWrite(Form("Error!!target:%s doesn't exist",inputReconName.c_str()));
    return 1;
  }

  if(inputName==""){
    cout << "!!ERROR!! please input run name." <<endl;
    cout << "if you don't know how to input, please see help."<<endl;
    cout << "help : ./wgDQCheck -h" <<endl;
    exit(1);
  }

  if(calibFile==""){
    cout << "!!ERROR!! please input calib_result file." <<endl;
    exit(1);
  }


  Log->Write(Form("READING RUN NAME : %s",inputName.c_str()));
  Log->Write(Form("OUTPUT DIRECTORY : %s",outputDir.c_str()));

  cout << " *****  READING RUN NAME     :" << inputName << "  *****" << endl;
  cout << " *****  OUTPUT DIRECTORY     :" << outputDir << "  *****" << endl;

  delete check;
  delete OpStr;

  vector<string> ReadFile;
  ReadFile.push_back(inputXMLName1);
  ReadFile.push_back(inputXMLName2);

  AnaXML(ReadFile,outputDir,calibFile);
  AnaRecon(inputReconName, outputDir, s_time);
  Log->Write("end summarizeing ... " );
  delete Log;  
}


//******************************************************************
void AnaXML(vector<string> &inputFileName, string& outputDir , string& calibFile){

  int FN=inputFileName.size();
  if(FN!=2){
    cout << "!! ERROR !! : the number of data is not enough !!" << endl;
    return;
  }

  int start_time,stop_time;
  double Gain[2][20][32];
  double Noise[2][20][32];
  double Pedestal[2][20][32][16];
  double e_Pedestal[2][20][32][16];

  double calib_gain[2][20][36];
  double calib_pedestal[2][20][36][16];
  double calib_ped_nohit[2][20][36][16];

  double diff_Gain[2][20][32];
  double diff_Pedestal[2][20][32][16];

  Raw_t rd[2]; 
  wgChannelMap *Map = new wgChannelMap();
  for(int idif=0;idif<2;idif++){
    for(int ichip=0;ichip<(int)NCHIPS;ichip++){
      Map->GetMap(idif,ichip,&rd[idif].view,rd[idif].pln[ichip],rd[idif].ch[ichip],rd[idif].grid[ichip],rd[idif].x[ichip],rd[idif].y[ichip],rd[idif].z[ichip]);
    }
  }
  delete Map;

  // ================ define histgram ================== //
  TH1F *h_Gain = new TH1F("gain","gain",80,20,60);
  h_Gain->SetFillColor(kRed);
  h_Gain->SetTitle("Gain;gain (~40);nEntry");
  TH1F *h_diffGain = new TH1F("diff_gain","diff_gain",80,-20,20);
  h_diffGain->SetFillColor(kViolet);
  h_diffGain->SetTitle("diff Gain;diff gain(-4~4);nEntry");
  TH1F *h_diffPedestal = new TH1F("diff_pedestal","diff_pedestal",80,-20,20);
  h_diffPedestal->SetFillColor(kBlue);
  h_diffPedestal->SetTitle("diff Pedestal ;diff Pedestal;nEntry");
  TH1F *h_Pedestal_w = new TH1F("pedestal_w","pedestal_w",100,1,10);
  h_Pedestal_w->SetFillColor(kBlue);
  h_Pedestal_w->SetTitle("Pedestal width;Pedestal;nEntry");
  TH1F *h_Noise = new TH1F("noise","noise",100,0,200);
  h_Noise->SetFillColor(kGreen);
  h_Noise->SetTitle("Noise;Noise Rate[Hz];nEntry");
  TH2F * gain_map = new TH2F("gain_map", "gain_map", 17,0,17,80,0,80);  
  gain_map -> SetStats(0);
  gain_map -> SetMinimum(20.);
  gain_map -> SetMaximum(60.);
  TH2F * noise_map = new TH2F("noise_map", "noise_map", 17,0,17,80,0,80);  
  noise_map -> SetStats(0);
  noise_map -> SetMinimum(0.);
  noise_map -> SetMaximum(200.);
  TH2F * ped_map = new TH2F("pedestal_map", "pedestal_map", 17,0,17,80,0,80);  
  ped_map -> SetStats(0);
  ped_map -> SetMinimum(350.);
  ped_map -> SetMaximum(650.);
  TH2F * pedw_map = new TH2F("pedestal_width_map", "pedestal_width_map", 17,0,17,80,0,80);  
  pedw_map -> SetStats(0);
  pedw_map -> SetMinimum(1.);
  pedw_map -> SetMaximum(10.);

  wgGetCalibData *getcalib = new wgGetCalibData();
  for(int idif=0;idif<=1;idif++){
    getcalib->Get_Pedestal(idif+1,calib_pedestal[idif],calib_ped_nohit[idif]);
    getcalib->Get_Gain(calibFile,idif+1,calib_gain[idif]);
  }
  delete getcalib;

  string xmlfile("");
  string name("");
  wgEditXML *Edit = new wgEditXML();

  // ================== Read XML data ==================== //
  for(int iFN=0;iFN<FN;iFN++){
    for(int ichip=0;ichip<(int)NCHIPS;ichip++){
      xmlfile=Form("%s/Summary_chip%d.xml",inputFileName[iFN].c_str(),ichip);
      Edit->Open(xmlfile);
      if(ichip==0 && iFN==0){
        name="start_time";
        start_time = Edit->SUMMARY_GetGlobalConfigValue(name);
        name="stop_time";
        stop_time = Edit->SUMMARY_GetGlobalConfigValue(name);
      }
      for(int ich=0;ich<32;ich++){
        name="Gain";
        Gain[iFN][ichip][ich]=Edit->SUMMARY_GetChFitValue(name,ich)/3.*1.05;
        diff_Gain[iFN][ichip][ich]= Gain[iFN][ichip][ich]-calib_gain[iFN][ichip][ich];
        name="Noise";
        Noise[iFN][ichip][ich]=Edit->SUMMARY_GetChFitValue(name,ich);
        for(int icol=0;icol<(int)MEMDEPTH;icol++){
        //for(int icol=0;icol<1;icol++){
          name=Form("ped_%d",icol);
          Pedestal[iFN][ichip][ich][icol]=Edit->SUMMARY_GetChFitValue(name,ich);
          diff_Pedestal[iFN][ichip][ich][icol]= Pedestal[iFN][ichip][ich][icol]-calib_ped_nohit[iFN][ichip][ich][icol];
          name=Form("eped_%d",icol);
          e_Pedestal[iFN][ichip][ich][icol]=Edit->SUMMARY_GetChFitValue(name,ich);
        }
      }
      Edit->Close();
    }
  }
  delete Edit;

  // =========== Unix time ============= //
  if(start_time<10){
    cout << "!!ERROR!! start time is wrong!" << endl;
    return;
  }
  time_t time = start_time;
  tm *tm = localtime(&time);
  cout << "Time : "
    << tm->tm_year + 1900 // year 0 corresponds to 1900
    << "/" << tm->tm_mon+1 // month starts from 0
    << "/" << tm->tm_mday  // mday means day in a month
    << " " << tm->tm_hour 
    << ":" << tm->tm_min
    << ":" << tm->tm_sec <<endl;
  s_time = Form("%04d-%02d-%02d-%02d-%02d",
      tm->tm_year+1900,
      tm->tm_mon+1,
      tm->tm_mday,
      tm->tm_hour,
      tm->tm_min);
  string str = Form("%s/%s",outputDir.c_str(),s_time.c_str());
  MakeDir(str);

  // =========== Define TREE =========== //
  string filename=Form("%s/%s/dq.root",outputDir.c_str(),s_time.c_str());
  fout = new TFile(filename.c_str(),"recreate");
  TTree * tree = new TTree("tree","tree");
  double t_gain,t_noise,t_pedestal[16],t_pedestal_w[16],tc_gain,tc_pedestal[16],tc_ped_nohit[16];
  int t_view,t_pln,t_ch,t_dif,t_chip,t_chipch,t_col[16];
  for(int i=0;i<16;i++){t_col[i]=i;}
  tree->Branch("start_time" ,&start_time    ,"start_time/I");
  tree->Branch("stop_time"  ,&stop_time     ,"stop_time/I");
  tree->Branch("gain"       ,&t_gain        ,"gain/D");
  tree->Branch("noise"      ,&t_noise       ,"noise/D");
  tree->Branch("pedestal"   ,t_pedestal     ,"pedestal[16]/D");
  tree->Branch("pedestal_w" ,t_pedestal_w   ,"pedestal width[16]/D");
  tree->Branch("calib_gain" ,&tc_gain       ,"calib_gain/D");
  tree->Branch("calib_pedestal" ,tc_pedestal       ,"calib_pedestal[16]/D");
  tree->Branch("calib_ped_nohit" ,tc_ped_nohit     ,"calib_ped_nohit[16]/D");
  tree->Branch("view"       ,&t_view        ,"view/I");
  tree->Branch("pln"        ,&t_pln         ,"pln/I");
  tree->Branch("ch"         ,&t_ch          ,"ch/I");
  tree->Branch("dif"        ,&t_dif         ,"dif/I");
  tree->Branch("chip"       ,&t_chip        ,"chip/I");
  tree->Branch("chipch"     ,&t_chipch      ,"chipch/I");
  tree->Branch("col"        , t_col         ,"col[16]/I");


  // ================== Fill data ==================== //
  for(int iFN=0;iFN<FN;iFN++){
    for(int ichip=0;ichip<(int)NCHIPS;ichip++){
      for(int ich=0;ich<32;ich++){
        h_Gain     ->Fill(Gain[iFN][ichip][ich]);
        h_diffGain->Fill(diff_Gain[iFN][ichip][ich]);
        h_Noise    ->Fill(Noise[iFN][ichip][ich]);
        t_gain    = Gain[iFN][ichip][ich];
        t_noise   = Noise[iFN][ichip][ich];
        tc_gain   = calib_gain[iFN][ichip][ich];
        t_view    = iFN;
        t_pln     = rd[iFN].pln[ichip][ich];    
        t_ch      = rd[iFN].ch[ichip][ich];    
        t_dif     = iFN+1;   
        t_chip    = ichip;   
        t_chipch  = ich;   

        for(int icol=0;icol<(int)MEMDEPTH;icol++){
          if(icol==0) h_diffPedestal->Fill(diff_Pedestal[iFN][ichip][ich][icol]);
          t_pedestal [icol] = Pedestal[iFN][ichip][ich][icol];
          t_pedestal_w [icol] = e_Pedestal[iFN][ichip][ich][icol];
          if(icol==0) h_Pedestal_w->Fill(t_pedestal_w[icol]);
          tc_ped_nohit[icol] = calib_ped_nohit[iFN][ichip][ich][icol];
          tc_pedestal[icol] = calib_pedestal [iFN][ichip][ich][icol];
        }
        int fill_ch = t_view*640 + t_pln*80 + t_ch;
        gain_map->Fill(fill_ch/640 + fill_ch/80,fill_ch%80, t_gain);
        noise_map->Fill(fill_ch/640 + fill_ch/80,fill_ch%80, t_noise);
        pedw_map->Fill(fill_ch/640 + fill_ch/80,fill_ch%80, t_pedestal_w[0]);
        ped_map->Fill(fill_ch/640 + fill_ch/80,fill_ch%80, t_pedestal[0]);

        tree->Fill();
      }
    }
  }
  delete Edit;

  TBox *box = new TBox();
  box->SetFillColor(kRed);
  box->SetLineColor(kRed);
  box->SetFillStyle(3004);
  box->IsTransparent();
  // ================== Print Histgram ==================== //
  TCanvas *c1 = new TCanvas("c1","c1",1200,800);
  c1->Divide(2,2);
  c1->GetPad(1)->SetLogy();
  c1->GetPad(2)->SetLogy();
  c1->GetPad(3)->SetLogy();
  c1->GetPad(4)->SetLogy();
  c1->cd(1);
  h_Gain->Draw();
  box->DrawBox(36,0,44,h_Gain->GetMaximum()*1.05);
  c1->cd(2);
  h_diffGain->Draw();
  box->DrawBox(-4,0,4,h_diffGain->GetMaximum()*1.05);
  c1->cd(3);
  h_diffPedestal->Draw();
  c1->cd(4);
  h_Noise->Draw();
  c1->Print(Form("%s/%s/dq.png",outputDir.c_str(),s_time.c_str()));
  delete c1;
  
  c1 = new TCanvas("c1","c1",1200,800);
  gain_map->Draw("colz");
  c1->Print(Form("%s/%s/gain_map.png",outputDir.c_str(),s_time.c_str()));
  tree->Write();
  gain_map->Write();
  noise_map->Write();
  pedw_map->Write();
  ped_map->Write();

  delete box;

  return;
}

//*****************************************************************
void AnaRecon(string &filename,string& outputDir, string& s_time){
  wgGetTree * gettree = new wgGetTree(filename);
  int spill;
  int spill_mode;
  Hit_t    type_hit;
  Track_t  type_track;

  gettree->SetTreeFile(type_hit);
  gettree->SetTreeFile(type_track);
  gettree->tree->SetBranchAddress("spill",&spill);
  gettree->tree->SetBranchAddress("spill_mode",&spill_mode);

  TCanvas *c1 = new TCanvas("c1","c1",1200,1000);
  c1->Divide(2,2);

  //============ Define histgram ==============//
  
  //Basic Hit Histgram 
  TH1F * spillnb   = new TH1F("spillnb","spillnb",0x10000,0,0x10000);
  TH2F * pe_map    = new TH2F("pe_map", "pe_map", 17,0,17,80,0,80);  
  TH2F * pe_permm_map  = new TH2F("pe_permm_map", "pe_permm_map", 17,0,17,80,0,80);  
  TH2F * nhit_map    = new TH2F("nhit_map", "nhit_map", 17,0,17,80,0,80);  
  TH1F * hittiming = new TH1F("hit_timing","hit_timing",60000,0,60000);
  TH1F * hitbcid   = new TH1F("hit_bcid","hit_bcid",20,20,40);
  TH1F * pe_onbeam  ;
  TH1F * pe_offbeam ;
  TH2F * pe_chip_onbeam ;
  TH2F * pe_chip_offbeam;
  TH1F * pe_permm_onbeam  ;
  TH1F * pe_permm_offbeam ;
  TH2F * pe_permm_chip_onbeam ;
  TH2F * pe_permm_chip_offbeam;

  TH1F * bcid_onbeam     ;
  TH1F * bcid_offbeam    ;
  TH2F * bcid_chip_onbeam ;
  TH2F * bcid_chip_offbeam;

  TH1F * hittime_onbeam  ;
  TH1F * hittime_offbeam ;
  TH2F * hittime_chip_onbeam ;
  TH2F * hittime_chip_offbeam;
  TH2F * diff_hittime_chip_onbeam ;
  TH2F * diff_hittime_chip_offbeam;
  TH2F * diff_hittime_diff_chip_onbeam ;
  TH2F * diff_hittime_diff_chip_offbeam;
  TH2F * diff_hittime_diff_chip_onbeam2 ;
  TH2F * diff_hittime_diff_chip_offbeam2;
  
  pe_onbeam       = new TH1F(Form("pe_onbeam"),"a"      ,100,0,50);
  pe_offbeam      = new TH1F(Form("pe_offbeam"),"a"     ,100,0,50);
  pe_permm_onbeam       = new TH1F(Form("pe_permm_onbeam"),"a"      ,100,0,50);
  pe_permm_offbeam      = new TH1F(Form("pe_permm_offbeam"),"a"     ,100,0,50);
  hittime_onbeam  = new TH1F(Form ("hittime_onbeam"),"a", time_onbeam_bin+2000, time_onbeam_start-1000, time_onbeam_end+1000);
  hittime_offbeam = new TH1F(Form ("hittime_offbeam"),"a",time_offbeam_bin+2000,time_offbeam_start-1000,time_offbeam_end+1000);
  bcid_onbeam     = new TH1F( Form("bcid_onbeam"),"a"   , bcid_onbeam_bin+10, bcid_onbeam_start-5, bcid_onbeam_end+5);
  bcid_offbeam    = new TH1F( Form("bcid_offbeam"),"a"  ,bcid_offbeam_bin+10,bcid_offbeam_start-5,bcid_offbeam_end+5);
  pe_permm_onbeam       -> SetTitle(Form("pe per3mm onbeampe;nEntry"));
  pe_permm_offbeam      -> SetTitle(Form("pe per3mm offbeam;pe;nEntry"));
  pe_onbeam       -> SetTitle(Form("pe onbeampe;nEntry"));
  pe_offbeam      -> SetTitle(Form("pe offbeam;pe;nEntry"));
  hittime_onbeam  -> SetTitle(Form("hittime onbeam;time;nEntry"));
  hittime_offbeam -> SetTitle(Form("hittime offbeam;time;nEntry"));
  bcid_onbeam     -> SetTitle(Form("bcid onbeam;bcid;nEntry"));
  bcid_offbeam    -> SetTitle(Form("bcid offbeam;bcid;nEntry"));

  pe_chip_onbeam         = new TH2F(Form("pe_onbeam_chip"),"a",41,0,41,100,0,50);
  pe_chip_offbeam        = new TH2F(Form("pe_offbeam_chip"),"a",41,0,41,100,0,50);
  pe_permm_chip_onbeam         = new TH2F(Form("pe_permm_onbeam_chip"),"a",41,0,41,100,0,50);
  pe_permm_chip_offbeam        = new TH2F(Form("pe_permm_offbeam_chip"),"a",41,0,41,100,0,50);
  hittime_chip_onbeam    = new TH2F(Form("hittime_onbeam_chip"),"a" ,41,0,41, time_onbeam_bin+2000, time_onbeam_start-1000, time_onbeam_end+1000);
  hittime_chip_offbeam   = new TH2F(Form("hittime_offbeam_chip"),"a",41,0,41,time_offbeam_bin+2000,time_offbeam_start-1000,time_offbeam_end+1000);
  bcid_chip_onbeam       = new TH2F(Form("bcid_onbeam_chip"),"a"    ,41,0,41, bcid_onbeam_bin+10, bcid_onbeam_start-5, bcid_onbeam_end+5);
  bcid_chip_offbeam      = new TH2F(Form("bcid_offbeam_chip"),"a"   ,41,0,41,bcid_offbeam_bin+10,bcid_offbeam_start-5,bcid_offbeam_end+5);
  diff_hittime_chip_onbeam    = new TH2F(Form("diff_hittime_onbeam_chip"),"a" ,41,0,41, 580*2, -580, 580);
  diff_hittime_chip_offbeam   = new TH2F(Form("diff_hittime_offbeam_chip"),"a",41,0,41, 580*2, -580, 580);
  diff_hittime_diff_chip_onbeam    = new TH2F(Form("diff_hittime_onbeam_diff_chip"),"a" ,41,0,41, 580*2, -580, 580);
  diff_hittime_diff_chip_offbeam   = new TH2F(Form("diff_hittime_offbeam_diff_chip"),"a",41,0,41, 580*2, -580, 580);
  diff_hittime_diff_chip_onbeam2   = new TH2F(Form("diff_hittime_onbeam_diff_chip2"),"a" ,41,0,41, 580*2, -580, 580);
  diff_hittime_diff_chip_offbeam2  = new TH2F(Form("diff_hittime_offbeam_diff_chip2"),"a",41,0,41, 580*2, -580, 580);
  pe_chip_onbeam         ->SetTitle(Form("pe onbeam chip;chip:pe"));
  pe_chip_offbeam        ->SetTitle(Form("pe offbeam chip;chip;pe"));
  pe_permm_chip_onbeam         ->SetTitle(Form("pe permm onbeam chip;chip:pe"));
  pe_permm_chip_offbeam        ->SetTitle(Form("pe permm offbeam chip;chip;pe"));
  hittime_chip_onbeam    ->SetTitle(Form("hittime onbeam chip;chip;time"));
  hittime_chip_offbeam   ->SetTitle(Form("hittime offbeam chip;chip;time"));
  bcid_chip_onbeam       ->SetTitle(Form("bcid onbeam chip;chip;bcid"));
  bcid_chip_offbeam      ->SetTitle(Form("bcid offbeam chip;chip"));
  diff_hittime_chip_onbeam    ->SetTitle(Form("diff hittime onbeam chip;chip;time"));
  diff_hittime_chip_offbeam   ->SetTitle(Form("diff hittime offbeam chip;chip;time"));
  diff_hittime_diff_chip_onbeam    ->SetTitle(Form("diff hittime onbeam diff chip;chip;time"));
  diff_hittime_diff_chip_offbeam   ->SetTitle(Form("diff hittime offbeam diff chip;chip;time"));
  diff_hittime_diff_chip_onbeam2    ->SetTitle(Form("diff hittime onbeam diff chip;chip;time"));
  diff_hittime_diff_chip_offbeam2   ->SetTitle(Form("diff hittime offbeam diff chip;chip;time"));

  // Classify view and grid plot
  TH1F * pe[2][2];
  TH1F * pe_permm[2][2];
  TH1F * pathlength[2][2];
  TH1F * cos_zen   [2][2];
  TH1F * cos_azi   [2][2];
  TH2F * cos_p_zen   [2][2];
  TH2F * cos_p_azi   [2][2];
  TH2F * pe_permm_p  [2][2];
  TH2F * pe_permm_cz  [2][2];
  TH2F * pe_permm_ca  [2][2];

  for(int i=0;i<2;i++){
    for(int j=0;j<2;j++){
      pe[i][j] = new TH1F(Form("pe_view%d_grid%d",i,j),"a",100,0,50);
      pe[i][j] -> SetTitle(Form("pe view=%d grid=%d;pe;nEntry",i,j));
      pe_permm[i][j] = new TH1F(Form("pe_permm_view%d_grid%d",i,j),"a",100,0,50);
      pe_permm[i][j] -> SetTitle(Form("pe per3mm view=%d grid=%d;pe;nEntry",i,j));
      pathlength[i][j] = new TH1F(Form("path_view%d_grid%d",i,j),"a",100,0,50);
      pathlength[i][j] -> SetTitle(Form("pathlength view=%d grid=%d;pathlength;nEntry",i,j));
      cos_zen[i][j] = new TH1F(Form("cos_z_view%d_grid%d",i,j),"a",100,-1,1);
      cos_zen[i][j] -> SetTitle(Form("cos zenith view=%d grid=%d;cos;nEntry",i,j));
      cos_azi[i][j] = new TH1F(Form("cos_azi_view%d_grid%d",i,j),"a",100,-1,1);
      cos_azi[i][j] -> SetTitle(Form("cos azimuth view=%d grid=%d;cos;nEntry",i,j));
      cos_p_zen[i][j] = new TH2F(Form("cos_path_z_view%d_grid%d",i,j),"a",100,-1,1,100,0,50);
      cos_p_zen[i][j] -> SetTitle(Form("path vs cos zenith view=%d grid=%d;cos;pathlength",i,j));
      cos_p_azi[i][j] = new TH2F(Form("cos_path_azi_view%d_grid%d",i,j),"a",100,-1,1,100,0,50);
      cos_p_azi[i][j] -> SetTitle(Form("path vs cos azimuth view=%d grid=%d;cos;pathlength",i,j));
      pe_permm_p[i][j] = new TH2F(Form("pe_permm_path_view%d_grid%d",i,j),"a",100,0,50,100,0,50);
      pe_permm_p[i][j] -> SetTitle(Form("path vs pe per3mm view=%d grid=%d;pathlength;pe per3mm",i,j));
      pe_permm_cz[i][j] = new TH2F(Form("pe_permm_zen_%d_%d",i,j),"a",100,-1,1,100,0,50);
      pe_permm_cz[i][j] -> SetTitle(Form("cos zenith vs pe per3mm view=%d grid=%d;cos;pe per3mm",i,j));
      pe_permm_ca[i][j] = new TH2F(Form("pe_permm_azi_%d_%d",i,j),"a",100,-1,1,100,0,50);
      pe_permm_ca[i][j] -> SetTitle(Form("cos azimuth vs pe per3mm view=%d grid=%d;cos;pe per3mm",i,j));
    }
  }

  // Sand muon event plot 
  
  



  //================================//
  int neve = wgGetTree::tree->GetEntries();
  vector<double> pe_eachch[1280];
  vector<double> pe_mm_eachch[1280];
  int nhit[1280];
  double mean_time_onbeam[40];
  double mean_time_offbeam[40];
  double sum_time_onbeam[40];
  int    ent_time_onbeam[40];
  double sum_time_offbeam[40];
  int    ent_time_offbeam[40];
  for(int ieve=0; ieve < neve; ieve++){
    gettree->GetEntry(ieve);
    if(spill_mode==1)spillnb-> Fill(spill&0xffff);
    //================
    for(int i=0;i<40;i++){
      sum_time_onbeam [i]=0;
      ent_time_onbeam [i]=0;
      sum_time_offbeam[i]=0;
      ent_time_offbeam[i]=0;
    }

    //================
    for(int ihit=0;ihit<type_hit.num_hits;ihit++){
      if(type_hit.hit_time[ihit] < time_offbeam_start && type_hit.hit_time[ihit] > time_offbeam_end) continue;
      sum_time_offbeam[type_hit.hit_chip[ihit]+ type_hit.hit_view[ihit]*20]+= type_hit.hit_time[ihit];
      ent_time_offbeam[type_hit.hit_chip[ihit]+ type_hit.hit_view[ihit]*20]++;
      if( spill_mode != 1 && (type_hit.hit_time[ihit] < time_onbeam_start || type_hit.hit_time[ihit] > time_onbeam_end)) continue;
      sum_time_onbeam[type_hit.hit_chip[ihit]+ type_hit.hit_view[ihit]*20]+= type_hit.hit_time[ihit];
      ent_time_onbeam[type_hit.hit_chip[ihit]+ type_hit.hit_view[ihit]*20]++;
    }//ihit

    for(int i=0;i<40;i++){
      if(ent_time_offbeam[i]==0)continue;
      mean_time_offbeam[i] = sum_time_offbeam[i]/ent_time_offbeam[i];
      if(ent_time_onbeam[i]==0)continue;
      mean_time_onbeam[i] = sum_time_onbeam[i]/ent_time_onbeam[i];
    }

    //================
    for(int ihit=0;ihit<type_hit.num_hits;ihit++){
      if(type_hit.hit_time[ihit] < time_offbeam_start && type_hit.hit_time[ihit] > time_offbeam_end) continue;
      pe_offbeam         -> Fill(type_hit.hit_pe  [ihit]);  
      pe_permm_offbeam         -> Fill(type_hit.hit_pe_permm  [ihit]);  
      hittime_offbeam    -> Fill(type_hit.hit_time[ihit]); 
      bcid_offbeam       -> Fill(type_hit.hit_bcid[ihit]); 
      pe_chip_offbeam     -> Fill(type_hit.hit_chip[ihit] + type_hit.hit_view[ihit]*21, type_hit.hit_pe  [ihit]);
      pe_permm_chip_offbeam     -> Fill(type_hit.hit_chip[ihit] + type_hit.hit_view[ihit]*21, type_hit.hit_pe_permm  [ihit]);
      hittime_chip_offbeam-> Fill(type_hit.hit_chip[ihit] + type_hit.hit_view[ihit]*21, type_hit.hit_time[ihit]);
      bcid_chip_offbeam   -> Fill(type_hit.hit_chip[ihit] + type_hit.hit_view[ihit]*21, type_hit.hit_bcid[ihit]);
      if(ent_time_offbeam[type_hit.hit_chip[ihit]+type_hit.hit_view[ihit]*20]>=2){
        diff_hittime_chip_offbeam-> Fill(type_hit.hit_chip[ihit] + type_hit.hit_view[ihit]*21, type_hit.hit_time[ihit]-mean_time_offbeam[type_hit.hit_chip[ihit]+type_hit.hit_view[ihit]*20]);
      }
      
      if(type_hit.hit_view[ihit]==SideView){
        if(ent_time_offbeam[type_hit.hit_chip[ihit]%5+type_hit.hit_view[ihit]*20]>=2){
          diff_hittime_diff_chip_offbeam-> Fill(type_hit.hit_chip[ihit] + type_hit.hit_view[ihit]*21, type_hit.hit_time[ihit]-mean_time_offbeam[type_hit.hit_chip[ihit]%5+type_hit.hit_view[ihit]*20]);
        }
        if(ent_time_offbeam[(int)(type_hit.hit_chip[ihit]/5)*5+4+type_hit.hit_view[ihit]*20]>=2){
          diff_hittime_diff_chip_offbeam2-> Fill(type_hit.hit_chip[ihit] + type_hit.hit_view[ihit]*21, type_hit.hit_time[ihit]-mean_time_offbeam[(int)(type_hit.hit_chip[ihit]/5)*5+4+type_hit.hit_view[ihit]*20]);
        }
      }else{
        if(ent_time_offbeam[15+type_hit.hit_chip[ihit]%5+type_hit.hit_view[ihit]*20]>=2){
          diff_hittime_diff_chip_offbeam-> Fill(type_hit.hit_chip[ihit] + type_hit.hit_view[ihit]*21, type_hit.hit_time[ihit]-mean_time_offbeam[15+type_hit.hit_chip[ihit]%5+type_hit.hit_view[ihit]*20]);
        }
        if(ent_time_offbeam[(int)(type_hit.hit_chip[ihit]/5)*5+type_hit.hit_view[ihit]*20]>=2){
          diff_hittime_diff_chip_offbeam2-> Fill(type_hit.hit_chip[ihit] + type_hit.hit_view[ihit]*21, type_hit.hit_time[ihit]-mean_time_offbeam[(int)(type_hit.hit_chip[ihit]/5)*5+type_hit.hit_view[ihit]*20]);
        }
      }

      if( spill_mode != 1 && (type_hit.hit_time[ihit] < time_onbeam_start || type_hit.hit_time[ihit] > time_onbeam_end)) continue;
      pe_onbeam       ->  Fill(type_hit.hit_pe  [ihit]);
      pe_permm_onbeam       ->  Fill(type_hit.hit_pe_permm  [ihit]);
      hittime_onbeam  ->  Fill(type_hit.hit_time[ihit]);
      bcid_onbeam     ->  Fill(type_hit.hit_bcid[ihit]);
      pe_chip_onbeam      ->  Fill(type_hit.hit_chip[ihit]+ type_hit.hit_view[ihit]*21, type_hit.hit_pe  [ihit]);
      pe_permm_chip_onbeam      ->  Fill(type_hit.hit_chip[ihit]+ type_hit.hit_view[ihit]*21, type_hit.hit_pe_permm  [ihit]);
      hittime_chip_onbeam ->  Fill(type_hit.hit_chip[ihit]+ type_hit.hit_view[ihit]*21, type_hit.hit_time[ihit]);
      bcid_chip_onbeam    ->  Fill(type_hit.hit_chip[ihit]+ type_hit.hit_view[ihit]*21, type_hit.hit_bcid[ihit]);
      if(ent_time_onbeam[type_hit.hit_chip[ihit]+type_hit.hit_view[ihit]*20]>=2){
        diff_hittime_chip_onbeam-> Fill(type_hit.hit_chip[ihit] + type_hit.hit_view[ihit]*21, type_hit.hit_time[ihit]-mean_time_onbeam[type_hit.hit_chip[ihit]+type_hit.hit_view[ihit]*20]);
      }
      if(type_hit.hit_view[ihit]==SideView){
        if(ent_time_onbeam[type_hit.hit_chip[ihit]%5+type_hit.hit_view[ihit]*20]>=2){
          diff_hittime_diff_chip_onbeam-> Fill(type_hit.hit_chip[ihit] + type_hit.hit_view[ihit]*21, type_hit.hit_time[ihit]-mean_time_onbeam[type_hit.hit_chip[ihit]%5+type_hit.hit_view[ihit]*20]);
        }
        if(ent_time_onbeam[(int)(type_hit.hit_chip[ihit]/5)*5+4+type_hit.hit_view[ihit]*20]>=2){
          diff_hittime_diff_chip_onbeam2-> Fill(type_hit.hit_chip[ihit] + type_hit.hit_view[ihit]*21, type_hit.hit_time[ihit]-mean_time_onbeam[(int)(type_hit.hit_chip[ihit]/5)*5+4+type_hit.hit_view[ihit]*20]);
        }
      }else{
        if(ent_time_onbeam[15+type_hit.hit_chip[ihit]%5+type_hit.hit_view[ihit]*20]>=2){
          diff_hittime_diff_chip_onbeam-> Fill(type_hit.hit_chip[ihit] + type_hit.hit_view[ihit]*21, type_hit.hit_time[ihit]-mean_time_onbeam[15+type_hit.hit_chip[ihit]%5+type_hit.hit_view[ihit]*20]);
        }
        if(ent_time_onbeam[(int)(type_hit.hit_chip[ihit]/5)*5+type_hit.hit_view[ihit]*20]>=2){
          diff_hittime_diff_chip_onbeam2-> Fill(type_hit.hit_chip[ihit] + type_hit.hit_view[ihit]*21, type_hit.hit_time[ihit]-mean_time_onbeam[(int)(type_hit.hit_chip[ihit]/5)*5+type_hit.hit_view[ihit]*20]);
        }
      }

    }//ihit
    //================

    if(type_track.num_trackid==0||type_track.num_trackid>3) continue;
    for(int i=0;i<type_track.num_track;i++){
      int view = type_track.track_view[i];
      for(int j=0;j<type_track.num_track_hits[i];j++){
        int grid=0;
        int hitid = type_track.track_hits_hitid[i][j];
        if(type_hit.hit_grid[hitid]) grid=1;
        pe           [view][grid] -> Fill( type_hit.hit_pe[hitid]         );
        pe_permm     [view][grid] -> Fill( type_hit.hit_pe_permm[hitid]*3 );
        pathlength   [view][grid] -> Fill( type_hit.hit_pathlength[hitid] );
        cos_zen      [view][grid] -> Fill( type_track.track_cos_zen[i]);
        cos_azi      [view][grid] -> Fill( type_track.track_cos_azi[i]);
        cos_p_zen    [view][grid] -> Fill( type_track.track_cos_zen[i],type_hit.hit_pathlength[hitid]);
        cos_p_azi    [view][grid] -> Fill( type_track.track_cos_azi[i],type_hit.hit_pathlength[hitid]);
        pe_permm_p   [view][grid] -> Fill( type_hit.hit_pathlength[hitid] , type_hit.hit_pe_permm[hitid]*3 );
        pe_permm_cz  [view][grid] -> Fill( type_track.track_cos_zen[i] , type_hit.hit_pe_permm[hitid]*3 );
        pe_permm_ca  [view][grid] -> Fill( type_track.track_cos_azi[i] , type_hit.hit_pe_permm[hitid]*3 );
        if(spill_mode==1){
          hittiming->Fill(type_hit.hit_time[hitid]);
          hitbcid  ->Fill(type_hit.hit_bcid[hitid]);
        }
        pe_eachch[view*640+type_hit.hit_pln[hitid]*80+type_hit.hit_ch[hitid]].push_back(type_hit.hit_pe[hitid]);
        pe_mm_eachch[view*640+type_hit.hit_pln[hitid]*80+type_hit.hit_ch[hitid]].push_back(type_hit.hit_pe_permm[hitid]);
        nhit[view*640+type_hit.hit_pln[hitid]*80+type_hit.hit_ch[hitid]]++;


      }//j
    }//i
  }
  for(int ich=0;ich<1280;ich++){
    int numhit = pe_eachch[ich].size();
    double ave_pe = 0.;
    double ave_pe_mm = 0.;
    for(int j=0;j<numhit;j++){ 
      ave_pe    += pe_eachch[ich][j]/numhit; 
      ave_pe_mm += pe_mm_eachch[ich][j]/numhit; 
    }
    pe_map->Fill(ich/80+ich/640,ich%80,ave_pe);
    pe_permm_map->Fill(ich/80+ich/640,ich%80,ave_pe_mm);
    nhit_map->Fill(ich/80+ich/640,ich%80,nhit[ich]);
  }//ich
  

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

  c1->Clear();
  c1->cd(0);
  hittiming->Write();
  hittiming->Draw();
  c1->Print(Form("%s/%s/hit_timing.png",outputDir.c_str(),s_time.c_str()));

  c1->Clear();
  c1->cd(0);
  hitbcid->Write();
  hitbcid->Draw();
  c1->Print(Form("%s/%s/hit_bcid.png",outputDir.c_str(),s_time.c_str()));

  c1->Clear();
  c1->cd(0);
  spillnb->Write();
  spillnb->Draw();
  c1->Print(Form("%s/%s/spillnb.png",outputDir.c_str(),s_time.c_str()));

  c1->Clear();
  gStyle->SetOptStat(0);
  c1->cd(0);
  pe_map->Write();
  pe_map->Draw("colz");
  c1->Print(Form("%s/%s/pe_map.png",outputDir.c_str(),s_time.c_str()));
  
  pe_permm_map->Write();
  pe_permm_map->Draw("colz");
  c1->Print(Form("%s/%s/pe_permm_map.png",outputDir.c_str(),s_time.c_str()));

  nhit_map->Write();
  nhit_map->Draw("colz");
  c1->Print(Form("%s/%s/nhit_map.png",outputDir.c_str(),s_time.c_str()));

  pe_onbeam       -> Write(); 
  pe_offbeam      -> Write();
  pe_permm_onbeam -> Write(); 
  pe_permm_offbeam-> Write();
  hittime_onbeam  -> Write();
  hittime_offbeam -> Write();
  bcid_onbeam     -> Write();
  bcid_offbeam    -> Write();
  pe_chip_onbeam      -> Write(); 
  pe_chip_offbeam     -> Write();
  pe_permm_chip_onbeam      -> Write(); 
  pe_permm_chip_offbeam     -> Write();
  hittime_chip_onbeam -> Write();
  hittime_chip_offbeam-> Write();
  bcid_chip_onbeam    -> Write();
  bcid_chip_offbeam   -> Write();
  diff_hittime_chip_onbeam -> Write();
  diff_hittime_chip_offbeam-> Write();
  diff_hittime_diff_chip_onbeam -> Write();
  diff_hittime_diff_chip_offbeam-> Write();
  diff_hittime_diff_chip_onbeam2 -> Write();
  diff_hittime_diff_chip_offbeam2-> Write();

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
