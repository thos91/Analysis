#include <iostream>

#include "wgConst.hpp"
#include "wgEditXML.hpp"

int main() {

  const std::string xml_config_file("/home/wagasci-access/Code/WAGASCI/Analysis/src/unit_tests/wagasci_config.xml");
  
  wgEditXML xml = wgEditXML();
  i2vector v;
  if (xml.GetConfig(xml_config_file, 2, 3, 10, 32, v) == true) {
    std::cout << "[GetConfig] test passed\n";
    std::cout << "Global threshould : " << v[0][0] << "\n" << 
        "Gain select threshould : "     << v[0][1] << "\n" <<
        "Input DAC : "                  << v[0][2] << "\n" <<
        "Feedback capacitor DAC : "     << v[0][3] << "\n" <<
        "Threshould adjustment : "      << v[0][4] << "\n";
  } else {
    std::cout << "[GetConfig] test failed\n";
  }
}
