// system C++ includes
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <exception>
#include <numeric>
#include <bitset>

// boost includes
#include <boost/algorithm/string.hpp>

// ROOT includes
#include "TFile.h"
#include "TTree.h"

// user includes
#include "wgConst.hpp"
#include "wgExceptions.hpp"
#include "wgFileSystemTools.hpp"
#include "wgLogger.hpp"
#include "wgEditConfig.hpp"

//#define DEBUG_WG_EDIT_CONFIG

using namespace wagasci_tools;

//*********************************************************************************
wgEditConfig::wgEditConfig(const std::string& input, bool bitstream_string){
  this->Clear();
  if (bitstream_string) this->SetBitstream(input);
  else this->Open(input);
}
  
//*********************************************************************************
std::vector<std::vector<std::string>> wgEditConfig::GetCSV(std::string spiroc2d_csv) {
  
  if (spiroc2d_csv.empty()) {
    wgEnvironment env;
    spiroc2d_csv = string(env.MAIN_DIRECTORY + "/configs/spiroc2d/spiroc2d.csv");
  }
  if (!check_exist::CsvFile(spiroc2d_csv))
    throw wgInvalidFile("[wgEditConfig::GetCSV][" + spiroc2d_csv + "] file not found");
  ifstream ifs(spiroc2d_csv.c_str());
	  
  std::string line;
  std::vector<std::vector<std::string>> output;
  std::vector<std::string> strvec;
  try {
    while (getline(ifs, line)) {
      boost::split(strvec, line, boost::is_any_of(","));
      output.push_back(strvec);
    }
  } catch(...) {
    ifs.close();
    throw;
  }
  
#ifdef DEBUG_WG_EDIT_CONFIG
  for(const std::vector<std::string> line: output) {
    for(std::string field: line) {
      cout << field ;
      cout << ",\t";
    }
    cout << endl;
  }
#endif
  
  return output;
}

//*********************************************************************************
void wgEditConfig::Get_MPPCinfo(int ichip){ 
  wgEnvironment env;
  
  
  std::string mppc_csv(env.CALICOES_DIRECTORY + "/config/spiroc2d/mppc_map.csv");
  std::string line;
  std::vector<std::string> tmp_mppc_map;
  double tmp_bdv;
  int tmp_serial,tmp_ch;
  int mppc_map[NCHIPS];

  if ( !check_exist::CsvFile(mppc_csv) )
    throw wgInvalidFile("[Get_MPPCinfo][" + mppc_csv + "] mppc_csv file not found");
  std::ifstream ifs(mppc_csv);
  
  for(unsigned i = 0; i < NCHIPS; i++){
    getline(ifs,line);
    boost::split(tmp_mppc_map, line, boost::is_any_of(","));
    mppc_map[stoi(tmp_mppc_map[0])] = stoi(tmp_mppc_map[1]);
  }

  std::string mppc_root(env.CALICOES_DIRECTORY + "/config/spiroc2d/arraymppc_data.root");
  if ( !check_exist::RootFile(mppc_root) )
    throw wgInvalidFile("[Get_MPPCinfo][" + mppc_root + "] arraymppc_data.root file not found");
  TFile *fmppc = new TFile(mppc_root.c_str(), "read");
  TTree *mppc = (TTree*)fmppc->Get("mppc");

  mppc->SetBranchAddress("BDV",    &tmp_bdv);
  mppc->SetBranchAddress("serial", &tmp_serial);
  mppc->SetBranchAddress("ch",     &tmp_ch);

  //set 51V to inputDAC=0, and 53.5V to inputDAC=255
  //if break down = A, over voltage = V-53.5
  for(unsigned i = 0; i < mppc->GetEntries(); i++) {
    mppc->GetEntry(i);
    if(mppc_map[ichip] == tmp_serial){
      this->fine_inputDAC[tmp_ch] = (tmp_bdv - 51.0) * 256 / 2.5;
      this->BDV[tmp_ch] = tmp_bdv;
#ifdef DEBUG_CHANGECONFIG
      std::stringstream ss;
      ss << ichip << " / " << tmp_serial << " / " << tmp_ch << " / "<< fine_inputDAC[tmp_ch];
      Log.Write(ss.str());
#endif
    }
  }
  ifs.close();
  fmppc->Close();
  delete mppc;
  delete fmppc;
  Read_MPPCData=true; 
}

