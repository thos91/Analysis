#ifndef WG_ANAHISTSUMMARY_HPP_
#define WG_ANAHISTSUMMARY_HPP_

// system C++ includes
#include <string>

// user includes
#include "wgConst.hpp"
#include "wgTopology.hpp"

// errors
#define APS_SUCCESS                 0
#define ERR_CANNOT_CREATE_DIRECTORY 1
#define ERR_GET_FILE_LIST           2
#define ERR_FAILED_OPEN_XML_FILE    3
#define ERR_WRONG_DIF_VALUE         4
#define ERR_WRONG_CHIP_VALUE        5
#define ERR_WRONG_CHAN_VALUE        6
#define ERR_WRONG_PE_VALUE          7
#define ERR_EMPTY_INPUT_FILE        8

#define PEDESTAL_DIFFERENCE_WARNING_THRESHOLD 0.1

using namespace std;

DirectoryTreeMap run_directory_tree{ {ONE_PE, "/OnePE/wgAnaHistSummary/Xml"}, {TWO_PE, "/TwoPE/wgAnaHistSummary/Xml"} };

// This is needed to call the following functions from Python using ctypes
#ifdef __cplusplus
extern "C" {
#endif

  int wgAnaPedestal(const char * inputDirName,
                    const char * outputXMLDirName,
                    const char * outputIMGDirName);

#ifdef __cplusplus
}
#endif

#endif // WG_ANAHISTSUMMARY_HPP_
