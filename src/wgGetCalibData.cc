#include <TROOT.h>

#include "Const.h"
#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgEditXML.h"
#include "wgGetCalibData.h"

//******************************************************************************
void wgGetCalibData::Get_Pedestal(int ndif,double pedestal[20][36][16],double ped_nohit[20][36][16]){

  for(unsigned int i=0;i<NCHIPS;i++){
    int ichip=i;
    for(unsigned int j=0;j<NCHANNELS;j++){
      int ich=j;
      for(unsigned int k=0;k<MEMDEPTH;k++){
        int icol=k;
        ped_nohit[ichip][ich][icol]=1.;
        pedestal[ichip][ich][icol]=1.;
      }
    }
  }

  string pedFileName("");
  wgConst *con = new wgConst();
  con->GetENV();
  pedFileName=Form("%s/pedestal_card.xml",con->CALIBDATA_DIRECTORY);
  delete con; 

  if(ndif!=1 && ndif!=2) return;

  wgEditXML *Edit = new wgEditXML();
  Edit->Open(pedFileName);
  for(unsigned int i=0;i<NCHIPS;i++){
    int ichip=i;
    for(unsigned int j=0;j<NCHANNELS;j++){
      int ich=j;
      if(ich>31){
        for(unsigned int k=0;k<MEMDEPTH;k++){
          int icol=k;
          ped_nohit[ichip][ich][icol]=1.;
          pedestal[ichip][ich][icol]=1.;
        }
      }else{
        for(unsigned int k=0;k<MEMDEPTH;k++){
          int icol=k;
          string name;
          name=Form("ped_nohit_%d",icol);
          ped_nohit[ichip][ich][icol]=Edit->Calib_GetValue(name,ndif,ichip,ich);
          name=Form("ped_%d",icol);
          pedestal[ichip][ich][icol]=Edit->Calib_GetValue(name,ndif,ichip,ich);
        }
      }
    }
  }
  Edit->Close();
  delete Edit;
}

//******************************************************************************
void wgGetCalibData::Get_TdcCoeff(int ndif,double slope[2][20][36],double intcpt[2][20][36])
{
  for(unsigned int i=0;i<NCHIPS;i++){
    int ichip=i;
    for(unsigned int j=0;j<NCHANNELS;j++){
      int ich=j;
      slope[0][ichip][ich]=1.;
      slope[1][ichip][ich]=1.;
      intcpt[0][ichip][ich]=1.;
      intcpt[1][ichip][ich]=1.;
    }
  }

  string pedFileName("");
  wgConst *con = new wgConst();
  con->GetENV();
  pedFileName=Form("%s/tdc_coefficient_card.xml",con->CALIBDATA_DIRECTORY);
  delete con; 

  if(ndif!=1 && ndif!=2) return;

  wgEditXML *Edit = new wgEditXML();
  Edit->Open(pedFileName);
  for(unsigned int i=0;i<NCHIPS;i++){
    int ichip=i;
    for(unsigned int j=0;j<NCHANNELS;j++){
      int ich=j;
      if(ich<32){
        string name;
        name="slope_even";
        slope[0][ichip][ich]=Edit->Calib_GetValue(name,ndif,ichip,ich);
        name="slope_odd";
        slope[1][ichip][ich]=Edit->Calib_GetValue(name,ndif,ichip,ich);
        name="intcpt_even";
        intcpt[0][ichip][ich]=Edit->Calib_GetValue(name,ndif,ichip,ich);
        name="intcpt_odd";
        intcpt[1][ichip][ich]=Edit->Calib_GetValue(name,ndif,ichip,ich);
      }
    }
  }
  Edit->Close();
  delete Edit;
}

//******************************************************************************
void wgGetCalibData::Get_Gain(string& calibFileName,int ndif, double gain[20][36]){
  for(unsigned int i=0;i<NCHIPS;i++){
    int ichip=i;
    for(unsigned int j=0;j<NCHANNELS;j++){
      int ich=j;
      gain[ichip][ich]=1.;
    }
  }

  if(ndif!=1 && ndif!=2) return;

  CheckExist *check  =  new CheckExist;
  if(calibFileName=="" || !check->XmlFile(calibFileName)){
    return;
  }
  delete check;

  wgEditXML *Edit = new wgEditXML();
  Edit->Open(calibFileName);
  for(unsigned int i=0;i<NCHIPS;i++){
    int ichip=i;
    for(unsigned int j=0;j<NCHANNELS;j++){
      int ich=j;
      if(ich>31){
        gain[ichip][ich]=1.;
      }else{
        string name("");
        name=Form("Gain");
        gain[ichip][ich]=Edit->Calib_GetValue(name,ndif,ichip,ich);
      }
    }
  }
  Edit->Close();
  delete Edit;
}