//*********************************************************************************
void wgEditConfig::Open(const std::string& input){
  std::string str;
  std::ifstream ifs(input.c_str());
  getline(ifs, str);
  if( str.size() != BITSTREAM_HEX_STRING_LENGTH) {
    ifs.close();
    throw std::invalid_argument("[wgEditConfig::SetBitstream] wrong size of the bitstream string : " + to_string(input.size()));
  }
  ifs.close();
  wgEditConfig::hex_config = str.substr(2, BITSTREAM_HEX_STRING_LENGTH - 2);
  wgEditConfig::bi_config = this->HexToBi(wgEditConfig::hex_config);
}

//*********************************************************************************
void wgEditConfig::SetBitstream(const std::string& input){
  if( input.size() != BITSTREAM_HEX_STRING_LENGTH - 1)
    throw std::invalid_argument("[wgEditConfig::SetBitstream] wrong size of the bitstream string : " + to_string(input.size()));
  wgEditConfig::hex_config = input.substr(2, BITSTREAM_HEX_STRING_LENGTH - 3) + "0";
  wgEditConfig::bi_config = this->HexToBi(wgEditConfig::hex_config);
}

//*********************************************************************************
void wgEditConfig::Modify(const std::string& input, const int start) {
  unsigned int length = input.size();
  if(length + start > BITSTREAM_BIN_STRING_LENGTH) {
    throw std::invalid_argument("[wgEditConfig::Modify] bad data size : length = " + to_string(length) + ", start = " + to_string(start));
  } 
  wgEditConfig::bi_config.replace(start + VALUE_OFFSET_IN_BITS, length, input.c_str());
}

//*********************************************************************************
void wgEditConfig::Write(const std::string& output){
  if(wgEditConfig::bi_config.size() != BITSTREAM_BIN_STRING_LENGTH) {
    throw runtime_error("[wgEditConfig::Write] wrong length for binary configuration string (bi_config) : " + to_string(bi_config.size()) + " != " + to_string(BITSTREAM_BIN_STRING_LENGTH));
  }
  std::string str("0x");
  str += this->BiToHex(wgEditConfig::bi_config);
  std::ofstream outputfile(output);
  outputfile << str.c_str();
  outputfile.close();  
  wgEditConfig::Read_MPPCData = false;
}

//*********************************************************************************
void wgEditConfig::Clear(){
  wgEditConfig::hex_config="";
  wgEditConfig::bi_config="";
  wgEditConfig::Read_MPPCData=false;
}

//*********************************************************************************
std::string wgEditConfig::GetValue(const unsigned start, const unsigned length) {
  if(start + length > BITSTREAM_BIN_STRING_LENGTH)
    throw std::invalid_argument("[wgEditConfig::GetValue] reached the edge of the bitstring string (start = " + to_string(start) + ", length = " + to_string(length));
  return wgEditConfig::bi_config.substr(start + VALUE_OFFSET_IN_BITS, length);
}

//*********************************************************************************
void wgEditConfig::CheckAll() {
  std::vector<std::vector<std::string>> csv(this->GetCSV());
  std::string name;
  std::string value;
  int start,length;
  std::stringstream ss;

  for(const std::vector<std::string> line: csv) {
    name = line[0];         // Name of the parameter
    length = stoi(line[1]); // Length in bits of the field
    start = stoi(line[3]);  // Starting bit of the field
    // If the length is a multiple of the number of channels it means that we
    // are dealing with a parameter that can be set individually for each
    // channel
    if (length % NCHANNELS == 0) {
      int chanbit = length / NCHANNELS;
      ss  << "name = " << name << " | values = ";
      for(unsigned ichan = 0; ichan < NCHANNELS; ichan++) {
        value = this->GetValue(start + ichan * chanbit, chanbit);
        ss << "[" << value << "],";
      }
      ss << endl;
    }
    else {
      value = this->GetValue(start, length);
      ss << "name = " << name << " | value = [" << value << "]" << endl;
    }
  }
  Log.Write("[wgEditConfig::CheckAll] " + ss.str());
}

//*********************************************************************************
std::string wgEditConfig::HexToBi(const std::string& input){
  std::string output("");
  for(const char& c : input) {
    if(c=='0')         output = "0000" + output;
    if(c=='1')         output = "1000" + output;
    if(c=='2')         output = "0100" + output;
    if(c=='3')         output = "1100" + output;
    if(c=='4')         output = "0010" + output;
    if(c=='5')         output = "1010" + output;
    if(c=='6')         output = "0110" + output;
    if(c=='7')         output = "1110" + output;
    if(c=='8')         output = "0001" + output;
    if(c=='9')         output = "1001" + output;
    if(c=='A'||c=='a') output = "0101" + output;
    if(c=='B'||c=='b') output = "1101" + output;
    if(c=='C'||c=='c') output = "0011" + output;
    if(c=='D'||c=='d') output = "1011" + output;
    if(c=='E'||c=='e') output = "0111" + output;
    if(c=='F'||c=='f') output = "1111" + output;
  } 
  return output;
}

