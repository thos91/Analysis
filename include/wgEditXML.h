#ifndef WG_EDITXML_H_INCLUDE
#define WG_EDITXML_H_INCLUDE

#include <string>
#include <vector>
#include "tinyxml2.h"

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
  void Open(const string&);
  void Close();
  void Write();
  void GetConfig(string&,unsigned int,unsigned int,vector<int>&);

  /* GetLog: Parse a .log file and fill a vector with:
      - v[0]: start_time
      - v[1]: stop_time
      - v[2]: nb_data_pkts
      - v[3]: nb_lost_pkts */
  void GetLog(const string&, vector<int>&);
  void SetConfigValue(string&,int,int);
  void SetColValue(string&,const int,double,int);
  void SetChValue(string&,double,int);
  void AddColElement(string&,const int);
  void AddChElement(string&);
  double GetColValue(string&,const int);
  double GetChValue(string&);
  int GetConfigValue(string&);

  void SUMMARY_Make(string&,const int);
  void SUMMARY_SetGlobalConfigValue(string&,int,int);
  void SUMMARY_SetChConfigValue(string&,int,int,int);
  void SUMMARY_SetChFitValue(string&,double,int,int);
  void SUMMARY_SetPedFitValue(double*,int,int);
  void SUMMARY_AddGlobalElement(string&);
  void SUMMARY_AddChElement(string&,int);
  int SUMMARY_GetGlobalConfigValue(string&);
  int SUMMARY_GetChConfigValue(string&,int);
  double SUMMARY_GetChFitValue(string&,int);
  void SUMMARY_GetPedFitValue(double*,int);

  void SCURVE_Make(string&);
  void SCURVE_SetValue(string&,int,double,int);
  double SCURVE_GetValue(string&,int);

  void OPT_Make(string&);
  void OPT_SetValue(string&,int,int,int,double,int);
  double OPT_GetValue(string&,int,int,int);
  void OPT_SetChipValue(string&,int,int,double,int);
  double OPT_GetChipValue(string&,int,int);

  void PreCalib_Make(string&);
  void PreCalib_SetValue(string&,int,int,int,double,int);
  double PreCalib_GetValue(string&,int,int,int);

  void Calib_Make(string&);
  void Calib_SetValue(string&,int,int,int,double,int);
  double Calib_GetValue(string&,int,int,int);
};

#endif
