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
#include "wgTopology.hpp"

//**********************************************************************
const char * GetTopologyCtypes(const char * x_configxml) {
  string configxml(x_configxml);
  string json("");

  char * topology_string = (char *) malloc(TOPOLOGY_STRING_LENGTH * sizeof(char));
  if (topology_string == NULL){
    Log.eWrite("[GetTopologyCtypes] failed to allocate topology string");
    return "";
  }
  
  TopologyMapDif topology_map;
  
  try {
    Topology topology(configxml, TopologySourceType::xml_file);
    topology_map = topology.dif_map;
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
  }
  else if ( source_type == TopologySourceType::json_string ) {
    this->GetTopologyFromString(source);
    this->GetGdccDifMapping();
    this->DifMapToGdccMap(); 
  }
  else
    throw std::invalid_argument("[wgTopology] TopologySourceType not recognized");
      
  this->n_difs = this->dif_map.size();
  for ( auto const & dif : this->dif_map) {
    unsigned n_chips_tmp = 0;
    for ( auto const & asu : dif.second) {
      n_chips_tmp++;
      if ( (unsigned) stoi(asu.second) > this->max_channels) max_channels = stoi(asu.second);
    }
    if (n_chips_tmp > this->max_chips) max_chips = n_chips_tmp;
  }
}

