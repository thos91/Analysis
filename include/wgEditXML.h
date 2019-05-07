#ifndef WG_EDITXML_H_INCLUDE
#define WG_EDITXML_H_INCLUDE

#include <string>
#include <vector>
#include "tinyxml2.h"
#include "Const.h"

using namespace tinyxml2;
using namespace std;

class wgEditXML
{
private:
  static string filename;
public:
  XMLDocument *xml;

  /* wgEditXML::Make: create an empty xml document with the following structure.
     All the fields are initialized with their default value.
---------------------------------------------------------------------------------
<data>
    <config>
        <chipid>*chip_number*</chipid>
        <chanid>*chan_number*</chanid>
        <start_time>0</start_time>
        <stop_time>0</stop_time>
        <trigth>-1</trigth>
        <gainth>-1</gainth>
        <inputDAC>-1</inputDAC>
        <HG>-1</HG>
        <LG>-1</LG>
		<trig_adj>-1<trig_adj>
    </config>
    <ch>
	    <col>0<col_0/>
        <col>1<col_1/>
		...
    </ch>
</data>
-------------------------------------------------------------------------------- */
  void Make(const string&, const unsigned, const unsigned);

  // Opens and loads a XML file into the member "XMLDocument * xml". If an error
  // is encountered the wgInvalidFile exception is thrown.
  void Open(const string&);

  void Close();
  void Write();

  // Open a configuration xml file (the one used by Pyrame for the acquisition)
  // and read the spiroc2d_bitstream parameter for the idif DIF and ichip
  // ASU. Then decode the bitstream and store the parameters in the 2D vector
  // v. The returned vector has this structure:
  // v[channel][0] = global 10-bit discriminator threshold
  // v[channel][1] = global 10-bit gain selection discriminator threshold
  // v[channel][2] = adjustable input 8-bit DAC
  // v[channel][3] = adjustable 6-bit high gain (HG) preamp feedback capacitance
  // v[channel][4] = adjustable 4-bit discriminator threshold
  bool GetConfig(const string& configxml, unsigned idif, unsigned n_chips, unsigned n_chans, vector<vector<int>>& v);

  // GetLog: Parse a .log file and fill a vector with:
  //    - v[0]: start_time
  //    - v[1]: stop_time
  //    - v[2]: nb_data_pkts
  //    - v[3]: nb_lost_pkts
  void GetLog(const string&, vector<int>&);

  // wgEditXML::SetConfigValue
  // In the XML file created by the wgEditXML::Make method set the configuration
  // parameter called "name" to the int value "value".  If mode is
  // CREATE_NEW_MODE (=1), if the parameter name doesn't exist, it is
  // created. Otherwise a wgElementNotFound exception is thrown.
  void SetConfigValue(const string& name, int value, int mode = NO_CREATE_NEW_MODE);

  void SetColValue(const string&,int,double,int);
  void SetChValue(const string&,double,int);
  void AddColElement(const string&,int);
  void AddChElement(const string&);
  double GetColValue(const string&,int);

  // wgEditXML::SetChValue
  // insert a child element in the data/ch field with name "name" and value
  // "value". If the mode is NO_CREATE_NEW_MODE, a new element is not
  // created. If the mode is CREATE_NEW_MODE a new element is created if it
  // didn't exist.
  double GetChValue(const string& name);
  
  int GetConfigValue(const string&);
  void SUMMARY_Make(const string&,int);
  void SUMMARY_SetGlobalConfigValue(const string&,int,int);
  void SUMMARY_SetChConfigValue(const string&,int,int,int);
  void SUMMARY_SetChFitValue(const string&,double,int,int);
  void SUMMARY_SetPedFitValue(double*,int,int);
  void SUMMARY_AddGlobalElement(const string&);
  void SUMMARY_AddChElement(const string&,int);
  int SUMMARY_GetGlobalConfigValue(const string&);
  int SUMMARY_GetChConfigValue(const string&,int);
  double SUMMARY_GetChFitValue(const string&,int);
  void SUMMARY_GetPedFitValue(double*,int);

  void SCURVE_Make(const string&);
  void SCURVE_SetValue(const string&,int,double,int);
  double SCURVE_GetValue(const string&,int);

  void OPT_Make(const string&);
  void OPT_SetValue(const string&,int,int,int,double,int);
  double OPT_GetValue(const string&,int,int,int);
  void OPT_SetChipValue(const string&,int,int,double,int);
  double OPT_GetChipValue(const string&,int,int);

  void PreCalib_Make(const string&);
  void PreCalib_SetValue(const string&,int,int,int,double,int);
  double PreCalib_GetValue(const string&,int,int,int);

  void Calib_Make(const string&);
  void Calib_SetValue(const string&,int,int,int,double,int);
  double Calib_GetValue(const string&,int,int,int);
};

#endif
