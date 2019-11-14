#ifndef WG_SCURVE_HPP_
#define WG_SCURVE_HPP_

// system C++ includes
#include <string>

// ROOT includes
#include <TGraphErrors.h>

// user includes
#include "wgConst.hpp"
#include "wgTopology.hpp"

// This is needed to call the following functions from Python using ctypes
#ifdef __cplusplus
extern "C" {
#endif
  int wgScurve(const char* x_inputDirName,
               const char* x_outputXMLDirName = "",
               const char* x_outputIMGDirName = "",
               const bool compatibility_mode = false);
#ifdef __cplusplus
}
#endif

// Fit the noise rate s-curve for each inputDAC, chip "ichip" and channel "ichan".
void fit_scurve1(TGraphErrors* Scurve, 
                 TF1* fit_scurve,
                 double& pe1_t, 
                 double& pe2_t, 
                 double& pe3_t, 
                 double& ChiSquare,
                 int&    NDF,
                 unsigned idif_id, 
                 unsigned ichip_id, 
                 unsigned ichan_id, 
                 unsigned inputDAC,
                 std::string outputIMGDir, 
                 bool print_flag = false);

// Fit the noise rate s-curve for each inputDAC, chip "ichip" and channel "ichan".
void fit_scurve2(TGraphErrors* Scurve, 
                 TF1* fit_scurve,
                 double& pe1_t, 
                 double& pe2_t, 
                 double& pe3_t, 
                 double& ChiSquare,
                 int&    NDF,
                 unsigned idif_id, 
                 unsigned ichip_id, 
                 unsigned ichan_id, 
                 unsigned inputDAC,
                 std::string outputIMGDir, 
                 bool print_flag = false);

// Fit the noise rate s-curve for each inputDAC, chip "ichip" and channel "ichan".
void fit_scurve3(TGraphErrors* Scurve, 
                 TF1* fit_scurve,
                 double& pe1_t, 
                 double& pe2_t, 
                 double& pe3_t, 
                 double& ChiSquare,
                 int&    NDF,
                 unsigned idif_id, 
                 unsigned ichip_id, 
                 unsigned ichan_id, 
                 unsigned inputDAC,
                 std::string outputIMGDir, 
                 bool print_flag = false);

#endif // WG_SCURVE_HPP_
