#include <string>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <ctime>

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

TFile *fout;

vector<string> GetIncludeFileName(string& inputXMLName);
void AnaRecon(string &filename,string &outfilename);


int main(int argc, char** argv){
  int opt;
  wgConst *con = new wgConst;
  con->GetENV();
  string inputName("");
  string recondir = con->RECON_DIRECTORY;
  string outputdir = "/home/data/hittiming";
  string inputReconName("");

  CheckExist *check = new CheckExist;


  while((opt = getopt(argc,argv, "f:")) !=-1 ){
    switch(opt){
      case 'f':
        inputName=optarg;
        inputReconName  =Form("%s/%s_recon.root" ,recondir.c_str(),inputName.c_str());
        if(!check->RootFile(inputReconName)){ 
          cout<<"!!Error!! "<<inputReconName.c_str()<<"doesn't exist!!";
          exit(0);
        }
        break;
      default:
        cout <<"This program is for data quality check. "<<endl;
        cout <<"You can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose run name you wanna read(must)"<<endl;
        exit(0);
    }   
  }

  if(inputName==""){
    cout << "!!ERROR!! please input run name." <<endl;
    cout << "if you don't know how to input, please see help."<<endl;
    cout << "help : ./wgDQCheck -h" <<endl;
    return 0;
  }
  string outputName = Form("%s/%s_hittiming.root" ,outputdir.c_str(),inputName.c_str());



  cout << " *****  READING RUN NAME     :" << inputName << "  *****" << endl;

  delete check;

  AnaRecon(inputReconName,outputName);

  return 0;
}




//*****************************************************************
void AnaRecon(string &filename,string &outfilename){
  wgGetTree * gettree = new wgGetTree(filename);
  Hit_t    type_hit;
  Track_t  type_track;

  gettree->SetTreeFile(type_hit);
  gettree->SetTreeFile(type_track);

  TCanvas *c1 = new TCanvas("c1","c1",1200,1000);
  c1->SetLogy(1);

  fout = new TFile(outfilename.c_str(),"recreate");

  TH1D *h_time1    = new TH1D("HitTime","HitTime",50,-100.,100.);
  TH1D *h_time2    = new TH1D("HitTime_Wide","HitTime_Wide",120,-1200.,1200.);
  TH1D *h_time3    = new TH1D("HitTime_SameBcid","HitTime_SameBcid",120,-1200.,1200.);
  TH1D *h_bcid     = new TH1D("HitBcid","HitBcid",12,-6.,6.);
  TH2D *h_tdctime  = new TH2D("TdcVsTime","TdcVsTime",4096,0,4096,580,0,580);
  TH2D *h_timebcid = new TH2D("HitTimeBcid","HitTimeBcid",12,-6,6,120,-1200,1200);
  TH1D *h_pe       = new TH1D("Pe","Pe",1000,0,100);
  TH2D *h_peadc    = new TH2D("AdcVsPe","AdcVsPe",4096,0,4096,1000,0,100);
  TH1D *h_slopezy  = new TH1D("SlopZY","SlopeZY",1000,0,1000.);
  TH1D *h_slopezx  = new TH1D("SlopZX","SlopeZX",1000,0,1000.);
  TH1D *h_angle    = new TH1D("Angle","Angle",90,0,90.);
  TH1D *h_angle_zy = new TH1D("AngleZY","AngleZY",180,0,180.);
  TH1D *h_angle_zx = new TH1D("AngleZX","AngleZX",180,0,180.);

  int neve = wgGetTree::tree->GetEntries();
  for(int ieve=0; ieve < neve; ieve++){
    gettree->GetEntry(ieve);
    if(type_track.num_trackid!=1) continue;
    for(int i=0;i<type_track.num_track;i++){
      int numhits = type_track.num_track_hits[i];
      vector<double> hit_bcid_list(numhits);
      vector<double> hit_time_list(numhits);
      vector<double> hit_tdc_list(numhits);
      //double average_bcid = 0.;
      double average_bcid = type_track.track_bcid[i];
      double average_time  = 0.;
      double average_time2 = 0.;
      int numhitsInBcid = 0;
      for(int j=0;j<numhits;j++){
        int hitid = type_track.track_hits_hitid[i][j];
        hit_bcid_list[j]=(double)type_hit.hit_bcid[hitid];
        hit_time_list[j]=(double)type_hit.hit_time[hitid];
        hit_tdc_list [j]=(double)type_hit.hit_tdc [hitid];
        //average_bcid += (double)type_hit.hit_bcid[hitid]/((double)numhits);
        average_time += (double)type_hit.hit_time[hitid]/((double)numhits);
        if(hit_bcid_list[j]==average_bcid){
          average_time2 += (double)type_hit.hit_time[hitid];
          numhitsInBcid++;
        }
        double pe  =(double)type_hit.hit_pe [hitid];
        double adc =(double)type_hit.hit_adc[hitid];
        h_pe->Fill(pe);
        h_peadc->Fill(adc,pe);
        if(type_track.track_view[i]==0){
          int trackid_pair = type_track.track_id[i];
          double slope_x = type_track.track_slope[i];
          double slope_y = type_track.track_slope[trackid_pair];
          double angle = acos(slope_y/sqrt(1+slope_x*slope_x+slope_y*slope_y))*180./3.141592;
          double anglezy = acos(slope_y)*180./3.141592+90.;
          double anglezx = acos(slope_x)*180./3.141592+90.;
          if(angle<0.) angle *= -1.;
          h_angle->Fill(angle);
          h_angle_zy->Fill(anglezy);
          h_angle_zx->Fill(anglezx);
        }
        if(type_track.track_view[i]==0){h_slopezx->Fill(type_track.track_slope[i]);}
        if(type_track.track_view[i]==1){h_slopezy->Fill(type_track.track_slope[i]);}
      }
      if(numhitsInBcid!=0) average_time2/=numhitsInBcid;
      else average_time2=0;
      for(int j=0;j<numhits;j++){
        double timediff   = hit_time_list[j]-average_time;
        double timediff2  = hit_time_list[j]-average_time2;
        double bciddiff   = hit_bcid_list[j]-average_bcid;
        double timeInBcid = hit_time_list[j]-580.*hit_bcid_list[j];
        double tdc        = hit_tdc_list[j];
        h_bcid ->Fill(bciddiff);
        h_time1->Fill(timediff);
        h_time2->Fill(timediff);
        if(hit_bcid_list[j]==average_bcid) h_time3->Fill(timediff2);
        h_timebcid->Fill(bciddiff,timediff);
        h_tdctime->Fill(tdc,timeInBcid);
                
      }
    }
  }
  h_bcid    ->Write();
  h_time1   ->Write();
  h_time2   ->Write();
  h_time3   ->Write();
  h_timebcid->Write();
  h_tdctime ->Write();
  h_pe      ->Write();
  h_peadc   ->Write();
  h_angle   ->Write();
  h_angle_zy->Write();
  h_angle_zx->Write();
  h_slopezy ->Write();
  h_slopezx ->Write();

  fout->Write();
  fout->Close();
  delete gettree; 
}

//******************************************************************************
