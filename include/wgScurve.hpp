#ifndef WG_SCURVE_HPP_
#define WG_SCURVE_HPP_

// system C++ includes
#include <string>

// system C includes
#include <cstdbool>

// user includes
#include "wgConst.hpp"

// #define M 5

// #define SELECT_NOISE     0
// #define SELECT_GAIN      1
// #define SELECT_PEDESTAL  2
// #define SELECT_RAWCHARGE 3
// #define SELECT_PRINT     4

#define SCURVE_SUCCESS              0
#define ERR_CANNOT_CREATE_DIRECTORY 1
#define ERR_WRONG_MODE              2
#define ERR_FAILED_CREATE_XML_FILE  3
#define ERR_WG_SCURVE               4

using namespace std;

// This is needed to call the following functions from Python using ctypes
#ifdef __cplusplus
extern "C" {
#endif
  int wgScurve(const char* x_inputDirName,
               const char* x_outputXMLDirName = "",
               const char* x_outputIMGDirName = "");
#ifdef __cplusplus
}
#endif

double NoiseToPe(double); // where is this function used??
void MakeXML(string& str,int ichip);
vector<string> GetIncludeFileName(const string& inputDirName);
void AnaXML(vector<string> &inputDirName,string& outputXMLDirName,string& outputIMGDirName,int ichip);
double Calcurate_Mean(vector<double>);
double Calcurate_Sigma(vector<double>);

#endif // WG_SCURVE_HPP_
