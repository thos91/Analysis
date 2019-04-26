#include <TROOT.h>

#include "wgGetCalibData.h"
#include "wgTools.h"
#include "Const.h"
#include "wgTools.h"
#include "wgErrorCode.h"
#include "wgEditXML.h"
#include "wgExceptions.h"

//******************************************************************************
int wgGetCalibData::Get_Pedestal(string& pedFileName, d3vector pedestal, d3vector ped_nohit, size_t dif) {
CheckExist *check  =  new CheckExist;
  if( pedFileName == "" || !check->XmlFile(pedFileName)){
	throw wgInvalidFile(Form("Pedestal file not found or invalid (%s)", pedFileName.c_str()));
  }
  delete check;

  // Number of channels where the pedestal or ped_nohit could not be found
  int n_not_found = 0;

  // Pedestal when there is no hit
  wgEditXML *Edit = new wgEditXML();
  Edit->Open(pedFileName);
  for(unsigned int ichip = 0; ichip < ped_nohit.size(); ichip++) {
    for(unsigned int ich = 0; ich < ped_nohit[ichip].size(); ich++) {
      for(unsigned int icol = 0; icol < ped_nohit[ich].size(); icol++) {
		string name;
		try {
		  name=Form("ped_nohit_%d", icol);
		  ped_nohit[ichip][ich][icol] = Edit->Calib_GetValue(name, dif, ichip, ich);
		}
		catch (const wgElementNotFound& e) {
		  ped_nohit[ichip][ich][icol] = 1.;
		  n_not_found++;
		}
	  }
	}
  }

  // Pedestal when there is a hit
  for(unsigned int ichip = 0; ichip < pedestal.size(); ichip++) {
    for(unsigned int ich = 0; ich < pedestal[ichip].size(); ich++) {
      for(unsigned int icol = 0; icol < pedestal[ich].size(); icol++) {
		string name;
		try {
		  name=Form("ped_%d",icol);
		  pedestal[ichip][ich][icol]=Edit->Calib_GetValue(name, dif, ichip, ich);
		}
		catch (const wgElementNotFound& e) {
		  ped_nohit[ichip][ich][icol] = 1.;
		  n_not_found++;
		}
	  }
	}
  }
  Edit->Close();
  delete Edit;
  return n_not_found;
}

//******************************************************************************
int wgGetCalibData::Get_TdcCoeff(string& tdcFileName, d3vector slope, d3vector intcpt, unsigned dif) {
  CheckExist *check  =  new CheckExist;
  if(tdcFileName=="" || !check->XmlFile(tdcFileName)){
	throw wgInvalidFile (Form("TDC calibration card file not found or invalid (%s)", tdcFileName.c_str()));
  }
  delete check;

  int n_not_found = 0;
  
  wgEditXML *Edit = new wgEditXML();
  Edit->Open(tdcFileName);
  for(unsigned int ichip = 0; ichip < slope.size(); ichip++){
    for(unsigned int ich = 0; ich < slope[ichip].size(); ich++){
	  string name;
	  try {		
		name="slope_even";
		slope[ichip][ich][TDC_RAMP_EVEN] = Edit->Calib_GetValue(name, dif, ichip, ich);
	  }
	  catch (const wgElementNotFound& e) {
		slope[ichip][ich][TDC_RAMP_EVEN] = 1.;
		n_not_found++;
	  }
	  try {
        name="slope_odd";
        slope[ichip][ich][TDC_RAMP_ODD] = Edit->Calib_GetValue(name, dif, ichip, ich);
	  }
	  catch (const wgElementNotFound& e) {
		slope[ichip][ich][TDC_RAMP_ODD] = 1.;
		n_not_found++;
	  }
	}
  }
  for(unsigned int ichip = 0; ichip < intcpt.size(); ichip++){
    for(unsigned int ich = 0; ich < intcpt[ichip].size(); ich++){
	  string name;
	  try {
        name="intcpt_even";
        intcpt[ichip][ich][TDC_RAMP_EVEN] = Edit->Calib_GetValue(name, dif, ichip, ich);
	  }
	  catch (const wgElementNotFound& e) {
		intcpt[ichip][ich][TDC_RAMP_EVEN] = 1.;
		n_not_found++;
	  }
	  try {
        name="intcpt_odd";
        intcpt[ichip][ich][TDC_RAMP_ODD] = Edit->Calib_GetValue(name, dif, ichip, ich);
	  }
	  catch (const wgElementNotFound& e) {
		intcpt[ichip][ich][TDC_RAMP_ODD] = 1.;
		n_not_found++;
	  }
	}
  }
  Edit->Close();
  delete Edit;
  return n_not_found;
}

//******************************************************************************
int wgGetCalibData::Get_Gain(string& calibFileName, d2vector gain, unsigned dif) {
  CheckExist *check  =  new CheckExist;
  if(calibFileName=="" || !check->XmlFile(calibFileName)){
	throw wgInvalidFile (Form("Calibration card file not found or invalid (%s)", calibFileName.c_str()));
  }
  delete check;

  // Number of channels where the gain info could not be found
  int n_not_found = 0;
  
  wgEditXML *Edit = new wgEditXML();
  Edit->Open(calibFileName);
  for(unsigned int ichip = 0; ichip < gain.size(); ichip++){
    for(unsigned int ich = 0; ich < gain[ichip].size(); ich++){
	  string name("");
	  try {
		name=Form("Gain");
		gain[ichip][ich]=Edit->Calib_GetValue(name, dif, ichip, ich);
	  }
	  catch (const wgElementNotFound& e) {
		gain[ichip][ich] = 1.;
		n_not_found++;
	  }
    }
  }
  Edit->Close();
  delete Edit;
  return n_not_found;
}

