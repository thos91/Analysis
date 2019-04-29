#include <string>
#include <TROOT.h>
#include <iostream>
#include "wgChannelMap.h"
#include "DetectorConst.h"
#include "Const.h"

// #define DEBUG_CHMAP
// #ifndef DEBUG
// #define DEBUG
// #endif

using namespace std;

//******************************************************************************
Map::Map() : Map(NumDif, NumChip, NumChipCh) {}
Map::Map(size_t n_difs, size_t n_chips, size_t n_chans) {
  view.reserve(n_difs);
  pln.reserve(n_difs);
  ch.reserve(n_difs);
  grid.reserve(n_difs);
  x.reserve(n_difs);
  y.reserve(n_difs);
  z.reserve(n_difs);
  for(unsigned int idif = 0; idif < n_difs; idif++) {
	view[idif].reserve(n_chips);
	pln[idif].reserve(n_chips);
	ch[idif].reserve(n_chips);
	grid[idif].reserve(n_chips);
	x[idif].reserve(n_chips);
	y[idif].reserve(n_chips);
	z[idif].reserve(n_chips);
    for(unsigned int ichip = 0; ichip < n_chips; ichip++) {
	  view[idif][ichip].reserve(n_chans);
	  pln[idif][ichip].reserve(n_chans);
	  ch[idif][ichip].reserve(n_chans);
	  grid[idif][ichip].reserve(n_chans);
	  x[idif][ichip].reserve(n_chans);
	  y[idif][ichip].reserve(n_chans);
	  z[idif][ichip].reserve(n_chans);
	}
  }
}

//******************************************************************************
MapInv::MapInv() : MapInv(NumView, NumPln, NumCh) {}
MapInv::MapInv(size_t n_views, size_t n_plns, size_t n_chans) {
  dif.reserve(n_views);
  chip.reserve(n_views);
  chipch.reserve(n_views);
  x.reserve(n_views);
  y.reserve(n_views);
  z.reserve(n_views);
  for(unsigned int iview = 0; iview < n_views; iview++) {
	dif[iview].reserve(n_plns);
	chip[iview].reserve(n_plns);
	chipch[iview].reserve(n_plns);
	x[iview].reserve(n_plns);
	y[iview].reserve(n_plns);
	z[iview].reserve(n_plns);
    for(unsigned int ipln = 0; ipln < n_plns; ipln++) {
	  dif[iview][ipln].reserve(n_chans);
	  chip[iview][ipln].reserve(n_chans);
	  chipch[iview][ipln].reserve(n_chans);
	  x[iview][ipln].reserve(n_chans);
	  y[iview][ipln].reserve(n_chans);
	  z[iview][ipln].reserve(n_chans);
	}
  }
}

//******************************************************************************
ReconMap::ReconMap() : ReconMap(NumReconAxis, NumView, NumPln, NumCh) {}
ReconMap::ReconMap(size_t n_axes, size_t n_views, size_t n_plns, size_t n_chans) {
  recon_view.reserve(n_axes);
  recon_pln.reserve(n_axes);
  recon_ch.reserve(n_axes);
  for(unsigned int iaxis = 0; iaxis < n_axes; iaxis++) {
	recon_view[iaxis].reserve(n_views);
	recon_pln[iaxis].reserve(n_views);
	recon_ch[iaxis].reserve(n_views);
	for(unsigned int iview = 0; iview < n_views; iview++) {
	  recon_view[iaxis][iview].reserve(n_plns);
	  recon_pln[iaxis][iview].reserve(n_plns);
	  recon_ch[iaxis][iview].reserve(n_plns);
	  for(unsigned int ipln = 0; ipln < n_plns; ipln++) {
		recon_view[iaxis][iview][ipln].reserve(n_chans);
		recon_pln[iaxis][iview][ipln].reserve(n_chans);
		recon_ch[iaxis][iview][ipln].reserve(n_chans);
	  }
	}
  }
}

