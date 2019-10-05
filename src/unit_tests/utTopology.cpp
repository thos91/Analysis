#include <iostream>
#include <string>

// json includes
#include <nlohmann/json.hpp>

// user includes
#include "wgTopology.hpp"

int main () {
  std::string xml_config_file("/home/neo/Threshold120/RawData/scurve_test_iDAC100_threshold120.xml");

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

  std::cout << "\n ### GetTopologyFromString test ###\n\n";
  
  // TopologyMap from JSON string
  std::string json_string(R"###({"0":{"0":32,"1":32,"2":32},"5":{"0":32,"1":32,"2":32}})###");
  Topology topology_from_string(json_string, TopologySourceType::json_string);
  topology_from_string.PrintMapDif();
  topology_from_string.PrintMapGdcc();

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
