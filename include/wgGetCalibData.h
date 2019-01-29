#ifndef WG_GETCALIBDATA_H_INCLUDE
#define WG_GETCALIBDATA_H_INCLUDE

#include <string>
#include <vector>
#include "tinyxml2.h"
#include "wgEditXML.h"

using namespace tinyxml2;
using namespace std;

class wgGetCalibData
{
private:
  static string filename;
public:
  void Get_Pedestal(int,double pedestal[20][36][16],double ped_nohit[20][36][16]);
  void Get_TdcCoeff(int ndif,double slope[2][20][36],double intcpt[2][20][36]);
  void Get_Gain(string& calibFileName,int ndif, double gain[20][36]);

};


#endif