//******************************************************************************
string PINtoNUM1(int pin){
  int num = 4 - pin%4;
  string str("");
  str.push_back(num);
  return str;
}

//******************************************************************************
string PINtoNUM2(int pin){
  int num = pin%4 + 1;
  string str("");
  str.push_back(num);
  return str;
}

//******************************************************************************
// Convert SPIROC channel to MPPC PIN
int wgChannelMap::SPIROCtoPIN(int ch){
  if(ch>=0 && ch<8){return 26-ch;}
  else if(ch>=8 && ch<16){return 16-ch;}
  else if(ch>=16 && ch<24){return ch+21;}
  else if(ch>=24 && ch<32){return ch+31;}
  return -1;  
}

//******************************************************************************
// Convert MPPC PIN to MPPC Position
string wgChannelMap::PINtoMPPC(int pin){
  if(pin>=1 && pin<5){  return ("A" + PINtoNUM1(pin-1));  }
  else if(pin>=5 && pin<9){ return ("B" + PINtoNUM1(pin-5));  }
  else if(pin>=19 && pin<23){ return ("C" + PINtoNUM1(pin-19));  }
  else if(pin>=23 && pin<27){ return ("D" + PINtoNUM1(pin-23));  }
  else if(pin>=59 && pin<63){ return ("E" + PINtoNUM2(pin-59));  }
  else if(pin>=55 && pin<59){ return ("F" + PINtoNUM2(pin-55));  }
  else if(pin>=41 && pin<45){ return ("G" + PINtoNUM2(pin-41));  }
  else if(pin>=37 && pin<41){ return ("H" + PINtoNUM2(pin-37));  }
  return ""; 
}


//******************************************************************************
// Convert MPPC Position to scintillator position of side view
int wgChannelMap::PINtoSIDE(string &pos, int nbund,int layer, int &pln,int &ch){
  if(pos=="") return -1;
  string row = pos.substr(0,1); 
  int col = atoi(pos.substr(1).c_str());
  if(col>2){ pln = 1 + layer*2 ; }
  else{ pln = 0 + layer*2 ; }
  int map=-1;
  if( row=="A" ){ map = 7; } 
  else if( row=="B" ){ map = 6; } 
  else if( row=="C" ){ map = 5; } 
  else if( row=="D" ){ map = 4; } 
  else if( row=="E" ){ map = 3; } 
  else if( row=="F" ){ map = 2; } 
  else if( row=="G" ){ map = 1; } 
  else if( row=="H" ){ map = 0; }

  bool grid=false;
  if(col%2==0) grid = true;

  if(!grid){ ch = map+nbund*8; }
  else{  
    if(map>3){ ch = map%4 + nbund*4 + 40; }
    else{ ch = map%4 + nbund*4 + 60; }
  }
  return 0;
}

//******************************************************************************
// Convert MPPC Position to scintillator position of top view
int wgChannelMap::PINtoTOP(string &pos, int nbund,int layer, int &pln, int &ch){
  if(pos=="") return -1;
  string row = pos.substr(0,1); 
  int col = atoi(pos.substr(1).c_str());
  if(col>2){ pln = 0 + layer*2 ; }
  else{ pln = 1 + layer*2 ; }

  int map=-1;
  if( row=="A" ){ map = 0; } 
  else if( row=="B" ){ map = 1; } 
  else if( row=="C" ){ map = 2; } 
  else if( row=="D" ){ map = 3; } 
  else if( row=="E" ){ map = 4; } 
  else if( row=="F" ){ map = 5; } 
  else if( row=="G" ){ map = 6; } 
  else if( row=="H" ){ map = 7; }

  if((map<4 && col%2 ==1)) ch = 60 + map%4 + nbund*4;
  else if((map>=4 && col%2 ==0)) ch = 40 + map%4 + nbund*4;
  else if((map<4 && col%2 ==0)) ch =  map%4 + nbund*8;
  else if((map>=4 && col%2 ==1)) ch =  4 + map%4 + nbund*8;

  return 0;
}

