// system C++ includes
#include <iostream>
#include <string>
#include <map>

// boost includes
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

// json includes
#include <nlohmann/json.hpp>

// tinyxml2 includes
#include "tinyxml2.hpp"

//user includes
#include "wgExceptions.hpp"
#include "wgFileSystemTools.hpp"
#include "wgLogger.hpp"
#include "wgEditXML.hpp"
#include "wgTopology.hpp"

using namespace wagasci_tools;

///////////////////////////////////////////////////////////////////////////////
//                               Public methods                              //
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//                              Ctypes wrappers                              //
///////////////////////////////////////////////////////////////////////////////

//**********************************************************************
const char * GetDifTopologyCtypes(const char * x_configxml) {
  std::string configxml(x_configxml);
  std::string json("");

  char * topology_string = (char *) malloc(TOPOLOGY_STRING_LENGTH * sizeof(char));
  if (topology_string == NULL) {
    Log.eWrite("[GetTopologyCtypes] failed to allocate topology string");
    return "";
  }
  
  StringTopologyMapDif topology_map;
  
  try {
    Topology topology(configxml, TopologySourceType::xml_file);
    topology_map = topology.m_string_dif_map;
  } // try/catch
  catch (const std::exception& e) {
    Log.eWrite("[GetTopologyCtypes] " + std::string(e.what()));
    topology_string[0] = '\0';
    return topology_string;
  }

  nlohmann::json topology_json = topology_map;
  int result = std::snprintf(topology_string, TOPOLOGY_STRING_LENGTH, "%s",
                             topology_json.dump().c_str());
  if (!(result > -1 && result < TOPOLOGY_STRING_LENGTH)) {
    Log.eWrite("[GetTopologyCtypes] failed to copy topology string");
    topology_string[0] = '\0';
  }
  return topology_string;
}

//**********************************************************************
const char * GetGdccTopologyCtypes(const char * x_configxml) {
  std::string configxml(x_configxml);
  std::string json("");

  char * topology_string = (char *) malloc(TOPOLOGY_STRING_LENGTH * sizeof(char));
  if (topology_string == NULL) {
    Log.eWrite("[GetTopologyCtypes] failed to allocate topology string");
    return "";
  }
  
  StringTopologyMapGdcc topology_map;
  
  try {
    Topology topology(configxml, TopologySourceType::xml_file);
    topology_map = topology.m_string_gdcc_map;
  } // try/catch
  catch (const std::exception& e) {
    Log.eWrite("[GetTopologyCtypes] " + std::string(e.what()));
    topology_string[0] = '\0';
    return topology_string;
  }

  nlohmann::json topology_json = topology_map;
  int result = std::snprintf(topology_string, TOPOLOGY_STRING_LENGTH, "%s",
                             topology_json.dump().c_str());
  if (!(result > -1 && result < TOPOLOGY_STRING_LENGTH)) {
    Log.eWrite("[GetTopologyCtypes] failed to copy topology string");
    topology_string[0] = '\0';
  }
  return topology_string;
}

//**********************************************************************
void FreeTopologyCtypes(char * topology_string) {
  free(topology_string);
}

///////////////////////////////////////////////////////////////////////////////
//                                Constructors                               //
///////////////////////////////////////////////////////////////////////////////

Topology::Topology(const char * source, TopologySourceType source_type) :
    Topology(std::string(source), source_type) {}

Topology::Topology(std::string source, TopologySourceType source_type) :
    m_dif_mapping_file_path("/opt/calicoes/config/dif_mapping.txt"),
    m_mac_mapping_file_path("/opt/calicoes/config/mac_mapping.txt") {

  switch (source_type) {
    case TopologySourceType::xml_file:
      this->GetGdccMacMapping();
      this->GetTopologyFromFile(source);
      this->GetGdccDifMapping();
      this->GdccMapToDifMap();
      this->StringToUnsigned();
      break;
    case TopologySourceType::json_string:
      this->GetTopologyFromString(source);
      this->GetGdccDifMapping();
      if (this->m_string_gdcc_map.empty())
        this->DifMapToGdccMap();
      else
        this->GdccMapToDifMap();
      this->StringToUnsigned();
      break;
    case TopologySourceType::scurve_tree:
      this->GetTopologyFromScurveTree(source);
      this->GetGdccDifMapping();
      this->DifMapToGdccMap();
      this->StringToUnsigned();
      break;
    case TopologySourceType::pedestal_tree:
      this->GetTopologyFromPedestalTree(source);
      this->GetGdccDifMapping();
      this->DifMapToGdccMap();
      this->StringToUnsigned();
      break;
    case TopologySourceType::gain_tree:
      this->GetTopologyFromGainTree(source);
      this->GetGdccDifMapping();
      this->DifMapToGdccMap();
      this->StringToUnsigned();
      break;
    default:
      throw std::invalid_argument("[wgTopology] TopologySourceType not recognized");
      break;
  }
      
  this->n_difs = this->dif_map.size();
  for ( auto const & dif : this->dif_map) {
    unsigned n_chips_tmp = 0;
    for ( auto const & asu : dif.second) {
      n_chips_tmp++;
      if ( asu.second > this->max_channels) max_channels = asu.second;
    }
    if (n_chips_tmp > this->max_chips) max_chips = n_chips_tmp;
  }
}

