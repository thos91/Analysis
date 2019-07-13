#ifndef WG_CHANNELMAP_H_INCLUDE
#define WG_CHANNELMAP_H_INCLUDE

// system includes
#include <string>

// user includes
#include "wgConst.hpp"
#include "wgContiguousVectors.hpp"
#include "wgDetectorConst.hpp"

#define NumView 2
#define NumPln 8
#define NumCh 80

using namespace std;

typedef struct Map
{
  Map();
  Map(size_t n_difs, size_t n_chips, size_t n_chans);
  i3CCvector view; // [NDIFS][NCHIPS][NCHANNELS];
  i3CCvector pln;  // [NDIFS][NCHIPS][NCHANNELS];
  i3CCvector ch;   // [NDIFS][NCHIPS][NCHANNELS];
  i3CCvector grid; // [NDIFS][NCHIPS][NCHANNELS];
  d3CCvector x;    // [NDIFS][NCHIPS][NCHANNELS];
  d3CCvector y;    // [NDIFS][NCHIPS][NCHANNELS];
  d3CCvector z;    // [NDIFS][NCHIPS][NCHANNELS];
} Map_t;

typedef struct MapInv
{
  MapInv();
  MapInv(size_t n_views, size_t n_plns, size_t n_chans);
  i3CCvector dif;    // [NumView][NumPln][NumCh];
  i3CCvector chip;   // [NumView][NumPln][NumCh];
  i3CCvector chipch; // [NumView][NumPln][NumCh];
  d3CCvector x;      // [NumView][NumPln][NumCh];
  d3CCvector y;      // [NumView][NumPln][NumCh];
  d3CCvector z;      // [NumView][NumPln][NumCh];
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
  static const int    DifView    [2];
  static const string MPPCch     [32];
  static const int    ChipAlloc  [2][20][2];
 
public:
  bool ChipchToMPPCch(const int chip_ch, string& mppc_ch);
  bool GetViewPlnCh(const int dif_id, const int chip_id, const int chip_ch, int& view, int& pln, int& ch, int& grid);
  bool GetMPPCPlnCh(string mppc_ch, int view, int& pln, int& ch, int& grid);
  bool GetXYZ(int view, int pln, int ch, double& x, double& y, double& z);
  bool GetChipAlloc(const int dif_id, const int chip_id, int& id_z, int& id_xy);

  bool GetMap(unsigned dif_id,
              unsigned chip_id,
              unsigned n_channels,
              int& view,
              int * pln,
              int * ch,
              int * grid,
              double * x,
              double * y,
              double * z);

public:
  Map_t load_mapping();
  MapInv_t load_mapping_inv();
  MapInv_t load_mapping_inv(size_t n_chans);
  ReconMap_t load_reconmap();

};

#endif