//******************************************************************************
const int wgChannelMap::SPIROC_ch[32][2] ={
  {4,63},{40,3},{4,63},{40,3},
  {5,62},{41,2},{5,62},{41,2},
  {6,61},{42,1},{6,61},{42,1},
  {7,60},{43,0},{7,60},{43,0},
  {0,7},{60,43},{0,7},{60,43},
  {1,6},{61,42},{1,6},{61,42},
  {2,5},{62,41},{2,5},{62,41},
  {3,4},{63,40},{3,4},{63,40}
};

//******************************************************************************
const int wgChannelMap::SPIROC_pln[32][2] ={
  {0,1},{0,1},{1,0},{1,0},
  {0,1},{0,1},{1,0},{1,0},
  {0,1},{0,1},{1,0},{1,0},
  {0,1},{0,1},{1,0},{1,0},
  {0,1},{0,1},{1,0},{1,0},
  {0,1},{0,1},{1,0},{1,0},
  {0,1},{0,1},{1,0},{1,0},
  {0,1},{0,1},{1,0},{1,0}
};
//******************************************************************************
void wgChannelMap::SPIROCtoSIDE(int spiroc_ch, int &pln, int &ch){
  pln = this->SPIROC_pln[spiroc_ch][0];
  ch  = this->SPIROC_ch [spiroc_ch][0];
}

//******************************************************************************
void wgChannelMap::SPIROCtoTOP(int spiroc_ch, int &pln, int &ch){
  pln = this->SPIROC_pln[spiroc_ch][1];
  ch  = this->SPIROC_ch [spiroc_ch][1];
}

//******************************************************************************
const int wgChannelMap::DifView[2] = {TopView,SideView};

//******************************************************************************
const string wgChannelMap::MPPCch[32] ={
  "B1","B2","B3","B4",
  "A1","A2","A3","A4",
  "C4","C2","D4","D2",
  "C3","C1","D3","D1",
  "H1","H3","G1","G3",
  "F1","F3","E1","E3",
  "F2","F4","E4","E2",
  "G4","G2","H4","H2"
};

//******************************************************************************
const int wgChannelMap::ChipAlloc[2][20][2] = {
  { //TopView
    {3,4},{3,3},{3,2},{3,1},{3,0},
    {2,4},{2,3},{2,2},{2,1},{2,0},
    {1,4},{1,3},{1,2},{1,1},{1,0},
    {0,4},{0,3},{0,2},{0,1},{0,0}
  },
  { //SideView
    {0,0},{0,1},{0,2},{0,3},{0,4},
    {1,0},{1,1},{1,2},{1,3},{1,4},
    {2,0},{2,1},{2,2},{2,3},{2,4},
    {3,0},{3,1},{3,2},{3,3},{3,4}
  }
};

//******************************************************************************
bool wgChannelMap::ChipchToMPPCch(const int chip_ch, string& mppc_ch)
{
  if(chip_ch>=0&&chip_ch<32){
    mppc_ch = this->MPPCch[chip_ch];
  }else if(chip_ch>=32 && chip_ch<(int)NCHANNELS){
    mppc_ch = this->MPPCch[chip_ch-32];
  }else{
    cout << "wrong chip_ch:" << chip_ch << endl;
    return false;
  }
#ifdef DEBUG
  cout << "ChipchToMPPCch >> chip_ch:" << chip_ch 
    << " mppc_sh:" << mppc_ch << endl;
#endif
  return true;
};

