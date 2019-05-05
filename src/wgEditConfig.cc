#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <cstdint>
#include <exception>

#include <bits/stdc++.h>
#include <math.h>
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"

#include "Const.h"
#include "wgEditConfig.h"
#include "wgTools.h"

using namespace std;

//*********************************************************************************
vector<string> wgEditConfig::split(string& input, char delimiter)
{
  istringstream stream(input);
  string field;
  vector<string> result;
  while (getline(stream, field, delimiter)) {
    result.push_back(field);
  }
  return result;
}

//*********************************************************************************
vector<string> wgEditConfig::GetCSV(){
  wgConst *con = new wgConst();
  con->GetENV();
  char* map_csv=Form("%s/src/spiroc2d.csv",con->MAIN_DIRECTORY.c_str());
  ifstream ifs(map_csv);
  string line;
  vector<string> output;
  vector<string> strvec;  
  for(unsigned int i=0;i<85;i++){
    getline(ifs, line);
    strvec = this->split(line, ',');
    for(unsigned int j=0;j<5;j++){
      output.push_back(strvec[j]);
    }
  }
  ifs.close();
  return output;
}

//*********************************************************************************
void wgEditConfig::Get_MPPCinfo(int ichip){ 
  wgConst *con = new wgConst();
  con->GetENV();
  const char* mppc_csv= Form("%s/config/spiroc2d/mppc_map.csv",con->CALICOES_DIRECTORY.c_str());
  ifstream ifs(mppc_csv);
  string line;
  vector<string> tmp_mppc_map;
  float tmp_bdv;
  int tmp_serial,tmp_ch;
  int mppc_map[NCHIPS];

  for(unsigned int i=0;i<NCHIPS;i++){
    getline(ifs,line);
    tmp_mppc_map = this->split(line,',');
    mppc_map[atoi(tmp_mppc_map[0].c_str())]=atoi(tmp_mppc_map[1].c_str());
  }

  TFile *fmppc = new TFile( Form("%s/config/spiroc2d/arraymppc_data.root",con->CALICOES_DIRECTORY.c_str()),"read");
  TTree *mppc = (TTree*)fmppc->Get("mppc");

  mppc->SetBranchAddress("BDV",&tmp_bdv);
  mppc->SetBranchAddress("serial",&tmp_serial);
  mppc->SetBranchAddress("ch",&tmp_ch);

  //set 51V to inputDAC=0, and 53.5V to inputDAC=255
  //if break down = A, over voltage = V-53.5
  for(int i=0;i<mppc->GetEntries();i++){
    mppc->GetEntry(i);
    if(mppc_map[ichip]==tmp_serial){
      this->fine_inputDAC[tmp_ch]=(tmp_bdv-51.0)*256.0/2.5;
      this->BDV[tmp_ch]=tmp_bdv;
#ifdef DEBUG_CHANGECONFIG
      cout <<ichip << " / " << tmp_serial << " / " << tmp_ch << " / "<< fine_inputDAC[tmp_ch] << endl;
#endif
    }
  }
  ifs.close();
  fmppc->Close();
  Read_MPPCData=true; 
}

//*********************************************************************************
void wgEditConfig::Open(const string& input){
  string str;
  ifstream ifs(input.c_str());
  getline(ifs, str);
  if( str.size() != BITSTREAM_HEX_STRING_LENGTH)
	throw std::invalid_argument("[wgEditConfig::SetBitstream] wrong size of the bitstream string : " + to_string(input.size()));
  wgEditConfig::hex_config = str.substr(2, BITSTREAM_HEX_STRING_LENGTH - 2);
  wgEditConfig::bi_config = this->HexToBi(wgEditConfig::hex_config);
}

//*********************************************************************************
void wgEditConfig::SetBitstream(const string& input){
  if( input.size() != BITSTREAM_HEX_STRING_LENGTH - 1)
	throw std::invalid_argument("[wgEditConfig::SetBitstream] wrong size of the bitstream string : " + to_string(input.size()));
  wgEditConfig::hex_config = input.substr(2, BITSTREAM_HEX_STRING_LENGTH - 3) + "0";
  wgEditConfig::bi_config = this->HexToBi(wgEditConfig::hex_config);
}

//*********************************************************************************
void wgEditConfig::Modify(string& input,int ini){
  unsigned int length = input.size();
  if(length+ini>1192){ printf("Error! Bad Data Size!"); return;} 
  wgEditConfig::bi_config.replace(ini+6,length,input.c_str());
}