///////////////////////////////////////////////////////////////////////////////
//                          Get topology properties                          //
///////////////////////////////////////////////////////////////////////////////

//**********************************************************************
std::string Topology::GetAbsDif(const std::string& gdcc, const std::string& dif) {
  return this->m_gdcc_to_dif_map[std::pair<std::string, std::string>(gdcc, dif)];
}

//**********************************************************************
unsigned Topology::GetAbsDif(unsigned gdcc, unsigned dif) {
  return std::stoi(GetAbsDif(std::to_string(gdcc), std::to_string(dif)));
}

//**********************************************************************
std::pair<std::string, std::string> Topology::GetGdccDifPair(const std::string& dif) {
  return this->m_dif_to_gdcc_map[dif];
}

//**********************************************************************
std::pair<unsigned, unsigned> Topology::GetGdccDifPair(unsigned dif) {
  std::pair<std::string, std::string> gdcc_dir_pair(GetGdccDifPair(std::to_string(dif)));
  return std::pair<unsigned, unsigned>(std::stoi(gdcc_dir_pair.first), std::stoi(gdcc_dir_pair.second));
}

//**********************************************************************
unsigned Topology::GetGdccID(std::string mac) {
  return (unsigned) std::stoi(this->m_mac_to_gdcc_map[boost::to_upper_copy(mac)]);
}

///////////////////////////////////////////////////////////////////////////////
//                              Private methods                              //
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//                            Topology converters                            //
///////////////////////////////////////////////////////////////////////////////

//**********************************************************************
void Topology::GdccMapToDifMap() {
  for (auto const& gdcc : this->m_string_gdcc_map) {
    for (auto const& dif: gdcc.second) {
      for (auto const& asu: dif.second) {
        std::string asu_mm(std::to_string(std::stoi(asu.first) - 1));
        this->m_string_dif_map[this->GetAbsDif(gdcc.first, dif.first)][asu_mm] = this->m_string_gdcc_map[gdcc.first][dif.first][asu.first];
      }
    }
  }
}

//**********************************************************************
void Topology::DifMapToGdccMap() {
  for (auto const& dif : this->m_string_dif_map) {
    for (auto const& asu: dif.second) {
      auto gdcc_dir_pair(this->GetGdccDifPair(dif.first));
      std::string asu_pp(std::to_string(std::stoi(asu.first) + 1));
      this->m_string_gdcc_map[gdcc_dir_pair.first][gdcc_dir_pair.second][asu_pp] = this->m_string_dif_map[dif.first][asu.first];
    }
  }
}

//**********************************************************************
void Topology::StringToUnsigned(void) {
  for (auto const& gdcc: this->m_string_gdcc_map) {
    for (auto const& dif: gdcc.second) {
      for (auto const& asu: dif.second) {
        this->gdcc_map[std::stoi(gdcc.first)][std::stoi(dif.first)][std::stoi(asu.first)] = std::stoi(asu.second);
      }
    }
  }
  for (auto const& dif: this->m_string_dif_map) {
    for (auto const& asu: dif.second) {
      this->dif_map[std::stoi(dif.first)][std::stoi(asu.first)] = std::stoi(asu.second);
    }
  }    
}

///////////////////////////////////////////////////////////////////////////////
//                          Parse mapping JSON files                         //
///////////////////////////////////////////////////////////////////////////////

//**********************************************************************
void Topology::GetGdccMacMapping() {
  
  if (!check_exist::TxtFile(m_mac_mapping_file_path))
    throw wgInvalidFile("[wgTopology] " + m_mac_mapping_file_path + " file not found");
  std::ifstream mapping_file(m_mac_mapping_file_path);
  nlohmann::json mapping_json = nlohmann::json::parse(mapping_file);
  mapping_file.close();

  auto mapping = mapping_json.get<std::map<std::string, nlohmann::json>>();

  for (auto const &item : mapping)
    this->m_mac_to_gdcc_map[item.first] = item.second.get<std::string>();
}