//******************************************************************************
bool wgChannelMap::GetMPPCPlnCh(string mppc_ch, int view, int& pln, int& ch, int& grid){
  if(view==SideView){
    if     (mppc_ch=="A1"){pln=0; ch= 7; grid=0;}
    else if(mppc_ch=="B1"){pln=0; ch= 6; grid=0;}
    else if(mppc_ch=="C1"){pln=0; ch= 5; grid=0;}
    else if(mppc_ch=="D1"){pln=0; ch= 4; grid=0;}
    else if(mppc_ch=="E1"){pln=0; ch= 3; grid=0;}
    else if(mppc_ch=="F1"){pln=0; ch= 2; grid=0;}
    else if(mppc_ch=="G1"){pln=0; ch= 1; grid=0;}
    else if(mppc_ch=="H1"){pln=0; ch= 0; grid=0;}
    else if(mppc_ch=="A2"){pln=0; ch=43; grid=1;}
    else if(mppc_ch=="B2"){pln=0; ch=42; grid=1;}
    else if(mppc_ch=="C2"){pln=0; ch=41; grid=1;}
    else if(mppc_ch=="D2"){pln=0; ch=40; grid=1;}
    else if(mppc_ch=="E2"){pln=0; ch=63; grid=1;}
    else if(mppc_ch=="F2"){pln=0; ch=62; grid=1;}
    else if(mppc_ch=="G2"){pln=0; ch=61; grid=1;}
    else if(mppc_ch=="H2"){pln=0; ch=60; grid=1;}
    else if(mppc_ch=="A3"){pln=1; ch= 7; grid=0;}
    else if(mppc_ch=="B3"){pln=1; ch= 6; grid=0;}
    else if(mppc_ch=="C3"){pln=1; ch= 5; grid=0;}
    else if(mppc_ch=="D3"){pln=1; ch= 4; grid=0;}
    else if(mppc_ch=="E3"){pln=1; ch= 3; grid=0;}
    else if(mppc_ch=="F3"){pln=1; ch= 2; grid=0;}
    else if(mppc_ch=="G3"){pln=1; ch= 1; grid=0;}
    else if(mppc_ch=="H3"){pln=1; ch= 0; grid=0;}
    else if(mppc_ch=="A4"){pln=1; ch=43; grid=1;}
    else if(mppc_ch=="B4"){pln=1; ch=42; grid=1;}
    else if(mppc_ch=="C4"){pln=1; ch=41; grid=1;}
    else if(mppc_ch=="D4"){pln=1; ch=40; grid=1;}
    else if(mppc_ch=="E4"){pln=1; ch=63; grid=1;}
    else if(mppc_ch=="F4"){pln=1; ch=62; grid=1;}
    else if(mppc_ch=="G4"){pln=1; ch=61; grid=1;}
    else if(mppc_ch=="H4"){pln=1; ch=60; grid=1;}
    else return false;
  }
  else if(view==TopView){
    if     (mppc_ch=="A1"){pln=1; ch=60; grid=1;}
    else if(mppc_ch=="B1"){pln=1; ch=61; grid=1;}
    else if(mppc_ch=="C1"){pln=1; ch=62; grid=1;}
    else if(mppc_ch=="D1"){pln=1; ch=63; grid=1;}
    else if(mppc_ch=="E1"){pln=1; ch= 4; grid=0;}
    else if(mppc_ch=="F1"){pln=1; ch= 5; grid=0;}
    else if(mppc_ch=="G1"){pln=1; ch= 6; grid=0;}
    else if(mppc_ch=="H1"){pln=1; ch= 7; grid=0;}
    else if(mppc_ch=="A2"){pln=1; ch= 0; grid=0;}
    else if(mppc_ch=="B2"){pln=1; ch= 1; grid=0;}
    else if(mppc_ch=="C2"){pln=1; ch= 2; grid=0;}
    else if(mppc_ch=="D2"){pln=1; ch= 3; grid=0;}
    else if(mppc_ch=="E2"){pln=1; ch=40; grid=1;}
    else if(mppc_ch=="F2"){pln=1; ch=41; grid=1;}
    else if(mppc_ch=="G2"){pln=1; ch=42; grid=1;}
    else if(mppc_ch=="H2"){pln=1; ch=43; grid=1;}
    else if(mppc_ch=="A3"){pln=0; ch=60; grid=1;}
    else if(mppc_ch=="B3"){pln=0; ch=61; grid=1;}
    else if(mppc_ch=="C3"){pln=0; ch=62; grid=1;}
    else if(mppc_ch=="D3"){pln=0; ch=63; grid=1;}
    else if(mppc_ch=="E3"){pln=0; ch= 4; grid=0;}
    else if(mppc_ch=="F3"){pln=0; ch= 5; grid=0;}
    else if(mppc_ch=="G3"){pln=0; ch= 6; grid=0;}
    else if(mppc_ch=="H3"){pln=0; ch= 7; grid=0;}
    else if(mppc_ch=="A4"){pln=0; ch= 0; grid=0;}
    else if(mppc_ch=="B4"){pln=0; ch= 1; grid=0;}
    else if(mppc_ch=="C4"){pln=0; ch= 2; grid=0;}
    else if(mppc_ch=="D4"){pln=0; ch= 3; grid=0;}
    else if(mppc_ch=="E4"){pln=0; ch=40; grid=1;}
    else if(mppc_ch=="F4"){pln=0; ch=41; grid=1;}
    else if(mppc_ch=="G4"){pln=0; ch=42; grid=1;}
    else if(mppc_ch=="H4"){pln=0; ch=43; grid=1;}
    else return false;
  }
  else return false;
#ifdef DEBUG
  cout << "GetMPPCPlnCh >> mppc_ch: " << mppc_ch << " pln:" << pln
       << " ch:" << ch << endl;
#endif
  return true;
}; //GetMPPCPlnCh


