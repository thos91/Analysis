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
  // wgEditConfig::DeToBi convert a decimal number (in the form a string) to its
  // binary representation (always in the form of a string)
  string DeToBi(const string&);
  // binary to decimal
  string BiToDe(const string&);
  // hexadecimal to binary
  string HexToBi(const string&);
  // binary to hexadecimal
  string BiToHex(const string&);
  //int fine_inputDAC[32];

public:
  int fine_inputDAC[32];
  float BDV[32];
  bool Read_MPPCData;
  
  vector<string>  split(string&,char);
  vector<string> GetCSV();
  void Get_MPPCinfo(int);
  // wgEditConfig::Open
  // Open a file using the passed path and read the first line (that is assumed
  // to contain a bitstream string). Then it is the same as the SetBitstream
  // method.
  void Open(const string&);
  // wgEditConfig::SetBitstream
  // copy the input bitstream into the hex_config member and into the bi_config
  // member after converting it into binary representation
  void SetBitstream(const string&);
  void Write(string&);
  void Modify(string&,int);
  void Clear();
  void CheckAll();

  void Change_inputDAC(int,int,int); 
  void Change_ampDAC(int,int); 
  void Change_trigadj(int,int); 
  void Change_trigth(int); 
  void Change_gainth(int);
  void Change_1bitparam(int,int);
  
  // wgEditConfig::Get methods There is no exception safety for all these
  // methods. This means that you should try/catch any exception at some higher
  // level. At least the same exception of the GetValue method can be thrown.

  // wgEditConfig::GetValue
  // extract a parameter value from the bi_string bitstream string.  The
  // parameter value starts on the "start+VALUE_OFFSET_IN_BITS" and spans for
  // "length" bits. If a value lying over the edge of the string is requested a
  // invalid_argument exception is thrown.
  string GetValue(int start, int length);
  // wgEditConfig::Get_inputDAC
  // Get the channel by channel adjustable input 8-bit DAC
  int Get_inputDAC(int chan);
  // Get the channel by channel adjustable 6-bit high gain (HG) preamp.
  // It corresponds to the value of the preamp feedback capacitor 
  int Get_ampDAC(int chan);
  // Get the channel by channel adjustable 4-bit discriminator threshold
  int Get_trigadj(int chan);
  // Get the global 10-bit threshold
  int Get_trigth();
  // Get the 10-bit dual DAC output 2 (Gain Selection Discriminator Threshold)
  int Get_gainth();
  int Get_1bitparam(int); 
};
