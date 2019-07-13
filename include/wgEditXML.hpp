#ifndef WG_EDITXML_H_INCLUDE
#define WG_EDITXML_H_INCLUDE

// system includes
#include <string>
#include <vector>

// user includes
#include "tinyxml2.hpp"
#include "wgTopology.hpp"
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
  bool GetConfig(const string& configxml,
                 unsigned igdcc,
                 unsigned idif,
                 unsigned n_chips,
                 unsigned n_chans,
                 vector<vector<int>>& v);

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
        <difid>1</difid>
        <chipid>1</chipid>
        <chanid>1</chanid>
        <start_time>0</start_time>
        <stop_time>0</stop_time>
        <trigth>-1</trigth>
        <gainth>-1</gainth>
        <inputDAC>-1</inputDAC>
        <HG>-1</HG>
        <LG>-1</LG>
	<trig_adj>-1<trig_adj>
    </config>
    <chan 1>
	<col>1<col_1/>
            <charge_nohit>-1</charge_nohit>
            <sigma_nohit>-1</sigma_nohit>
            <charge_hit>-1</charge_hit>
            <sigma_hit>-1</sigma_hit>
            <charge_hit_HG>-1</charge_hit_HG>
            <sigma_hit_HG>-1</sigma_hit_HG>
        <col>2<col_2/>
	...
    </chan 1>