//******************************************************************************
bool wgChannelMap::GetViewPlnCh(const int dif_id, const int chip_id, const int chip_ch, int& view, int& pln, int& ch, int& grid){
  view = this->DifView[dif_id];
  string MPPC_ch;
  if(!this->ChipchToMPPCch(chip_ch, MPPC_ch)){
    cout << "Failed to ChipchToMPPCch" << endl;
    return false;
  }
  if(!this->GetMPPCPlnCh(MPPC_ch, view, pln, ch, grid)){
    cout << "Failed to GetMPPCPln" << endl;
    return false;
  }
  int id_xy, id_z;
  if(!this->GetChipAlloc(dif_id, chip_id, id_z, id_xy)){
    cout << "Failed to GetChipAlloc" << endl;
    return false;
  }
  pln = pln + 2 * id_z;
  if     ( grid == 0 ) {ch  = ch  + 8 * id_xy;}
  else if( grid == 1 ) {ch  = ch  + 4 * id_xy;}
  else{
    cout << "grid flag should be 0 or 1." << endl;
    return false;
  }

  if(( view < 0) || ( view > 1) || ( pln < 0) || ( pln >= C_WMNumPln ) || ( ch < 0) || ( ch >= C_WMNumCh )) {
    cout << "wrong alignement of view/pln/ch, view:" << view << " pln:" << pln << " ch:" << ch << endl;
    return false;
  }

#ifdef DEBUG
  cout << "GetViewPlnCh >> view:" << view << " pln:" << pln << " ch:" << ch << endl;
#endif

  return true;

}; //GetViewPlnCh

