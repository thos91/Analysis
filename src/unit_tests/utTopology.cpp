#include <iostream>
#include <string>

// json includes
#include <nlohmann/json.hpp>

// user includes
#include "wgTopology.hpp"

int main () {
  std::string xml_config_file("/home/wagasci-ana/Code/Analysis/src/unit_tests/wagasci_config.xml");

  std::cout << "\n ### GetTopologyFromFile test ###\n\n";
  std::cout << " # C API test #\n\n";
  
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

  std::cout << "\n # C++ API test #\n\n";
    
  // GetTopology : C++ API
  Topology topol(xml_config_file);
  topol.PrintMapDif();
  std::cout << "\n";
  topol.PrintMapGdcc();

  // Max number of chips
  std::cout << "\nnumber of DIFs = " << topol.n_difs << std::endl;
  std::cout << "max number of ASUs per DIF = " << topol.max_chips << std::endl;
  std::cout << "max number of channels per ASU = " << topol.max_channels << std::endl;

  std::cout << "\n ### GetTopologyFromString test1 ###\n\n";
  
  // TopologyMap from JSON string
  std::string json_string1(R"###({"0":{"0":32,"1":32,"2":32},"5":{"0":32,"1":32,"2":32}})###");
  Topology topology_from_string1(json_string1, TopologySourceType::json_string);
  topology_from_string1.PrintMapDif();
  topology_from_string1.PrintMapGdcc();

  std::cout << "\n ### GetTopologyFromString test2 ###\n\n";

  std::string json_string2(R"###({"1":{"1":{"1-3":32},"2":{"1-3":32},"3":{"1-3":32},"4":{"1-3":32}},"2":{"1":{"1-20":32},"2":{"1-20":32},"3":{"1-20":32},"4":{"1-20":32}}})###");
  Topology topology_from_string2(json_string2, TopologySourceType::json_string);
  topology_from_string2.PrintMapDif();
  topology_from_string2.PrintMapGdcc();

  std::cout << "\n ### GetTopologyFromPedestalTree test ###\n\n";
  
  // TopologyMap from wgPedestalCalib directory tree
  Topology topology_from_pedestal_tree("/home/neo/Code/WAGASCI/Analysis/src/unit_tests/pedestal_topology_test_run", TopologySourceType::pedestal_tree);
  topology_from_pedestal_tree.PrintMapDif();
  topology_from_pedestal_tree.PrintMapGdcc();

  std::cout << "\n ### GetTopologyFromScurveTree test ###\n\n";
  
  // TopologyMap from wgPedestalCalib directory tree
  Topology topology_from_scurve_tree("/home/neo/Code/WAGASCI/Analysis/src/unit_tests/scurve_topology_test_run", TopologySourceType::scurve_tree);
  topology_from_scurve_tree.PrintMapDif();
  topology_from_scurve_tree.PrintMapGdcc();
}
