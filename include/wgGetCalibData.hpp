#ifndef WG_GETCALIBDATA_H_INCLUDE
#define WG_GETCALIBDATA_H_INCLUDE

// system includes
#include <string>
#include <vector>

// user includes
#include "tinyxml2.hpp"
#include "wgEditXML.hpp"
#include "wgTools.hpp"
#include "Const.hpp"

#define TDC_RAMP_ODD  1
#define TDC_RAMP_EVEN 0

using namespace tinyxml2;
using namespace std;

class wgGetCalibData
{
private:
  static string filename;
  
public:
  // Get pedestal ADC count from the pedestal card file pedFileName
  // The DIF number "dif" is a positive non zero integer.
  // Two arrays are filled (for the DIF "dif"):
  //                         pedestal[n_chips][n_chans][n_cols]
  //                         ped_nohit[n_chips][n_chans][n_cols]
  // The pedestal array contains the pedestal when the hit bit is one,
  // while the ped_nohit array contains the pedestal when the hit bit is zero.
  // The hit bit is set to one when the ADC value is above the threshold
  int Get_Pedestal(const string& pedFileName, unsigned dif, d3vector& pedestal, d3vector& ped_nohit);

  // Get the TDC ramp slope and intercept the TDC calibration card file tdcFileName
  // Two arrays are filled (for the DIF "dif"):
  //                         slope[n_chips][n_chans][2]
  //                         intcpt[n_chips][n_chans][2]
  int Get_TdcCoeff(const string& tdcFileName, unsigned dif, d3vector& slope, d3vector& intcpt);

  // Get the gain from the gain calibration card file calibFileName
  // One arrays is filled (for the DIF "dif"):
  //                         gain[n_chips][n_chans]
  int Get_Gain(const string& calibFileName, unsigned dif, d2vector& gain);
};

#endif // WG_GETCALIBDATA_H_INCLUDE
