#ifndef WG_SCURVE_SUMMARY_HPP_
#define WG_SCURVE_SUMMARY_HPP_

// system C++ includes
#include <string>

// system C includes
#include <cstdbool>

// user includes
#include "wgConst.hpp"

#define SCURVESUMMARY_SUCCESS       0
#define ERR_CANNOT_CREATE_DIRECTORY 1
#define ERR_WRONG_MODE              2
#define ERR_FAILED_CREATE_XML_FILE  3
#define ERR_WG_SCURVESUMMARY        4

using namespace std;

// This is needed to call the following functions from Python using ctypes
#ifdef __cplusplus
extern "C" {
#endif
  int wgScurveSummary(const char* x_inputDirName,
                      const char* x_outputXMLDirName = "",
                      const char* x_outputIMGDirName = "");
#ifdef __cplusplus
}
#endif

void MakeDir(string& str);
void MakeXML(string& str);
void AnaXML(string& inputDirName, string& outputXMLDirName,string& outputIMGDirName,int idif ,int ichip);

#endif // WG_SCURVE_SUMMARY_HPP_
