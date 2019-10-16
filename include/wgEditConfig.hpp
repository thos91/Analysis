#ifndef WAGASCI_EDIT_CONFIG_HPP_
#define WAGASCI_EDIT_CONFIG_HPP_

// system C++ includes
#include <string>
#include <vector>

// user includes
#include "wgConst.hpp"

class wgEditConfig
{
private:
  std::string hex_config;
  std::string bi_config;
  // wgEditConfig::DeToBi convert a decimal number (in the form a string) to its
  // binary representation (always in the form of a string)
  std::string DeToBi(const std::string&);
  // binary to decimal
  std::string BiToDe(const std::string&);
  // hexadecimal to binary
  std::string HexToBi(const std::string&);
  // binary to hexadecimal
  std::string BiToHex(const std::string&);
  //int fine_inputDAC[32];

  // wgEditConfig::Open
  // Open a file using the passed path and read the first line (that is assumed
  // to contain a bitstream std::string). Then it is the same as the SetBitstream
  // method.
  void Open(const std::string&);

  // wgEditConfig::SetBitstream
  // copy the input bitstream into the hex_config member and into the bi_config
  // member after converting it into binary representation
  void SetBitstream(const std::string&);

  // wgEditConfig::GetCSV
  // read the content of the spiroc2d.csv file into a vector of vectors strings
  // Each line is decomposed using the ',' delimeter and each field is saved as
  // a string (so each line is stored as a vector of strings and the whole file
  // is stored as a vector of vector of strings)
  std::vector<std::vector<std::string>> GetCSV(std::string spiroc2d_csv = "");
  
  int m_fine_inputDAC[NCHANNELS] = {}; // fine-tuned input DAC (voltage adjustment)
  
public:
  // constructor
  // Open a bitstream file and initialized the members to zero
  wgEditConfig(const std::string& input, bool is_bitstream_string);
  
  // wgEditConfig::Write
  // Write the bi_config string member to the output file
  void Write(const std::string& output);

  // wgEditConfig::Modify
  // Modify the value "input" (it is a string with the binary representation of
  // value) starting from position "start"
  void Modify(const std::string& input, int start);

  // wgEditConfig::Clear
  // set the hex_config and bi_config strings to empty
  void Clear();

  // wgEditConfig::CheckAll
  // Print a list of all the parameters contained in the opened bitstream The
  // parameters names and other properties are retrieved through the GetCSV
  // method
  void CheckAll();

  // ==================================================================== //
  //                        Change* methods                               //
  // ==================================================================== //

  // wgEditConfig::Change_inputDAC
  // Change the channel by channel adjustable 8-bit inputDAC.
  // The inputDAC is the channel by channel high voltage correction.
  void Change_inputDAC(unsigned chan, int value);

  // wgEditConfig::Change_ampDAC
  // Change the channel by channel adjustable 6-bit high gain (HG) preamp.
  // It corresponds to the value of the preamp feedback capacitor 
  void Change_ampDAC(unsigned chan, unsigned value);

  // wgEditConfig::Change_trigadj
  // Change the channel by channel adjustable 4-bit discriminator threshold
  void Change_trigadj(unsigned chan, unsigned value);

  // wgEditConfig::Change_trigth
  void Change_trigth(unsigned value);

  // wgEditConfig::Change_trigth_and_adj
  // Change both the global and adjustable threshold for all the channels
  void Change_trigth_and_adj(std::vector<unsigned> threshold);

  // wgEditConfig::Change_gainth
  void Change_gainth(unsigned value);

  // wgEditConfig::Change_chipid
  void Change_chipid(unsigned value);
  
  // wgEditConfig::Change_1bitparam
  void Change_1bitparam(unsigned value, int subadd);

  // ==================================================================== //
  //                           Get* methods                               //
  // ==================================================================== //
  
  // There is no exception safety for all these methods. This means that you
  // should try/catch any exception at some higher level. At least the same
  // exception of the GetValue method can be thrown.

  // wgEditConfig::GetValue
  // extract a parameter value from the bi_string bitstream string.  The
  // parameter value starts on the "start+VALUE_OFFSET_IN_BITS" and spans for
  // "length" bits. If a value lying over the edge of the string is requested a
  // invalid_argument exception is thrown.
  std::string GetValue(unsigned start, unsigned length);

  // wgEditConfig::Get_inputDAC
  // Get the channel by channel adjustable input 8-bit DAC
  int Get_inputDAC(unsigned chan);

  // Get the channel by channel adjustable 6-bit high gain (HG) preamp.
  // It corresponds to the value of the preamp feedback capacitor 
  int Get_ampDAC(unsigned chan);

  // Get the channel by channel adjustable 4-bit discriminator threshold
  int Get_trigadj(unsigned chan);

  // Get the global 10-bit threshold
  int Get_trigth();

  // Get the 10-bit dual DAC output 2 (Gain Selection Discriminator Threshold)
  int Get_gainth();

  int Get_1bitparam(int);
};

#endif // WAGASCI_EDIT_CONFIG_HPP_
