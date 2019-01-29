#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <stdio.h>

#include "TDirectory.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2F.h"
#include "TTree.h"

#include "Const.h"
#include "wgColor.h"
#include "wgTools.h"
#include "wgGetTree.h"
#include "wgErrorCode.h"

using namespace std;

void ReadFile(string inputFileName, bool overwrite, unsigned int maxEvt, string outputDir, string logoutputDir, int mode);

OperateString* OptStr;
CheckExist* check;
Logger* Log;

int main(int argc, char** argv) {

  OptStr = new OperateString;
  check = new CheckExist;
  Log = new Logger;

  int opt;
  int mode = 0;
  wgConst *con = new wgConst();
  con->GetENV();
  string inputFileName("");
  string outputDir    =con->HIST_DIRECTORY;
  string logoutputDir =con->LOG_DIRECTORY;
  string outputFile("");
  bool overwrite = false;
  delete con;

  Log->Initialize();

  while((opt = getopt(argc,argv, "f:o:m:rh")) !=-1 ){
    switch(opt){
      case 'f':
        inputFileName=optarg;
        if(!check->RootFile(inputFileName)){ 
          cout<<"!!Error!! "<<inputFileName.c_str()<<"is wrong!!";
          Log->eWrite(Form("Error!!target:%s is wrong",inputFileName.c_str()));
          return 1;
        }
        cout << "== readfile :" << inputFileName.c_str() << " ==" << endl;
        Log->Write(Form("[%s][wgMakeHist]start wgMakeHist",inputFileName.c_str()));
        break;
      case 'o':
        outputDir = optarg;
        break;
      case 'm':
        mode = atoi(optarg); 
        break;
      case 'r':
        overwrite = true;
        cout << "== mode : overwrite  ==" << endl;
        Log->Write(Form("[%s][wgMakeHist] overwrite mode",inputFileName.c_str()));
        break;
      case 'h':
        cout <<"this program is for making histogram from _tree.root file to _hist.root file "<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose inputfile you wanna read(must)"<<endl;
        cout <<"  -o (char*) : choose output directory. (default=WAGSASCI_HISTDIR)" << endl;
        cout <<"  -m (int)   : choose acquisition mode"<<endl;
        cout <<"  -r         : overwrite mode "<<endl;
        exit(0);
      default:
        cout <<"this program is for making histogram from _tree.root file to _hist.root file "<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose inputfile you wanna read(must)"<<endl;
        cout <<"  -o (char*) : choose output directory. (default=WAGSASCI_HISTDIR)" << endl;
        cout <<"  -m (int)   : choose acquisition mode"<<endl;
        cout <<"  -r         : overwrite mode "<<endl;
        exit(0);
    }
  }

  if(inputFileName==""){
    cout << "!!ERROR!! please input filename." <<endl;
    cout << "if you don't know how to input, please see help."<<endl;
    cout << "help : ./wgMakeHist -h" <<endl;
    exit(1);
  }

  const unsigned int maxEvt =99999999;
  cout << "Maximum number of events treated = " << maxEvt << endl;

  ReadFile(inputFileName, overwrite, maxEvt, outputDir, logoutputDir, mode);
  return 0;
}

//******************************************************************************