//*********************************************************************************
void wgEditConfig::Write(string& output){
  if(wgEditConfig::bi_config.size()!=1192){ printf("Error!\n"); return;}
  string str("0x");
  str += this->BiToHex(wgEditConfig::bi_config);
  ofstream outputfile(output);
  outputfile<<str.c_str();
  outputfile.close();  
  cout << "  Write :: " << output.c_str() << endl;
  wgEditConfig::Read_MPPCData=false;
}

//*********************************************************************************
void wgEditConfig::Clear(){
  wgEditConfig::hex_config="";
  wgEditConfig::bi_config="";
  wgEditConfig::Read_MPPCData=false;
}

//*********************************************************************************
string wgEditConfig::GetValue(int start, int length) {
  if(start + length > BITSTREAM_BIN_STRING_LENGTH)
	throw std::invalid_argument("[wgEditConfig::GetValue] reached the edge of the bitstring string (start = " + to_string(start) + ", length = " + to_string(length));
  return wgEditConfig::bi_config.substr(start + VALUE_OFFSET_IN_BITS, length);
}

//*********************************************************************************
void wgEditConfig::CheckAll(){
  vector<string> csv;
  csv = this->GetCSV();
  string name;
  string value;
  int ini,length;
  for(int i=0;i<85;i++){
    name = csv[i*5];
    length = stoi(csv[i*5+1].c_str());
    ini = stoi(csv[i*5+3].c_str()) ;
    if(length%36==0){
      int chanbit = length/36;
      cout << name << " / ";
      for(int chan=0;chan<36;chan++){
        value = this->GetValue(ini+chan*chanbit,chanbit);
        cout << "[" << value << "],";
      }
      cout << endl;
    }else{
      value = this->GetValue(ini,length);
      cout << name << " / " << value << endl;
    }
  }  
}

//*********************************************************************************
string wgEditConfig::HexToBi(const string& input){
  string output("");
  for(const char& c : input) {
    if(c=='0')         output += "0000";
    if(c=='1')         output += "1000";
    if(c=='2')         output += "0100";
    if(c=='3')         output += "1100";
    if(c=='4')         output += "0010";
    if(c=='5')         output += "1010";
    if(c=='6')         output += "0110";
    if(c=='7')         output += "1110";
    if(c=='8')         output += "0001";
    if(c=='9')         output += "1001";
    if(c=='A'||c=='a') output += "0101";
    if(c=='B'||c=='b') output += "1101";
    if(c=='C'||c=='c') output += "0011";
    if(c=='D'||c=='d') output += "1011";
    if(c=='E'||c=='e') output += "0111";
    if(c=='F'||c=='f') output += "1111";
  } 
  return output;
}

