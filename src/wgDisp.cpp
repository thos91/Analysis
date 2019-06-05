// system C++ includes
#include <iostream>
#include <fstream>
#include <sstream>
#include <complex>

// system C includes
#include "cmath"

// ROOT includes
#include "TROOT.h"
#include "TApplication.h"
#include "TFile.h"
#include "TH1D.h"
#include "TF1.h"
#include "TGraph.h"
#include "TGraphAsymmErrors.h"
#include "TH2D.h"
#include "TStyle.h"
#include "TString.h"
#include "TSystem.h"
#include "TSpectrum.h"
#include "TTree.h"
#include "TArc.h"
#include "TBox.h"
#include "TPolyLine.h"
#include "TLine.h"
#include "TCanvas.h"
#include "TText.h"
#include "TPolyLine.h"

// user includes
#include "Const.hpp"
#include "DetectorConst.hpp"
#include "wgDetectorDimension.hpp"
#include "wgChannelMap.hpp"
#include "wgTools.hpp"
#include "wgErrorCode.hpp"
#include "wgDisp.hpp"

//**************************************************************
void wgDisp::sci_ing(double x,double y,double x_len,double y_len,double deg_ing=0.){
  complex<double> pos_tmp[5] = {
    complex<double>(-x_len/2.,-y_len/2.),
    complex<double>( x_len/2.,-y_len/2.),
    complex<double>( x_len/2., y_len/2.),
    complex<double>(-x_len/2., y_len/2.),
    complex<double>(-x_len/2.,-y_len/2.)
  };
  Double_t x_tmp[5], y_tmp[5];
  for(int i=0;i<5;i++){
    pos_tmp[i] *= exp(complex<double>(0.,-deg_ing));
    x_tmp[i] = x + pos_tmp[i].real();
    y_tmp[i] = y + pos_tmp[i].imag();
  }

  TPolyLine *pline = new TPolyLine(5,x_tmp,y_tmp);
  pline->SetLineStyle(1);
  pline->SetLineColor(kGreen);
  //pline->SetFillStyle(3244);
  pline->SetFillColor(kGreen);
  pline->SetFillStyle(1001);
  pline->SetLineWidth(LineWid);
  pline->Draw("f SAME");
};

//**************************************************************
void wgDisp::sci_par(double x,double y,double x_len,double y_len,double deg_par=0.){
  complex<double> pos_tmp[5] = {
    complex<double>(-x_len/2.,-y_len/2.),
    complex<double>( x_len/2.,-y_len/2.),
    complex<double>( x_len/2., y_len/2.),
    complex<double>(-x_len/2., y_len/2.),
    complex<double>(-x_len/2.,-y_len/2.)
  };
  Double_t x_tmp[5], y_tmp[5];
  for(int i=0;i<5;i++){
    pos_tmp[i] *= exp(complex<double>(0.,-deg_par));
    x_tmp[i] = x + pos_tmp[i].real();
    y_tmp[i] = y + pos_tmp[i].imag();
  }

  TPolyLine *pline = new TPolyLine(5,x_tmp,y_tmp);
  pline->SetLineColor(kBlack);
  pline->SetLineStyle(4);
  pline->SetFillColor(kYellow);
  pline->SetFillStyle(3001);
  pline->SetLineWidth(LineWid2);
  pline->Draw("f SAME");
};

//**************************************************************
void wgDisp::sci_sci(double x,double y,double x1,double y1){
  TBox *b1=new TBox(x,y,x1,y1);
  b1->SetLineColor(1);
  //b1->SetFillStyle(0);
  b1->SetFillStyle(3001);
  b1->SetLineStyle(1);
  //b1->SetLineStyle(0);
  b1->SetLineWidth(LineWid);
  b1->Draw("f SAME");
};

