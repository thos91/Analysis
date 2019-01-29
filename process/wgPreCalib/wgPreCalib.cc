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

vector<string> GetIncludeFileName(string& inputDirName);
void MakeDir(string& str);
void AnaXML(vector<string> &inputDirName,string& outputXMLDirName,string& outputIMGDirName,int mode);
double cal_mean(vector<double>);

int main(int argc, char** argv){
  int opt;
  int mode=0;
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

  while((opt = getopt(argc,argv, "f:o:i:h:m:")) !=-1 ){
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
      case 'i':
        outputIMGDirName = optarg; 
        break;
     
      case 'm':
        mode = atoi(optarg); 
        break;

      case 'h':
        cout <<"this program is for calibration from the data of wgAnaHistSummary. "<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose input directory you wanna read(must)"<<endl;
        cout <<"  -o (char*) : choose output directory (default: input directory) "<<endl;
        cout <<"  -i (char*) : choose output image directory (default: image directory) "<<endl;
        cout <<"  -m (int)   : 0:use only 2pe. 1:use 1pe and 2pe. (default:0)"<<endl;
        exit(0);
      default:
        cout <<"this program is for calibration from the data of wgAnaHistSummary. "<<endl;
        cout <<"you can take several option..."<<endl;
        cout <<"  -h         : help"<<endl;
        cout <<"  -f (char*) : choose input directory you wanna read(must)"<<endl;
        cout <<"  -o (char*) : choose output directory (default: input directory) "<<endl;
        cout <<"  -i (char*) : choose output image directory (default: image directory) "<<endl;
        cout <<"  -m (int)   : 0:use only 2pe. 1:use 1pe and 2pe. (default:0)"<<endl;
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
    outputXMLDirName = Form("%s/%s",con->CALIBDATA_DIRECTORY,OpStr->GetName(inputDirName).c_str());
    MakeDir(outputXMLDirName);
  } 
  //outputIMGDirName = Form("%s/%s/image",con->CALIBDATA_DIRECTORY,OpStr->GetName(inputDirName).c_str());
  MakeDir(outputIMGDirName);
  for(int idif=0;idif<2;idif++){
    string str("");
    str=Form("%s/dif%d",outputIMGDirName.c_str(),idif+1);
    MakeDir(str);
  }

  Log->Write(Form("READING DIRECTORY : %s",inputDirName.c_str()));
  Log->Write(Form("OUTPUT DIRECTORY : %s",outputXMLDirName.c_str()));

  cout << " *****  READING DIRECTORY     :" << inputDirName << "  *****" << endl;
  cout << " *****  OUTPUT XML DIRECTORY :" << outputXMLDirName << "  *****" << endl;
  cout << " *****  OUTPUT IMAGE DIRECTORY :" << outputIMGDirName << "  *****" << endl;

  delete check;
  delete OpStr;

  vector<string> ReadFile = GetIncludeFileName(inputDirName); 
  cout << " Finish  reading file"<<endl;

  AnaXML(ReadFile,
      outputXMLDirName,
      outputIMGDirName,
      mode
      );
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
    string name=entry->d_name;
    if((entry->d_name[0])!='.' && name.size()>8){
      openxmlfile.push_back(Form("%s/%s",inputDirName.c_str(),entry->d_name));
      cout << "ReadFile : " << inputDirName << "/" << entry->d_name << endl;
    }
  }
  closedir(dp);
  return openxmlfile;
} 


