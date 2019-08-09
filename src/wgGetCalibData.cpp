// ROOT includes
#include <TROOT.h>

// user includes
#include "wgGetCalibData.hpp"
#include "wgExceptions.hpp"
#include "wgFileSystemTools.hpp"
#include "wgConst.hpp"
#include "wgFileSystemTools.hpp"
#include "wgEditXML.hpp"
#include "wgExceptions.hpp"

using namespace wagasci_tools;


//******************************************************************************
wgGetCalibData::wgGetCalibData(unsigned dif) : m_dif(dif) {
  wgEnvironment env;
  wgGetCalibData(env.CONF_DIRECTORY, m_dif);
}

//******************************************************************************
wgGetCalibData::wgGetCalibData(const std::string& calibration_dir, unsigned dif) :
    m_dif(dif), m_calibration_dir(calibration_dir) {
  if (!check_exist::Dir(m_calibration_dir))
    throw wgInvalidFile("[wgGetCalibData] calibration directory not found : " + m_calibration_dir);
  m_have_pedestal_calibration = FindPedestalCard();
  m_have_gain_calibration     = FindGainCard();
  m_have_tdc_calibration      = FindTDCCalibrationCard();
}

//******************************************************************************
bool wgGetCalibData::FindPedestalCard() {
  for (auto const xmlfile : ListFilesWithExtension(m_calibration_dir, "xml")) {
    if (findStringIC(xmlfile, "ped")) {
      m_pedestal_card = xmlfile;
      return true;
    }
  }
  return false;
}

//******************************************************************************
bool wgGetCalibData::FindGainCard() {
  for (auto const xmlfile : ListFilesWithExtension(m_calibration_dir, "xml")) {
    if (findStringIC(xmlfile, "adc") || findStringIC(xmlfile, "gain")) {
      m_gain_card = xmlfile;
      return true;
    }
  }
  return false;
}

//******************************************************************************
bool wgGetCalibData::FindTDCCalibrationCard() {
  for (auto const xmlfile : ListFilesWithExtension(m_calibration_dir, "xml")) {
    if (findStringIC(xmlfile, "tdc") || findStringIC(xmlfile, "time")) {
      m_tdc_calibration_card = xmlfile;
      return true;
    }
  }
  return false;
}

//******************************************************************************
int wgGetCalibData::GetPedestal(unsigned dif_id, d3CCvector& pedestal) {

  int count = 0;
  // Pedestal when there is no hit
  wgEditXML Edit;
  Edit.Open(m_pedestal_card);
  unsigned n_chips = Edit.Pedestal_GetDifConfigValue("n_chips", dif_id);
  if (n_chips != pedestal.size())
    throw std::invalid_argument("[wgGetCalibData] pedestal d3vector size ("
                                + std::to_string(pedestal.size())
                                + ") and number of chips in pedestal card ("
                                + std::to_string(n_chips) + ") mismatch for DIF "
                                + std::to_string(dif_id));
  
  for(unsigned int ichip = 0; ichip < n_chips; ichip++) {
    unsigned n_chans = Edit.Pedestal_GetChipConfigValue("n_chans", dif_id, ichip);
    if (n_chans != pedestal[ichip].size())
      throw std::invalid_argument("[wgGetCalibData] pedestal d3vector size ("
                                  + std::to_string(pedestal[ichip].size())
                                  + ") and number of channels in pedestal card ("
                                  + std::to_string(n_chans) + ") mismatch for DIF "
                                  + std::to_string(dif_id) + " and chip " + std::to_string(ichip));

    for(unsigned ichan = 0; ichan < n_chans; ichan++) {
      if (pedestal[ichip][ichan].size() != MEMDEPTH)
        throw std::invalid_argument("[wgGetCalibData] pedestal d3vector size ("
                                    + std::to_string(pedestal.size())
                                    + ") and number of columns ("
                                    + std::to_string(MEMDEPTH) + ") mismatch for DIF "
                                    + std::to_string(dif_id) + ", chip " + std::to_string(ichip)
                                    + " and channel"  + std::to_string(ichan));
      
      for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
        try {
          pedestal[ichip][ichip][icol] = Edit.Pedestal_GetChanValue(
              "pedestal_%d" + std::to_string(icol), dif_id, ichip, ichan);
          count++;
        } catch (const wgElementNotFound& e) {
          pedestal[ichip][ichan][icol] = -1;
        }
      }
    }
  }
  Edit.Close();
  return count;
}

//******************************************************************************
int wgGetCalibData::GetTDC(const unsigned dif, d3CCvector& slope, d3CCvector& intcpt) {
  return 0;
}

//******************************************************************************
int wgGetCalibData::GetGain(const unsigned dif, d3CCvector& gain) {
  return 0;
}

//******************************************************************************
bool wgGetCalibData::isPedestalCalibrated() {
  return m_have_pedestal_calibration;
}

//******************************************************************************
bool wgGetCalibData::isGainCalibrated() {
  return m_have_gain_calibration;
}

//******************************************************************************
bool wgGetCalibData::isTDCCalibrated() {
  return m_have_tdc_calibration;
}