//**************************************************************
void wgDisp::sci_veto(double x,double y,double x_len,double y_len,double deg_veto=0.){

  complex<double> pos_tmp[5] = {
    complex<double>(-x_len/2.,-y_len/2.),
    complex<double>( x_len/2.,-y_len/2.),
    complex<double>( x_len/2., y_len/2.),
    complex<double>(-x_len/2., y_len/2.),
    complex<double>(-x_len/2.,-y_len/2.)
  };
  Double_t x_tmp[5], y_tmp[5];
  for(int i=0;i<5;i++){
    pos_tmp[i] *= exp(complex<double>(0.,-deg_veto));
    x_tmp[i] = x + pos_tmp[i].real();
    y_tmp[i] = y + pos_tmp[i].imag();
  }

  TPolyLine *pline = new TPolyLine(5,x_tmp,y_tmp);
  pline->SetLineColor(kBlue);
  pline->SetFillColor(kBlue);
  pline->SetLineWidth(LineWid);
  pline->SetFillStyle(1001);
  pline->Draw("f SAME");

};

//**************************************************************
void wgDisp::iron(double x,double y,double x_len,double y_len,double deg_iron=0.){
  complex<double> pos_tmp[5] = {
    complex<double>(-x_len/2.,-y_len/2.),
    complex<double>( x_len/2.,-y_len/2.),
    complex<double>( x_len/2., y_len/2.),
    complex<double>(-x_len/2., y_len/2.),
    complex<double>(-x_len/2.,-y_len/2.)
  };
  Double_t x_tmp[5], y_tmp[5];
  for(int i=0;i<5;i++){
    pos_tmp[i] *= exp(complex<double>(0.,-deg_iron));
    x_tmp[i] = x + pos_tmp[i].real();
    y_tmp[i] = y + pos_tmp[i].imag();
  }

  TPolyLine *pline = new TPolyLine(5,x_tmp,y_tmp);
  pline->SetFillColor(17);
  pline->SetLineWidth(LineWid);
  pline->Draw("f SAME");
};


//**************************************************************
void wgDisp::watertank(double x,double y,double x1,double y1){
  TBox *b1=new TBox(x,y,x1,y1);
  b1->SetFillColor(kBlack);
  b1->SetFillStyle(0);
  b1->SetLineWidth(LineWid);
  b1->Draw("f SAME");
};

//**************************************************************
void wgDisp::DrawProtonModule(int view, double x_center, double y_center, double z_center){
#ifdef DEBUG_DISP
  cout << "DrawProtonModule" << endl;
#endif
  int ch,ch_num;
  double x,y,z,xy; 
  wgDetectorDimension *detdim = new wgDetectorDimension();

  for(int pln=0;pln<22;pln++){
    if(pln==0) ch_num = 24;
    else if(pln<18) ch_num = 32;
    else if(pln<22) ch_num = 17;
    for(ch=0;ch<ch_num;ch++){

      detdim->GetPosPM(pln,view,ch,&x,&y,&z);
      x = x + x_center;
      y = y + y_center;
      z = z + z_center;
      if(view==SideView_i) xy = y;
      else        xy = x;
      if (pln<18){
        if(pln==0||ch<8||ch>=24) sci_ing(z,xy,C_INGScintiThick,C_INGScintiWidth);
        else                     sci_ing(z,xy,C_PMScintiThick ,C_PMScintiWidth);
      }
      else if((view==SideView_i&&(pln==18||pln==20))||
          (view==TopView_i&&(pln==19||pln==21)))
        sci_veto(z,xy,C_INGScintiWidth,C_INGScintiThick);
    }
  }
  
  int counter_view;
  if(view==SideView_i) counter_view=TopView_i; 
  else counter_view=SideView_i;
  for(int pln=0;pln<22;pln++){
    if(pln<18){
      detdim->GetPosPM(pln,counter_view,0,&x,&y,&z);
      x = x + x_center;
      y = y + y_center;
      z = z + z_center;	  
      if(view==SideView_i) xy = y; else xy = x;
      sci_par(z,xy,C_PMScintiThick,C_PMScintiLength);
    }
  }
}

