#ifndef wgFit_H_INCLUDE
#define wgFit_H_INCLUDE

#include <string>
#include <vector>
#include "TROOT.h"
#include "Const.h"
#include "wgTools.h"
#include "wgGetHist.h"
#include "wgErrorCode.h"

using namespace std;

class wgFit
{
public:
  wgFit();
  wgFit(const string&);
  ~wgFit(); 
  void clear(); 
  void swap(int,double*,double*); 
  void NoiseRate(unsigned int,unsigned int,double*,int);
  void low_pe_charge(unsigned int,unsigned int,double*,int);
  void low_pe_charge_HG(unsigned int,unsigned int,unsigned int,double*,int);
  void charge_nohit(unsigned int,unsigned int,unsigned int,double*,int);
  void GainSelect(unsigned int,unsigned int,unsigned int,double*,int);
  void SetoutputIMGDir(const string&);

  wgGetHist *GetHist;
  string outputIMGDir;
};
#endif
