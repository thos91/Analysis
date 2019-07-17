#include <iostream>
#include <string>

// json includes
#include <nlohmann/json.hpp>

// user includes
#include "wgTopology.hpp"

int main () {
  std::string xml_config_file("/home/neo/Code/WAGASCI/Configs/wagasci_config_6asu.xml");

  std::cout << " ### C API test ###\n\n";
  
  // GetTopology : C API : First time
  char * topology_string = const_cast <char *>(GetDifTopologyCtypes(xml_config_file.c_str()));

  if ( topology_string[0] == '\0' )
    std::cout << "First time : Topology string is empty" << std::endl;
  else
    std::cout << topology_string << std::endl;
  FreeTopologyCtypes(topology_string);
  
  // GetTopology : C API : Second time
  topology_string = const_cast <char *>(GetDifTopologyCtypes(xml_config_file.c_str()));

  if ( topology_string[0] == '\0' )
    std::cout << "Second time : Topology string is empty" << std::endl;
  else
    std::cout << topology_string << std::endl;
  FreeTopologyCtypes(topology_string);

  std::cout << "\n ### GetTopologyFromFile test ###\n\n";
  
  // GetTopology : C++ API
  Topology topol(xml_config_file);
  topol.PrintMapDif();
  topol.PrintMapGdcc();

  // Max number of chips
  std::cout << "# DIFs = " << topol.n_difs << std::endl;
  std::cout << "max # ASUs per DIF = " << topol.max_chips << std::endl;
  std::cout << "max # channels per ASU = " << topol.max_channels << std::endl;

  std::cout << "\n ### GetTopologyFromString test ###\n\n";
  
  // TopologyMap from JSON string
  std::string json_string(R"###({"1":{"1":32,"2":32,"3":32},"2":{"1":32,"2":32,"3":32}})###");
  Topology topology_from_string(json_string, TopologySourceType::json_string);
  topology_from_string.PrintMapDif();
  topology_from_string.PrintMapGdcc();

  std::cout << "\n ### GetTopologyFromPedestalTree test ###\n\n";
  
  // TopologyMap from wgAnaPedestal directory tree
  Topology topology_from_pedestal_tree("/home/neo/Code/WAGASCI/Analysis/src/unit_tests/pedestal_topology_test_run", TopologySourceType::pedestal_tree);
  topology_from_pedestal_tree.PrintMapDif();
  topology_from_pedestal_tree.PrintMapGdcc();

  std::cout << "\n ### GetTopologyFromScurveTree test ###\n\n";
  
  // TopologyMap from wgAnaPedestal directory tree
  Topology topology_from_scurve_tree("/home/neo/Code/WAGASCI/Analysis/src/unit_tests/scurve_topology_test_run", TopologySourceType::scurve_tree);
  topology_from_scurve_tree.PrintMapDif();
  topology_from_scurve_tree.PrintMapGdcc();
}