//**************************************************************
void wgDisp::DrawINGRID(int mod, int view, double x_center, double y_center, double z_center, double deg=0.){
#ifdef DEBUG_DISP
  cout << "DrawINGRID" << endl;
#endif
  int ch,ch_num;
  double x,y,z,xy,rotate; 
  int sideview = SideView_i;
  int topview  = TopView_i;
  if(mod==14){ //B2 INGRID
    sideview = SideView; 
    topview  = TopView; 
  }

  wgDetectorDimension *detdim = new wgDetectorDimension();
  rotate = deg*PI/180.;

  for(int pln=0;pln<15;pln++){
    if(pln<11) ch_num = 24;
    else if(pln<15) ch_num = 22;
    for(ch=0;ch<ch_num;ch++){

      detdim->GetPosING(mod,pln,view,ch,&x,&y,&z);
      //x_tmp = cos(rotate)*x - sin(rotate)*z; 
      //z_tmp = sin(rotate)*x + cos(rotate)*z; 
      x = x + x_center;
      y = y + y_center;
      z = z + z_center;
      if(view==sideview) xy = y; else xy = x;
      if  (pln<11){
        sci_ing(z,xy,C_INGScintiThick,C_INGScintiWidth,rotate);
      }
      else if((view==sideview&&(pln==13||pln==14))||
              (view==topview &&(pln==11||pln==12)))
        sci_veto(z,xy,C_INGScintiWidth,C_INGScintiThick,rotate);
    }
  }

  int counter_view = 1 - view;
  for(int pln=0;pln<15;pln++){
    if(pln<11){
      detdim->GetPosING(mod,pln,counter_view,0,&x,&y,&z);
      //x_tmp = cos(rotate)*x - sin(rotate)*z; 
      //z_tmp = sin(rotate)*x + cos(rotate)*z; 
      x = x + x_center;
      y = y + y_center;
      z = z + z_center;	  
      if(view==sideview) xy = y; else xy = x;

      sci_par(z,xy,C_INGScintiThick,C_INGScintiLength,rotate);

      if(pln<9){
        detdim->GetPosING(mod,pln,counter_view,0,&x,&y,&z);
        //x_tmp = cos(rotate)*x - sin(rotate)*z; 
        //z_tmp = sin(rotate)*x + cos(rotate)*z; 
        //x_tmp = x_tmp - sin(rotate)*(C_INGIronStart - C_INGPlnStart);
        //z_tmp = z_tmp + cos(rotate)*(C_INGIronStart - C_INGPlnStart);
        x = x + x_center;
        y = y + y_center;
        z = z + z_center + (C_INGIronStart - C_INGPlnStart);	  
        if(view==sideview) xy = y; else xy = x;

        iron(z,xy,C_INGIronThick,C_INGIronXY,rotate);
      }
    }
  }
}

//**************************************************************
void wgDisp::DrawWaterModule(int view, double x_center, double y_center, double z_center){
  int ch;
  double x,y,z,xy; 
  double z1,z2,xy1,xy2; 
  wgDetectorDimension *detdim = new wgDetectorDimension();

  z1 = z_center-C_WMWaterTargetSizeZ/2.;
  z2 = z_center+C_WMWaterTargetSizeZ/2.;
  if(view==SideView){
    xy1 = y_center-C_WMWaterTargetSizeY/2.;
    xy2 = y_center+C_WMWaterTargetSizeY/2.;
  }else if(view==TopView){
    xy1 = x_center-C_WMWaterTargetSizeX/2.;
    xy2 = x_center+C_WMWaterTargetSizeX/2.;
  }

  watertank(z1,xy1,z2,xy2);

  for(int pln=0;pln<8;pln++){
    for(ch=0;ch<40;ch++){

      detdim->GetPosWM(pln,view,ch,0,&x,&y,&z);
      x = x + x_center;
      y = y + y_center;
      z = z + z_center;
      if(view==SideView) xy = y;
      else        xy = x;

      sci_ing(z,xy,C_WMScintiThick,C_WMScintiWidth,0.);

      if(ch<20){
        detdim->GetPosWM(pln,view,ch+40,1,&x,&y,&z);
        x = x + x_center;
        y = y + y_center;
        z = z + z_center;
        if(view==SideView) xy = y;
        else        xy = x;
        sci_ing(z,xy,C_WMScintiWidth,C_WMScintiThick,0.);

        detdim->GetPosWM(pln,view,ch+60,2,&x,&y,&z);
        x = x + x_center;
        y = y + y_center;
        z = z + z_center;
        if(view==SideView) xy = y;
        else        xy = x;
        sci_ing(z,xy,C_WMScintiWidth,C_WMScintiThick,0.);
      }
    }
  }

  int counter_view;
  if(view==SideView) counter_view=TopView; 
  else counter_view=SideView;

  for(int pln=0;pln<8;pln++){
    detdim->GetPosWM(pln,counter_view,0,0,&x,&y,&z);
    x = x + x_center;
    y = y + y_center;
    z = z + z_center;	  
    if(view==SideView)xy = y;
    else        xy = x;
    sci_par(z,xy,C_WMScintiThick,C_WMScintiLength,0.);
  }
  delete detdim;
}

