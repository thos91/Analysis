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

  //=======================================================================//
  //                         Generic methods                               //
  //=======================================================================//

  // Opens and loads a XML file into the member "XMLDocument * xml". If an error
  // is encountered the wgInvalidFile exception is thrown.
  void Open(const string&);

  // Closes a file precedently opened by the Open method 
  void Close();

  // Write the changes to a file precedently opened by the Open method
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

  //=======================================================================//
  //                         standard XML files                            //
  //=======================================================================//

  // wgEditXML::Make: create an empty xml document with the following structure.
  //   All the fields are initialized with their default value.
  /* -----------------------------------------------------------------------------
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
  void Make(const string& filename, unsigned ichip, unsigned ichan);
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

  //=======================================================================//
  //                         SUMMARY XML files                             //
  //=======================================================================//
  
  // wgEditXML::SUMMARY_Make
  // Create a template summary file like shown below: 
  /* -----------------------------------------------------------------------------
<data>
    <config>
        <start_time>0</start_time>
        <stop_time>0</stop_time>
        <trigth>-1</trigth>
        <gainth>-1</gainth>
    </config>
    <ch_0>
        <fit>
            <Gain>-1</Gain>
            <Noise>-1</Noise>
			<ped_0>-1</ped_0>
			<ped_1>-1</ped_1>
			...
			<ped_15>-1</ped_15>
        </fit>
        <config>
            <inputDAC>-1</inputDAC>
            <ampDAC>-1</ampDAC>
            <adjDAC>-1</adjDAC>
        </config>
    </ch_0>
...
</data>
----------------------------------------------------------------------------- */
  void SUMMARY_Make(const string& filename, unsigned n_chans);
  // wgEditXML::SUMMARY_SetGlobalConfigValue
  // data -> config -> name
  // In the XML file created by the wgEditXML::SUMMARY_Make method set the
  // configuration parameter called "name" to the int value "value".  If mode is
  // CREATE_NEW_MODE (=1), if the parameter name doesn't exist, it is
  // created. Otherwise a wgElementNotFound exception is thrown.
  void SUMMARY_SetGlobalConfigValue(const string& name, int value, int mode = NO_CREATE_NEW_MODE);
  // wgEditXML::SUMMARY_SetChConfigValue
  // data -> ch_%d -> config -> name
  // Same as above but set a parameter for a specific channel
  void SUMMARY_SetChConfigValue(const string& name, int value, int ichan, int mode = NO_CREATE_NEW_MODE);
  // wgEditXML::SUMMARY_SetChFitValue
  // data -> ch_%d -> fit -> name
  void SUMMARY_SetChFitValue(const string& name, int value, int ichan, int mode = NO_CREATE_NEW_MODE);
  void SUMMARY_SetPedFitValue(double*,int,int);
  void SUMMARY_AddGlobalElement(const string&);
  void SUMMARY_AddChElement(const string&,int);
  // wgEditXML::SUMMARY_GetChConfigValue
  // data -> config -> name
  int SUMMARY_GetGlobalConfigValue(const string&);
  // wgEditXML::SUMMARY_GetChConfigValue
  // data -> ch_%d -> config -> name
  int SUMMARY_GetChConfigValue(const string&,int);
  // wgEditXML::SUMMARY_GetChFitValue
  // data -> ch_%d -> fit -> name
  double SUMMARY_GetChFitValue(const string&,int);
  void SUMMARY_GetPedFitValue(double*,int);

  //=======================================================================//
  //                         S-curve XML files                             //
  //=======================================================================//
  
  void SCURVE_Make(const string&);
  void SCURVE_SetValue(const string&,int,double,int);
  double SCURVE_GetValue(const string&,int);

  //=======================================================================//
  //                           ??? XML files                               //
  //=======================================================================//
  
  void OPT_Make(const string&);
  void OPT_SetValue(const string&,int,int,int,double,int);
  double OPT_GetValue(const string&,int,int,int);
  void OPT_SetChipValue(const string&,int,int,double,int);
  double OPT_GetChipValue(const string&,int,int);

  //=======================================================================//
  //                           ??? XML files                               //
  //=======================================================================//
  
  void PreCalib_Make(const string&);
  void PreCalib_SetValue(const string&,int,int,int,double,int);
  double PreCalib_GetValue(const string&,int,int,int);

  //=======================================================================//
  //               Gain and pedestal calibration XML files                 //
  //=======================================================================//

  // wgEditXML::Calib_Make
  // Create a template pedestal_card.xml file with the following structure:
  // pe1 is the position of the 1 p.e. peak with respect to the pedestal
  // pe2 is the position of the 2 p.e. peak with respect to the pedestal
  // Gain is the gain in ADC counts
  /* ----------------------------------------------------------------------------
<data>
    <n_difs> n_difs </n_difs>
    <n_chips>n_chips</n_chips>
    <n_chans>n_chans</n_chans>
    <dif_1>
        <chip_0>
             <ch_0>
                  <pe1></pe1>
                  <pe2></pe2>
                  <Gain></Gain>
             </ch_0>
             ...
        </chip_0>
        ...
    </dif_1>
    ...
</data>
-------------------------------------------------------------------------------- */
  void Calib_Make(const string& filename, unsigned n_difs = NDIFS, unsigned n_chips = NCHIPS, unsigned n_chans = NCHANNELS);
  void Calib_SetValue(const string&,int,int,int,double,int);
  double Calib_GetValue(const string&,int,int,int);
};

#endif
