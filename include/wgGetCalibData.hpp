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
  std::string m_adc_calibration_card;
  std::string m_tdc_calibration_card;
  bool m_have_pedestal_calibration = false;
  bool m_have_adc_calibration = false;
  bool m_have_tdc_calibration = false;

  bool FindPedestalCard();
  bool FindADCCalibrationCard();
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
  int GetPedestal(unsigned dif_id, d3vector& pedestal);
  
  // Get the TDC ramp slope and intercept the TDC calibration card file tdcFileName
  // Two arrays are filled (for the DIF "dif"):
  //                         slope[n_chips][n_chans][2]
  //                         intcpt[n_chips][n_chans][2]
  int GetTDC(unsigned dif, d3vector& slope, d3vector& intcpt);

  // Get the gain from the gain calibration card file calibFileName
  // One arrays is filled (for the DIF "dif"):
  //                         gain[n_chips][n_chans][n_cols]
  int GetADC(const unsigned dif, d3vector& gain);

  bool isPedestalCalibrated();
  bool isADCCalibrated();
  bool isTDCCalibrated();
};

#endif // WG_GETCALIBDATA_H_INCLUDE