//******************************************************************************
bool wgChannelMap::GetXYZ(int view, int pln, int ch, float& x, float& y, float& z)
{

  unsigned short grid = (ch/20)&0b11;
  if(view==SideView){
    x = 0.;
    if((grid&0b10)==0b00){
      y = C_WMChStart + C_WMScintiWidth*ch;
      z = C_WMPlnStart + C_WMPlnDist*pln;
    }
    else if((grid&0b11)==0b10){
      y = C_WMGridChStart + C_WMScintiSlitStep*(ch-C_WMNumXYLayerCh);
      z = C_WMPlnStart + C_WMPlnDist*pln + C_WMScintiThick/2. + C_WMScintiWidth/2.+0.1;
    }
    else if((grid&0b11)==0b11){
      y = C_WMGridChStart + C_WMScintiSlitStep*(ch-C_WMNumXYLayerCh-C_WMNumGridCh);
      z = C_WMPlnStart + C_WMPlnDist*pln + C_WMLayerDist + C_WMScintiThick/2. + C_WMScintiWidth/2.+0.1;
    }
    else{
      cout << "GetXYZ >> wrong ch alignment" << endl;
      return false;
    }
  }
  else if(view==TopView){
    y = 0.;
    if((grid&0b10)==0b00){
      x = C_WMChStart + C_WMScintiWidth*ch;
      z = C_WMPlnStart + C_WMPlnDist*pln + C_WMLayerDist;
    }
    else if((grid&0b11)==0b10){
      x = C_WMGridChStart + C_WMScintiSlitStep*(ch-C_WMNumXYLayerCh);
      z = C_WMPlnStart + C_WMPlnDist*pln + C_WMScintiThick/2. + C_WMScintiWidth/2.+0.1;
    }
    else if((grid&0b11)==0b11){
      x = C_WMGridChStart + C_WMScintiSlitStep*(ch-C_WMNumXYLayerCh-C_WMNumGridCh);
      z = C_WMPlnStart + C_WMPlnDist*pln + C_WMLayerDist + C_WMScintiThick/2. + C_WMScintiWidth/2.+0.1;
    }
    else{
     cout << "GetXYZ >> wrong ch alignment" << endl;
      return false;
    }
  }
  else{
    cout << "GetXYZ >> view should be 0 or 1" << endl;
    return false;
  }
  return true;
}; //GetXYZ

//******************************************************************************
bool wgChannelMap::GetChipAlloc(const int dif_id, const int chip_id, int& id_z, int& id_xy){
  if((dif_id<0)||(dif_id>=2)||(chip_id<0)||(chip_id>=20)){
    cout << "wrong dif_id/chip_id, dif_id:" 
      << dif_id << " chip_id:" << chip_id << endl;
    return false;
  }
  else{
    id_z =this->ChipAlloc[dif_id][chip_id][0];
    id_xy=this->ChipAlloc[dif_id][chip_id][1];
  };

#ifdef DEBUG
  cout << "GetChipAlloc >> dif_id:" << dif_id << " chip_id:" << chip_id
    << " id_z:" << *id_z << " id_xy:" << *id_xy << endl;
#endif

  return true;

} //GetChipAlloc

//******************************************************************************
bool wgChannelMap::GetMap(const int dif_id, const int chip_id, int& view, vector<int> pln, vector<int> ch, vector<int> grid, vector<float> x, vector<float> y, vector<float> z) {
  for(int ich = 0; ich < NumChipCh; ich++) {
    if( !GetViewPlnCh(dif_id, chip_id, ich, view, pln[ich], ch[ich], grid[ich])) {
      cout << "Failed to GetViewPlnCh" << endl;
      return false;
    }
    
    if(!GetXYZ(view, pln[ich], ch[ich], x[ich], y[ich], z[ich])){
      cout << "Failed to GetXYZ" << endl;
      return false;
    }
  } 
  return true;
}

