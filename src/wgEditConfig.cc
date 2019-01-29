#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <math.h>
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"

#include "Const.h"
#include "wgEditConfig.h"

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
  char* map_csv=Form("%s/src/spiroc2d.csv",con->MAIN_DIRECTORY);
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
  const char* mppc_csv= Form("%s/config/spiroc2d/mppc_map.csv",con->CALICOE_DIRECTORY);
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

  TFile *fmppc = new TFile( Form("%s/config/spiroc2d/arraymppc_data.root",con->CALICOE_DIRECTORY),"read");
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
void wgEditConfig::Open(string& input){
  string str;
  ifstream ifs(input.c_str());
  getline(ifs,str);
  if(str.size()!= 300){printf("[wgEditConfig::Open] Error! Bad word number!!"); return;}
  wgEditConfig::hex_config = str.substr(2,298);
  wgEditConfig::bi_config = this->HexToBi(wgEditConfig::hex_config);
}

//*********************************************************************************
void wgEditConfig::SetBitstream(string& input){
  if(input.size()!= 300-1){printf("[wgEditConfig::Open] Error! Bad word number!!"); return;}
  wgEditConfig::hex_config = input.substr(2,297) + "0";
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
string wgEditConfig::GetValue(int ini,int length){
  if(ini+length>1192){return NULL;}
  return wgEditConfig::bi_config.substr(ini+6,length);
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
    length = atoi(csv[i*5+1].c_str());
    ini = atoi(csv[i*5+3].c_str()) ;
    if(length%36==0){
      int chbit = length/36;
      cout << name << " / ";
      for(int ch=0;ch<36;ch++){
        value = this->GetValue(ini+ch*chbit,chbit);
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
string wgEditConfig::HexToBi(string& input){
  string output("");
  for(unsigned int i=0;i<input.size();i++){
    string word = input.substr(i,1); 
    if(word=="0") output = "0000" + output;
    if(word=="1") output = "1000" + output;
    if(word=="2") output = "0100" + output;
    if(word=="3") output = "1100" + output;
    if(word=="4") output = "0010" + output;
    if(word=="5") output = "1010" + output;
    if(word=="6") output = "0110" + output;
    if(word=="7") output = "1110" + output;
    if(word=="8") output = "0001" + output;
    if(word=="9") output = "1001" + output;
    if(word=="A"||word=="a") output = "0101" + output;
    if(word=="B"||word=="b") output = "1101" + output;
    if(word=="C"||word=="c") output = "0011" + output;
    if(word=="D"||word=="d") output = "1011" + output;
    if(word=="E"||word=="e") output = "0111" + output;
    if(word=="F"||word=="f") output = "1111" + output;
  } 
  return output;
}

//*********************************************************************************
string wgEditConfig::BiToHex(string& input){
  string output("");
  if(input.size()%4!=0) return output;
  for(unsigned int i=input.size();i>3;i=i-4){
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
string wgEditConfig::BiToDe(string& input){
  string output("");
  string word;
  int decimal=0;
  
  for(unsigned int i=0; i<input.size(); i++){
    word = input.substr(input.size()-1-i,1);
    decimal += atoi(word.c_str()) * pow(2,i);
  } 
  output=Form("%d",decimal);
  return output;
}

//*********************************************************************************
string wgEditConfig::DeToBi(string& input){
  string output("");
  char num[1];
  int decimal=atoi(input.c_str());
 
  while(decimal>0){
    sprintf(num,"%d",decimal%2);
    output = num + output; 
    decimal = decimal / 2;
  }
  return output;
}

//*********************************************************************************
void wgEditConfig::Change_inputDAC(int ch,int value,int mode){
  if(ch <0 || ch>36){printf("WARNING! ch is out of range!\n"); return;}
  string num;

  if(mode==1 && this->Read_MPPCData){
    if(value+fine_inputDAC[ch]>255){
      printf("WARNING! value is out of range! set 0\n");
      num = Form("%d",255);
    }else{
      num = Form("%d",value+this->fine_inputDAC[ch]);
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
  num = Form("%08d1",atoi(num.c_str()));
  if(ch==36){
    for(int ich=0;ich<36;ich++){
      this->Modify(num,37+ich*9);
    }
  }else{
    this->Modify(num,37+ch*9);
  }
}

//*********************************************************************************
void wgEditConfig::Change_ampDAC(int ch,int value){
  if(value <0 || value>63){printf("WARNING! value is out of range!\n"); return;}
  if(ch <0 || ch>36){printf("WARNING! ch is out of range!\n"); return;}
  string num;
  num = Form("%d",value);
  num = DeToBi(num);
  num = Form("%06d%06d000",atoi(num.c_str()),atoi(num.c_str()));
  if(ch==36){
    for(int ich=0;ich<36;ich++){
      this->Modify(num,367+ich*15);
    }
  }else{
    this->Modify(num,367+ch*15);
  }
}

//*********************************************************************************
void wgEditConfig::Change_trigadj(int ch,int value){
  if(value <0 || value>15){printf("WARNING! value is out of range!\n"); return;}
  if(ch <0 || ch>36){printf("WARNING! ch is out of range!\n"); return;}
  string num;
  num = Form("%d",value);
  num = DeToBi(num);
  num = Form("%04d",atoi(num.c_str()));
  if(ch==36){
    for(int ich=0;ich<36;ich++){
      this->Modify(num,1006+ich*4);
    }
  }else{
    this->Modify(num,1006+ch*4);
  }
}

//*********************************************************************************
void wgEditConfig::Change_trigth(int value){
  if(value <0 || value>1023){printf("WARNING! value is out of range!\n"); return;}
  string num;
  num = Form("%d",value);
  num = DeToBi(num);
  num = Form("%010d",atoi(num.c_str()));
  this->Modify(num,931);
}

//*********************************************************************************
void wgEditConfig::Change_gainth(int value){
  if(value <0 || value>1023){printf("WARNING! value is out of range!\n"); return;}
  string num;
  num = Form("%d",value);
  num = DeToBi(num);
  num = Form("%010d",atoi(num.c_str()));
  this->Modify(num,941);
}

//*********************************************************************************
int wgEditConfig::Get_inputDAC(int ch){
  if(ch <0 || ch>35){printf("WARNING! ch is out of range!\n"); return 0;}
  string num("");
  num = GetValue(37+ch*9,8);
  num = BiToDe(num);
  return atoi(num.c_str());
}

//*********************************************************************************
int wgEditConfig::Get_ampDAC(int ch){
  if(ch <0 || ch>35){printf("WARNING! ch is out of range!\n"); return 0;}
  string num("");
  num = GetValue(367+ch*15,6);
  num = BiToDe(num);
  return atoi(num.c_str());
}

//*********************************************************************************
int wgEditConfig::Get_trigadj(int ch){
  if(ch <0 || ch>35){printf("WARNING! ch is out of range!\n"); return 0;}
  string num("");
  num = GetValue(1006+ch*4,4);
  num = BiToDe(num);
  return atoi(num.c_str());
}

//*********************************************************************************
int wgEditConfig::Get_trigth(){
  string num("");
  num = GetValue(931,10);
  num = BiToDe(num);
  return atoi(num.c_str());
}

//*********************************************************************************
int wgEditConfig::Get_gainth(){
  string num("");
  num = GetValue(941,10);
  num = BiToDe(num);
  return atoi(num.c_str());
}

//*********************************************************************************
void wgEditConfig::Change_1bitparam(int value,int subadd){
  string num;
  num = Form("%d",value);
  num = DeToBi(num);
  num = Form("%d",atoi(num.c_str()));
  this->Modify(num,subadd);
}

//*********************************************************************************
int wgEditConfig::Get_1bitparam(int subadd){
  string num("");
  num = GetValue(subadd,1);
  num = BiToDe(num);
  return atoi(num.c_str());
}
