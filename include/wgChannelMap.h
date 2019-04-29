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
  Map();
  Map(size_t n_difs, size_t n_chips, size_t n_chans);
  i3vector view; // [NumDif][NumChip][NumChipCh];
  i3vector pln;  // [NumDif][NumChip][NumChipCh];
  i3vector ch;   // [NumDif][NumChip][NumChipCh];
  i3vector grid; // [NumDif][NumChip][NumChipCh];
  f3vector x;    // [NumDif][NumChip][NumChipCh];
  f3vector y;    // [NumDif][NumChip][NumChipCh];
  f3vector z;    // [NumDif][NumChip][NumChipCh];
} Map_t;

typedef struct MapInv
{
  MapInv();
  MapInv(size_t n_views, size_t n_plns, size_t n_chans);
  i3vector dif;    // [NumView][NumPln][NumCh];
  i3vector chip;   // [NumView][NumPln][NumCh];
  i3vector chipch; // [NumView][NumPln][NumCh];
  f3vector x;      // [NumView][NumPln][NumCh];
  f3vector y;      // [NumView][NumPln][NumCh];
  f3vector z;      // [NumView][NumPln][NumCh];
} MapInv_t;

typedef struct ReconMap
{
  ReconMap();
  ReconMap(size_t n_axes, size_t n_views, size_t n_plns, size_t n_chans);
  i4vector recon_view;  // [NumReconAxis][NumView][NumPln][NumCh];
  i4vector recon_pln;   // [NumReconAxis][NumView][NumPln][NumCh];
  i4vector recon_ch;    // [NumReconAxis][NumView][NumPln][NumCh];
} ReconMap_t;

class wgChannelMap
{
public:
  int  SPIROCtoPIN(int ch);
  string  PINtoMPPC(int pin);
  int  PINtoSIDE(string& pos, int nbund, int layer, int &pln, int &ch);
  int  PINtoTOP(string& pos, int nbund, int layer, int &pln, int &ch);

public:
  void SPIROCtoSIDE(int spiroc_ch, int &pln, int &ch);
  void SPIROCtoTOP(int spiroc_ch, int &pln, int &ch);
private:
  static const int SPIROC_pln[32][2];
  static const int SPIROC_ch[32][2];
private:
  static const int    DifView    [NumDif];
  static const string MPPCch     [NumChipCh];
  static const int    ChipAlloc  [NumDif][NumChip][NumView];
 
public:
  bool ChipchToMPPCch(const int chip_ch, string& mppc_ch);
  bool GetViewPlnCh(const int dif_id, const int chip_id, const int chip_ch, int& view, int& pln, int& ch, int& grid);
  bool GetMPPCPlnCh(string mppc_ch, int view, int& pln, int& ch, int& grid);
  bool GetXYZ(int view, int pln, int ch, float& x, float& y, float& z);
  bool GetChipAlloc(const int dif_id, const int chip_id, int& id_z, int& id_xy);
  bool GetMap(const int dif_id, const int chip_id, int& view, vector<int> pln, vector<int> ch, vector<int> grid, vector<float> x, vector<float> y, vector<float> z);

public:
  Map_t load_mapping();
  MapInv_t load_mapping_inv();
  MapInv_t load_mapping_inv(size_t n_chans);
  ReconMap_t load_reconmap();

};

#endif

