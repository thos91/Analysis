#include <iostream>
#include <string>

// json includes
#include <nlohmann/json.hpp>

// user includes
#include "wgTopology.hpp"

int main () {
  std::string xml_config_file("/home/neo/Code/WAGASCI/Configs/wagasci_config_6asu.xml");

  
  // GetTopology : C API : First time
  char * topology_string = const_cast <char *>(GetTopologyCtypes(xml_config_file.c_str()));

  if ( topology_string[0] == '\0' )
    std::cout << "First time : Topology string is empty" << std::endl;
  else
    std::cout << topology_string << std::endl;
  FreeTopologyCtypes(topology_string);
  
  // GetTopology : C API : Second time
  topology_string = const_cast <char *>(GetTopologyCtypes(xml_config_file.c_str()));

  if ( topology_string[0] == '\0' )
    std::cout << "Second time : Topology string is empty" << std::endl;
  else
    std::cout << topology_string << std::endl;
  FreeTopologyCtypes(topology_string);

  // GetTopology : C++ API
  Topology topol(xml_config_file);
  topol.Print();

  // Max number of chips
  int n_difs = 0, n_chips = 0, n_chans = 0;
  for ( auto const & gdcc : topol.topology_map) {
    for ( auto const & dif : gdcc.second) {
      n_difs++;
      int n_chips_tmp = 0;
      for ( auto const & asu : dif.second) {
        n_chips_tmp++;
        if (asu.second > n_chans) n_chans = asu.second;
      }
      if (n_chips_tmp > n_chips) n_chips = n_chips_tmp;
    }
  }
  std::cout << "# DIFs = " << n_difs << std::endl;
  std::cout << "max # ASUs per DIF = " << n_chips << std::endl;
  std::cout << "max # channels per ASU = " << n_chans << std::endl;

  // TopologyMap from JSON string
  std::string json_string(R"###({"1":{"1":{"1":32,"2":32,"3":32},"2":{"1":32,"2":32,"3":32}}})###");
  Topology topology(json_string, TopologySourceType::json_string);
  topology.Print();  
}