</data>
-------------------------------------------------------------------------------- */
  void Make(const string& filename, unsigned idif, unsigned ichip, unsigned ichan);

  // wgEditXML::SetConfigValue
  // data -> config -> name
  void SetConfigValue(const string& name, int value, bool create_new = false);

  // wgEditXML::SetColValue
  // data -> chan -> col_%d -> name
  void SetColValue(const string& name, int icol, int value, bool create_new = false);

  // wgEditXML::SetChValue
  // data -> chan -> name
  void SetChValue(const string& name, int value, bool create_new = false);

  // wgEditXML::AddColElement
  // data -> chan -> col_%d -> name
  void AddColElement(const string& name, int icol);

  // wgEditXML::AddChElement
  // data -> chan -> name
  void AddChElement(const string& name);

  // wgEditXML::GetColValue
  // data -> chan -> col_%d -> name
  int GetColValue(const string& name, int icol);

  // wgEditXML::GetChValue
  // data -> chan -> name
  int GetChValue(const string& name);

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
        <difid>1</difid>
        <chipid>1</chipid>
        <n_chans>32</n_chans>
        <start_time>0</start_time>
        <stop_time>0</stop_time>
        <trigth>-1</trigth>
        <gainth>-1</gainth>
    </config>
    <chan_1>
        <config>
            <chanid>1</chanid>
            <inputDAC>-1</inputDAC>
            <ampDAC>-1</ampDAC>
            <adjDAC>-1</adjDAC>
        </config>
        <fit>
            <noise>-1</noise>
            <sigma_noise>-1</sigma_noise>
            <pe_level>-1</pe_level>
            <!-- for each column -->
	    <charge_nohit_1>-1</charge_nohit_1>
	    <sigma_nohit_1>-1</sigma_nohit_1>
	    <charge_hit_1>-1</charge_hit_1>
	    <sigma_hit_1>-1</sigma_hit_1>
	    <diff_1>-1</diff_1>
	    <sigma_diff_1>-1</sigma_diff_1>
        </fit>

    </chan_1>
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
  void SUMMARY_SetPedFitValue(int value[MEMDEPTH], int ichan, bool create_new = false);

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
  int SUMMARY_GetChFitValue(const string& name, int ich);

  // wgEditXML::SUMMARY_GetPedFitValue
  // data -> ch_%d -> fit -> -> ped_%d
  void SUMMARY_GetPedFitValue(int value[MEMDEPTH], int ich);

  //=======================================================================//
  //                   Optimized Threshold XML files                       //
  //=======================================================================//
  
  void OPT_Make(const string& filename, 
                vector<unsigned> inputDACs,
                unsigned n_difs,
                vector<unsigned> n_chips,
                vector<vector<unsigned>> n_chans);

  // wgEditXML::OPT_SetValue
  // data -> dif_%d -> chip_%d -> chan_%d -> inputDAC_%d
  void OPT_SetValue(const string& name,
                    unsigned idif, 
                    unsigned ichip, 
                    unsigned ichan, 
                    unsigned iDAC, 
                    double value, 
                    bool create_new);
  
  // wgEditXML::OPT_GetValue
  // data -> dif_%d -> chip_%d -> chan_%d -> inputDAC_%d
  double OPT_GetValue(const string& name,
                      unsigned idif,
                      unsigned ichip,
                      unsigned ichan,
                      unsigned iDAC);

  // wgEditXML::OPT_SetChanValue
  // data -> dif_%d -> chip_%d -> chan_%d 
  void OPT_SetChanValue(const string& name,
                        unsigned idif,
                        unsigned ichip,
                        unsigned ichan,
                        double value,
                        bool create_new);

  // wgEditXML::OPT_GetChanValue
  // data -> dif_%d -> chip_%d -> chan_%d 
  double OPT_GetChanValue(const string& name,
                          unsigned idif,
                          unsigned ichip,
                          unsigned ichan);

  //=======================================================================//
  //                           ??? XML files                               //
  //=======================================================================//
  
  void PreCalib_Make(const string&);
  void PreCalib_SetValue(const string&,int,int,int,int,bool);
  int PreCalib_GetValue(const string&,int,int,int);

  //=======================================================================//
  //                   Pedestal calibration XML files                      //
  //=======================================================================//

  // wgEditXML::Pedestal_Make
  // Create a template pedestal_card.xml file with the following structure:
  // pe1 is the position of the 1 p.e. peak with respect to the pedestal
  // pe2 is the position of the 2 p.e. peak with respect to the pedestal
  // Gain is the gain in ADC counts
  /* ----------------------------------------------------------------------------
<data>
    <config>
        <n_difs> n_difs </n_difs>
    </config>
    <dif_1>
        <config>
             <n_chips> n_chips </n_chips>
        </config>
        <chip_1>
             <config>
                  <n_chans> n_chans </n_chans>
             </config>
             <chan_1>
                  <!-- for each column -->
                  <pe1></pe1>
                  <pe2></pe2>
                  <gain></gain>
                  <sigma_gain></sigma_gain>
                  <ped></ped>
                  <sigma_ped></sigma_ped>
                  <meas_ped></meas_ped>
                  <sigma_meas_ped></sigma_meas_ped>
             </chan_1>
             ...
        </chip_1>
        ...
    </dif_1>
    ...
</data>
-------------------------------------------------------------------------------- */
  void Pedestal_Make(const string& filename, Topology& topol);

  // wgEditXML::Pedestal_GetDifConfigValue
  // data -> dif_%d -> config -> name
  int Pedestal_GetDifConfigValue(const string& name, int idif);

  // wgEditXML::Pedestal_SetDifConfigValue
  // data -> dif_%d -> config -> name
  void Pedestal_SetDifConfigValue(const string& name, int idif,
                                  int value, bool create_new);
  
  // wgEditXML::Pedestal_GetChipConfigValue
  // data -> dif_%d -> chip_%d -> config -> name
  int Pedestal_GetChipConfigValue(const string& name, int idif, int ichip);

  // wgEditXML::Pedestal_SetChipConfigValue
  // data -> dif_%d -> chip_%d -> config -> name
  void Pedestal_SetChipConfigValue(const string& name, int idif,
                                   int ichip, int value, bool create_new);
  
  // wgEditXML::Pedestal_SetValue
  // data -> dif_%d -> chip_%d -> ch_%d -> name
  void Pedestal_SetChanValue(const string& name, int idif, int ichip, int ich,
                             int value, bool create_new = false);

  // wgEditXML::Pedestal_GetValue
  // data -> dif_%d -> chip_%d -> ch_%d -> name
  int Pedestal_GetChanValue(const string& name, int idif, int ichip, int ich);
};

#endif
