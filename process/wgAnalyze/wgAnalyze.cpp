#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>

#include <THStack.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TH1.h>
#include <TGraphErrors.h>
#include <TMultiGraph.h>

#include "Const.hpp"
#include "wgColor.h"
#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgFit.h"
#include "wgFitConst.hpp"
#include "wgGetHist.h"
#include "wgEditXML.h"

using namespace std;

double sum_error(double a,double b){ return sqrt(a*a+b*b);}

int main(int argc, char** argv){

  int opt;
  int mode=0;
  string outputFileName("");
  string outputDIR=IMGDATA_DIRECTORY;
  string logoutputDir=LOG_DIRECTORY;

  Logger *Log = new Logger;
  CheckExist *Check = new CheckExist;

  Log->Initialize();
  Log->Write("start Fitting");

  while((opt = getopt(argc,argv, "m:f:o:h")) !=-1 ){
    switch(opt){
      case 'f':
        outputFileName=optarg;
        cout << "== outputfile :" << outputFileName.c_str() << " ==" << endl;
        Log->Write(Form("target:%s",outputFileName.c_str()));
        break;
      case 'm':
        mode = atoi(optarg); 
        break;
      case 'o':
        outputDIR = optarg;
        break;
      case 'h':
        cout <<"this program is for caliburation "<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : write output file name"<<endl;
        cout <<"  -m (int)   : choose fit mode"<<endl;
        cout <<"      0 : only make xml file."<<endl;
        exit(0);
      default:
        cout <<"this program is for fitting to _hist.root file "<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h        : help"<<endl;
        cout <<"  -f (char*) : write output file name"<<endl;
        cout <<"  -m (int)  : choose fit mode"<<endl;
        cout <<"      0 : only make xml file."<<endl;
        exit(0);
    }   
  }

  string inputFileName[argc-optind];
  bool check_option = true;
  const int nfile = argc-optind;

  for(int i=0; i < nfile; i++){
    inputFileName[i] = argv[i+optind];
    if(!Check->Dir(inputFileName[i])){
      cout << inputFileName[i].c_str() << " doesn't exist ! " << endl;
      check_option = false;
    }
    cout << "*** READFILE"<<i<< "  : " << inputFileName[i].c_str() <<" ***"<< endl;
  }

  if(nfile==0 || !check_option){
    cout << "!!ERROR!!" <<endl;
    cout << "if you don't know how to input, please see help."<<endl;
    cout << "help : ./wgAnalize -h" <<endl;
    exit(1);
  }

  cout << " *****  LOG DIRECTORY :" << logoutputDir << "  *****" << endl;
  cout << "\n\nstart Analyzing..." << endl;

  //TFile *outputfile = new TFile(outputFileName.c_str(),"recreate");


  unsigned int i,j,k;
  int ichip,ich,icol;

  if(mode==0 || mode==1 || mode==2 || mode==3){  //for periodic noise mode
    double x_trigth[nfile][NCHIPS][NCHANNELS];
    double x_inputDAC[nfile][NCHIPS][NCHANNELS];
    double y_noise_mean[nfile][NCHIPS][NCHANNELS];
    double y_noise_error[nfile][NCHIPS][NCHANNELS];
    double y_ped_mean[nfile][NCHIPS][NCHANNELS][MEMDEPTH];
    double y_ped_error[nfile][NCHIPS][NCHANNELS][MEMDEPTH];
    double y_hit_mean[nfile][NCHIPS][NCHANNELS][MEMDEPTH];
    double y_hit_error[nfile][NCHIPS][NCHANNELS][MEMDEPTH];
    for(int ifile=0; ifile < nfile; ifile++){
      string OpenXmlFile("");
      string name("");
      wgEditXML *Edit = new wgEditXML;
      for(i=0;i<NCHIPS;i++){
        cout << "start chip : "<< i << endl;
        ichip=i;
        for(j=0;j<NCHANNELS;j++){
          ich=j;
          OpenXmlFile = Form("%s/chip%d/ch%d.xml",inputFileName[ifile].c_str(),ichip,ich);
          Edit->Open(OpenXmlFile,ichip,ich);
          name="inputDAC";
          x_inputDAC[ifile][ichip][ich] = Edit->GetConfigValue(name); 
          name="trigth";
          x_trigth[ifile][ichip][ich] = Edit->GetConfigValue(name); 
          name="noise_m";
          y_noise_mean[ifile][ichip][ich] = Edit->GetChValue(name); 
          name="noise_e";
          y_noise_error[ifile][ichip][ich] = Edit->GetChValue(name); 
          for(k=0;k<MEMDEPTH;k++){ 
            icol=k;
            name="nohit_0_m";
            y_ped_mean[ifile][ichip][ich][icol] = Edit->GetColValue(name,icol);
            name="nohit_0_e";
            y_ped_error[ifile][ichip][ich][icol] = Edit->GetColValue(name,icol);
            name="hit_HG_0_m";
            y_hit_mean[ifile][ichip][ich][icol] = Edit->GetColValue(name,icol);
            name="hit_HG_0_e";
            y_hit_error[ifile][ichip][ich][icol] = Edit->GetColValue(name,icol);
          }       
          Edit->Write();
          Edit->Close();
        }
      }
      delete Edit;
    }

    if(mode==0){     //Noise rate Scurve
      wgColor wgColor;
      for(i=0;i<NCHIPS;i++){
        TMultiGraph *mg = new TMultiGraph();
        mg->SetTitle("S_curve_Noiserate;trig_threshold;Noise rate[Hz]");
        TGraphErrors *h_bcid[NCHANNELS];
        for(j=0;j<NCHANNELS;j++){
          h_bcid[j]= new TGraphErrors;
          h_bcid[j]->SetMarkerStyle(22);
          h_bcid[j]->SetMarkerColor(wgColor::wgcolors[j]);
          h_bcid[j]->SetMarkerSize(1);
          h_bcid[j]->SetTitle("S_curve_Noiserate;trig_threshold;Noise rate[Hz]");
          for(int ifile=0; ifile < nfile; ifile++){
            h_bcid[j]->SetPoint(ifile,x_trigth[ifile][i][j],y_noise_mean[ifile][i][j]);
            h_bcid[j]->SetPointError(ifile,1,y_noise_error[ifile][i][j]);
          }//end ifile
          mg->Add(h_bcid[j]);
          TCanvas *c1= new TCanvas("c1","c1");
          c1->SetLogy();
          h_bcid[j]->Draw("ap");
          c1->Print(Form("%s/%s_chip%d_ch%d.png",outputDIR.c_str(),outputFileName.c_str(),i,j));
          delete c1;
        }
        TCanvas *c1= new TCanvas("c1","c1");
        c1->SetLogy();
        mg->Draw("ap");
        c1->Print(Form("%s/%s_chip%d_allch.png",outputDIR.c_str(),outputFileName.c_str(),i));
        delete c1;
        delete mg;
      }
    }

    if(mode==1){  //Gain Scurve (merge column)
      wgColor wgColor;
      for(i=0;i<NCHIPS;i++){
        for(j=0;j<NCHANNELS;j++){
          TGraphErrors *h_gain[MEMDEPTH];
          TMultiGraph *mg = new TMultiGraph();
          mg->SetTitle("Peak Distance b/w pedestal and hit_HG;inputDAC;distance[ADC count]");
          for(k=0;k<MEMDEPTH;k++){
            h_gain[k] = new TGraphErrors;
            h_gain[k]->SetMarkerStyle(22);
            h_gain[k]->SetMarkerColor(wgColor::wgcolors[k]);
            h_gain[k]->SetMarkerSize(1);
            for(int ifile=0; ifile < nfile; ifile++){
              if(y_hit_mean[ifile][i][j][k]-y_ped_mean[ifile][i][j][k]>150||y_hit_mean[ifile][i][j][k]-y_ped_mean[ifile][i][j][k]<-50 || y_ped_mean[ifile][i][j][k]<300){
                h_gain[k]->SetPoint(ifile,0,0);
                double error=sum_error(0,0);
                h_gain[k]->SetPointError(ifile,1,error);

              }else{
                h_gain[k]->SetPoint(ifile,x_inputDAC[ifile][i][j],y_hit_mean[ifile][i][j][k]-y_ped_mean[ifile][i][j][k]);
                double error=sum_error(y_hit_error[ifile][i][j][k],y_ped_error[ifile][i][j][k]);
                h_gain[k]->SetPointError(ifile,1,error);
              }
            }//end ifile
            mg->Add(h_gain[k]);
          }//end colum 
          TCanvas *c1= new TCanvas("c1","c1");
          mg->Draw("ap");
          c1->Print(Form("%s/%s_chip%d_ch%d.png",outputDIR.c_str(),outputFileName.c_str(),i,j));
          delete c1;
          delete mg;
          //for(k=0;k<MEMDEPTH;k++){delete h_gain[k];}
        }
      }
    }

    if(mode==2){  //Gain Scurve (merge channel)
      wgColor wgColor;
      for(i=0;i<NCHIPS;i++){
        for(k=0;k<MEMDEPTH;k++){
          TGraphErrors *h_gain[NCHANNELS];
          TMultiGraph *mg = new TMultiGraph();
          mg->SetTitle("Peak Distance b/w pedestal and hit_HG;inputDAC;distance[ADC count]");
          for(j=0;j<32;j++){
            h_gain[j] = new TGraphErrors;
            h_gain[j]->SetMarkerStyle(22);
            h_gain[j]->SetMarkerColor(wgColor::wgcolors[j]);
            h_gain[j]->SetMarkerSize(1);
            for(int ifile=0; ifile < nfile; ifile++){
              if(y_hit_mean[ifile][i][j][k]-y_ped_mean[ifile][i][j][k]>150||y_hit_mean[ifile][i][j][k]-y_ped_mean[ifile][i][j][k]<-50 || y_ped_mean[ifile][i][j][k]<300){
                h_gain[j]->SetPoint(ifile,0,0);
                double error=sum_error(0,0);
                h_gain[j]->SetPointError(ifile,1,error);
              }else{
                h_gain[j]->SetPoint(ifile,x_inputDAC[ifile][i][j],y_hit_mean[ifile][i][j][k]-y_ped_mean[ifile][i][j][k]);
                double error=sum_error(y_hit_error[ifile][i][j][k],y_ped_error[ifile][i][j][k]);
                h_gain[j]->SetPointError(ifile,1,error);
              }
            }//end ifile
            mg->Add(h_gain[j]);
          }//end colum 
          TCanvas *c1= new TCanvas("c1","c1");
          mg->Draw("ap");
          c1->Print(Form("%s/%s_chip%d_col%d.png",outputDIR.c_str(),outputFileName.c_str(),i,k));
          delete c1;
          delete mg;
        }
      }
    }

  }else if(mode==10||mode==11){  //for LED mode
    double x_trigth[nfile][NCHIPS][NCHANNELS];
    double y_gseff_mean[nfile][NCHIPS][NCHANNELS][MEMDEPTH];
    double y_gseff_error[nfile][NCHIPS][NCHANNELS][MEMDEPTH];
    for(int ifile=0; ifile < nfile; ifile++){
      string OpenXmlFile("");
      string name("");
      wgEditXML *Edit = new wgEditXML;
      for(i=0;i<NCHIPS;i++){
        cout << "start chip : "<< i << endl;
        ichip=i;
        for(j=0;j<NCHANNELS;j++){
          ich=j;
          OpenXmlFile = Form("%s/chip%d/ch%d.xml",inputFileName[ifile].c_str(),ichip,ich);
          Edit->Open(OpenXmlFile,ichip,ich);
          name="trigth";
          x_trigth[ifile][ichip][ich] = Edit->GetConfigValue(name); 
          for(k=0;k<MEMDEPTH;k++){ 
            icol=k;
            name="GS_eff_m";
            y_gseff_mean[ifile][ichip][ich][icol] = Edit->GetColValue(name,icol); 
            name="GS_eff_e";
            y_gseff_error[ifile][ichip][ich][icol] = Edit->GetColValue(name,icol); 
          }       
          Edit->Write();
          Edit->Close();
        }
      }
      delete Edit;
    }

    if(mode==10){  //GainSelect Efficiency for trigger (merge channel)
      wgColor wgColor;
      for(i=0;i<NCHIPS;i++){
        for(k=0;k<MEMDEPTH;k++){
          TMultiGraph *mg = new TMultiGraph();
          mg->SetTitle("gain efficiency;trig_th;efficiency");
          TGraphErrors *h_gseff[32];
          for(j=0;j<32;j++){
            h_gseff[j] = new TGraphErrors;
            h_gseff[j]->SetMarkerStyle(22);
            h_gseff[j]->SetMarkerColor(wgColor::wgcolors[j]);
            h_gseff[j]->SetMarkerSize(1);
            for(int ifile=0; ifile < nfile; ifile++){
              h_gseff[j]->SetPoint(ifile,x_trigth[ifile][i][j],y_gseff_mean[ifile][i][j][k]);
              h_gseff[j]->SetPointError(ifile,1,y_gseff_error[ifile][i][j][k]);
            }//end ifile
            mg->Add(h_gseff[j]);
          }
          TCanvas *c1= new TCanvas("c1","c1");
          mg->Draw("ap");
          c1->Print(Form("%s/%s_chip%d_col%d.png",outputDIR.c_str(),outputFileName.c_str(),i,k));
          delete c1;
          delete mg;
        }
      }
    }else if(mode==11){  //GainSelect Efficiency for trigger (merge col)
      wgColor wgColor;
      for(i=0;i<NCHIPS;i++){
        for(j=0;j<32;j++){
          TMultiGraph *mg = new TMultiGraph();
          mg->SetTitle("gain efficiency;trig_th;efficiency");
          TGraphErrors *h_gseff[16];
          TLegend *leg = new TLegend(0.85,0.25,0.95,0.95);
          for(k=0;k<MEMDEPTH;k++){
            h_gseff[k] = new TGraphErrors;
            h_gseff[k]->SetMarkerStyle(22);
            h_gseff[k]->SetMarkerColor(wgColor::wgcolors[8*(k%4)+(k/4)]);
            h_gseff[k]->SetMarkerSize(1);
            leg->AddEntry(h_gseff[k] ,Form("col %d",k),"p");
            for(int ifile=0; ifile < nfile; ifile++){
              h_gseff[k]->SetPoint(ifile,x_trigth[ifile][i][j],y_gseff_mean[ifile][i][j][k]);
              h_gseff[k]->SetPointError(ifile,1,y_gseff_error[ifile][i][j][k]);
            }//end ifile
            mg->Add(h_gseff[k]);
          }
          TCanvas *c1= new TCanvas("c1","c1");
          mg->Draw("ap");
          leg->Draw();
          c1->Print(Form("%s/%s_chip%d_ch%d.png",outputDIR.c_str(),outputFileName.c_str(),i,j));
          delete c1;
          delete mg;
        }
      }
    }
  }
  return 0;
}


