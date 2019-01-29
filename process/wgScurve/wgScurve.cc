#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>

#include <THStack.h>
#include <TMultiGraph.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TF1.h>
#include <TSpectrum.h>

#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgEditXML.h"
#include "wgColor.h"
#include "wgFit.h"
#include "wgFitConst.h"
#include "wgGetHist.h"

using namespace std;

//#define DEBUG_WGCALIB

double Calcurate_Mean(vector<double>);
double Calcurate_Sigma(vector<double>);

double NoiseToPe(double);
void MakeDir(string& str);
void MakeXML(string& str,int ichip);
vector<string> GetIncludeFileName(string& inputDirName);
void AnaXML(vector<string> &inputDirName,string& outputXMLDirName,string& outputIMGDirName,int ichip);

//******************************************************************
int main(int argc, char** argv){

  int opt;
  wgConst *con = new wgConst;
  con->GetENV();
  string inputDirName("");
  string outputXMLDirName("");
  string outputIMGDirName=con->IMGDATA_DIRECTORY;
  string logoutputDir=con->LOG_DIRECTORY;

  OperateString *OpStr = new OperateString;
  Logger *Log = new Logger;
  CheckExist *check = new CheckExist;

  Log->Initialize();
  Log->Write("start calibration...");

  while((opt = getopt(argc,argv, "f:o:h")) !=-1 ){
    switch(opt){
      case 'f':
        inputDirName=optarg;
        if(!check->Dir(inputDirName)){ 
          cout<<"!!Error!! "<<inputDirName.c_str()<<"doesn't exist!!";
          Log->eWrite(Form("Error!!target:%s doesn't exist",inputDirName.c_str()));
          return 1;
        }   
        Log->Write(Form("target:%s",inputDirName.c_str()));
        break;

      case 'o':
        outputXMLDirName = optarg; 
        break;

      case 'h':
        cout <<"this program is for calibration from the data of wgAnaHistSummary. "<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose input directory you wanna read(must)"<<endl;
        cout <<"  -o (char*) : choose output directory (default: input directory) "<<endl;
        exit(0);
      default:
        cout <<"this program is for calibration from the data of wgAnaHistSummary. "<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose input directory you wanna read(must)"<<endl;
        cout <<"  -o (char*) : choose output directory (default: input directory) "<<endl;
        exit(0);
    }   
  }

  if(inputDirName==""){
    cout << "!!ERROR!! please input filename." <<endl;
    cout << "if you don't know how to input, please see help."<<endl;
    cout << "help : ./wgAnaHistSummary -h" <<endl;
    exit(1);
  }

  if(outputXMLDirName==""){
    outputXMLDirName=con->CALIBDATA_DIRECTORY;
  }
  outputIMGDirName = Form("%s/%s",outputIMGDirName.c_str(),OpStr->GetName(inputDirName).c_str());

  Log->Write(Form("READING DIRECTORY : %s",inputDirName.c_str()));
  Log->Write(Form("OUTPUT DIRECTORY : %s",outputXMLDirName.c_str()));

  cout << " *****  READING DIRECTORY     :" << inputDirName << "  *****" << endl;
  cout << " *****  OUTPUT XML DIRECTORY :" << outputXMLDirName << "  *****" << endl;
  cout << " *****  OUTPUT IMAGE DIRECTORY :" << outputIMGDirName << "  *****" << endl;

  delete con;
  delete check;
  delete OpStr;

  vector<string> ReadFile = GetIncludeFileName(inputDirName); 
  cout << " Finish  reading file"<<endl;
  MakeDir(outputIMGDirName);

  int nchip=NCHIPS;
  //int nchip=1;

  for(int ichip=0;ichip<nchip;ichip++){
    AnaXML(ReadFile,
        outputXMLDirName,
        outputIMGDirName,
        ichip
        );
  }
  Log->Write("end summarizeing ... " );
  delete Log;  
}

//******************************************************************
vector<string> GetIncludeFileName(string& inputDirName){
  DIR *dp;
  struct dirent *entry;
  vector<string> openxmlfile;

  dp = opendir(inputDirName.c_str());
  if(dp==NULL){
    cout << " !! WARNING !! no data is in "<< inputDirName << endl;
    return openxmlfile;
  }

  while( (entry = readdir(dp))!=NULL ){
    if((entry->d_name[0])!='.'){
      openxmlfile.push_back(Form("%s/%s",inputDirName.c_str(),entry->d_name));
      cout << "ReadFile : " << inputDirName << "/" << entry->d_name << endl;
    }
  }
  closedir(dp);
  return openxmlfile;
} 