//*********************************************************************************
std::string wgEditConfig::BiToHex(const std::string& input){
  std::string output("");
  if(input.size() % 4 != 0) return output;
  for(unsigned i = input.size(); i > 3; i -= 4) {
    std::string word = input.substr(i - 4, 4); 
    if(word=="0000") output += "0";
    if(word=="1000") output += "1";
    if(word=="0100") output += "2";
    if(word=="1100") output += "3";
    if(word=="0010") output += "4";
    if(word=="1010") output += "5";
    if(word=="0110") output += "6";
    if(word=="1110") output += "7";
    if(word=="0001") output += "8";
    if(word=="1001") output += "9";
    if(word=="0101") output += "A";
    if(word=="1101") output += "B";
    if(word=="0011") output += "C";
    if(word=="1011") output += "D";
    if(word=="0111") output += "E";
    if(word=="1111") output += "F";
  } 
  return output;
}

//*********************************************************************************
std::string wgEditConfig::BiToDe(const std::string& input){
  std::string word;
  int decimal=0;
  
  for(unsigned int i=0; i<input.size(); i++){
    word = input.substr(input.size()-1-i,1);
    decimal += stoi(word.c_str()) * pow(2,i);
  } 
  return to_string(decimal);
}

//*********************************************************************************
std::string wgEditConfig::DeToBi(const std::string& input){
  int decimal = std::stoi(input);
  if ( decimal < 0 )
    throw std::invalid_argument("DeToBi: cannot convert negative numbers");
  if ( decimal > INT_MAX )
    throw std::invalid_argument("DeToBi: input greater than INT_MAX");
  std::stringstream ss;
  std::bitset<32> binary((uint32_t) decimal);
  for (int i = 31; i >= 0; i--) {
    ss << binary[i];
  }
  std::string output(ss.str());
  output.erase(0, min(output.find_first_not_of('0'), output.size() - 1));
  return output;
}

//*********************************************************************************
void wgEditConfig::Change_inputDAC(const unsigned chan, unsigned value) {
  if(chan < 0 || chan > NCHANNELS) {
    throw invalid_argument("channel is out of range : " + to_string(chan));
  }
  if(value + fine_inputDAC[chan] > MAX_VALUE_8BITS) {
    throw invalid_argument("value is out of range : " + to_string(value + fine_inputDAC[chan]));
  }

  if(this->Read_MPPCData) {
    value += this->fine_inputDAC[chan];
  }

  std::stringstream num;
  num << setfill('0') << setw(ADJ_INPUTDAC_LENGTH) << DeToBi(to_string(value)) << '1';

  if(chan == NCHANNELS) {
    for(unsigned ichan = 0; ichan < NCHANNELS; ichan++) {
      this->Modify(num.str(), ADJ_INPUTDAC_START + ichan * ADJ_INPUTDAC_OFFSET);
    }
  }
  else {
    this->Modify(num.str(), ADJ_INPUTDAC_START + chan * ADJ_INPUTDAC_OFFSET);
  }
}

//*********************************************************************************
void wgEditConfig::Change_ampDAC(const unsigned chan, const unsigned value) {
  if (value > MAX_VALUE_6BITS) {
    throw std::invalid_argument("value is out of range : " + to_string(value));
  }
  if (chan > NCHANNELS) {
    throw std::invalid_argument("channel is out of range : " + to_string(chan));
  }
  std::stringstream num;
  num << setfill('0') << setw(ADJ_AMPDAC_LENGTH) << DeToBi(to_string(value)) << setfill('0') << setw(ADJ_AMPDAC_LENGTH) << DeToBi(to_string(value)) << "000";

  if (chan == NCHANNELS) {
    for(unsigned ichan = 0; ichan < NCHANNELS; ichan++) {
      this->Modify(num.str(), ADJ_AMPDAC_START + ichan * ADJ_AMPDAC_OFFSET);
    }
  }
  else {
    this->Modify(num.str(), ADJ_AMPDAC_START + chan * ADJ_AMPDAC_OFFSET);
  }
}

