#ifndef WAGASCI_DISP_HPP_
#define WAGASCI_DISP_HPP_

#include "Const.hpp"

class wgDisp
{
  private:
    double PEpara  = 1.5;
    double PEth    = 0.0;
    double LineWid  = 3.0; 
    double LineWid2 = 1.5; 
  public:
  
  void sci_ing(double x,double y,double x_len,double y_len,double deg);
  void sci_par(double x,double y,double x_len,double y_len,double deg);
  void sci_sci(double x,double y,double x1,double y1);
  void sci_veto(double x,double y,double x_len,double y_len,double deg_veto);
  void iron(double x,double y,double x_len,double y_len,double deg);

  void DrawINGRID(int mod,int view, double x_center, double y_center, double z_center, double deg);
  void DrawProtonModule(int view, double x_center, double y_center, double z_center);
  void DrawWaterModule(int view, double x_center, double y_center, double z_center);
  void DrawPartWaterModule(int view, double x_center, double y_center, double z_center, bool two_palet);
  void DrawHits(int mod, int pln, int view, int ch, double pe, double x_center, double y_center, double z_center, double deg);

  void watertank(double x,double y,double x1, double y1);
  void drawx(int targetmod);
  void drawy(int targetmod);

  void drawpartx(bool two_palet);
  void drawparty(bool two_palet);

  void tline(double iX,double iY,double fX,double fY);
  void drawdotline(double iX,double iY,double fX,double fY);
  void drawtext(const char* text, double x, double y,double font);

  Hit_t type_hit;
  Recon_t type_recon;
  Track_t type_track;
};

#endif // WAGASCI_DISP_HPP_
