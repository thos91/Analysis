#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "Const.h"
#include "TH1F.h"
#include <string>

using namespace std;

class wgGetTree
{
public:
  Raw_t rd;
  static TTree* tree;
  static TFile* finput;    
  static string finputname;
  static TTree* ingtree;
  static TFile* ingfinput;    
  static string ingfinputname;
  static TTree* tree_out;
  static TFile* foutput;
  static string foutputname;
  wgGetTree();
  wgGetTree(string&);
  ~wgGetTree();
  
  bool Open(string&);
  void Close();
  
  wgGetTree(string&,Raw_t&);
  wgGetTree(string&,Hit_t&);
  wgGetTree(string&,Recon_t&);
  wgGetTree(string&,Track_t&);
  wgGetTree(string&,IngRecon_t&);

  bool SetTreeFile(Raw_t&); 
  bool SetTreeFile(Hit_t&);  
  bool SetTreeFile(Recon_t&);
  bool SetTreeFile(Track_t&);
  bool SetTreeFile(IngRecon_t&);

  bool MakeTreeFile(string&,Hit_t&);  
  bool MakeTreeFile(string&,Recon_t&);
  bool MakeTreeFile(string&,Track_t&);
  void GetEntry(int);

  double GetStartTime();
  double GetStopTime();
  double GetDataPacket();
  double GetLostPacket();
  TH1F*  GetHist_StartTime();
  TH1F*  GetHist_StopTime();
  TH1F*  GetHist_DataPacket();
  TH1F*  GetHist_LostPacket();
  
};
