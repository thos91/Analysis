#ifndef WG_TOPOLOGY_HPP_INCLUDE
#define WG_TOPOLOGY_HPP_INCLUDE

// system includes
#include <map>
#include <string>

// tinyxml2 includes
#include "tinyxml2.hpp"

#define TOPOLOGY_STRING_LENGTH 4096

using namespace tinyxml2;

typedef std::map<std::string, std::map<std::string, std::map< std::string, std::string>>> StringTopologyMapGdcc;
typedef std::map<std::string, std::map< std::string, std::string>> StringTopologyMapDif;
typedef std::map<unsigned, std::map<unsigned, std::map< unsigned, unsigned>>> TopologyMapGdcc;
typedef std::map<unsigned, std::map< unsigned, unsigned>> TopologyMapDif;
typedef std::map<std::pair<std::string, std::string>, std::string> GdccToDifMap;
typedef std::map<std::string, std::pair< std::string, std::string>> DifToGdccMap;
typedef std::map<std::string, std::string>  MacToGdccMap;

enum class TopologySourceType {
  xml_file,
  json_string,
  scurve_tree,
  pedestal_tree,
  gain_tree
};

//=======================================================================//
//                           GetTopologyCtypes                           //
//=======================================================================//

// The GetTopologyCtypes function is meant to be called by Python code (using
// ctypes). The input value is the name of the Pyrame configuration file to
// read. The number of DIFs, ASUs and channels is retrieved from the
// configuration file. A map of type "TopologyMapDif" is then populated. The
// function returns a string containing the json representation of the
// map. Something like:
//
// {"dif", {"asu", "n_channels"}} ...
//
// The returned string is a constant C string (a pointer to char). To avoid
// memory leaks the memory allocated for this string must be freed using the
// FreeTopologyCtypes function. This might seem clumsy from a C++ point of view,
// but remember that ctypes supports only C-style syntax and that the JSON
// string must be allocated on the heap to be available outside of the function.
// The GetTopologyCtypes calls the Topology class methods internally.

#ifdef __cplusplus
extern "C" {
#endif
  const char * GetDifTopologyCtypes(const char * configxml);
  const char * GetGdccTopologyCtypes(const char * configxml);
  void FreeTopologyCtypes(char * topology_string);
#ifdef __cplusplus
}
#endif

//=======================================================================//
//                           Topology class                              //
//=======================================================================//

// This class is used to manage the number of DIFs, ASU (SPIROC2D) chips and
// channels.

class Topology {

  friend const char * GetDifTopologyCtypes(const char * configxml);
  friend const char * GetGdccTopologyCtypes(const char * configxml);
  friend const char * FreeTopologyCtypes(const char * configxml);
  
 private:
  // This file contains the GDCC,DIF to DIF mapping. This mapping is needed
  // because we want to label each DIF using an absolute number and not the
  // position relative to the GDCC. So for example, instead of saying "second
  // DIF connected to the third GDCC" we want to just call it "eigth DIF" and so
  // on so forth.
  const std::string m_dif_mapping_file_path;
  /* Example of dif_mapping.txt file:
     {
     "1": {
     "1": 0,
     "2": 1,
     "3": 2,
     "4": 3
     },
     "2": {
     "1": 4,
     "2": 5,
     "3": 6,
     "4": 7
     }
     }
  */
  const std::string m_mac_mapping_file_path;
  /* Example of mac_mapping.txt file:
     {
     "00:0A:35:01:FE:06": 1,
     "00:0A:35:01:FE:01": 2
     }
  */

  // Map from the GDCC, DIF pair into the absolute DIF number
  GdccToDifMap m_gdcc_to_dif_map = {};
  // Map from the absolute DIF number into the GDCC, DIF pair
  DifToGdccMap m_dif_to_gdcc_map = {};
  // Map from GDCC MAC address to GDCC number
  MacToGdccMap m_mac_to_gdcc_map = {};
  // String version of the gdcc_map
  StringTopologyMapGdcc m_string_gdcc_map = {};
  // String version of the dif_map
  StringTopologyMapDif m_string_dif_map = {};
  
  // The input value is the name of the Pyrame configuration file to
  // read. The number of GDCCs, DIFs, ASUs and channels is retrieved
  // from the configuration file. The "this->gdcc_map" of type
  // "TopologyMapGdcc" is then populated.
  //
  // {"gdcc", {"dif", {"asu", "n_channels"}}} ...
  void GetTopologyFromFile(const std::string& configxml);

  // Parse a JSON string containing a representation of the
  // TopologyMapDif The "this->dif_map" of type "TopologyMapDif" is
  // then populated.
  void GetTopologyFromString(const std::string& json_string);