//******************************************************************
void AnaXML(vector<string> &inputFileName, string& outputXMLDirName,string& outputIMGDirName,int mode){

  int FN=inputFileName.size();
  /*
  if(FN%2!=0){
    cout << "!! ERROR !! : the number of data is not enough to calbration!!" << endl;
    return;
  }
  */
  int inputDAC[FN];
  string xmlfile("");
  string name("");
  wgEditXML *Edit = new wgEditXML();
  for(int iFN=0;iFN<FN;iFN++){
    xmlfile=Form("%s/Summary_chip%d.xml",inputFileName[iFN].c_str(),0);
    Edit->Open(xmlfile);
    name="inputDAC";
    inputDAC[iFN]=Edit->SUMMARY_GetChConfigValue(name,0);
    Edit->Close();
  }

  // ****  list up inputDAC **** //
  vector<int> list_inputDAC;
  bool add_list_inputDAC=false;
  list_inputDAC.push_back(inputDAC[0]);
  for(int iFN=1;iFN<FN;iFN++){
    add_list_inputDAC=true;
    for(unsigned int l=0;l<list_inputDAC.size();l++){
      if(list_inputDAC[l]==inputDAC[iFN]){
        add_list_inputDAC=false;
      }
    }
    if(add_list_inputDAC)list_inputDAC.push_back(inputDAC[iFN]);
  }
  sort(list_inputDAC.begin(), list_inputDAC.end());

  unsigned int size_inputDAC = list_inputDAC.size();
  /*
  if((int)size_inputDAC*2!=FN){
    cout << "!! ERROR !! : the number of data is not enough to calbration!!" << endl;
    return;
  }
  */
  int ndif=0;
  int npe=0;
  double Gain[2][NCHIPS][32][size_inputDAC][2];
  double Pedestal[2][NCHIPS][32][size_inputDAC][2][16];

  cout << "  ~~~ Start Reading ~~~  " <<endl;
  //*** Read data ***
  for(int iFN=0;iFN<FN;iFN++){
    for(unsigned int i=0;i<NCHIPS;i++){
      int ichip=i;
      int pos = inputFileName[iFN].find("dif_1_1_")+8;
      if(inputFileName[iFN][pos]=='1'){
        ndif=0;
      }else if(inputFileName[iFN][pos]=='2'){
        ndif=1;
      }

      pos = inputFileName[iFN].find("pe")+2;
      if(inputFileName[iFN][pos]=='1'){
        npe=0;
      }else if(inputFileName[iFN][pos]=='2'){
        npe=1;
      }

      xmlfile=Form("%s/Summary_chip%d.xml",inputFileName[iFN].c_str(),ichip);
      Edit->Open(xmlfile);
      name="inputDAC";
      inputDAC[iFN]=Edit->SUMMARY_GetChConfigValue(name,0);
      int list=0;
      for(unsigned int l=0;l<size_inputDAC;l++){
        if(list_inputDAC[l]==inputDAC[iFN])break;
        list++;
      }
      for(unsigned int j=0;j<32;j++){
        int ich=j;
        // TODO this gain should be twice as big as true gain.
        name="Gain";
        Gain[ndif][ichip][ich][list][npe]=Edit->SUMMARY_GetChFitValue(name,ich);
        if(mode==1){
          Edit->SUMMARY_GetPedFitValue(Pedestal[ndif][ichip][ich][list][npe],ich);
        }
      }
      Edit->Close();
    }
  }
  delete Edit;

  double slope[2][NCHIPS][32];
  double inter[2][NCHIPS][32];

  for(int idif=0;idif<2;idif++){
    for(unsigned int i=0;i<NCHIPS;i++){
      int ichip=i;

      TMultiGraph * mg = new TMultiGraph();
      TGraphErrors * g_Dist[32]; 
      TGraph * g_Gain0[32];
      TGraph * g_Gain1[32];
      TGraph * g_ped[16];

      TCanvas *c1 = new TCanvas("c1","c1");
      if(mode==1){
        c1->Divide(4,4);
        for(int k=0;k<16;k++){
          int icol=k;
          double x_ch[32*size_inputDAC];
          double y_ped[32*size_inputDAC];
          for(unsigned int l=0;l<size_inputDAC;l++){
            for(int j=0;j<32;j++){
              int ich=j;
              x_ch[ich+32*l]=ich;
              y_ped[ich+32*l]=Pedestal[idif][ichip][ich][l][1][icol];
            }
          }

          g_ped[k] = new TGraph(32*2*size_inputDAC,x_ch,y_ped);
          g_ped[k]->SetMarkerColor(880+k);
          g_ped[k]->SetMarkerSize(0.5);
          g_ped[k]->SetMaximum(700);
          g_ped[k]->SetMinimum(350);
          g_ped[k]->SetTitle(Form("chip%d col%d;ch;ped",ichip,icol));
          c1->cd(k+1);
          c1->SetGrid();
          g_ped[k]->Draw("ap");
        }
        c1->Print(Form("%s/dif%d/ped_chip%d.png",outputIMGDirName.c_str(),idif+1,ichip));
        delete c1;
        for(int k=0;k<16;k++){delete g_ped[k];}
      }

      double mean_Dist[size_inputDAC];
      for(int j=0;j<32;j++){
        int ich=j;
        for(unsigned int l=0;l<size_inputDAC;l++){
          if(mode==0){
            mean_Dist[l] += (Gain[idif][ichip][ich][l][1])/2./32.;
          }else if(mode==1){
            mean_Dist[l] += (Gain[idif][ichip][ich][l][1]-Gain[idif][ichip][ich][l][0])/32.;
          }
        }
      }

      for(int j=0;j<32;j++){
        int ich=j;
        double x_inputDAC[size_inputDAC];
        double ex_inputDAC[size_inputDAC];
        double y_Dist[size_inputDAC];
        double ey_Dist[size_inputDAC];
        double y_Gain0[size_inputDAC];
        double y_Gain1[size_inputDAC];
        for(unsigned int l=0;l<size_inputDAC;l++){
          x_inputDAC[l]=list_inputDAC[l];
          ex_inputDAC[l]=1.;
          if(mode==0){
            if(mean_Dist[l]+6. > Gain[idif][ichip][ich][l][1]/2.  && mean_Dist[l]-6.<Gain[idif][ichip][ich][l][1]/2.){
              y_Dist[l]=Gain[idif][ichip][ich][l][1]/2.;
              ey_Dist[l]=0.5;
            }else{
              y_Dist[l]=Gain[idif][ichip][ich][l][1]/2.;
              ey_Dist[l]=20.;
            }
          }else if(mode==1){
            if(mean_Dist[l]+6. > Gain[idif][ichip][ich][l][1]-Gain[idif][ichip][ich][l][0]  && mean_Dist[l]-6.<Gain[idif][ichip][ich][l][1]-Gain[idif][ichip][ich][l][0]){
              y_Dist[l]=Gain[idif][ichip][ich][l][1]-Gain[idif][ichip][ich][l][0];
              ey_Dist[l]=0.5;
            }else{
              y_Dist[l]=Gain[idif][ichip][ich][l][1]-Gain[idif][ichip][ich][l][0];
              ey_Dist[l]=20.;
            }
            y_Gain0[l]=Gain[idif][ichip][ich][l][0];    
            y_Gain1[l]=Gain[idif][ichip][ich][l][1];    
          }
        }
        g_Dist [ich] = new TGraphErrors(size_inputDAC,x_inputDAC,y_Dist,ex_inputDAC,ey_Dist);
        TF1 *f_Dist  = new TF1("f_Dist","[0]*x+[1]");
        f_Dist->SetLineColor(kGreen);
        g_Dist [ich]->Fit("f_Dist","Q+ E","same"); 
        g_Dist [ich]->SetMarkerColor(632);
        g_Dist [ich]->SetMarkerSize(1);
        g_Dist [ich]->SetMarkerStyle(8);
        mg->Add(g_Dist[ich]);

        if(mode==1){
          g_Gain0[ich] = new TGraph(size_inputDAC,x_inputDAC,y_Gain0);
          g_Gain0[ich]->SetMarkerColor(600);
          g_Gain0[ich]->SetMarkerSize(1);
          g_Gain0[ich]->SetMarkerStyle(8);
          g_Gain1[ich] = new TGraph(size_inputDAC,x_inputDAC,y_Gain1);
          g_Gain1[ich]->SetMarkerColor(616);
          g_Gain1[ich]->SetMarkerSize(1);
          g_Gain1[ich]->SetMarkerStyle(8);
          mg->Add(g_Gain0[ich]);
          mg->Add(g_Gain1[ich]);
        }

        slope[idif][ichip][ich]=f_Dist->GetParameter(0);
        inter[idif][ichip][ich]=f_Dist->GetParameter(1);
      }
      c1 = new TCanvas("c1","c1");

      mg->SetTitle(Form("chip%d;inputDAC;gain",ichip));
      mg->Draw("ap");
      c1->Print(Form("%s/dif%d/chip%d.png",outputIMGDirName.c_str(),idif+1,ichip));
      for(int ich=0;ich<32;ich++){
        delete g_Dist [ich];
        if(mode==1){
          delete g_Gain0[ich];
          delete g_Gain1[ich];
        }
      }
      delete mg;
      delete c1;
    }
  }

  Edit = new wgEditXML();
  xmlfile=Form("%s/calibration_card.xml",outputXMLDirName.c_str());
  Edit->PreCalib_Make(xmlfile);
  Edit->Open(xmlfile);
  for(int idif=0;idif<2;idif++){
    for(unsigned int i=0;i<NCHIPS;i++){
      int ichip=i;
      for(int j=0;j<32;j++){
        int ich=j;
        name= Form("s_Gain");
        Edit->PreCalib_SetValue(name,idif+1,ichip,ich,slope[idif][ichip][ich],0);
        name= Form("i_Gain");
        Edit->PreCalib_SetValue(name,idif+1,ichip,ich,inter[idif][ichip][ich],0);
        if(mode==1){
          for(int k=0;k<16;k++){
            vector<double> v_ped;
            for(int pe=0;pe<1;pe++){
              for(int l=0;l<(int)size_inputDAC;l++){
                if( Pedestal[idif][ichip][ich][l][0][k]<350 
                    || Pedestal[idif][ichip][ich][l][0][k]>680)continue;
                v_ped.push_back(Pedestal[idif][ichip][ich][l][0][k]);
              }
            }
            unsigned int v_ped_size=v_ped.size();
            double v_ped_sum = 0.;
            for(unsigned int m=0;m<v_ped_size;m++){
              v_ped_sum += v_ped[m];
            }
            double v_ped_mean = v_ped_sum/v_ped_size;
            name= Form("ped_%d",k);
            Edit->PreCalib_SetValue(name,idif+1,ichip,ich,v_ped_mean,1);
          }
        }
      }
    }
  }
  Edit->Write();
  Edit->Close();
  delete Edit;
}

//******************************************************************
void MakeDir(string& str){
  CheckExist *check = new CheckExist;
  if(!check->Dir(str)){
    system(Form("mkdir %s",str.c_str()));
  }
  delete check;
}

