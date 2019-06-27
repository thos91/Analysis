// system C++ includes
#include <iostream>
#include <string>
#include <map>

// boost includes
#include <boost/tokenizer.hpp>

// json includes
#include <nlohmann/json.hpp>

// tinyxml2 includes
#include "tinyxml2.hpp"

//user includes
#include "wgExceptions.hpp"
#include "wgFileSystemTools.hpp"
#include "wgErrorCode.hpp"
#include "wgLogger.hpp"
#include "wgEditXML.hpp"
#include "wgTopology.hpp"

using namespace wagasci_tools;

//**********************************************************************
const char * GetTopologyCtypes(const char * x_configxml) {
  string configxml(x_configxml);
  string json("");

  char * topology_string = (char *) malloc(TOPOLOGY_STRING_LENGTH * sizeof(char));
  if (topology_string == NULL){
    Log.eWrite("[GetTopologyCtypes] failed to allocate topology string");
    return "";
  }
  
  StringTopologyMapDif topology_map;
  
  try {
    Topology topology(configxml, TopologySourceType::xml_file);
    topology_map = topology.m_string_dif_map;
  } // try/catch
  catch (const exception& e) {
    Log.eWrite("[GetTopologyCtypes] " + string(e.what()));
    topology_string[0] = '\0';
    return topology_string;
  }

  nlohmann::json topology_json = topology_map;
  int result = snprintf(topology_string, TOPOLOGY_STRING_LENGTH, "%s",
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

Topology::Topology(const char * configxml, TopologySourceType source_type) :
  Topology(string(configxml), source_type) {}

Topology::Topology(string source, TopologySourceType source_type) :
  m_mapping_file_path("/opt/calicoes/config/dif_mapping.txt") {

  if ( source_type == TopologySourceType::xml_file ) {
    this->GetTopologyFromFile(source);
    this->GetGdccDifMapping();
    this->GdccMapToDifMap();
    this->StringToUnsigned();
  }
  else if ( source_type == TopologySourceType::json_string ) {
    this->GetTopologyFromString(source);
    this->GetGdccDifMapping();
    this->DifMapToGdccMap();
    this->StringToUnsigned();
  }
  else if ( source_type == TopologySourceType::scurve_tree ) {
    this->GetTopologyFromScurveTree(source);
    this->GetGdccDifMapping();
    this->DifMapToGdccMap();
    this->StringToUnsigned();
  }
  else if ( source_type == TopologySourceType::pedestal_tree ) {
    this->GetTopologyFromPedestalTree(source);
    this->GetGdccDifMapping();
    this->DifMapToGdccMap();
    this->StringToUnsigned();
  }
  else
    throw invalid_argument("[wgTopology] TopologySourceType not recognized");
      
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

//**********************************************************************
void Topology::GetTopologyFromString(const string& json_string) {
  nlohmann::json json = nlohmann::json::parse(json_string);
  map<string, nlohmann::json> dif_map = json;
  for ( const auto& dif : dif_map ) {
    map<string, unsigned> asu_map = dif.second;
    for ( const auto& asu : asu_map ) {
      this->m_string_dif_map[dif.first][asu.first] = to_string(asu.second);
    }
  }
}

//**********************************************************************
void Topology::GetTopologyFromFile(const string& configxml) {
  string json("");
  unsigned igdcc = 1, idif = 1, iasu = 1;
  bool found = false;
  
  CheckExist Check;
  if(!Check.XmlFile(configxml))
    throw wgInvalidFile(configxml + " wasn't found or is not valid");

  XMLDocument configfile;
  configfile.LoadFile(configxml.c_str()); 
  XMLElement* ecal = configfile.FirstChildElement("ecal");
  XMLElement* domain = ecal->FirstChildElement("domain");
  XMLElement* acqpc = domain->FirstChildElement("acqpc");
  // GDCCs loop
  for(XMLElement* gdcc = acqpc->FirstChildElement("gdcc"); gdcc != NULL; gdcc = gdcc->NextSiblingElement("gdcc")) {
    if( string(gdcc->Attribute("name")) != "gdcc_1_" + to_string(igdcc) ) {
      Log.eWrite("[GetTopology] inconsistency found when counting (GDCC = " + to_string(igdcc) + ")");
    }
    // DIFs loop
    for(XMLElement* dif = gdcc->FirstChildElement("dif"); dif != NULL; dif = dif->NextSiblingElement("dif")) {
      if( string(dif->Attribute("name")) != "dif_1_" + to_string(igdcc) + "_" + to_string(idif) ) {
        Log.eWrite("[GetTopology] inconsistency found when counting (DIF = " + to_string(idif) + ")");
      }
      // ASUs loop
      for(XMLElement* asu = dif->FirstChildElement("asu"); asu != NULL; asu = asu->NextSiblingElement("asu")) {
        if( string(asu->Attribute("name")) != "asu_1_" + to_string(igdcc) + "_" + to_string(idif) + "_" + to_string(iasu) ) {
          Log.eWrite("[GetTopology] inconsistency found when counting (ASU = " + to_string(iasu) + ")");
        }
        XMLElement* spiroc2d = asu;
        if ( asu->FirstChildElement("spiroc2d") != NULL) spiroc2d = asu->FirstChildElement("spiroc2d");
        // param loop
        XMLElement* param;
        for(param = spiroc2d->FirstChildElement("param"); param != NULL; param = param->NextSiblingElement("param")) {
          if( string(param->Attribute("name")) == "spiroc2d_enable_preamp_chans" ) {
            string enabled_channels(param->GetText());
            boost::char_separator<char> * sep;
            if (enabled_channels.find('-') != string::npos)
              sep = new boost::char_separator<char>("-");
            else if (enabled_channels.find(',') != string::npos)
              sep = new boost::char_separator<char>(",");
            else {
              this->m_string_gdcc_map[to_string(igdcc)][to_string(idif)][to_string(iasu)] = enabled_channels;
              found = true;
              break;
            }
            typedef boost::tokenizer<boost::char_separator<char>> t_tokenizer;
            t_tokenizer token(enabled_channels, *sep);
            boost::tokenizer<boost::char_separator<char>>::iterator first = token.begin();
            boost::tokenizer<boost::char_separator<char>>::iterator last = token.end();
            advance(first, distance(first, last) - 1);
            // Number of enabled channels
            this->m_string_gdcc_map[to_string(igdcc)][to_string(idif)][to_string(iasu)] = to_string(stoi(*first) + 1);
            found = true;
            break;
          }

        } // params loop
        if (!found)
          throw wgElementNotFound("Number of channels not found : GDCC " + to_string(igdcc) + ", DIF " + to_string(idif) + ", ASU " + to_string(iasu));
        found = false;
        iasu++;
      } // ASUs loop
      idif++;
      iasu = 1;
    } // DIFs loop
    igdcc++;
    idif = iasu = 1;
  } // GDCCs loop
}

//**********************************************************************
void Topology::StringToUnsigned(void) {
  for (auto const& gdcc: this->m_string_gdcc_map) {
    for (auto const& dif: gdcc.second) {
      for (auto const& asu: dif.second) {
        this->gdcc_map[stoi(gdcc.first)][stoi(dif.first)][stoi(asu.first)] = stoi(asu.second);
      }
    }
  }
  for (auto const& dif: this->m_string_dif_map) {
    for (auto const& asu: dif.second) {
      this->dif_map[stoi(dif.first)][stoi(asu.first)] = stoi(asu.second);
    }
  }    
}

//**********************************************************************
string Topology::GetAbsDif(const string& gdcc, const string& dif) {
  return this->m_gdcc_to_dif_map[pair<string, string>(gdcc, dif)];
}

//**********************************************************************
unsigned Topology::GetAbsDif(unsigned gdcc, unsigned dif) {
  return stoi(GetAbsDif(to_string(gdcc), to_string(dif)));
}

//**********************************************************************
pair<string, string> Topology::GetGdccDifPair(const string& dif) {
  return this->m_dif_to_gdcc_map[dif];
}

//**********************************************************************
pair<unsigned, unsigned> Topology::GetGdccDifPair(unsigned dif) {
  pair<string, string> gdcc_dir_pair(GetGdccDifPair(to_string(dif)));
  return pair<unsigned, unsigned>(stoi(gdcc_dir_pair.first), stoi(gdcc_dir_pair.first));
}

//**********************************************************************
void Topology::GetGdccDifMapping() {
  CheckExist check;
  if (!check.TxtFile(m_mapping_file_path))
    throw wgInvalidFile(m_mapping_file_path + " file not found");
  ifstream mapping_file(m_mapping_file_path);
  nlohmann::json mapping_json = nlohmann::json::parse(mapping_file);
  mapping_file.close();

  for (auto const &i : mapping_json.get<map<string, nlohmann::json>>() ) {
    for (auto const &j : i.second.get<map<string, unsigned>>()) {
      this->m_dif_to_gdcc_map[to_string(j.second)] = pair<string, string>(i.first, j.first);
      this->m_gdcc_to_dif_map[pair<string, string>(i.first, j.first)] = to_string(j.second);
    }
  }
}

//**********************************************************************
void Topology::GdccMapToDifMap() {
  for (auto const& gdcc : this->m_string_gdcc_map) {
    for (auto const& dif: gdcc.second) {
      for (auto const& asu: dif.second) {
        this->m_string_dif_map[this->GetAbsDif(gdcc.first, dif.first)][asu.first] = this->m_string_gdcc_map[gdcc.first][dif.first][asu.first];
      }
    }
  }
}

//**********************************************************************
void Topology::DifMapToGdccMap() {
  for (auto const& dif : this->m_string_dif_map) {
    for (auto const& asu: dif.second) {
      auto gdcc_dir_pair(this->GetGdccDifPair(dif.first));
      this->m_string_gdcc_map[gdcc_dir_pair.first][gdcc_dir_pair.second][asu.first] = this->m_string_dif_map[dif.first][asu.first];
    }
  }
}

//**********************************************************************
void Topology::PrintMapDif() {
  for (auto const& dif: this->dif_map) {
    cout << "DIF[" << dif.first << "] ";
    for (auto const& asu: dif.second) {
      cout << "ASU["<< asu.first << "] = " << asu.second << " ";
    }
  }
  cout << endl;
}

//**********************************************************************
void Topology::PrintMapGdcc() {
  for (auto const& gdcc: this->gdcc_map) {
    cout << "GDCC[" << gdcc.first << "] ";
    for (auto const& dif: gdcc.second) {
      cout << "DIF[" << dif.first << "] ";
      for (auto const& asu: dif.second) {
        cout << "ASU["<< asu.first << "] = " << asu.second << " ";
      }
    }
  }
  cout << endl;
}

//**********************************************************************
void Topology::GetTopologyFromPedestalTree(string input_run_dir) {

  // First of all let's do some preliminary sanity checks. The most
  // common mistake is to pass an empty or non existant directory.
  
  // Check the arguments
  CheckExist check;
  if (!check.Dir(input_run_dir))
    throw wgInvalidFile("[wgTopology] Input directory doesn't exist : " + input_run_dir);

  // Number of acquisitions for the pe
  vector<string> pe_dir_list = ListDirectories(input_run_dir);
  if ( pe_dir_list.size() == 0 )
    throw invalid_argument("[wgTopology] Empty pe directory tree");

  // tentative variables
  //
  // basically we count the number of DIFs, chips and channels for
  // each acquisition and compare them.
  //
  //  pe            n_difs        n_chips   n_chans
  map<unsigned, map<unsigned, map<unsigned, unsigned>>> n_chans_t;
  
  wgEditXML Edit;

  /////////////////////////////////////////////////////////////////////////////
  //    We descend into the AnaPedestal directory tree and count the number  //
  //    of directories at each level. When we get to the Summary_chip*.xml   //
  //    file we open it and read the number of channels from it.             //
  /////////////////////////////////////////////////////////////////////////////
  
  // p.e.
  for (auto & pe_directory : pe_dir_list) {
    unsigned ipe;
    if ( (ipe = extractIntegerFromString(GetName(pe_directory))) == UINT_MAX )
      throw wgInvalidFile("[wgTopology] failed to read the p.e. value from directory name : " + pe_directory);

    // DIF
    pe_directory += "/wgAnaHistSummary/Xml";
    vector<string> dif_dir_list = ListDirectories(pe_directory);
    if ( dif_dir_list.size() == 0 )
      throw invalid_argument("[wgTopology] empty p.e. directory : " + pe_directory);
    for (auto const & idif_directory : dif_dir_list) {
      unsigned idif_id;
      if ( (idif_id = extractIntegerFromString(GetName(idif_directory))) == UINT_MAX ) {
        throw wgInvalidFile("[wgTopology] failed to read DIF ID from directory name : " + idif_directory);
      }
        
      // chip
      vector<string> chip_xml_list = ListFilesWithExtension(idif_directory, "xml");
      if ( chip_xml_list.size() == 0 )
        throw invalid_argument("[wgTopology] empty DIF directory : " + idif_directory);
      for (auto const & ichip_xml : chip_xml_list) {
        unsigned ichip_id;
        if ( (ichip_id = extractIntegerFromString(GetName(ichip_xml))) == UINT_MAX )
          throw wgInvalidFile("[wgTopology] failed to read chip ID from xml file name : " + ichip_xml);
        
        // chan
        try { Edit.Open(ichip_xml); }
        catch (const exception& e) {
          throw wgInvalidFile("[wgTopology] : " + string(e.what()));
        }
        this->dif_map[idif_id][ichip_id] = n_chans_t[ipe][idif_id][ichip_id] = Edit.SUMMARY_GetGlobalConfigValue("n_chans");
        this->m_string_dif_map[to_string(idif_id)][to_string(ichip_id)] = to_string(this->dif_map[idif_id][ichip_id]);
        Edit.Close();
      }
    }  // end loop for dif
  }  // end loop for inputDAC

  for (auto const& pe : n_chans_t) {
    unsigned ipe = pe.first;
    for (auto const& dif : pe.second) {
      unsigned idif_id = dif.first;
      if (n_chans_t[ipe].size() != this->dif_map.size()) {
        throw runtime_error("There is something wrong with the number of DIFs detection : pe = " + to_string(ipe) + ", idif_id = " + idif_id);
      }
      for (auto const& chip : dif.second) {
        unsigned ichip_id = chip.first;
        if (n_chans_t[ipe][idif_id].size() != this->dif_map[idif_id].size() ) {
          throw runtime_error("There is something wrong with the number of chips detection : pe = " + to_string(ipe) + ", idif_id = " + idif_id + ", ichip_id = " + ichip_id);
        }
        if (n_chans_t[ipe][idif_id][ichip_id] != this->dif_map[idif_id][ichip_id]) {
          throw runtime_error("[wgTopology] There is something wrong with the number of channels detection : pe = " + to_string(ipe) + ", idif_id = " + idif_id + ", ichip_id = " + ichip_id);
        }
      }
    }
  }
  
  // Check that the dif numbers are contiguous
  for (unsigned idif_id = 1; idif_id <= n_difs; idif_id++) {
    if ( this->dif_map.count(idif_id) != 1 ) {
      throw runtime_error("[wgTopology] DIF number is not contiguous");
    }
  }
}

//**********************************************************************
void Topology::GetTopologyFromScurveTree(string input_run_dir) {

  // First of all let's do some preliminary sanity checks. The most
  // common mistake is to pass an empty or non existant directory.
  
  // Check the arguments
  CheckExist check;
  if (!check.Dir(input_run_dir))
    throw wgInvalidFile("[wgTopology] Input directory doesn't exist : " + input_run_dir);

  // Number of acquisitions for the iDAC
  vector<string> iDAC_dir_list = ListDirectories(input_run_dir);
  if ( iDAC_dir_list.size() == 0 )
    throw invalid_argument("[wgTopology] Empty iDAC directory tree");

  // tentative variables
  //
  // basically we count the number of DIFs, chips and channels for
  // each acquisition and compare them.
  //
  //  iDAC          thr           n_difs        n_chips   n_chans
  map<unsigned, map<unsigned, map<unsigned, map<unsigned, unsigned>>>> n_chans_t;
  
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
      throw wgInvalidFile("[wgTopology] failed to read input DAC value from directory name : " + iDAC_directory);
    vector<string> th_dir_list = ListDirectories(iDAC_directory);
    if ( th_dir_list.size() == 0 )
      throw invalid_argument("[wgTopology] empty iDAC directory : " + iDAC_directory);

    // threshold
    for (auto & th_directory : th_dir_list) {
      unsigned threshold;
      if ( (threshold = extractIntegerFromString(GetName(th_directory))) == UINT_MAX )
        throw wgInvalidFile("[wgTopology] failed to read threshold value from directory name : " + th_directory);

      // DIF
      th_directory += "/wgAnaHistSummary/Xml";
      vector<string> dif_dir_list = ListDirectories(th_directory);
      if ( dif_dir_list.size() == 0 )
        throw invalid_argument("[wgTopology] empty threshold directory : " + th_directory);
      for (auto const & idif_directory : dif_dir_list) {
        unsigned idif_id;
        if ( (idif_id = extractIntegerFromString(GetName(idif_directory))) == UINT_MAX )
          throw wgInvalidFile("[wgTopology] failed to read DIF ID from directory name : " + idif_directory);

        // chip
        vector<string> chip_xml_list = ListFilesWithExtension(idif_directory, "xml");
        if ( chip_xml_list.size() == 0 )
          throw invalid_argument("[wgTopology] empty DIF directory : " + idif_directory);
        for (auto const & ichip_xml : chip_xml_list) {
          unsigned ichip_id;
          if ( (ichip_id = extractIntegerFromString(GetName(ichip_xml))) == UINT_MAX )
            throw wgInvalidFile("[wgTopology] failed to read chip ID from xml file name : " + ichip_xml);
        
          // chan
          try { Edit.Open(ichip_xml); }
          catch (const exception& e) {
            throw wgInvalidFile("[wgTopology] : " + string(e.what()));
          }
          this->dif_map[idif_id][ichip_id] = n_chans_t[iDAC][threshold][idif_id][ichip_id] = Edit.SUMMARY_GetGlobalConfigValue("n_chans");
          this->m_string_dif_map[to_string(idif_id)][to_string(ichip_id)] = to_string(this->dif_map[idif_id][ichip_id]);
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
        unsigned idif_id = dif.first;
        if (n_chans_t[iiDAC][ith].size() != this->dif_map.size()) {
          throw runtime_error("There is something wrong with the number of DIFs detection : iDAC = " + to_string(iiDAC) + ", threshold = " + to_string(ith) + ", idif_id = " + idif_id);
        }
        for (auto const& chip : dif.second) {
          unsigned ichip_id = chip.first;
          if (n_chans_t[iiDAC][ith][idif_id].size() != this->dif_map[idif_id].size() ) {
            throw runtime_error("There is something wrong with the number of chips detection : iDAC = " + to_string(iiDAC) + ", threshold = " + to_string(ith) + ", idif_id = " + idif_id + ", ichip_id = " + ichip_id);
          }
          if (n_chans_t[iiDAC][ith][idif_id][ichip_id] != this->dif_map[idif_id][ichip_id]) {
            throw runtime_error("[wgTopology] There is something wrong with the number of channels detection : iDAC = " + to_string(iiDAC) + ", threshold = " + to_string(ith) + ", idif_id = " + idif_id + ", ichip_id = " + ichip_id);
          }
        }
      }
    }
  }

  // Check that the dif numbers are contiguous
  for (unsigned idif_id = 1; idif_id <= n_difs; idif_id++) {
    if ( this->dif_map.count(idif_id) != 1 ) {
      throw runtime_error("[wgTopology] DIF number is not contiguous");
    }
  }
}
