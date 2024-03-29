#ifndef WG_GETCALIBDATA_H_INCLUDE
#define WG_GETCALIBDATA_H_INCLUDE

// system includes
#include <string>
#include <vector>

// user includes
#include "tinyxml2.hpp"
#include "wgEditXML.hpp"
#include "wgFileSystemTools.hpp"
#include "wgConst.hpp"

#define TDC_RAMP_ODD  1
#define TDC_RAMP_EVEN 0

using namespace tinyxml2;

class wgGetCalibData
{
private:

  const unsigned m_dif;
  const std::string m_calibration_dir;
  std::string m_pedestal_card;
  std::string m_gain_card;
  std::string m_tdc_calibration_card;
  bool m_have_pedestal_calibration = false;
  bool m_have_gain_calibration = false;
  bool m_have_tdc_calibration = false;

  bool FindPedestalCard();
  bool FindGainCard();
  bool FindTDCCalibrationCard();
  
public:

  wgGetCalibData(unsigned dif);
  wgGetCalibData(const std::string& calibration_dir, unsigned dif);
  
  // Get pedestal ADC count from the pedestal card file pedFileName
  // The DIF number "dif" is a positive non zero integer.""
  // Two arrays are filled (for the DIF "dif"):
  //                         pedestal[n_chips][n_chans][n_cols]
  //                         ped_nohit[n_chips][n_chans][n_cols]
  // The pedestal array contains the pedestal when the hit bit is one,
  // while the ped_nohit array contains the pedestal when the hit bit is zero.
  // The hit bit is set to one when the ADC value is above the threshold
  int GetPedestal(unsigned dif_id, d3CCvector& pedestal);
  
  // Get the TDC ramp slope and intercept the TDC calibration card file tdcFileName
  // Two arrays are filled (for the DIF "dif"):
  //                         slope[n_chips][n_chans][2]
  //                         intcpt[n_chips][n_chans][2]
  int GetTDC(unsigned dif, d3CCvector& slope, d3CCvector& intcpt);

  // Get the gain from the gain calibration card file calibFileName
  // One arrays is filled (for the DIF "dif"):
  //                         gain[n_chips][n_chans][n_cols]
  int GetGain(const unsigned dif, d3CCvector& gain);

  bool isPedestalCalibrated();
  bool isGainCalibrated();
  bool isTDCCalibrated();
};

#endif // WG_GETCALIBDATA_H_INCLUDE