void ReadFile(string inputFileName, bool overwrite, unsigned int maxEvt, string outputDir, string logoutputDir, int mode){

  cout << " *****  READING FILE     :" << inputFileName << "  *****" << endl;

  string outputHistFileName = OptStr->GetNameBeforeLastUnderBar(inputFileName)+"_hist.root";
  cout << " *****  OUTPUT Hist FILE      :" << outputHistFileName << "  *****" << endl;
  cout << " *****  OUTPUT DIRECTORY :" << outputDir << "  *****" << endl;
  cout << " *****  LOG DIRECTORY :" << logoutputDir << "  *****" << endl;
  TFile * outputHistFile;

  if (!overwrite){
    outputHistFile = new TFile(Form("%s/%s",outputDir.c_str(),outputHistFileName.c_str()), "create");
    if ( !outputHistFile->IsOpen() ) {
      Log->eWrite(Form("[%s][wgMakeHist]Error!!:%s/%s has alreagy exist",inputFileName.c_str(),outputDir.c_str(),outputHistFileName.c_str()));
      cout << "!! ERROR !!\tFile already created!" << endl;
      return;
    }
  }
  else {
    outputHistFile = new TFile(Form("%s/%s",outputDir.c_str(),outputHistFileName.c_str()), "recreate");
  }

  Log->Write(Form("[%s][wgMakeHist]%s/%s is being created",inputFileName.c_str(),outputDir.c_str(),outputHistFileName.c_str()));

  // === start make histgram ===
  Raw_t rd;
  wgColor wgColor;

  unsigned int i,j,k;

  TH1F* h_spill; 
  TH1F* h_charge[NCHIPS][NCHANNELS]; 
  TH1F* h_charge_hit_HG[NCHIPS][NCHANNELS][MEMDEPTH+1];
  TH1F* h_charge_nohit[NCHIPS][NCHANNELS][MEMDEPTH+1]; 
  //TH1F* h_time_hit[NCHIPS][NCHANNELS][MEMDEPTH+1]; 
  //TH1F* h_time_nohit[NCHIPS][NCHANNELS][MEMDEPTH+1]; 
  TH1F* h_pe[NCHIPS][NCHANNELS]; 
  TH1F* h_bcid[NCHIPS][NCHANNELS]; 

  int min_bin=0;
  int max_bin=4096;
  int bin=4096;

  for (i=0; i<NCHIPS; i++) {
    for (j=0; j<NCHANNELS; j++) {
      for (k=0; k<MEMDEPTH+1; k++) {
        
        h_charge_hit_HG[i][j][k] =
          new TH1F(Form("charge_hit_HG_chip%d_ch%d_col%d",i,j,k),
              Form("charge_hit_HG_chip%d_ch%d_col%d",i,j,k),
              bin,min_bin,max_bin);
        h_charge_hit_HG[i][j][k]->SetLineColor(wgColor::wgcolors[k]);
        h_charge_nohit[i][j][k] =
          new TH1F(Form("charge_nohit_chip%d_ch%d_col%d",i,j,k),
              Form("charge_nohit_chip%d_ch%d_col%d",i,j,k),
              bin,min_bin,max_bin);
        h_charge_nohit[i][j][k]->SetLineColor(wgColor::wgcolors[k+MEMDEPTH*2+2]);
        /*
           h_time_hit[i][j][k] =
           new TH1F(Form("time_hit_chip%d_ch%d_col%d",i,j,k),
           Form("time_hit_chip%d_ch%d_col%d",i,j,k),
           bin,min_bin,max_bin);
           h_time_hit[i][j][k]->SetLineColor(wgColor::wgcolors[k]);


           h_time_nohit[i][j][k] =
           new TH1F(Form("time_nohit_chip%d_ch%d_col%d",i,j,k),
           Form("time_nohit_chip%d_ch%d_col%d",i,j,k),
           bin,min_bin,max_bin);   
           h_time_nohit[i][j][k]->SetLineColor(wgColor::wgcolors[k+MEMDEPTH*2+2]);
           */
      }//end col
      h_charge[i][j] =
        new TH1F(Form("charge_chip%d_ch%d",i,j),
            Form("charge_chip%d_ch%d",i,j),
            bin,min_bin,max_bin);
      h_charge[i][j]->SetLineColor(kBlack);
      
      h_bcid[i][j] =
        new TH1F(Form("bcid_chip%d_ch%d",i,j),
            Form("bcid_chip%d_ch%d",i,j),
            bin,min_bin,max_bin);
      h_bcid[i][j]->SetLineColor(wgColor::wgcolors[k]);
      h_pe[i][j] =
        new TH1F(Form("pe_chip%d_ch%d",i,j),
            Form("pe_chip%d_ch%d",i,j),
            bin,min_bin,max_bin);
      h_pe[i][j]->SetLineColor(wgColor::wgcolors[k]);
    }//end ch
  }//end chip
  // === end make histgram ===

  wgGetTree* GetTree = new wgGetTree( inputFileName ); 
  GetTree->SetTreeFile(rd);

  TH1F * start_time;
  TH1F * stop_time;
  TH1F * nb_data_pkts;
  TH1F * nb_lost_pkts;
  start_time   = GetTree->GetHist_StartTime();
  stop_time    = GetTree->GetHist_StopTime();
  nb_data_pkts = GetTree->GetHist_DataPacket();
  nb_lost_pkts = GetTree->GetHist_LostPacket();

  float max_spill = wgGetTree::tree->GetMaximum("spill");
  float min_spill = wgGetTree::tree->GetMinimum("spill");
  if(min_spill<0){
    Log->eWrite(Form("[%s][wgMakeHist]Error!!: some spill value is missed!!",inputFileName.c_str()));
    cout << "Error!!: some spill value is missed!!"<< endl;
  }

  outputHistFile->cd();
  h_spill = new TH1F("spill","spill",(int)max_spill-min_spill+200,(int)min_spill-100,(int)max_spill+100);

  wgGetTree::finput->cd();
  int ieve=0;
  int neve=wgGetTree::tree->GetEntries();

  for(ieve=0; ieve < neve; ieve++){
    if(ieve%1000==0) cout << ieve << " / " << neve << "finish... " <<endl;
    wgGetTree::tree->GetEntry(ieve);
    h_spill->Fill(rd.spill,rd.spill_flag);
    for(unsigned int i=0; i<NCHIPS; i++){
      int nchips = NCHIPS;
      if(rd.chipid[i] < 0 || rd.chipid[i] >= nchips) continue;
      int ichip = rd.chipid[i];
      for(unsigned int j=0; j<NCHANNELS; j++){
        int ich = j;
        for(unsigned int k=0; k<MEMDEPTH; k++){ 
          int icol = k;
          if(rd.hit[i][ich][icol]==1){
            h_bcid[ichip][ich]->Fill(rd.bcid[i][icol]);
            //h_pe[ichip][ich]->Fill(rd.pe[i][ich][icol]);
            //h_time_hit[ichip][ich][icol]->Fill(rd.time[i][ich][icol]);
            if(rd.gs[i][ich][icol]==1){ 
              h_charge       [ichip][ich]->Fill(rd.charge[i][ich][icol]-rd.pedestal[ichip][ich][icol]);
              //h_charge_hit_HG[ichip][ich][icol]->Fill(rd.charge[i][ich][icol]);
              //h_charge_hit_HG[ichip][ich][16]  ->Fill(rd.charge[i][ich][icol]);
            }else if(rd.gs[i][ich][icol]==0){
              //h_charge_hit_LG[ichip][ich][icol]->Fill(rd.charge[i][ich][icol]);
              //h_charge_hit_LG[ichip][ich][16]  ->Fill(rd.charge[i][ich][icol]);
            }//end gain
          }else if(rd.hit[i][ich][icol]==0){
            h_charge_nohit[ichip][ich][icol]->Fill(rd.charge[i][ich][icol]);
            h_charge_nohit[ichip][ich][16]  ->Fill(rd.charge[i][ich][icol]);
            //h_time_nohit[ichip][ich][icol]->Fill(rd.time[i][ich][icol]);
          }//end hit
        }//end icol
      }//end ich
    }//end V_chipid
  }//end ieve

  cout << endl;
  cout << "     *****  Finished reading file  *****     " << endl;
  cout << endl;

  outputHistFile->cd();

  start_time   -> Write();
  stop_time    -> Write();
  nb_data_pkts -> Write();
  nb_lost_pkts -> Write();
  h_spill->Write();
  for (i=0; i < NCHIPS; i++){
    for (j=0; j < NCHANNELS; j++){
      h_bcid           [i][j]->Write();
      //h_pe             [i][j]->Write();
      h_charge         [i][j]   ->Write();
      for (k=0; k < MEMDEPTH+1; k++){
        if(mode==1) h_charge_hit_HG  [i][j][k]->Write();
        h_charge_nohit   [i][j][k]->Write();
        //h_time_hit       [i][j][k]->Write();
        //h_time_nohit     [i][j][k]->Write();
      }
    }
  }

  outputHistFile->Close();
  Log->Write(Form("[%s][wgMakeHist]Finish Make histgram",inputFileName.c_str()));
  delete Log;
  delete GetTree;
}