  // Get the topology from a directory tree and the xml files
  // contained therein. It is useful only when reading the output of
  // the wgAnaHistSummary program.
  // This works for wgPedestalCalib.
  //
  // input_run_dir is the directory where all the files corresponding
  // to a certain run are contained. run_directory_tree is a map that
  // associates to each integer starting from zero a path (relative to
  // the input_run_dir) corresponding to the location of the Xml
  // directory for each acquisition.
  void GetTopologyFromPedestalTree(std::string input_run_dir);

  // Same as above but for wgGainCalib.
  void GetTopologyFromGainTree(std::string input_run_dir);

  // Same as above but for wgScurve.
  void GetTopologyFromScurveTree(std::string input_run_dir);

  // This function reads the JSON file
  // "/opt/calicoes/config/dif_mapping.txt" containing the mapping of
  // the (gdcc, dif) pair into the absolute dif number. Then the
  // "this->m_gdcc_to_dif_map" and "this->m_dif_to_gdcc_map" maps are
  // populated.
  void GetGdccDifMapping();

  // This function reads the JSON file
  // "/opt/calicoes/config/mac_mapping.txt" containing the mapping of
  // the GDCC MAC address into the GDCC number. Then the
  // "this->m_mac_to_gdcc_map" map is populated.
  void GetGdccMacMapping();

  // Transform the TopologyMapGdcc "gdcc_map" into the TopologyMapDif
  // "dif_map". Must be called after the GetTopology* and
  // GetGdccDifMapping functions.
  void GdccMapToDifMap();

  // Transform the TopologyMapDif "dif_map" into the TopologyMapGdcc
  // "gdcc_map". Must be called after the GetTopology* and
  // GetGdccDifMapping functions.
  void DifMapToGdccMap();

  // copy the string version m_string_gdcc_map and m_string_dif_map
  // into the gdcc_map and dif_map respectively
  void StringToUnsigned(void);
  
 public:
  // These are the most important members of the class. Basically this
  // is the only thing that the user must be concerned with.
  //
  // dif_map = absolute dif -> chip -> number of channels
  TopologyMapDif dif_map = {};
  // gdcc_map = gdcc -> dif -> chip -> number of channels
  TopologyMapGdcc gdcc_map = {};
  // Number of DIFs
  unsigned n_difs = 0;
  // Maximum number of chips in one DIF. For instance if one DIF has 10 chips
  // and another 20 chips, the max_chips member will be 20.
  unsigned max_chips = 0;
  // Maximum number of channels in one chip.
  unsigned max_channels = 0;

  // - TopologySourceType::xml_file
  //
  //     First GetTopologyFromFile is called. Then GetGdccDifMapping
  //     is called. Then the TopologyMapGdcc is converted to
  //     TopologyMapDif using the GdccMapToDifMap method.
  //
  // - TopologySourceType::json_string
  //
  //     GetTopologyFromString is called. Then GetGdccDifMapping is
  //     called. Then the TopologyMapDif is converted to
  //     TopologyMapGdcc using the DifMapToGdccMap method.
  //
  // - TopologySourceType::pedestal_tree
  //
  //     GetTopologyFromPedestalTree is called. Then GetGdccDifMapping is
  //     called. Then the TopologyMapDif is converted to
  //     TopologyMapGdcc using the DifMapToGdccMap method.
  //
  // - TopologySourceType::scurve_tree
  //
  //     GetTopologyFromScurveTree is called. Then GetGdccDifMapping is
  //     called. Then the TopologyMapDif is converted to
  //     TopologyMapGdcc using the DifMapToGdccMap method.
  //
  // - TopologySourceType::gain_tree
  //
  //     GetTopologyFromGainTree is called. Then GetGdccDifMapping is
  //     called. Then the TopologyMapDif is converted to
  //     TopologyMapGdcc using the DifMapToGdccMap method.
  
  Topology(std::string source, TopologySourceType source_type = TopologySourceType::xml_file);
  Topology(const char * source, TopologySourceType source_type = TopologySourceType::xml_file);
  
  // Returns m_gdcc_to_dif_map[pair<gdcc, dif>]
  std::string GetAbsDif(const std::string& gdcc, const std::string& dif);
  // Returns m_gdcc_to_dif_map[pair<gdcc, dif>] after converting the arguments into strings
  unsigned GetAbsDif(unsigned gdcc, unsigned dif);

  // Returns m_dif_to_gdcc_map[dif]
  std::pair<std::string, std::string> GetGdccDifPair(const std::string& dif);
  // Returns m_dif_to_gdcc_map[dif] after converting the argument into string
  std::pair<unsigned, unsigned> GetGdccDifPair(unsigned dif);
  // Return the GDCC ID from its MAC address
  unsigned GetGdccID(std::string mac);
  
  // Print the TopologyMapDif map member to cout
  void PrintMapDif();
  // Print the TopologyMapGdcc map member to cout
  void PrintMapGdcc();

  // Return true if the channel is connected to an MPPC (only WallMRD)
  static bool IsWallMRDChannelEnabled(unsigned dif, unsigned chip,
                                      unsigned channel);
};

#endif
