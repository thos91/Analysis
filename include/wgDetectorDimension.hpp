#ifndef wgDetectorDimension_H_INCLUDE
#define wgDetectorDimension_H_INCLUDE

// system includes
#include <iostream>
#include <sstream>
#include <fstream>

// user includes
#include "Const.hpp"


using namespace std;

class wgDetectorDimension{
private:

public:
  wgDetectorDimension(){};

  ~wgDetectorDimension(){};

  //Position in World
  void GetPos(int mod, int pln, int view, int ch, double *x, double *y, double *z);
  void GetPosInMod(int mod, int pln, int view, int ch, double *x, double *y, double *z);

  //Position in each mother volume
  void GetPosING(int mod, int pln, int view, int ch, double *x, double *y, double *z);
  void GetPosPM(int pln, int view, int ch, double *x, double *y, double *z);
  void GetPosWM(int pln, int view, int ch, int grid, double *x, double *y, double *z);


  // WaterModule channel rearrangement
  bool GetWMGridCh(int pln, int view, int ch, int *grid, int *gridch);
  bool GetWMGridCellID(int pln, int view, int ch, double posx, double posy, double posz,
				int* gridcell_id_x1, int* gridcell_id_x2, int* gridcell_id_y1, int* gridcell_id_y2);
  void GetScintiID(int mod, int view, int pln, int gridcell_id, int* cross_n, int* ch);

  int INGNumVetoPln;

  //For TwoDimRecon
  bool  GetReconPlnCh(int mod, int view, int pln, int ch, int axis,
			                      int* reconpln, int* reconch);
  bool  GetRawPlnChGrid(int mod, int view, int reconpln, int reconch, int axis,
			                      int* pln, int* gridch, int* grid);
  int   GetChMax(int mod, int view, int pln, int axis);
  int   GetPlnMax(int mod, int view, int pln, int axis);
  bool  GetPos_TwoDimRecon(int mod, int view, int reconpln, int reconch, int axis,
	   		                                double* posxy, double* posz);
  double GetScintiWidth(int mod, int view, int reconpln, int reconch, int axis);
  double GetScintiThick(int mod, int view, int reconpln, int reconch, int axis);

};
#endif

