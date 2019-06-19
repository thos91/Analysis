#ifndef WG_TOPOLOGY_HPP_INCLUDE
#define WG_TOPOLOGY_HPP_INCLUDE

// system includes
#include <map>
#include <string>

// tinyxml2 includes
#include "tinyxml2.hpp"

#define TOPOLOGY_STRING_LENGTH 4096

using namespace std;
using namespace tinyxml2;

typedef std::map<string, std::map<string, std::map< string, unsigned>>> TopologyMapGdcc;
typedef std::map<string, std::map< string, unsigned>> TopologyMapDif;
typedef std::map<string, std::map< string, unsigned>> GdccToDifMap;

enum class TopologySourceType {
  xml_file,
  json_string
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
  const char * GetTopologyCtypes(const char * configxml);
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
private:
  // This file contains the GDCC,DIF to DIF mapping. This mapping is needed
  // because we want to label each DIF using an absolute number and not the
  // position relative to the GDCC. So for example, instead of saying "second
  // DIF connected to the third GDCC" we want to just call it "eigth DIF" and so
  // on so forth.
  const string m_mapping_file_path;
  /* Example of dif_mapping.txt file:
     {
       "1": {
         "1": 1,
         "2": 2,
         "3": 3,
         "4": 4
        },
      "2": {
         "1": 5,
         "2": 6,
         "3": 7,
         "4": 8
        }
      }
  */

  // Map from the GDCC, DIF pair into the absolute DIF number
  GdccToDifMap m_gdcc_to_dif_map = {};
  // The input value is the name of the Pyrame configuration file to
  // read. The number of GDCCs, DIFs, ASUs and channels is retrieved from the
  // configuration file. A map of type "TopologyMapGdcc" is then populated.
  //
  // {"gdcc", {"dif", {"asu", "n_channels"}}} ...
  TopologyMapGdcc GetTopologyFromFile(const string& configxml);
  // Parse a JSON string containing a representation of the TopologyMapDif
  TopologyMapDif  GetTopologyFromString(const string& json_string);
  // This function reads the JSON file "/opt/calicoes/config/dif_mapping.txt"
  // containing the mapping of the (gdcc, dif) pair into the absolute dif
  // number. The content is translated into a DifMap map.
  GdccToDifMap GetDifMapping();
  // Returns m_gdcc_to_dif_map[gdcc][dif]
  unsigned Dif(const string& gdcc, const string& dif);
  // Returns m_gdcc_to_dif_map[gdcc][dif] after converting the arguments into strings
  unsigned Dif(unsigned gdcc, unsigned dif);
  // Transform the TopologyMapGdcc into
  TopologyMapDif GdccToDif(TopologyMapGdcc& gdcc_map);
  
public:
  // This is the most important member of the class. Basically this the only
  // thing that the user must be concerned with
  TopologyMapDif map = {};
  // Number of DIFs
  unsigned n_difs = 0;
  // Maximum number of chips in one DIF. For instance if one DIF has 10 chips
  // and another 20 chips, the max_chips member will be 20.
  unsigned max_chips = 0;
  // Maximum number of channels in one chip.
  unsigned max_channels = 0;

  // - TopologySourceType::xml_file
  //
  //     First GetTopologyFromFile is called. Then GetDifMapping is called. Then
  //     the TopologyMapGdcc is converted to TopologyMapDif using the GdccToDif
  //     method
  //
  // - TopologySourceType::json_string
  //
  //      GetTopologyFromString is called and that's it.

  Topology(string configxml, TopologySourceType source_type = TopologySourceType::xml_file);
  Topology(const char * configxml, TopologySourceType source_type = TopologySourceType::xml_file);

  // Print the TopologyMapDif map member to cout
  void Print();


};

#endif