//******************************************************************
void AnaXML(vector<string> &inputFileName, string& outputXMLDirName,string& outputIMGDirName,int ichip){

  cout << "*********************************"<<endl;    
  cout << "       chip " << ichip << " start... " <<endl;   
  cout << "*********************************"<<endl;    
  MakeXML(outputXMLDirName,ichip);

  int FN=inputFileName.size();
  int ndif[FN];
  int *trig_th[FN];
  int *inputDAC[FN];
  double *Gain[FN];
  double *Noise[FN];
  for(int iFN=0;iFN<FN;iFN++){
    trig_th[iFN]=new int;
    inputDAC[iFN]=new int;
    Gain[iFN]=new double;
    Noise[iFN]=new double;
  }

  vector<int> list_inputDAC;
  vector<int> list_trigth;

  TCanvas *c1 = new TCanvas("c1","c1");
  wgColor wgColor;
  wgEditXML *Edit = new wgEditXML();
  string xmlfile("");
  string str_dir;
  str_dir=Form("%s/Gain",outputIMGDirName.c_str());
  MakeDir(str_dir);
  str_dir=Form("%s/Noise",outputIMGDirName.c_str());
  MakeDir(str_dir);
  str_dir=Form("%s/Gain/chip%d",outputIMGDirName.c_str(),ichip);
  MakeDir(str_dir);
  str_dir=Form("%s/Noise/chip%d",outputIMGDirName.c_str(),ichip);
  MakeDir(str_dir);

  cout << "  ~~~ Start Reading ~~~  " <<endl;

  //*** Read data ***
  string name("");
  for(int iFN=0;iFN<FN;iFN++){
    int pos = inputFileName[iFN].find("dif_1_1_")+8;
    if(inputFileName[iFN][pos]=='1'){
      ndif[iFN]=0;
    }else if(inputFileName[iFN][pos]=='2'){
      ndif[iFN]=1;
    }

    xmlfile=Form("%s/Summary_chip%d.xml",inputFileName[iFN].c_str(),ichip);
    Edit->Open(xmlfile);
    name="tr_th";
    *trig_th[iFN]=Edit->SUMMARY_GetGlobalConfigValue(name);
    name="gs_th";
    int ich=0;
    name="inputDAC";
    *inputDAC[iFN]=Edit->SUMMARY_GetChConfigValue(name,ich);
    name="Noise";
    *Noise[iFN]=Edit->SUMMARY_GetChFitValue(name,ich);
    name="Gain";
    *Gain[iFN]=Edit->SUMMARY_GetChFitValue(name,ich);
    Edit->Close();
  }
  delete Edit;

  // ****  list up variable **** //
  bool add_list_inputDAC=false;
  bool add_list_trigth=false;
  list_inputDAC.push_back(*inputDAC[0]);
  list_trigth.push_back(*trig_th[0]);
  for(int iFN=0;iFN<FN;iFN++){
    add_list_inputDAC=true;
    add_list_trigth=true;
    for(unsigned int l=0;l<list_inputDAC.size();l++){
      if(list_inputDAC[l]==*inputDAC[iFN]){
        add_list_inputDAC=false;
      }
    }
    for(unsigned int l=0;l<list_trigth.size();l++){
      if(list_trigth[l]==*trig_th[iFN]){
        add_list_trigth=false;
      }
    }
    if(add_list_inputDAC)list_inputDAC.push_back(*inputDAC[iFN]);
    if(add_list_trigth)list_trigth.push_back(*trig_th[iFN]);
  }

  sort(list_inputDAC.begin(), list_inputDAC.end());
  sort(list_trigth.begin(), list_trigth.end());

  const unsigned int size_inputDAC = list_inputDAC.size();
  const unsigned int size_trigth = list_trigth.size();

#ifdef DEBUG_WGCALIB
  for(unsigned int l=0;l<size_inputDAC;l++){
    cout << "inputDAC : " << list_inputDAC[l] << endl;
  }
  for(unsigned int l=0;l<size_trigth;l++){
    cout << "trig_th : " << list_trigth[l] << endl;
  }  
#endif
  // ****  summrize and sort data **** //
  cout << "  ~~~ Start summarizing ~~~  " <<endl;
  double summary_Noise[2][size_inputDAC][size_trigth];
  double summary_Gain[2][size_inputDAC][size_trigth];
  double summary_trigth[2][size_inputDAC][size_trigth];
  bool Gain_fit[2][size_inputDAC][size_trigth];

  for(int iFN=0;iFN<FN;iFN++){
    for(unsigned int l=0;l<size_inputDAC;l++){
      for(unsigned int m=0;m<size_trigth;m++){
        summary_Noise[ndif[iFN]][l][m]    =0.;
        summary_Gain[ndif[iFN]][l][m]     =0.;
        summary_trigth[ndif[iFN]][l][m]   =0.;
        Gain_fit[ndif[iFN]][l][m]=true;
      }
    }
  }

  for(int iFN=0;iFN<FN;iFN++){
    for(unsigned int l=0;l<size_inputDAC;l++){
      if(list_inputDAC[l]!=*inputDAC[iFN]) continue;
      for(unsigned int m=0;m<size_trigth;m++){
        if(list_trigth[m]!=*trig_th[iFN]) continue;
        summary_Noise[ndif[iFN]][l][m]=*Noise[iFN];
        summary_Gain[ndif[iFN]][l][m]=*Gain[iFN];
        summary_trigth[ndif[iFN]][l][m]=*trig_th[iFN];
      }//end trigth
    }//end inputDAC
  }//end FN

  for(int iFN=0;iFN<FN;iFN++){
    delete trig_th[iFN];
    delete Gain[iFN];
    delete Noise[iFN];
    delete inputDAC[iFN];
  }

  //*** Define histgram ***//
  TH1F *h_Gain[size_inputDAC][2];
  TH1F *h_Noise_classfied[size_inputDAC][2][10];

  cout << "  ~~~ Start defining histgram ~~~  " <<endl;
  for(unsigned int l=0;l<size_inputDAC;l++){
    for(int idif=0;idif<2;idif++){
      //* Histgram of Gain per inputDAC *//
      h_Gain[l][idif]=new TH1F(Form("h_Gain_dif%d_chip%d_inputDAC%d",idif,ichip,list_inputDAC[l]),Form("h_Gain_dif%d_chip%d_inputDAC%d",idif,ichip,list_inputDAC[l]),120,-40,200);
      h_Gain[l][idif]->SetTitle(Form("Gain chip:%d,inputDAC:%d;Gain;Entry",ichip,list_inputDAC[l]));
      h_Gain[l][idif]->SetMarkerStyle(8);
      h_Gain[l][idif]->SetMarkerSize(1);
      h_Gain[l][idif]->SetMarkerColor(wgColor::wgcolors[60]);
      h_Gain[l][idif]->SetStats(0);

      //* Scurve of Noise classified by Gain * //
      for(int n=0;n<10;n++){
        h_Noise_classfied[l][idif][n]=new TH1F(Form("h_Noise_iDAC%d_dif%d_chip%d_class%d",list_inputDAC[l],idif,ichip,n),Form("h_Noise_iDAC%d_dif%d_chip%d",list_inputDAC[l],idif,ichip),90,85,175);
        h_Noise_classfied[l][idif][n]->SetTitle(Form("Noise dif:%d, chip:%d,(inputDAC:%d);trig_th;Noise Rate[Hz]",idif, ichip,list_inputDAC[l]));
        h_Noise_classfied[l][idif][n]->SetMarkerStyle(8);
        h_Noise_classfied[l][idif][n]->SetMarkerSize(0.8);
        h_Noise_classfied[l][idif][n]->SetMarkerColor(wgColor::wgcolors[n+60]);
        h_Noise_classfied[l][idif][n]->SetStats(0);
      } 
    }//idif
  }//inputDAC

  // ***  Fill data ***  //
  cout << "  ~~~ Start filling data ~~~  " <<endl;
  for(int idif=0;idif<2;idif++){
    for(unsigned int l=0;l<size_inputDAC;l++){
      for(unsigned int m=0;m<size_trigth;m++){ 
        if(summary_Gain[idif][l][m]==0.)continue;
        h_Gain[l][idif]->Fill(summary_Gain[idif][l][m]);
      }//end trigth
    }//end inputDAC
  }//end dif

  // *** Fit Histgram *** //
  // 1. Fit gain histgrams and set the range of each pe level.
  // 2. Sort the threshold to each pe.
  // 3. Rearrange the data.
  // 4. Sort the threshold to each pe again by rearranged data.
  // 5. Check the sorted p.e. based on noise rate.
  // 6. Fill the empty data.        

  cout << "  ~~~ Start fit histgram ~~~  " <<endl;
  // 1. Fit gain histgrams and set the range of each pe level.
  wgFit* fit = new wgFit();
  for(int idif=0;idif<2;idif++){
    vector<double> summary_Range_Gain;
    vector<double> summary_id_Gain;
    double summary_Range_Noise_min[size_inputDAC][3];
    double summary_Range_Noise_max[size_inputDAC][3];
    TSpectrum *ts_Gain;

    for(unsigned int l=0;l<size_inputDAC;l++){
      for(int pe=0;pe<3;pe++){
        summary_Range_Noise_min[l][pe]=0.;
        summary_Range_Noise_max[l][pe]=0.;
      }
    }

    double y_mean_Noise[3][size_inputDAC];
    double y_sigma_Noise[3][size_inputDAC];
    double mean_Noise_inputDAC[size_inputDAC];
    double sigma_Noise_inputDAC[size_inputDAC];
    double pe_center[3][size_inputDAC];
    double pe_width[3][size_inputDAC];
    for(unsigned int l=0;l<size_inputDAC;l++){
      for(int pe=0;pe<3;pe++){
        pe_center[pe][l]=0.;
        pe_width[pe][l]=0.;    
      }
    }
    for(unsigned int l=0;l<size_inputDAC;l++){
      //* Fit Gain and classfy trigger threshold level*//
      ts_Gain=new TSpectrum(5);
      ts_Gain->Search(h_Gain[l][idif],1.,"",0.05);
      int Npeaks_Gain = ts_Gain->GetNPeaks();
      double* px_Gain=ts_Gain->GetPositionX();
      double* py_Gain=ts_Gain->GetPositionY();
//#ifdef DEBUG_WGCALIB
      h_Gain[l][idif]->Draw();
      c1->Print(Form("%s/Gain/chip%d/Gain_dif%d_chip%d_inputDAC%d.png",outputIMGDirName.c_str(),ichip,idif+1,ichip,list_inputDAC[l]));
//#endif
      delete h_Gain[l][idif];
      fit->swap(Npeaks_Gain,px_Gain,py_Gain);

      for(int ipeak=0;ipeak<Npeaks_Gain-1;ipeak++){
        if(Npeaks_Gain>1){
          summary_Range_Gain.push_back((px_Gain[ipeak]+px_Gain[ipeak+1])/2.0);
        }else{
          summary_Range_Gain.push_back(0.);
        }
      }
      delete ts_Gain;

      // 2. Sort the threshold to each pe.
      for(unsigned int m=0;m<size_trigth;m++){
        for(unsigned int ipeak=0;ipeak<summary_Range_Gain.size();ipeak++){
          if(ipeak==summary_Range_Gain.size()-1){
            if(ipeak!=0){
              if(summary_Range_Gain[ipeak-1] < summary_Gain[idif][l][m]
                  && summary_Range_Gain[ipeak] > summary_Gain[idif][l][m]){
                summary_id_Gain.push_back(ipeak);
                break;
              }else{
                summary_id_Gain.push_back(ipeak+1);
                break;
              }
            }else{
              if(summary_Range_Gain[ipeak] > summary_Gain[idif][l][m]){
                summary_id_Gain.push_back(ipeak);
                break;
              }else{
                summary_id_Gain.push_back(ipeak+1);
                break;
              }
            }
          }else if(ipeak==0){
            if(summary_Range_Gain[ipeak] > summary_Gain[idif][l][m]){
              summary_id_Gain.push_back(ipeak);
              break;
            }
          }else if(ipeak<summary_Range_Gain.size()-1){
            if(summary_Range_Gain[ipeak-1] < summary_Gain[idif][l][m]
                && summary_Range_Gain[ipeak] > summary_Gain[idif][l][m]){
              summary_id_Gain.push_back(ipeak);
              break;
            }
          }
        }
      }

      // 3. Rearrange the data.
      if(summary_id_Gain.size()==0)continue;
      int id_tmp=0;
      vector<double> v_trigth[10];
      vector<double> v_Noise[10];
      vector<double> v_row[10];

      Double_t tmp_trigth[size_trigth];
      Double_t tmp_id_trigth[size_trigth];
      Double_t tmp_Noise[size_trigth];
      Double_t tmp_Rank[size_trigth];

      for(unsigned int k1=0;k1<size_trigth;k1++){
        unsigned int rank=0;
        unsigned int rank0=0;
        for(unsigned int k2=0;k2<size_trigth;k2++){
          if(summary_trigth[idif][l][k2]==0.){rank0++; continue;} 
          if(summary_trigth[idif][l][k1]>summary_trigth[idif][l][k2]) rank++;
        }
        tmp_trigth[rank]=summary_trigth[idif][l][k1];
        tmp_id_trigth[rank]=summary_id_Gain[k1];
        tmp_Noise[rank]=summary_Noise[idif][l][k1];
        if(k1<rank0){
          tmp_trigth[size_trigth-rank0+k1]=0.;
          tmp_id_trigth[size_trigth-rank0+k1]=0.;
          tmp_Noise[size_trigth-rank0+k1]=0.;
        }
      }

      // 4. Sort the threshold to each pe again by rearranged data.
      for(unsigned int m=0; m<size_trigth;m++){
        if(tmp_trigth[m]==0){
          tmp_Rank[m]=-1.;
          Gain_fit[idif][l][m]=false;
          continue;
        }
        if(m==size_trigth-1){
          if(tmp_id_trigth[m-1]==tmp_id_trigth[m]){
            v_trigth[id_tmp].push_back(tmp_trigth[m]);
            v_Noise[id_tmp].push_back(tmp_Noise[m]);
            v_row[id_tmp].push_back(m);
            tmp_Rank[m]=0;
          }else{
            tmp_Rank[m]=-1.;
            Gain_fit[idif][l][m]=false;
          }
        }else if(m==0){
          if(tmp_id_trigth[m]==tmp_id_trigth[m+1]){
            v_trigth[id_tmp].push_back(tmp_trigth[m]);
            v_Noise[id_tmp].push_back(tmp_Noise[m]);
            v_row[id_tmp].push_back(m);
            tmp_Rank[m]=0;
          }else{
            tmp_Rank[m]=-1.;
            Gain_fit[idif][l][m]=false;
          }
        }else if(m>0 && m<size_trigth-1){
          if(tmp_id_trigth[m+1]==tmp_id_trigth[m]){
            if( tmp_id_trigth[m]==tmp_id_trigth[m-1]){
              v_trigth[id_tmp].push_back(tmp_trigth[m]);
              v_Noise[id_tmp].push_back(tmp_Noise[m]);
              v_row[id_tmp].push_back(m);
              tmp_Rank[m]=0;
            }else{
              id_tmp++;
              v_trigth[id_tmp].push_back(tmp_trigth[m]);
              v_Noise[id_tmp].push_back(tmp_Noise[m]);
              v_row[id_tmp].push_back(m);
              tmp_Rank[m]=0;
            }
          }else if(tmp_id_trigth[m]==tmp_id_trigth[m-1]){
            v_trigth[id_tmp].push_back(tmp_trigth[m]);
            v_Noise[id_tmp].push_back(tmp_Noise[m]);
            v_row[id_tmp].push_back(m);
            tmp_Rank[m]=0;
          }else{
            tmp_Rank[m]=-1.;
            Gain_fit[idif][l][m]=false;
          }
        }
      }

      // 5. Check the sorted p.e. based on noise rate.
      double tmp_mean_Noise[10]={};
      double tmp_sigma_Noise[10]={};
      double mean_Noise[10]={};
      double sigma_Noise[10]={};
      for(int i=9;i>=0;i--){
        if(v_Noise[i].size()==0){mean_Noise[i]=0.; continue;}
        if(v_Noise[i].size()%2==1){
          tmp_mean_Noise[i]=v_Noise[i][(int)((v_Noise[i].size()-1)/2)];
        }else{
          tmp_mean_Noise[i]=(v_Noise[i][(int)((v_Noise[i].size())/2)]+v_Noise[i][(int)((v_Noise[i].size()/2)-1)])/2.;
        }
        for(unsigned int j=0;j<v_Noise[i].size();j++){
          tmp_sigma_Noise[i] += pow( v_Noise[i][j]-tmp_mean_Noise[i],2.);
        }
        double size = v_Noise[i].size();
        tmp_sigma_Noise[i] = sqrt( tmp_sigma_Noise[i]/size);
      }

      int mean_rank=0;
      for(int i=9;i>=0;i--){
        if(tmp_mean_Noise[i]==0.) continue;
        double tmp_Noise_center=log10(tmp_mean_Noise[i]);
        double tmp_Noise_var = pow(tmp_sigma_Noise[i]/tmp_mean_Noise[i],2.0);
        double tmp_Noise_center_size=1.;
        for(int i2=i-1;i2>=0;i2--){
          if( tmp_mean_Noise[i2]==0. ) continue;
          if( abs( log10(tmp_mean_Noise[i])-log10(tmp_mean_Noise[i2]) )<1.0 ){
            tmp_Noise_center += log10(tmp_mean_Noise[i2]);
            tmp_Noise_center_size++;
            tmp_Noise_var += pow(tmp_sigma_Noise[i2]/tmp_mean_Noise[i2],2.0);
            tmp_mean_Noise[i2]=0.;
          } 
        }
        mean_Noise[mean_rank]=tmp_Noise_center/tmp_Noise_center_size;
        mean_Noise[mean_rank]=pow(10.,mean_Noise[mean_rank]);
        sigma_Noise[mean_rank]=mean_Noise[mean_rank]*sqrt(tmp_Noise_var);
        mean_rank++;
      }

      for(int pe=0;pe<3;pe++){
        y_mean_Noise[pe][l]=mean_Noise[pe];
        y_sigma_Noise[pe][l]=sigma_Noise[pe];
      }
      mean_Noise_inputDAC[l]=list_inputDAC[l];
      sigma_Noise_inputDAC[l]=10;


      for(unsigned int m=0; m<size_trigth;m++){ 
        for(int pe=0;pe<(3-1);pe++){
          if(abs(log10(tmp_Noise[m])-log10(y_mean_Noise[pe][l]))<1.0){
            if(abs(log10(tmp_Noise[m])-log10(y_mean_Noise[pe+1][l]))<1.0){
              if(abs(log10(tmp_Noise[m])-log10(y_mean_Noise[pe][l])) < abs(log10(tmp_Noise[m])-log10(y_mean_Noise[pe+1][l]))){
                v_Noise[pe+1].push_back(tmp_Noise[m]);
                v_trigth[pe+1].push_back(tmp_trigth[m]);
                v_row[pe+1].push_back(m);
                tmp_Rank[m]=pe+1;
              }else{
                v_Noise[pe+2].push_back(tmp_Noise[m]);
                v_trigth[pe+2].push_back(tmp_trigth[m]);
                v_row[pe+2].push_back(m);
                tmp_Rank[m]=pe+2;
              }
            }else{
              v_Noise[pe+1].push_back(tmp_Noise[m]);
              v_trigth[pe+1].push_back(tmp_trigth[m]);
              v_row[pe+1].push_back(m);
              tmp_Rank[m]=pe+1;
            }
          }else if(abs(log10(tmp_Noise[m])-log10(y_mean_Noise[pe+1][l]))<1.0){
            v_Noise[pe+2].push_back(tmp_Noise[m]);
            v_trigth[pe+2].push_back(tmp_trigth[m]);
            v_row[pe+2].push_back(m);
            tmp_Rank[m]=pe+2;
          }else{
            v_Noise[0].push_back(tmp_Noise[m]);
            v_trigth[0].push_back(tmp_trigth[m]);
            v_row[0].push_back(m);
            tmp_Rank[m]=0;
          }
        }
      }
      for(int i=0;i<10;i++){
        v_Noise[i].clear();
        v_trigth[i].clear();
        v_row[i].clear();
      }

      int id[2]={};
      for(unsigned int m=0; m<size_trigth;m++){
        id[1] = id[0];
        id[0] = tmp_Rank[m];
        if(tmp_Rank[m]<0.) id[0]=0;
        if(id[1]!=0 && id[1]==0){
          for(int i=0;i<1;i++){
            if(m+i+1>=size_trigth)continue;
            if(id[1]==tmp_Rank[m+i+1]){
              id[0]=id[1];
              break;
            } 
          }
        }
        v_Noise[id[0]].push_back(tmp_Noise[m]);
        v_trigth[id[0]].push_back(tmp_trigth[m]);
        v_row[id[0]].push_back(m);
        tmp_Rank[m]=id[0];
        h_Noise_classfied[l][idif][id[0]]->Fill(tmp_trigth[m],tmp_Noise[m]);
      }

      THStack* hs = new THStack(Form("Noise_classfied_dif%d_chip%d_inputDAC%d",idif,ichip,list_inputDAC[l]),Form("Noise_classfied_dif%d_chip%d_inputDAC%d",idif,ichip,list_inputDAC[l]));
      for(int n=0;n<10;n++){
        hs->Add(h_Noise_classfied[l][idif][n]);
      }
      c1->SetLogy(1);
      hs->Draw("nostack HIST P");
      c1->Print(Form("%s/Noise/chip%d/Noise_classified_dif%d_chip%d_inputDAC%d.png",outputIMGDirName.c_str(),ichip,idif+1,ichip,list_inputDAC[l]));
      c1->SetLogy(0);
      delete hs;

      // 6. Calcurate the noise rate range based on above data.
      for(int pe=0;pe<3;pe++){
        for(unsigned int j=0;j<v_Noise[pe+1].size();j++){
          if(j==0){
            summary_Range_Noise_min[l][pe]=v_Noise[pe+1][j]; 
            summary_Range_Noise_max[l][pe]=v_Noise[pe+1][j]; 
          }else{
            if(summary_Range_Noise_min[l][pe] > v_Noise[pe+1][j]) summary_Range_Noise_min[l][pe]=v_Noise[pe+1][j]; 
            if(summary_Range_Noise_max[l][pe] < v_Noise[pe+1][j]) summary_Range_Noise_max[l][pe]=v_Noise[pe+1][j];
          }
        }
      }

      // 7. Fill data to xmlfile.       
      for(int pe=0;pe<=2;pe++){
        pe_center[pe][l]=Calcurate_Mean(v_trigth[pe+1]);
        pe_width[pe][l]=Calcurate_Sigma(v_trigth[pe+1]);
      }
      for(int i=0;i<10;i++){
        v_Noise[i].clear();
        v_trigth[i].clear();
      }
      summary_Range_Gain.clear();
      summary_id_Gain.clear();
    }//inputDAC

    Edit = new wgEditXML();
    string outputxml;
    string name;
    int ich=0;
    outputxml=Form("%s/dif%d/chip%d/ch%d.xml",outputXMLDirName.c_str(),idif+1,ichip,ich);
    Edit->Open(outputxml);
    for(unsigned int l=0;l<size_inputDAC;l++){
      name = "pe_center1";
      Edit->SCURVE_SetValue(name,list_inputDAC[l],pe_center[0][l],0);
      name = "pe_width1";
      Edit->SCURVE_SetValue(name,list_inputDAC[l],pe_width[0][l],0);
      name = "pe_center2";
      Edit->SCURVE_SetValue(name,list_inputDAC[l],pe_center[1][l],0);
      name = "pe_width2";
      Edit->SCURVE_SetValue(name,list_inputDAC[l],pe_width[1][l],0);
      name = "pe_center3";
      Edit->SCURVE_SetValue(name,list_inputDAC[l],pe_center[2][l],1);
      name = "pe_width3";
      Edit->SCURVE_SetValue(name,list_inputDAC[l],pe_width[2][l],1);
    }
    Edit->Write();
    Edit->Close();
    delete Edit;

#ifdef DEBUG_WGCALIB
    TMultiGraph * mg = new TMultiGraph();
    TGraphErrors * g_Range[3];
    for(int pe=0;pe<3;pe++){
      g_Range[pe]  = new TGraphErrors(size_inputDAC,mean_Noise_inputDAC,y_mean_Noise[pe],sigma_Noise_inputDAC,y_sigma_Noise[pe]);
      g_Range[pe]->SetMarkerColor(pe+1);
      g_Range[pe]->SetMarkerSize(1);
      g_Range[pe]->SetMarkerStyle(8);
      //g_Range[pe]->Fit("expo","Q");
      mg->Add(g_Range[pe]);
    }
    mg->Draw("ap");
    c1->SetLogy(1);
    c1->Print(Form("%s/Noise/chip%d/Range_Noise_dif%d_chip%d.png",outputIMGDirName.c_str(),ichip,idif+1,ichip));
    c1->SetLogy(0);
    for(int pe=0;pe<3;pe++){ delete g_Range[pe]; }
    delete mg;
#endif
    for(unsigned int l=0;l<size_inputDAC;l++){
      for(int n=0;n<10;n++){
        delete h_Noise_classfied[l][idif][n];
      }
    }

  }//dif
  cout << "Finish!" << endl;
}