//*********************************************************************************
string wgEditConfig::BiToHex(const string& input){
  string output("");
  if(input.size() % 4 != 0) return output;
  for(unsigned i = input.size(); i > 3; i -= 4) {
    string word = input.substr(i-4,4); 
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
string wgEditConfig::BiToDe(const string& input){
  string word;
  int decimal=0;
  
  for(unsigned int i=0; i<input.size(); i++){
    word = input.substr(input.size()-1-i,1);
    decimal += stoi(word.c_str()) * pow(2,i);
  } 
  return to_string(decimal);
}

//*********************************************************************************
string wgEditConfig::DeToBi(const string& input){
  int decimal = std::stoi(input);
  if ( decimal < 0 )
	throw std::invalid_argument("DeToBi: cannot convert negative numbers");
  if ( decimal > (int) UINT32_MAX )
	throw std::invalid_argument("DeToBi: input greater than UINT32_MAX");
  std::stringstream ss;
  bitset<32> binary((uint32_t) decimal);
  for (int i = 31; i >= 0; i--) {
	ss << binary[i];
  }
  std::string output(ss.str());
  output.erase(0, min(output.find_first_not_of('0'), output.size() - 1));
  return output;
}

//*********************************************************************************
void wgEditConfig::Change_inputDAC(int chan,int value,int mode){
  if(chan <0 || chan>36){printf("WARNING! chan is out of range!\n"); return;}
  string num;

  if(mode==1 && this->Read_MPPCData){
    if(value+fine_inputDAC[chan]>255){
      printf("WARNING! value is out of range! set 0\n");
      num = Form("%d",255);
    }else{
      num = Form("%d",value+this->fine_inputDAC[chan]);
    }

  }else if(mode==1 && !this->Read_MPPCData){
    printf(" [wgEditConfig] WARNING! Read MPPC data before change inputDAC! set 0 \n");
    num = Form("%d",0);
 
  }else{
    if(value <0 || value>255){
      printf("WARNING! value is out of range! set 0\n");
      num = Form("%d",0);
    }else{
      num = Form("%d",value);
    }
  }

  num = DeToBi(num);
  num = Form("%08d1",stoi(num.c_str()));
  if(chan==36){
    for(int ichan=0;ichan<36;ichan++){
      this->Modify(num,37+ichan*9);
    }
  }else{
    this->Modify(num,37+chan*9);
  }
}

//*********************************************************************************
void wgEditConfig::Change_ampDAC(int chan,int value){
  if(value <0 || value>63){printf("WARNING! value is out of range!\n"); return;}
  if(chan <0 || chan>36){printf("WARNING! chan is out of range!\n"); return;}
  string num;
  num = Form("%d",value);
  num = DeToBi(num);
  num = Form("%06d%06d000",stoi(num.c_str()),stoi(num.c_str()));
  if(chan==36){
    for(int ichan=0;ichan<36;ichan++){
      this->Modify(num,367+ichan*15);
    }
  }else{
    this->Modify(num,367+chan*15);
  }
}

//*********************************************************************************
void wgEditConfig::Change_trigadj(int chan,int value){
  if(value <0 || value>15){printf("WARNING! value is out of range!\n"); return;}
  if(chan <0 || chan>36){printf("WARNING! chan is out of range!\n"); return;}
  string num;
  num = Form("%d",value);
  num = DeToBi(num);
  num = Form("%04d",stoi(num.c_str()));
  if(chan==36){
    for(int ichan=0;ichan<36;ichan++){
      this->Modify(num,1006+ichan*4);
    }
  }else{
    this->Modify(num,1006+chan*4);
  }
}

//*********************************************************************************
void wgEditConfig::Change_trigth(int value){
  if(value <0 || value>1023){printf("WARNING! value is out of range!\n"); return;}
  string num;
  num = Form("%d",value);
  num = DeToBi(num);
  num = Form("%010d",stoi(num.c_str()));
  this->Modify(num,931);
}

//*********************************************************************************
void wgEditConfig::Change_gainth(int value){
  if(value <0 || value>1023){printf("WARNING! value is out of range!\n"); return;}
  string num;
  num = Form("%d",value);
  num = DeToBi(num);
  num = Form("%010d",stoi(num.c_str()));
  this->Modify(num,941);
}

//*********************************************************************************
int wgEditConfig::Get_inputDAC(int chan){
  if( chan < 0 || chan >= NCHANNELS)
	throw std::invalid_argument("channel " + to_string(chan) + " is out of range");
  return stoi( BiToDe( GetValue(ADJ_INPUTDAC_START + chan * ADJ_INPUTDAC_OFFSET, ADJ_INPUTDAC_LENGTH) ) );
}

//*********************************************************************************
int wgEditConfig::Get_ampDAC(int chan){
  if( chan < 0 || chan >= NCHANNELS)
	throw std::invalid_argument("channel " + to_string(chan) + " is out of range");
  return stoi( BiToDe( GetValue(ADJ_AMPDAC_START + chan * ADJ_AMPDAC_OFFSET, ADJ_AMPDAC_LENGTH) ) );
}

//*********************************************************************************
int wgEditConfig::Get_trigadj(int chan){
  if( chan < 0 || chan >= NCHANNELS)
	throw std::invalid_argument("channel " + to_string(chan) + " is out of range");
  return stoi( BiToDe( GetValue(ADJ_THRESHOLD_START + chan * ADJ_THRESHOLD_OFFSET, ADJ_THRESHOLD_LENGTH ) ) );
}

//*********************************************************************************
int wgEditConfig::Get_trigth(){
  return std::stoi( BiToDe ( GetValue(GLOBAL_THRESHOLD_START, GLOBAL_THRESHOLD_LENGTH ) ) );
}

//*********************************************************************************
int wgEditConfig::Get_gainth(){
  return stoi( BiToDe( GetValue( GLOBAL_GS_THRESHOLD_START, GLOBAL_GS_THRESHOLD_LENGTH ) ) );
}

//*********************************************************************************
void wgEditConfig::Change_1bitparam(int value,int subadd){
  string num;
  num = Form("%d",value);
  num = DeToBi(num);
  num = Form("%d",stoi(num.c_str()));
  this->Modify(num,subadd);
}

//*********************************************************************************
int wgEditConfig::Get_1bitparam(int subadd){
  string num("");
  num = GetValue(subadd,1);
  num = BiToDe(num);
  return stoi(num.c_str());
}
