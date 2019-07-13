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
  wgConst con;
  wgGetCalibData(con.CONF_DIRECTORY, m_dif);
}

//******************************************************************************
wgGetCalibData::wgGetCalibData(const string& calibration_dir, unsigned dif) :
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
                                + to_string(pedestal.size())
                                + ") and number of chips in pedestal card ("
                                + to_string(n_chips) + ") mismatch for DIF "
                                + to_string(dif_id));
  
  for(unsigned int ichip_id = 1; ichip_id <= n_chips; ichip_id++) {
    unsigned n_chans = Edit.Pedestal_GetChipConfigValue("n_chans", dif_id, ichip_id);
    if (n_chans != pedestal[ichip_id].size())
      throw std::invalid_argument("[wgGetCalibData] pedestal d3vector size ("
                                  + to_string(pedestal[ichip_id].size())
                                  + ") and number of channels in pedestal card ("
                                  + to_string(n_chans) + ") mismatch for DIF "
                                  + to_string(dif_id) + " and chip " + to_string(ichip_id));

    for(unsigned ichan_id = 1; ichan_id <= n_chans; ichan_id++) {
      if (pedestal[ichip_id][ichan_id].size() != MEMDEPTH)
        throw std::invalid_argument("[wgGetCalibData] pedestal d3vector size ("
                                    + to_string(pedestal.size())
                                    + ") and number of columns ("
                                    + to_string(MEMDEPTH) + ") mismatch for DIF "
                                    + to_string(dif_id) + ", chip " + to_string(ichip_id)
                                    + " and channel"  + to_string(ichan_id));
      
      for(unsigned icol_id = 1; icol_id <= MEMDEPTH; icol_id++) {
        try {
          pedestal[ichip_id][ichip_id][icol_id] = Edit.Pedestal_GetChanValue(
              "pedestal_%d" + to_string(icol_id), dif_id, ichip_id, ichan_id);
          count++;
        } catch (const wgElementNotFound& e) {
          pedestal[ichip_id][ichan_id][icol_id] = -1;
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