//**************************************************************
//Draw a part of WaterModule.
void wgDisp::DrawPartWaterModule(int view, double x_center, double y_center, double z_center, bool two_palet){
  int ch,ch_num_plane,ch_num_grid;
  double x,y,z,xy; 
  wgDetectorDimension *detdim = new wgDetectorDimension();

  for(int pln=0;pln<2;pln++){
    if(two_palet){ch_num_plane=8; ch_num_grid=4;}
    else{ch_num_plane=16; ch_num_grid=8;}
    for(ch=0;ch<ch_num_plane;ch++){
      detdim->GetPosWM(pln,view,ch,0,&x,&y,&z);
      x = x + x_center;
      y = y + y_center;
      z = z + z_center;
      if(view==1) xy = y;
      else        xy = x;

      sci_ing(z,xy,C_WMScintiThick,C_WMScintiWidth,0.);

      if(ch<ch_num_grid){
        detdim->GetPosWM(pln,view,ch+40,1,&x,&y,&z);
        x = x + x_center;
        y = y + y_center;
        z = z + z_center;
        if(view==1) xy = y;
        else        xy = x;
        sci_ing(z,xy,C_WMScintiWidth,C_WMScintiThick,0.);

        detdim->GetPosWM(pln,view,ch+60,2,&x,&y,&z);
        x = x + x_center;
        y = y + y_center;
        z = z + z_center;
        if(view==1) xy = y;
        else        xy = x;
        sci_ing(z,xy,C_WMScintiWidth,C_WMScintiThick,0.);
      }
    }

    detdim->GetPosWM(pln,1-view,0,0,&x,&y,&z);
    x = x + x_center;
    y = y + y_center;
    z = z + z_center;	  
    if(view==1) xy = y;
    else        xy = x;

    sci_par(z,xy,C_WMScintiThick,220,0.);
  }

  delete detdim;
}


//**************************************************************
void wgDisp::DrawHits(int mod, int pln, int view, int ch, double pe, double x_center, double y_center, double z_center, double deg){
  double X=0.,Y=0.,Z=0.,R=0., XY=0.;
  double X_tmp,Z_tmp;
  double rotate = deg*PI/180.;
  wgDetectorDimension *detdim = new wgDetectorDimension();
  detdim->GetPosInMod(mod,pln,view,ch,&X,&Y,&Z);
  delete detdim;
  X_tmp = cos(rotate)*X - sin(rotate)*Z; 
  Z_tmp = sin(rotate)*X + cos(rotate)*Z; 

  X = X_tmp + x_center;
  Y = Y     + y_center;
  Z = Z_tmp + z_center;
  if(view==1) XY=Y;
  else        XY=X;

  if(pe<PEth)R=0.;
  else R=sqrt(pe-PEth)*PEpara;

  TArc *arc=new TArc(Z,XY,R);
  arc->SetFillColor(kRed);
  arc->SetLineColor(kRed);
  arc->SetLineWidth(0.1);
  arc->Draw("SAME");
}


