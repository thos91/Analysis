#ifndef WG_EDITXML_H_INCLUDE
#define WG_EDITXML_H_INCLUDE

// system includes
#include <string>
#include <vector>

// user includes
#include "tinyxml2.hpp"
#include "wgConst.hpp"

#define XML_ELEMENT_STRING_LENGTH 32

using namespace tinyxml2;
using namespace std;

//=======================================================================//
//                                                                       //
//                         wgEditXML class                               //
//                                                                       //
//=======================================================================//

// The purpose of this class is to open, close, read and write XML files. Each
// object of the wgEditXML class refers to a single XML file at a time. After
// the object has been created and before anything else, the file needs to be
// opened with the Open method There are generic methods and method used only
// for a specific type of XML files.

// In general all the "Get*" methods read something from the XML file and the
// "Set*" methods write something. The "Make*" methods create an default
// template for the type of XML file they refer to.
// So the logical progression is:
//    create wgEditXML object -> Open -> Make* -> Set* -> Close -> delete object
// to set a value;
//    create wgEditXML object -> Open -> Get* -> Close -> delete object
// to get a value.

// All the Get* and Set* methods throw a wgElementNotFound exception if the
// element that they refer to is not found.

// All the Set* methods can be called in one of the following two modes:
// NO_CREATE_NEW_MODE : don't create an element if it is not found
// CREATE_NEW_MODE    : create the element if it is not found

class wgEditXML
{
private:
  string filename;
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
  // data -> config -> name
  void SetConfigValue(const string& name, int value, bool create_new = false);

  // wgEditXML::SetColValue
  // data -> ch -> col_%d -> name
  void SetColValue(const string& name, int icol, double value, bool create_new = false);

  // wgEditXML::SetChValue
  // data -> ch -> name
  void SetChValue(const string& name, double value, bool create_new = false);

  // wgEditXML::AddColElement
  // data -> ch -> col_%d -> name
  void AddColElement(const string& name, int icol);

  // wgEditXML::AddChElement
  // data -> ch -> name
  void AddChElement(const string& name);

  // wgEditXML::GetColValue
  // data -> ch -> col_%d -> name
  double GetColValue(const string& name, int icol);

  // wgEditXML::GetChValue
  // data -> ch -> name
  double GetChValue(const string& name);

  // wgEditXML::GetConfigValue
  // data -> config -> name
  int GetConfigValue(const string& name);

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
  void SUMMARY_SetGlobalConfigValue(const string& name, int value, bool create_new = false);

  // wgEditXML::SUMMARY_SetChConfigValue
  // data -> ch_%d -> config -> name
  void SUMMARY_SetChConfigValue(const string& name, int value, int ichan, bool create_new = false);

  // wgEditXML::SUMMARY_SetChFitValue
  // data -> ch_%d -> fit -> name
  void SUMMARY_SetChFitValue(const string& name, int value, int ichan, bool create_new = false);

  // wgEditXML::SUMMARY_SetPedFitValue
  // data -> ch_%d -> fit -> ped_%d
  void SUMMARY_SetPedFitValue(double value[MEMDEPTH], int ichan, bool create_new = false);

  // wgEditXML::SUMMARY_AddGlobalElement
  // data -> name
  void SUMMARY_AddGlobalElement(const string& name);

  // wgEditXML::SUMMARY_AddChElement
  // data -> ch_%d -> name
  void SUMMARY_AddChElement(const string& name, int ich);

  // wgEditXML::SUMMARY_GetChConfigValue
  // data -> config -> name
  int SUMMARY_GetGlobalConfigValue(const string& name);

  // wgEditXML::SUMMARY_GetChConfigValue
  // data -> ch_%d -> config -> name
  int SUMMARY_GetChConfigValue(const string& name, int ich);

  // wgEditXML::SUMMARY_GetChFitValue
  // data -> ch_%d -> fit -> name
  double SUMMARY_GetChFitValue(const string& name, int ich);

  // wgEditXML::SUMMARY_GetPedFitValue
  // data -> ch_%d -> fit -> -> ped_%d
  void SUMMARY_GetPedFitValue(double value[MEMDEPTH], int ich);

  //=======================================================================//
  //                         S-curve XML files                             //
  //=======================================================================//
  
  void SCURVE_Make(const string&);
  void SCURVE_SetValue(const string&,int,double,bool);
  double SCURVE_GetValue(const string&,int);

  //=======================================================================//
  //                           ??? XML files                               //
  //=======================================================================//
  
  void OPT_Make(const string&);
  void OPT_SetValue(const string&,int,int,int,double,bool);
  double OPT_GetValue(const string&,int,int,int);
  void OPT_SetChipValue(const string&,int,int,double,bool);
  double OPT_GetChipValue(const string&,int,int);

  //=======================================================================//
  //                           ??? XML files                               //
  //=======================================================================//
  
  void PreCalib_Make(const string&);
  void PreCalib_SetValue(const string&,int,int,int,double,bool);
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

  // wgEditXML::Calib_SetValue
  // data -> dif_%d -> chip_%d -> ch_%d -> name
  void Calib_SetValue(const string& name, int idif, int ichip, int ich, double value, bool create_new = false);

  // wgEditXML::Calib_GetValue
  // data -> dif_%d -> chip_%d -> ch_%d -> name
  double Calib_GetValue(const string& name,int idif, int ichip, int ich);
};

#endif
