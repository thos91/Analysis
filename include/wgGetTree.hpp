#ifndef WAGASCI_GET_TREE_HPP_
#define WAGASCI_GET_TREE_HPP_ 

// system includes
#include <string>

// ROOT includes
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"

// user includes
#include "wgConst.hpp"

using namespace std;

class wgGetTree
{
public:
  TTree* tree_in;
  TTree* tree_out;
  TFile* finput;
  TFile* foutput;
  // The constructors just call wgGetTree::Open followed by
  // wgGetTree::SetTreeFile. The exception thrown are just those thrown by those
  // two functions (there is no exception handling in the constructors)
  wgGetTree(const string&, Raw_t&);
  wgGetTree(const string&, Hit_t&);
  wgGetTree(const string&, Recon_t&);
  wgGetTree(const string&, Track_t&);
  wgGetTree(const string&, IngRecon_t&);

  // The destructor just calls the wgGetTree::Close function
  ~wgGetTree();
  
  bool MakeTreeFile(const string&, Hit_t&);
  bool MakeTreeFile(const string&, Recon_t&);
  bool MakeTreeFile(const string&, Track_t&);
  void GetEntry(int);

  // The difference between Get...(something) and GetHist...(something) is that
  // in the former the center of the maximum bin is returned as a double while in
  // the latter the entire TH1D is returned.
  double GetStartTime();
  double GetStopTime();
  double GetDataPacket();
  double GetLostPacket();
  TH1D* GetHist_StartTime();
  TH1D* GetHist_StopTime();
  TH1D* GetHist_DataPacket();
  TH1D* GetHist_LostPacket();

protected:

  string finputname;

  string foutputname;

  // Open a ROOT file:
  // The string argument is a path to a valid ROOT file.
  // It is assumed that the ROOT file contains a TTree named "tree"
  // throw wgInvalidFile if the file is not found or it is not valid (if the
  // CheckExist::RootFile call has failed).
  // throw ROOT exceptions if the TFile or the TTree cannot be read.
  void Open(const string&);

  // Close a ROOT file. No exception is thrown.
  void Close();

  // The SetTreeFile reads a TTree into the class passed as a reference.
  // Before calling the SetTreeFile methods always call the wgGetTree::Open
  // method.
  //If the ROOT file was not opened a wgInvalidFile exception is thrown.
  //If there was an error in the reading of the TTree a wgElementNotFound
  //exception is thrown
  void SetTreeFile(Raw_t&);
  void SetTreeFile(Hit_t&);
  void SetTreeFile(Recon_t&);
  void SetTreeFile(Track_t&);
  void SetTreeFile(IngRecon_t&);

  // Check if a branch exists in the tree_in TTree. Return true if it exists and
  // false otherwise.
  bool BranchExists(const string&);
};

#endif // WAGASCI_GET_TREE_HPP_ 