//**************************************************************
//sideview
void wgDisp::drawx(int targetmod){
  int modules[3];
  double offset_x,offset_y,offset_z;

  if(targetmod==7||targetmod==8){
    modules[0] = 21;
    modules[1] = 22;
    modules[2] = 23;
  }
  else if(targetmod==5){
    modules[0] = 20;
    modules[1] = 3;
    modules[2] = -1;
  }

  for(int mod=0;mod<1;mod++){
    if(modules[mod]==3){
      offset_x = C_INGHMotherPosX;
      offset_y = C_INGHMotherPosY;
      offset_z = C_INGHMotherPosZ;
      DrawINGRID(3,0,offset_x,offset_y,offset_z);
    }
    else if(modules[mod]==20){
      offset_x = C_PMMotherPosX;
      offset_y = C_PMMotherPosY;
      offset_z = C_PMMotherPosZ;
      DrawWaterModule(0,offset_x,offset_y,offset_z);
    }
    else if(modules[mod]==21){
      //offset_x = C_B2WMPosX;
      //offset_y = C_B2WMPosY;
      //offset_z = C_B2WMPosZ;
      offset_x = 0;
      offset_y = 0;
      offset_z = 0;
      DrawWaterModule(0,offset_x,offset_y,offset_z);
    }
    else if(modules[mod]==22){
      offset_x = C_B2CHPosX;
      offset_y = C_B2CHPosY;
      offset_z = C_B2CHPosZ;
      DrawProtonModule(0,offset_x,offset_y,offset_z);
    }
    else if(modules[mod]==23){
      offset_x = C_B2d1INGPosX;
      offset_y = C_B2d1INGPosY;
      offset_z = C_B2d1INGPosZ;
      DrawINGRID(8,0,offset_x,offset_y,offset_z);
    }
  }
};

//**************************************************************
//topview
void wgDisp::drawy(int targetmod){
  int modules[3];
  double offset_x,offset_y,offset_z;

  if(targetmod==7||targetmod==8){
    modules[0] = 21;
    modules[1] = 22;
    modules[2] = 23;
  }
  else if(targetmod==5){
    modules[0] = 20;
    modules[1] = 3;
    modules[2] = -1;
  }

  for(int mod=0;mod<1;mod++){
    if(modules[mod]==3){
      offset_x = C_INGHMotherPosX;
      offset_y = C_INGHMotherPosY;
      offset_z = C_INGHMotherPosZ;
      DrawINGRID(3,1,offset_x,offset_y,offset_z);
    }
    if(modules[mod]==20){
      offset_x = C_PMMotherPosX;
      offset_y = C_PMMotherPosY;
      offset_z = C_PMMotherPosZ;
      DrawWaterModule(1,offset_x,offset_y,offset_z);
    }
    if(modules[mod]==21){
      //offset_x = C_B2WMPosX;
      //offset_y = C_B2WMPosY;
      //offset_z = C_B2WMPosZ;
      offset_x = 0;
      offset_y = 0;
      offset_z = 0;
      DrawWaterModule(1,offset_x,offset_y,offset_z);
    }
    if(modules[mod]==22){
      offset_x = C_B2CHPosX;
      offset_y = C_B2CHPosY;
      offset_z = C_B2CHPosZ;
      DrawProtonModule(1,offset_x,offset_y,offset_z);
    }
    if(modules[mod]==23){
      offset_x = C_B2d1INGPosX;
      offset_y = C_B2d1INGPosY;
      offset_z = C_B2d1INGPosZ;
      DrawINGRID(8,1,offset_x,offset_y,offset_z);
    }
  }
};

//**************************************************************
void wgDisp::drawpartx(bool two_palet){
  double offset_x,offset_y,offset_z;
  offset_x = 0;
  offset_y = 0;
  offset_z = 0;
  DrawPartWaterModule(0,offset_x,offset_y,offset_z,two_palet);
};

//**************************************************************
void wgDisp::drawparty(bool two_palet){
  double offset_x,offset_y,offset_z;
  offset_x = 0;
  offset_y = 0;
  offset_z = 0;
  DrawPartWaterModule(1,offset_x,offset_y,offset_z,two_palet);
};

//*************************************************************
void wgDisp::tline(double iX,double iY,double fX,double fY){
  TLine *l1=new TLine(iX,iY,fX,fY);
  l1->SetLineWidth(LineWid2);
  l1->Draw("SAME");
};

//**************************************************************
void wgDisp::drawdotline(double iX,double iY,double fX,double fY){
  TLine *l1=new TLine(iX,iY,fX,fY);
  l1->SetLineStyle(2);
  l1->SetLineColor(kBlue);
  l1->SetLineWidth(LineWid2);
  l1->Draw("SAME");
};

//**************************************************************
void wgDisp::drawtext(const char* text, double x, double y,double font){
  TText *t1 = new TText(x,y,text);
  t1->SetTextSize(font);  
  t1->Draw("SAME");
}
