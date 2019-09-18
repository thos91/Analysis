// system includes
#include <fstream>
#include <iostream>
#include <string>

// boost includes
#include <boost/program_options.hpp>

// user includes
#include "wgErrorCodes.hpp"
#include "wgLogger.hpp"
#include "wgOnlineMonitor.hpp"

namespace po = boost::program_options;

// ========================== MAIN =============================

int main(int argc, char **argv) {

  unsigned dif_id = 0;
  std::string pyrame_config;
  
  try {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("dif_id, d", po::value<unsigned>()->default_value(0), "DIF number")
        ("pyrame_config, i",
         po::value<std::string>(&pyrame_config)->default_value(
             "wagasci_mapping_table.txt"), "Pyrame XML configuration file");

     // create the object "vm" that will contain the command line
     // arguments. These are read through the method "parse_command_line"
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    // The --help option produces the description of all the allowed options
    if (vm.count("help")) {
      std::cout << desc << "\n";
      return 0;
    }

    /****************** Summary of all the command-line parameters ******************/
    
    if (vm.count("dif_id")) {
      std::cout << "  The DIF ID was set to " << vm["dif_id"].as<unsigned>() << "\n";
      dif_id = vm["dif_id"].as<unsigned>();
    } else {
      std::cout << "  The DIF ID is assumed to be " << dif_id << "\n";
    }
    if (vm.count("pyrame_config")) {
      std::cout << "  The pyrame_config file name was set to "
                << vm["pyrame_config"].as<std::string>() << "\n";
    } else {
      std::cout << "  The pyrame XML config file is assumed to be "
                << pyrame_config << "\n";
    }
  }
  catch(std::exception& e) {
    std::cerr << "  Error: " << e.what() << "\n";
    return 1;
  }

 int result;
  if ( (result = wgOnlineMonitor(pyrame_config.c_str(), dif_id) ) != WG_SUCCESS ) {
    Log.Write("[wgOnlineMonitor] Returned error code " + std::to_string(result));
  }
  
  // should be never reached
  return result;
}