//******************************************************************
void MakeDir(string& str){
  CheckExist *check = new CheckExist;
  if(!check->Dir(str)){
    system(Form("mkdir %s",str.c_str()));
  }
  delete check;
}

//******************************************************************
double NoiseToPe(double noise){
  if(noise>u_limit_1pe){ return 0.5;
  }else if(noise>=l_limit_1pe && noise<u_limit_1pe){ return 1.0;
  }else if(noise>=u_limit_2pe && noise<l_limit_1pe){ return 1.5;
  }else if(noise>=l_limit_2pe && noise<u_limit_2pe){ return 2.0;
  }else if(noise>=u_limit_3pe && noise<l_limit_2pe){ return 2.5;
  }else if(noise>=l_limit_3pe && noise<u_limit_3pe){ return 3.0;
  }else if( noise<l_limit_3pe && noise>0.){ return 3.5;
  }else{  return 0.; } 
}

//******************************************************************
void MakeXML(string& outputXMLDirName,int ichip){
  string name;
  wgEditXML *Edit = new wgEditXML();
  for(int idif=0;idif<2;idif++){
    name=Form("%s/dif%d",outputXMLDirName.c_str(),idif+1);
    MakeDir(name);
    name=Form("%s/dif%d/chip%d",outputXMLDirName.c_str(),idif+1,ichip);
    MakeDir(name);
    for(int ich=0;ich<32;ich++){
      name=Form("%s/dif%d/chip%d/ch%d.xml",outputXMLDirName.c_str(),idif+1,ichip,ich);
      Edit->SCURVE_Make(name);
    }
  }
  delete Edit;
}

