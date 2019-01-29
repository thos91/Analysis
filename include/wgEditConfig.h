#include <string>
#include <vector>
#include <array>
#include "Const.h"

using namespace std;

class wgEditConfig
{
private:
  string hex_config;
  string bi_config;
  string DeToBi(string&);
  string BiToDe(string&);
  string HexToBi(string&);
  string BiToHex(string&);
  //int fine_inputDAC[32];

public:
  vector<string>  split(string&,char);
  vector<string> GetCSV();
  void Get_MPPCinfo(int);
  void Open(string&);
  void SetBitstream(string&);
  void Write(string&);
  void Modify(string&,int);
  void Clear();
  void CheckAll();
  string GetValue(int,int);
  void Change_inputDAC(int,int,int); 
  void Change_ampDAC(int,int); 
  void Change_trigadj(int,int); 
  void Change_trigth(int); 
  void Change_gainth(int); 
  int Get_inputDAC(int); 
  int Get_ampDAC(int); 
  int Get_trigadj(int); 
  int Get_trigth(); 
  int Get_gainth(); 
  void Change_1bitparam(int,int); 
  int Get_1bitparam(int); 

  int fine_inputDAC[32];
  float BDV[32];
  bool Read_MPPCData;

};