//**********************************************************************
void Topology::GetGdccDifMapping() {
  
  if (!check_exist::TxtFile(m_dif_mapping_file_path))
    throw wgInvalidFile("[wgTopology] " + m_dif_mapping_file_path + " file not found");
  std::ifstream mapping_file(m_dif_mapping_file_path);
  nlohmann::json mapping_json = nlohmann::json::parse(mapping_file);
  mapping_file.close();

  for (auto const &i : mapping_json.get<std::map<std::string, nlohmann::json>>() ) {
    for (auto const &j : i.second.get<std::map<std::string, unsigned>>()) {
      this->m_dif_to_gdcc_map[std::to_string(j.second)] = std::pair<std::string, std::string>(i.first, j.first);
      this->m_gdcc_to_dif_map[std::pair<std::string, std::string>(i.first, j.first)] = std::to_string(j.second);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
//                               Print topology                              //
///////////////////////////////////////////////////////////////////////////////

//**********************************************************************
void Topology::PrintMapDif() {
  for (auto const& dif: this->dif_map) {
    std::cout << "DIF[" << dif.first << "] ";
    for (auto const& asu: dif.second) {
      std::cout << "ASU["<< asu.first << "] = " << asu.second << " ";
    }
  }
  std::cout << "\n";
}

//**********************************************************************
void Topology::PrintMapGdcc() {
  for (auto const& gdcc: this->gdcc_map) {
    std::cout << "GDCC[" << gdcc.first << "] ";
    for (auto const& dif: gdcc.second) {
      std::cout << "DIF[" << dif.first << "] ";
      for (auto const& asu: dif.second) {
        std::cout << "ASU["<< asu.first << "] = " << asu.second << " ";
      }
    }
  }
  std::cout << "\n";
}

///////////////////////////////////////////////////////////////////////////////
//                                From string                                //
///////////////////////////////////////////////////////////////////////////////

std::map<std::string, unsigned> ParseChipRange(std::pair<std::string, unsigned> asu_key) {
  boost::char_separator<char> sep("-");
  boost::tokenizer<boost::char_separator<char>> token(asu_key.first, sep);
  boost::tokenizer<boost::char_separator<char>>::iterator first = token.begin();
  boost::tokenizer<boost::char_separator<char>>::iterator last = token.end();
  if (std::distance(first, last) != 2)
    throw std::invalid_argument("[wgTopology] chip range is invalid : " + asu_key.first);
  int first_chip = std::stoi(*first);
  int last_chip = std::stoi(*++first);
  if (first_chip == last_chip || last_chip < first_chip ||
      last_chip - first_chip >= (int) NCHIPS || first_chip != 1)
    throw std::invalid_argument("[wgTopology] chip range is invalid : " + asu_key.first);
  std::map<std::string, unsigned> asu_map;
  for (int i = 1; i < last_chip; ++i) {
    asu_map[std::to_string(i)] = asu_key.second;
  }
  return asu_map;
}

//**********************************************************************
void Topology::GetTopologyFromString(const std::string& json_string) {
  int max_depth = wagasci_tools::maxDepth(json_string);
  if (max_depth == 3) {
    nlohmann::json json = nlohmann::json::parse(json_string);
    std::map<std::string, nlohmann::json> gdcc_map = json;
    for ( const auto& gdcc : gdcc_map ) {
      std::map<std::string, nlohmann::json> dif_map = gdcc.second;
      for ( auto& dif : dif_map ) {
        std::map<std::string, unsigned> asu_map = dif.second;
        if (asu_map.begin()->first.find('-') != std::string::npos) {
          asu_map.clear();
          asu_map = ParseChipRange(*asu_map.begin());
        }
        for ( const auto& asu : asu_map ) {
          if (gdcc.first == "0" || gdcc.first.empty() ||
              dif.first == "0" || dif.first.empty() ||
              asu.first == "0" || asu.first.empty() ||
              asu.second == 0)
            throw std::invalid_argument("[wgTopology] zero or empty key or value in topology string."
                                        " Remember in the GDCC topology everything starts from 1");
          this->m_string_gdcc_map[gdcc.first][dif.first][asu.first] = std::to_string(asu.second);
        }
      }
    }
  } else if (max_depth == 2) {
    nlohmann::json json = nlohmann::json::parse(json_string);
    std::map<std::string, nlohmann::json> dif_map = json;
    for ( const auto& dif : dif_map ) {
      std::map<std::string, unsigned> asu_map = dif.second;
      for ( const auto& asu : asu_map ) {
        this->m_string_dif_map[dif.first][asu.first] = std::to_string(asu.second);
      }
    }
  }
  else {
    throw std::runtime_error("[wgTopology] Topology JSON string is malformed");
  }
}

///////////////////////////////////////////////////////////////////////////////
//                        From Pyrame config xml file                        //
///////////////////////////////////////////////////////////////////////////////

//**********************************************************************
void Topology::GetTopologyFromFile(const std::string& configxml) {
  std::string json("");
  unsigned igdcc = 1, idif = 1, iasu = 1;
  
  if (!check_exist::XmlFile(configxml))
    throw wgInvalidFile(configxml + " wasn't found or is not valid");

  XMLDocument configfile;
  configfile.LoadFile(configxml.c_str()); 
  XMLElement* ecal = configfile.FirstChildElement("ecal");
  XMLElement* domain = ecal->FirstChildElement("domain");
  XMLElement* acqpc = domain->FirstChildElement("acqpc");
  // GDCCs loop
  for (XMLElement* gdcc = acqpc->FirstChildElement("gdcc"); gdcc != NULL; gdcc = gdcc->NextSiblingElement("gdcc")) {
    if( std::string(gdcc->Attribute("name")) != "gdcc_1_" + std::to_string(igdcc) ) {
      Log.eWrite("[GetTopology] inconsistency found when counting (GDCC = " + std::to_string(igdcc) + ")");
    }
    XMLElement* param_gdcc;
    bool found_gdcc = false;
    std::string gdcc_mac_addr;
    std::string gdcc_id;
    for (param_gdcc = gdcc->FirstChildElement("param"); param_gdcc != NULL; param_gdcc = param_gdcc->NextSiblingElement("param")) {
      if (std::string(param_gdcc->Attribute("name")) == "gdcc_mac_addr") {
        gdcc_mac_addr = param_gdcc->GetText();
        found_gdcc = true;
        // Check if the GDCC MAC address is found in the mac_mapping.txt file
        MacToGdccMap::iterator it = m_mac_to_gdcc_map.find(boost::to_upper_copy(gdcc_mac_addr));
        if (it != m_mac_to_gdcc_map.end()) {
          gdcc_id = it->second;
        } else {
          gdcc_id = std::to_string(igdcc);
          Log.eWrite("[wgTopology] GDCC address \"" + boost::to_upper_copy(gdcc_mac_addr) + "\" not found in mac_mapping.txt");
        }
        break;
      }
    }
    if (!found_gdcc) {
      throw wgElementNotFound("[GetTopology] GDCC mac address not found (gdcc count = " + std::to_string(igdcc) + ")");
    }
    // DIFs loop
    for (XMLElement* dif = gdcc->FirstChildElement("dif"); dif != NULL; dif = dif->NextSiblingElement("dif")) {
      if( std::string(dif->Attribute("name")) != "dif_1_" + std::to_string(igdcc) + "_" + std::to_string(idif) ) {
        Log.eWrite("[GetTopology] inconsistency found when counting (DIF = " + std::to_string(idif) + ")");
      }
      XMLElement* param_dif;
      bool found_dif = false;
      int dif_gdcc_port;
      for (param_dif = dif->FirstChildElement("param"); param_dif != NULL; param_dif = param_dif->NextSiblingElement("param")) {
        if (std::string(param_dif->Attribute("name")) == "dif_gdcc_port") {
          dif_gdcc_port = std::stoi(param_dif->GetText());
          found_dif = true;
          break;
        }
      }
      if (!found_dif) {
        throw wgElementNotFound("[GetTopology] DIF GDCC port not found (dif count = " + std::to_string(idif) + ")");
      }
      // ASUs loop
      for (XMLElement* asu = dif->FirstChildElement("asu"); asu != NULL; asu = asu->NextSiblingElement("asu")) {
        if (std::string(asu->Attribute("name")) != "asu_1_" + std::to_string(igdcc) + "_" + std::to_string(idif) + "_" + std::to_string(iasu) ) {
          Log.eWrite("[GetTopology] inconsistency found when counting (ASU = " + std::to_string(iasu) + ")");
        }
        XMLElement* spiroc2d = asu;
        if (asu->FirstChildElement("spiroc2d") != NULL) spiroc2d = asu->FirstChildElement("spiroc2d");
        // param loop
        XMLElement* param;
        bool found_n_channels = false;
        for (param = spiroc2d->FirstChildElement("param"); param != NULL; param = param->NextSiblingElement("param")) {
          if (std::string(param->Attribute("name")) == "spiroc2d_enable_preamp_chans" ) {
            std::string enabled_channels(param->GetText());
            boost::char_separator<char> * sep;
            if (enabled_channels.find('-') != std::string::npos)
              sep = new boost::char_separator<char>("-");
            else if (enabled_channels.find(',') != std::string::npos)
              sep = new boost::char_separator<char>(",");
            else
              found_n_channels = true;
            if (!found_n_channels) {
              boost::tokenizer<boost::char_separator<char>> token(enabled_channels, *sep);
              boost::tokenizer<boost::char_separator<char>>::iterator first = token.begin();
              boost::tokenizer<boost::char_separator<char>>::iterator last = token.end();
              std::advance(first, std::distance(first, last) - 1);
              enabled_channels = std::to_string(std::stoi(*first) + 1);
              delete sep;
            }
            // Number of enabled channels
            this->m_string_gdcc_map[gdcc_id][std::to_string(dif_gdcc_port)][std::to_string(iasu)] = enabled_channels;
            found_n_channels = true;
            break;
          }
        } // params loop
        if (!found_n_channels)
          throw wgElementNotFound("[wgTopology] Number of channels not found : GDCC " + gdcc_id +
                                  ", DIF " + std::to_string(dif_gdcc_port) + ", ASU " + std::to_string(iasu));
        iasu++;
      } // ASUs loop
      idif++;
      iasu = 1;
    } // DIFs loop
    igdcc++;
    idif = iasu = 1;
  } // GDCCs loop
}

///////////////////////////////////////////////////////////////////////////////
//                             From pedestal tree                            //
///////////////////////////////////////////////////////////////////////////////

//**********************************************************************
void Topology::GetTopologyFromPedestalTree(std::string input_run_dir) {

  // First of all let's do some preliminary sanity checks. The most
  // common mistake is to pass an empty or non existant directory.
  
  // Check the arguments
  
  if (!check_exist::Dir(input_run_dir))
    throw wgInvalidFile("[wgTopology] Input directory doesn't exist : " + input_run_dir);

  // Number of acquisitions for the pe
  std::vector<std::string> pe_dir_list = ListDirectories(input_run_dir);
  if ( pe_dir_list.size() == 0 )
    throw std::invalid_argument("[wgTopology] Empty pe directory tree");

  // tentative variables
  //
  // basically we count the number of DIFs, chips and channels for
  // each acquisition and compare them.
  //
  //   pe                 n_difs             n_chips   n_chans
  std::map<unsigned, std::map<unsigned, std::map<unsigned, unsigned>>> n_chans_t;
  
  wgEditXML Edit;

  /////////////////////////////////////////////////////////////////////////////
  //    We descend into the PedestalCalib directory tree and count the number  //
  //    of directories at each level. When we get to the Summary_chip*.xml   //
  //    file we open it and read the number of channels from it.             //
  /////////////////////////////////////////////////////////////////////////////
  
  // p.e.
  for (auto & pe_directory : pe_dir_list) {
    unsigned ipe = 0;
    if ( (ipe = extractIntegerFromString(GetName(pe_directory))) == UINT_MAX ||
         (ipe != 1 && ipe != 2))
      continue;
    
    // DIF
    pe_directory += "/wgAnaHistSummary/Xml";
    std::vector<std::string> dif_dir_list = ListDirectories(pe_directory);
    if ( dif_dir_list.size() == 0 )
      throw std::invalid_argument("[wgTopology] empty p.e. directory : " + pe_directory);
    for (auto const & idif_directory : dif_dir_list) {
      unsigned idif;
      if ( (idif = extractIntegerFromString(GetName(idif_directory))) == UINT_MAX ) {
        throw wgInvalidFile("[wgTopology] failed to read DIF ID from directory name : "
                            + idif_directory);
      }
        
      // chip
      std::vector<std::string> chip_xml_list = ListFilesWithExtension(idif_directory, "xml");
      if ( chip_xml_list.size() == 0 )
        throw std::invalid_argument("[wgTopology] empty DIF directory : " + idif_directory);
      for (auto const & ichip_xml : chip_xml_list) {
        unsigned ichip;
        if ( (ichip = extractIntegerFromString(GetName(ichip_xml))) == UINT_MAX )
          throw wgInvalidFile("[wgTopology] failed to read chip ID from xml file name : "
                              + ichip_xml);
        
        // chan
        try { Edit.Open(ichip_xml); }
        catch (const std::exception& e) {
          throw wgInvalidFile("[wgTopology] : " + std::string(e.what()));
        }
        this->dif_map[idif][ichip] = n_chans_t[ipe][idif][ichip] =
                                     Edit.SUMMARY_GetGlobalConfigValue("n_chans");
        this->m_string_dif_map[std::to_string(idif)][std::to_string(ichip)] =
            std::to_string(this->dif_map[idif][ichip]);
        Edit.Close();
      }
    }  // end loop for dif
  }  // end loop for inputDAC

  for (auto const& pe : n_chans_t) {
    unsigned ipe = pe.first;
    for (auto const& dif : pe.second) {
      unsigned idif = dif.first;
      if (n_chans_t[ipe].size() != this->dif_map.size()) {
        throw std::runtime_error("[wgTopology] There is something wrong with the number of "
                                 "DIFs detection : pe = " + std::to_string(ipe) +
                                 ", idif = " + idif);
      }
      for (auto const& chip : dif.second) {
        unsigned ichip = chip.first;
        if (n_chans_t[ipe][idif].size() != this->dif_map[idif].size() ) {
          throw std::runtime_error("[wgTopology] There is something wrong with the number of "
                                   "chips detection : pe = " + std::to_string(ipe) +
                                   ", idif = " + idif + ", ichip = " + ichip);
        }
        if (n_chans_t[ipe][idif][ichip] != this->dif_map[idif][ichip]) {
          throw std::runtime_error("[wgTopology] There is something wrong with the number of "
                                   "channels detection : pe = " + std::to_string(ipe) +
                                   ", idif = " + idif + ", ichip = " + ichip);
        }
      }
    }
  }
  
  // Check that the dif numbers are contiguous
  for (unsigned idif = 0; idif < n_difs; idif++) {
    if ( this->dif_map.count(idif) != 1 ) {
      throw std::runtime_error("[wgTopology] DIF number is not contiguous");
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
//                              From Scurve tree                             //
///////////////////////////////////////////////////////////////////////////////

//**********************************************************************
void Topology::GetTopologyFromScurveTree(std::string input_run_dir) {

  // First of all let's do some preliminary sanity checks. The most
  // common mistake is to pass an empty or non existant directory.
  
  // Check the arguments
  
  if (!check_exist::Dir(input_run_dir))
    throw wgInvalidFile("[wgTopology] Input directory doesn't exist : " + input_run_dir);

  // Number of acquisitions for the iDAC
  std::vector<std::string> iDAC_dir_list = ListDirectoriesWithInteger(input_run_dir);
  if ( iDAC_dir_list.size() == 0 )
    throw std::invalid_argument("[wgTopology] Empty iDAC directory tree");

  // tentative variables
  //
  // basically we count the number of DIFs, chips and channels for
  // each acquisition and compare them.
  //
  //   iDAC               thr                n_difs             n_chips       n_chans
  std::map<unsigned, std::map<unsigned, std::map<unsigned, std::map<unsigned, unsigned>>>>
      n_chans_t;
  
  wgEditXML Edit;

  /////////////////////////////////////////////////////////////////////////////
  //    We descend into the Scurve directory tree and count the number of    //
  //    directories at each level. When we get to the Summary_chip*.xml      //
  //    file we open it and read the number of channels from it.             //
  /////////////////////////////////////////////////////////////////////////////
  
  // input DAC
  for (auto const & iDAC_directory : iDAC_dir_list) {
    unsigned iDAC;
    if ( (iDAC = extractIntegerFromString(GetName(iDAC_directory))) == UINT_MAX )
      continue;
    std::vector<std::string> th_dir_list = ListDirectoriesWithInteger(iDAC_directory);
    if ( th_dir_list.size() == 0 )
      throw std::invalid_argument("[wgTopology] empty iDAC directory : " + iDAC_directory);

    // threshold
    for (auto & th_directory : th_dir_list) {
      unsigned threshold;
      if ( (threshold = extractIntegerFromString(GetName(th_directory))) == UINT_MAX )
        continue;

      // DIF
      th_directory += "/wgAnaHistSummary/Xml";
      std::vector<std::string> dif_dir_list = ListDirectories(th_directory);
      if ( dif_dir_list.size() == 0 )
        throw std::invalid_argument("[wgTopology] empty threshold directory : " +
                                    th_directory);
      for (auto const & idif_directory : dif_dir_list) {
        unsigned idif;
        if ( (idif = extractIntegerFromString(GetName(idif_directory))) == UINT_MAX )
          throw wgInvalidFile("[wgTopology] failed to read DIF ID from directory name : "
                              + idif_directory);

        // chip
        std::vector<std::string> chip_xml_list = ListFilesWithExtension(idif_directory, "xml");
        if ( chip_xml_list.size() == 0 )
          throw std::invalid_argument("[wgTopology] empty DIF directory : " + idif_directory);
        for (auto const & ichip_xml : chip_xml_list) {
          unsigned ichip;
          if ( (ichip = extractIntegerFromString(GetName(ichip_xml))) == UINT_MAX )
            throw wgInvalidFile("[wgTopology] failed to read chip ID from xml file name : "
                                + ichip_xml);
        
          // chan
          try { Edit.Open(ichip_xml); }
          catch (const std::exception& e) {
            throw wgInvalidFile("[wgTopology] : " + std::string(e.what()));
          }
          this->dif_map[idif][ichip] = n_chans_t[iDAC][threshold][idif][ichip] =
                                       Edit.SUMMARY_GetGlobalConfigValue("n_chans");
          this->m_string_dif_map[std::to_string(idif)][std::to_string(ichip)] =
              std::to_string(this->dif_map[idif][ichip]);
          Edit.Close();
        }
      }  // end loop for dif
    }  // end loop for threshold
  }  // end loop for inputDAC

  for (auto const& iDAC : n_chans_t) {
    unsigned iiDAC = iDAC.first;
    
    for (auto const& th : iDAC.second) {
      unsigned ith = th.first;
      for (auto const& dif : th.second) {
        unsigned idif = dif.first;
        if (n_chans_t[iiDAC][ith].size() != this->dif_map.size()) {
          throw std::runtime_error("[wgTopology] There is something wrong with the number of "
                                   "DIFs detection : iDAC = " + std::to_string(iiDAC) +
                                   ", threshold = " + std::to_string(ith) + ", idif = "
                                   + std::to_string(idif));
        }
        for (auto const& chip : dif.second) {
          unsigned ichip = chip.first;
          if (n_chans_t[iiDAC][ith][idif].size() != this->dif_map[idif].size() ) {
            throw std::runtime_error("[wgTopology] There is something wrong with the number of"
                                     " chips detection : iDAC = " + std::to_string(iiDAC) +
                                     ", threshold = " + std::to_string(ith) +
                                     ", idif = " + std::to_string(idif) +
                                     ", ichip = " + ichip);
          }
          if (n_chans_t[iiDAC][ith][idif][ichip] != this->dif_map[idif][ichip]) {
            throw std::runtime_error("[wgTopology] There is something wrong with the number of"
                                     " channels detection : iDAC = " + std::to_string(iiDAC) +
                                     ", threshold = " + std::to_string(ith) +
                                     ", idif = " + std::to_string(idif) +
                                     ", ichip = " + ichip);
          }
        }
      }
    }
  }

  // Check that the dif numbers are contiguous
  for (unsigned idif = 0; idif < n_difs; idif++) {
    if ( this->dif_map.count(idif) != 1 ) {
      throw std::runtime_error("[wgTopology] DIF number is not contiguous");
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
//                               From gain tree                              //
///////////////////////////////////////////////////////////////////////////////

//**********************************************************************
void Topology::GetTopologyFromGainTree(std::string input_run_dir) {

  // First of all let's do some preliminary sanity checks. The most
  // common mistake is to pass an empty or non existant directory.
  
  // Check the arguments
  
  if (!check_exist::Dir(input_run_dir))
    throw wgInvalidFile("[wgTopology] Input directory doesn't exist : " + input_run_dir);

  // Number of acquisitions for the iDAC
  std::vector<std::string> iDAC_dir_list = ListDirectories(input_run_dir);
  if ( iDAC_dir_list.size() == 0 )
    throw std::invalid_argument("[wgTopology] Empty iDAC directory tree");

  // tentative variables
  //
  // basically we count the number of DIFs, chips and channels for
  // each acquisition and compare them.
  //
  //   iDAC               PEU                n_difs             n_chips       n_chans
  std::map<unsigned, std::map<unsigned, std::map<unsigned, std::map<unsigned, unsigned>>>> n_chans_t;
  
  wgEditXML Edit;

  /////////////////////////////////////////////////////////////////////////////
  //    We descend into the GainCalib directory tree and count the number of //
  //    directories at each level. When we get to the Summary_chip*.xml      //
  //    file we open it and read the number of channels from it.             //
  /////////////////////////////////////////////////////////////////////////////
  
  // input DAC
  for (auto const & iDAC_directory : iDAC_dir_list) {
    unsigned iDAC;
    if ( (iDAC = extractIntegerFromString(GetName(iDAC_directory))) == UINT_MAX )
      continue;
    std::vector<std::string> pe_dir_list = ListDirectories(iDAC_directory);
    if (pe_dir_list.size() != N_PE_GAIN_CALIB)
      throw std::invalid_argument("[wgTopology] empty iDAC directory : " + iDAC_directory);

    // pereshold
    for (auto & pe_directory : pe_dir_list) {
      unsigned photo_equivalent_unit;
      if ((photo_equivalent_unit = extractIntegerFromString(GetName(pe_directory))) == UINT_MAX)
        continue;

      // DIF
      pe_directory += "/wgAnaHistSummary/Xml";
      std::vector<std::string> dif_dir_list = ListDirectories(pe_directory);
      if ( dif_dir_list.size() == 0 )
        throw std::invalid_argument("[wgTopology] empty photo_equivalent_unit directory : " + pe_directory);
      for (auto const & idif_directory : dif_dir_list) {
        unsigned idif;
        if ((idif = extractIntegerFromString(GetName(idif_directory))) == UINT_MAX)
          throw wgInvalidFile("[wgTopology] failed to read DIF ID from directory name : " + idif_directory);

        // chip
        std::vector<std::string> chip_xml_list = ListFilesWithExtension(idif_directory, "xml");
        if ( chip_xml_list.size() == 0 )
          throw std::invalid_argument("[wgTopology] empty DIF directory : " + idif_directory);
        for (auto const & ichip_xml : chip_xml_list) {
          unsigned ichip;
          if ( (ichip = extractIntegerFromString(GetName(ichip_xml))) == UINT_MAX )
            throw wgInvalidFile("[wgTopology] failed to read chip ID from xml file name : " + ichip_xml);
        
          // chan
          try { Edit.Open(ichip_xml); }
          catch (const std::exception& e) {
            throw wgInvalidFile("[wgTopology] : " + std::string(e.what()));
          }
          this->dif_map[idif][ichip] = n_chans_t[iDAC][photo_equivalent_unit][idif][ichip] = Edit.SUMMARY_GetGlobalConfigValue("n_chans");
          this->m_string_dif_map[std::to_string(idif)][std::to_string(ichip)] = std::to_string(this->dif_map[idif][ichip]);
          Edit.Close();
        }
      }  // end loop for dif
    }  // end loop for photo_equivalent_unit
  }  // end loop for inputDAC

  for (auto const& iDAC : n_chans_t) {
    unsigned iiDAC = iDAC.first;
    
    for (auto const& pe : iDAC.second) {
      unsigned ipe = pe.first;
      for (auto const& dif : pe.second) {
        unsigned idif = dif.first;
        if (n_chans_t[iiDAC][ipe].size() != this->dif_map.size()) {
          throw std::runtime_error("[wgTopology] There is something wrong with the number of DIFs detection : iDAC = " + std::to_string(iiDAC) +
                                   ", photo_equivalent_unit = " + std::to_string(ipe) + ", idif = " + idif);
        }
        for (auto const& chip : dif.second) {
          unsigned ichip = chip.first;
          if (n_chans_t[iiDAC][ipe][idif].size() != this->dif_map[idif].size() ) {
            throw std::runtime_error("[wgTopology] There is something wrong with the number of chips detection : iDAC = " + std::to_string(iiDAC) +
                                     ", photo_equivalent_unit = " + std::to_string(ipe) + ", idif = " + idif + ", ichip = " + ichip);
          }
          if (n_chans_t[iiDAC][ipe][idif][ichip] != this->dif_map[idif][ichip]) {
            throw std::runtime_error("[wgTopology] There is something wrong with the number of channels detection : iDAC = " + std::to_string(iiDAC) +
                                     ", photo_equivalent_unit = " + std::to_string(ipe) + ", idif = " + idif + ", ichip = " + ichip);
          }
        }
      }
    }
  }

  // Check that the dif numbers are contiguous
  for (unsigned idif = 0; idif < n_difs; idif++) {
    if ( this->dif_map.count(idif) != 1 ) {
      throw std::runtime_error("[wgTopology] DIF number is not contiguous");
    }
  }
}
