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

typedef map<string, map<string, map< string, int>>> TopologyMap;
typedef map<string, map< string, int>> DifMap;

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

//=======================================================================//
//                           GetTopology                                 //
//=======================================================================//

// This function is meant to be called by Python code (using ctypes). The input
// value is the name of the Pyrame configuration file to read. The number of
// GDCC, DIF, ASU and channels is retrieved from the configuration file. A map
// of type "TopologyMap" is then populated. The function returns a string
// containing the json representation of the map. Something like:
//
// {"igdcc", {"idif", {"iasu", "n_channels"}}}
//
// The returned string is a constant C string (a pointer to char). To avoid
// memory leaks the memory allocated for this string must be freed using the
// FreeTopology function. This might seem clumsy from a C++ point of view, but
// remember that ctypes supports only C-style syntax and that the string must be
// allocated on the heap to be available outside of the function.

#ifdef __cplusplus
extern "C" {
#endif
  const char * GetTopologyCtypes(const char * configxml);
  void FreeTopologyCtypes(char * topology_string);
#ifdef __cplusplus
}
#endif

class Topology {
private:
  const string m_mapping_file_path;
  // The behavior of this function is the same as above. Actually the C API
  // function just calls this one internally. The only difference is that the
  // Topology map is returned directly without converting it to a JSON string.
  TopologyMap GetTopology(string configxml);
  // This function reads the JSON file "/opt/calicoes/config/dif_mapping.txt"
  // containing the mapping of the (gdcc, dif) pair into the absolute dif
  // number. The content is translated into a DifMap map.
  DifMap GetDifMapping();
  DifMap m_dif_map = {};

public:
  TopologyMap topology_map = {};
  unsigned max_difs = 0;
  unsigned max_chips = 0;
  unsigned max_channels = 0;

  // The constructors just call GetTopology and GetDifMapping internally
  Topology(string configxml);
  Topology(const char * configxml);

  // Returns dif_map[gdcc][dif]
  unsigned Dif(std::string gdcc, std::string dif);
  // Returns dif_map[gdcc][dif] after converting the arguments to strings
  unsigned Dif(unsigned gdcc, unsigned dif);
};

#endif
