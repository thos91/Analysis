#ifndef WG_CHANNELMAP_H_INCLUDE
#define WG_CHANNELMAP_H_INCLUDE

#include <string>
#include "Const.h"
#include "DetectorConst.h"

#define NumView 2
#define NumPln 8
#define NumCh 80

using namespace std;

typedef struct Map
{
  int            view[NumDif][NumChip][NumChipCh];
  int            pln [NumDif][NumChip][NumChipCh];
  int            ch  [NumDif][NumChip][NumChipCh];
  int            grid[NumDif][NumChip][NumChipCh];
  double         x   [NumDif][NumChip][NumChipCh];
  double         y   [NumDif][NumChip][NumChipCh];
  double         z   [NumDif][NumChip][NumChipCh];
} Map_t;

typedef struct MapInv
{
  int            dif   [NumView][NumPln][NumCh];
  int            chip  [NumView][NumPln][NumCh];
  int            chipch[NumView][NumPln][NumCh];
  double         x     [NumView][NumPln][NumCh];
  double         y     [NumView][NumPln][NumCh];
  double         z     [NumView][NumPln][NumCh];
} MapInv_t;

typedef struct ReconMap
{
  public:
    int recon_view[NumReconAxis][NumView][NumPln][NumCh];
    int recon_pln [NumReconAxis][NumView][NumPln][NumCh];
    int recon_ch  [NumReconAxis][NumView][NumPln][NumCh];
} ReconMap_t;

class wgChannelMap
{
  public:
    int  EASIROCtoPIN(int ch);
    string  PINtoMPPC(int pin);
    int  PINtoSIDE(string& pos, int nbund, int layer, int &pln, int &ch);
    int  PINtoTOP(string& pos, int nbund, int layer, int &pln, int &ch);

  public:
    void EASIROCtoSIDE(int easiroc_ch, int &pln, int &ch);
    void EASIROCtoTOP(int easiroc_ch, int &pln, int &ch);
  private:
    static const int EASIROC_pln[32][2];
    static const int EASIROC_ch[32][2];
  private:
    static const int    DifView    [NumDif];
    static const string MPPCch     [NumChipCh];
    static const int    ChipAlloc  [NumDif][NumChip][NumView];
 
  public:
    bool ChipchToMPPCch(int chip_ch, string* mppc_ch);
    bool GetViewPlnCh(int dif_id,int chip_id,int chip_ch,int* view,int* pln,int* ch,int* grid);
    bool GetMPPCPlnCh(string mppc_ch,int view,int* pln,int*ch,int *grid);
    bool GetXYZ(int view, int pln, int ch, double *x, double *y, double *z);
    bool GetChipAlloc(int dif_id,int chip_id,int* id_z,int* id_xy);
    bool GetMap(const int dif_id, const int chip_id,int *view, int* pln, int* ch, int* grid, double *x, double* y, double* z);

  public:
    Map_t load_mapping();
    MapInv_t load_mapping_inv();
    ReconMap_t load_reconmap();

};

#endif