//******************************************************************
double Calcurate_Mean(vector<double> v){
  double mean=0;
  double size=v.size();
  double max=0.;
  double min=0.;
  double col[(int)size];
  for(unsigned int i=0;i<v.size();i++){col[i]=0.;}
  if(v.size()==0){return 0.;}
  if(v.size()==1){return v[0];}
  for(unsigned int i=0;i<v.size();i++){
    int rank=0;
    for(unsigned int i2=0;i2<v.size();i2++){
      if(v[i]>v[i2])rank++;
    }
    col[rank]=v[i];
  }
  for(unsigned int i=0;i<v.size()-1;i++){
    if(col[i+1] > col[i]+2){
      size=i;
    }
  }
  max=v[(int)size-1];
  min=v[0];
  mean=((max+min)/2.);
  return mean;
}

//******************************************************************
double Calcurate_Sigma(vector<double> v){
  double mean=0;
  double size=v.size();
  double max=0.;
  double min=0.;
  double col[(int)size];
  for(unsigned int i=0;i<v.size();i++){col[i]=0.;}
  if(v.size()==0){return 0.;}
  if(v.size()==1){return v[0];}
  for(unsigned int i=0;i<v.size();i++){
    int rank=0;
    for(unsigned int i2=0;i2<v.size();i2++){
      if(v[i]>v[i2])rank++;
    }
    col[rank]=v[i];
  }
  for(unsigned int i=0;i<v.size()-1;i++){
    if(col[i+1] > col[i]+2){
      size=i;
    }
  }
  max=v[(int)size-1];
  min=v[0];
  mean=((max-min)/2.);
  return mean;
}
