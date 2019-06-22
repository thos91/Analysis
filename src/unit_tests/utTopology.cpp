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
  topol.PrintMapDif();
  topol.PrintMapGdcc();

  // Max number of chips
  std::cout << "# DIFs = " << topol.n_difs << std::endl;
  std::cout << "max # ASUs per DIF = " << topol.max_chips << std::endl;
  std::cout << "max # channels per ASU = " << topol.max_channels << std::endl;

  // TopologyMap from JSON string
  std::string json_string(R"###({"1":{"1":32,"2":32,"3":32},"2":{"1":32,"2":32,"3":32}})###");
  Topology topology_from_string(json_string, TopologySourceType::json_string);
  topology_from_string.PrintMapDif();
  topology_from_string.PrintMapGdcc();
}