//******************************************************************************
Map_t wgChannelMap::load_mapping(){

  Map_t map_struct;
  //reading mapping
  
  for(int idif=0;idif<NumDif;idif++){
    for(int ichip=0;ichip<NumChip;ichip++){
      this->GetMap(idif,ichip,
          map_struct.view[idif][ichip][0],
          map_struct.pln[idif][ichip],
          map_struct.ch[idif][ichip],
          map_struct.grid[idif][ichip],
          map_struct.x[idif][ichip],
          map_struct.y[idif][ichip],
          map_struct.z[idif][ichip]
          );
      for(int ich=1;ich<NumChipCh;ich++){
        map_struct.view[idif][ichip][ich]=map_struct.view[idif][ichip][0];
      }
    }
  }

#ifdef DEBUG_CHMAP
  for(int idif=0;idif<NumDif;idif++){
    for(int ichip=0;ichip<NumChip;ichip++){
      for(int ich=0;ich<NumChipCh;ich++){
        cout << "dif=" << idif << ", chip=" << ichip << ", ch=" << ich << ", view=" << map_struct.view[idif][ichip][ich] << ", pln=" << map_struct.pln[idif][ichip][ich] << ", ch=" << map_struct.ch[idif][ichip][ich] << endl;
      }
    }
  }
#endif

  return map_struct;
}

//******************************************************************************
MapInv_t wgChannelMap::load_mapping_inv(size_t n_chans){
  MapInv_t mapinv_struct;
  //reading mapping
  for(int idif=0;idif<NumDif;idif++){
    for(int ichip=0;ichip<NumChip;ichip++){
      int view;
      vector<int> pln(n_chans), ch(n_chans), grid(n_chans);
      vector<float> x(n_chans), y(n_chans), z(n_chans);
      this->GetMap(idif, ichip, view, pln, ch, grid, x, y, z);
        
      for(int ich=0;ich<NumChipCh;ich++){
        mapinv_struct.dif[view][pln[ich]][ch[ich]] = idif;
        mapinv_struct.chip[view][pln[ich]][ch[ich]] = ichip;
        mapinv_struct.chipch[view][pln[ich]][ch[ich]] = ich;
        mapinv_struct.x[view][pln[ich]][ch[ich]] = x[ich];
        mapinv_struct.y[view][pln[ich]][ch[ich]] = y[ich];
        mapinv_struct.z[view][pln[ich]][ch[ich]] = z[ich];
      }
    }
  }
  
  return mapinv_struct;
}

//******************************************************************************
ReconMap_t wgChannelMap::load_reconmap() 
{
  ReconMap_t map_struct;

  for(int axis=0;axis<NumReconAxis;axis++){
    for(int view=0;view<NumView;view++){
      for(int pln=0;pln<NumPln;pln++){
        for(int ch=0;ch<NumCh;ch++){
          int reconview,reconpln,reconch;
          reconview = view;
          if(axis==0){ // reconstruction along z-axis (neutrino beam axis)
            if(ch<C_WMNumXYLayerCh){
              if(view==SideView) reconpln = pln*3;
              else               reconpln = pln*3 +1;
              reconch = ch;
            }
            else if(ch<C_WMNumXYLayerCh+C_WMNumGridCh){
              if(view==SideView) reconpln = pln*3 +1;
              else               reconpln = pln*3;
              reconch = ch-C_WMNumXYLayerCh;
            }
            else{
              if(view==SideView) reconpln = pln*3 +2;
              else               reconpln = pln*3 +2;
              reconch = ch-C_WMNumXYLayerCh-C_WMNumGridCh;
            }
          }
          else{ //reconstruction along x/y-axis (perpendicular to beam axis)
            if(ch<C_WMNumXYLayerCh){
              reconch = pln;
              reconpln = (int)(ch/2)*3+(ch%2)*2;
            }
            else if(ch<C_WMNumXYLayerCh+C_WMNumGridCh){
              reconch = pln*2;
              reconpln = (ch-C_WMNumXYLayerCh)*3+1;
            }
            else{
              reconch = pln*2 +1;
              reconpln = (ch-C_WMNumXYLayerCh-C_WMNumGridCh)*3+1;
            }
          }
          map_struct.recon_view[axis][view][pln][ch] = reconview;
          map_struct.recon_pln [axis][view][pln][ch] = reconpln;
          map_struct.recon_ch  [axis][view][pln][ch] = reconch;
        }
      }
    }
  }
  return map_struct;

}


//******************************************************************************
//******************************************************************************
//******************************************************************************
//******************************************************************************