//**********************************************************************
void Topology::GetTopologyFromString(const string& json_string) {
  nlohmann::json json = nlohmann::json::parse(json_string);
  std::map<string, nlohmann::json> dif_map = json;
  for ( const auto& dif : dif_map ) {
    std::map<string, unsigned> asu_map = dif.second;
    for ( const auto& asu : asu_map ) {
      this->dif_map[dif.first][asu.first] = to_string(asu.second);
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
            if (enabled_channels.find('-') != std::string::npos)
              sep = new boost::char_separator<char>("-");
            else if (enabled_channels.find(',') != std::string::npos)
              sep = new boost::char_separator<char>(",");
            else {
              this->gdcc_map[to_string(igdcc)][to_string(idif)][to_string(iasu)] = enabled_channels;
              found = true;
              break;
            }
            typedef boost::tokenizer<boost::char_separator<char>> t_tokenizer;
            t_tokenizer token(enabled_channels, *sep);
            boost::tokenizer<boost::char_separator<char>>::iterator first = token.begin();
            boost::tokenizer<boost::char_separator<char>>::iterator last = token.end();
            std::advance(first, std::distance(first, last) - 1);
            // Number of enabled channels
            this->gdcc_map[to_string(igdcc)][to_string(idif)][to_string(iasu)] = to_string(stoi(*first) + 1);
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
string Topology::GetAbsDif(const string& gdcc, const string& dif) {
  return this->m_gdcc_to_dif_map[std::pair<string, string>(gdcc, dif)];
}

//**********************************************************************
unsigned Topology::GetAbsDif(unsigned gdcc, unsigned dif) {
  return stoi(GetAbsDif(to_string(gdcc), to_string(dif)));
}

//**********************************************************************
std::pair<string, string> Topology::GetGdccDifPair(const string& dif) {
  return this->m_dif_to_gdcc_map[dif];
}

//**********************************************************************
std::pair<unsigned, unsigned> Topology::GetGdccDifPair(unsigned dif) {
  std::pair<string, string> gdcc_dir_pair(GetGdccDifPair(to_string(dif)));
  return std::pair<unsigned, unsigned>(stoi(gdcc_dir_pair.first), stoi(gdcc_dir_pair.first));
}

//**********************************************************************
void Topology::GetGdccDifMapping() {
  CheckExist check;
  if (!check.TxtFile(m_mapping_file_path))
    throw wgInvalidFile(m_mapping_file_path + " file not found");
  std::ifstream mapping_file(m_mapping_file_path);
  nlohmann::json mapping_json = nlohmann::json::parse(mapping_file);
  mapping_file.close();

  for (auto const &i : mapping_json.get<std::map<string, nlohmann::json>>() ) {
    for (auto const &j : i.second.get<std::map<string, unsigned>>()) {
      this->m_dif_to_gdcc_map[to_string(j.second)] = std::pair<string, string>(i.first, j.first);
      this->m_gdcc_to_dif_map[std::pair<string, string>(i.first, j.first)] = to_string(j.second);
    }
  }
}

//**********************************************************************
void Topology::GdccMapToDifMap() {
  for (auto const& gdcc : this->gdcc_map) {
    for (auto const& dif: gdcc.second) {
      for (auto const& asu: dif.second) {
        this->dif_map[this->GetAbsDif(gdcc.first, dif.first)][asu.first] = this->gdcc_map[gdcc.first][dif.first][asu.first];
      }
    }
  }
}

//**********************************************************************
void Topology::DifMapToGdccMap() {
  for (auto const& dif : this->dif_map) {
    for (auto const& asu: dif.second) {
      std::pair<string, string> gdcc_dir_pair(this->GetGdccDifPair(dif.first));
      this->gdcc_map[gdcc_dir_pair.first][gdcc_dir_pair.second][asu.first] = this->dif_map[dif.first][asu.first];
    }
  }
}

//**********************************************************************
void Topology::PrintMapDif() {
    for (auto const& dif: this->dif_map) {
      std::cout << "DIF[" << dif.first << "] ";
      for (auto const& asu: dif.second) {
        std::cout << "ASU["<< asu.first << "] = " << asu.second << " ";
      }
    }
  std::cout << std::endl;
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
  std::cout << std::endl;
}

//**********************************************************************
Topology::Topology(DirectoryTreeMap run_directory_tree, string input_run_dir):
  m_mapping_file_path("/opt/calicoes/config/dif_mapping.txt") {

  this->GetTopologyFromDirectoryTree(run_directory_tree, input_run_dir);
  this->GetGdccDifMapping();
  this->DifMapToGdccMap(); 
  this->n_difs = this->dif_map.size();
  for ( auto const & dif : this->dif_map) {
    unsigned n_chips_tmp = 0;
    for ( auto const & asu : dif.second) {
      n_chips_tmp++;
      if ( (unsigned) stoi(asu.second) > this->max_channels) max_channels = stoi(asu.second);
    }
    if (n_chips_tmp > this->max_chips) max_chips = n_chips_tmp;
  }
}

}
void Topology::GetTopologyFromDirectoryTree(DirectoryTreeMap run_directory_tree, string input_run_dir){
	
  // The topology for each acquisition (each photo-electron equivalent
  // threshold) MUST be the same. I mean same number of DIFs, chips
  // and channels
  
  unsigned n_difs;
  vector<unsigned> n_chips;
  vector<vector<unsigned>> n_chans;
  
  array<unsigned, run_directory_tree.size()> n_difs_x;
  array<vector<unsigned>, run_directory_tree.size()> n_chips_x;
  array<vector<vector<unsigned>>, run_directory_tree.size()> n_chans_x;

  // Get topology for each acquisition
  
  for (auto const & it : run_directory_tree) {
    unsigned x = it.first;
    string x_directory = it.second;
    n_difs_x[x] = HowManyDirectories(input_run_dir + x_directory);
    if ( n_difs_x[x] == 0) {
      Log.eWrite("[wgAnaPedestal] DIF directories not found");
      return ERR_WRONG_DIF_VALUE;
    }
    for (unsigned idif = 0; idif < n_difs_x[x]; idif++) {
      string idif_directory(input_run_dir + x_directory + "/dif" + to_string(idif + 1));
      n_chips_x[x].push_back(HowManyFilesWithExtension(idif_directory, "xml"));
      n_chans_x[x].push_back( vector<unsigned>() );
      
      for (unsigned ichip = 0; ichip < n_chips_x[x][idif]; ichip++) {
        string xmlfile(idif_directory + "/Summary_chip" + to_string(ichip + 1) + ".xml");
        try { Edit.Open(xmlfile); }
        catch (const exception& e) {
          Log.eWrite("[wgAnaPedestal] " + string(e.what()));
          return ERR_FAILED_OPEN_XML_FILE;
        }
        n_chans_x[x][idif].push_back(Edit.SUMMARY_GetGlobalConfigValue("n_chans"));
        Edit.Close();

      }
    }
  }

  // Copy the topology for the first case into the global topology

  n_difs = n_difs_x[0];
  for (unsigned idif = 0; idif < n_difs_x[0]; idif++) {
    n_chips.push_back(n_chips_x[0][idif]);
    n_chans.push_back( vector<unsigned>() );
    for (unsigned ichip = 0; ichip < n_chips_x[0][idif]; ichip++) {
      n_chans[idif].push_back( n_chans_x[0][idif][ichip] );
    }
  }

  // Check that the topology for each acquisition is the same
          
  for (unsigned x = 0; x < run_directory_tree.size(); ++x) {
    if (n_difs_x[x] != n_difs) {
      Log.eWrite("There is something wrong with the number of DIFs detection");
      return ERR_WRONG_DIF_VALUE;
    }
    for (unsigned idif = 0; idif < n_difs_x[x]; idif++) {
      if ( n_chips_x[x][idif] != n_chips[idif] ) {
        Log.eWrite("There is something wrong with the number of chips detection");
        return ERR_WRONG_CHIP_VALUE;
      }
      for (unsigned ichip = 0; ichip < n_chips_x[x][idif]; ichip++) {
        if ( n_chans_x[x][idif][ichip] != n_chans[idif][ichip] ) {
          Log.eWrite("There is something wrong with the number of channels detection");
          return ERR_WRONG_CHAN_VALUE;
        }
      }
    }
  }

	for(unsigned idif=1; idif<n_difs; idif++){
		for(unsigned ichip=1; ichip<n_chips[idif]; ichip++){
			Topology::dif_map[to_string(idif)][to_string(ichip)] = n_chans[idif][ichip];
		}
	}
}