//*********************************************************************************
void wgEditConfig::Change_trigadj(const unsigned chan, const unsigned value) {
  if(value > MAX_VALUE_4BITS) {
    throw std::invalid_argument("value is out of range : " + to_string(value));
  }
  if (chan > NCHANNELS) {
    throw std::invalid_argument("channel is out of range : " + to_string(chan));
  }
  std::stringstream num;
  num << setfill('0') << setw(ADJ_THRESHOLD_LENGTH) << DeToBi(to_string(value));

  if (chan == NCHANNELS) {
    for(unsigned ichan = 0; ichan < NCHANNELS; ichan++) {
      this->Modify(num.str(), ADJ_THRESHOLD_START + ichan * ADJ_THRESHOLD_OFFSET);
    }
  }
  else {
    this->Modify(num.str(), ADJ_THRESHOLD_START + chan * ADJ_THRESHOLD_OFFSET);
  }
}

//*********************************************************************************
void wgEditConfig::Change_trigth(const unsigned value) {
  if(value > MAX_VALUE_10BITS) {
    throw invalid_argument("value is out of range : " + to_string(value));
  }
  std::stringstream num;
  num << setfill('0') << setw(GLOBAL_THRESHOLD_LENGTH) << DeToBi(to_string(value));
  this->Modify(num.str(), GLOBAL_THRESHOLD_START);
}

//*********************************************************************************
void wgEditConfig::Change_gainth(const unsigned value) {
  if (value > MAX_VALUE_10BITS) {
    throw invalid_argument("value is out of range : " + to_string(value));
  }
  std::stringstream num;
  num << setfill('0') << setw(GLOBAL_GS_THRESHOLD_LENGTH) << DeToBi(to_string(value));
  this->Modify(num.str(), GLOBAL_GS_THRESHOLD_START);
}

//*********************************************************************************
int wgEditConfig::Get_inputDAC(const unsigned chan) {
  if (chan >= NCHANNELS)
    throw std::invalid_argument("channel " + to_string(chan) + " is out of range");
  return stoi( BiToDe( GetValue(ADJ_INPUTDAC_START + chan * ADJ_INPUTDAC_OFFSET, ADJ_INPUTDAC_LENGTH) ) );
}

//*********************************************************************************
int wgEditConfig::Get_ampDAC(const unsigned chan) {
  if (chan >= NCHANNELS)
    throw std::invalid_argument("channel " + to_string(chan) + " is out of range");
  return stoi( BiToDe( GetValue(ADJ_AMPDAC_START + chan * ADJ_AMPDAC_OFFSET, ADJ_AMPDAC_LENGTH) ) );
}

//*********************************************************************************
int wgEditConfig::Get_trigadj(const unsigned chan) {
  if (chan >= NCHANNELS)
    throw std::invalid_argument("channel " + to_string(chan) + " is out of range");
  return stoi( BiToDe( GetValue(ADJ_THRESHOLD_START + chan * ADJ_THRESHOLD_OFFSET, ADJ_THRESHOLD_LENGTH ) ) );
}

//*********************************************************************************
int wgEditConfig::Get_trigth() {
  return std::stoi( BiToDe ( GetValue(GLOBAL_THRESHOLD_START, GLOBAL_THRESHOLD_LENGTH ) ) );
}

//*********************************************************************************
int wgEditConfig::Get_gainth() {
  return stoi( BiToDe( GetValue( GLOBAL_GS_THRESHOLD_START, GLOBAL_GS_THRESHOLD_LENGTH ) ) );
}

//*********************************************************************************
void wgEditConfig::Change_1bitparam(const unsigned value, int subadd) {
  if (value != 0 && value != 1) {
    throw invalid_argument("value is out of range : " + to_string(value));
  }
  this->Modify(to_string(value), subadd);
}

//*********************************************************************************
int wgEditConfig::Get_1bitparam(int subadd){
  return stoi(GetValue(subadd,1));
}

//*********************************************************************************
void wgEditConfig::Change_trigth_and_adj(std::vector<unsigned> threshold) {
  unsigned mean_threshold = std::accumulate(threshold.begin(), threshold.end(), 0.0) / threshold.size();
  this->Change_trigth(mean_threshold);
  for (unsigned ichan = 0; ichan < threshold.size(); ++ichan) {
    this->Change_trigadj(ichan, threshold.at(ichan) - mean_threshold);
  }
}
