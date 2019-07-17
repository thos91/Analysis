#ifndef WAGASCI_GET_TREE_HPP_
#define WAGASCI_GET_TREE_HPP_ 

// system includes
#include <string>

// ROOT includes
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1I.h"

// user includes
#include "wgConst.hpp"
#include "wgRawData.hpp"

class wgGetTree
{
public:
  TTree* tree;
  TFile* finput;

  // The constructors just call wgGetTree::Open followed by
  // wgGetTree::SetTreeFile. The exception thrown are just those
  // thrown by those two methods (there is no exception handling in
  // the constructors)
  wgGetTree(const std::string&, Raw_t&);

  // The destructor just calls the wgGetTree::Close function
  ~wgGetTree();

  // Get one event from the TTree and store in the Raw_t rd object
  void GetEntry(int event);

  // The difference between Get...(something) and
  // GetHist...(something) is that in the former the center of the
  // maximum bin is returned as a double while in the latter the
  // entire TH1I is returned.
  double GetStartTime();      // "start_time"
  double GetStopTime();       // "stop_time"
  double GetDataPacket();     // "nb_data_pkts"
  double GetLostPacket();     // "nb_lost_pkts"
  TH1I* GetHist_StartTime();
  TH1I* GetHist_StopTime();
  TH1I* GetHist_DataPacket();
  TH1I* GetHist_LostPacket();

protected:
  std::string m_finputname;

  // Open a ROOT file containing a TTree named "tree":
  // The string argument is a path to a valid ROOT file.
  // It is assumed that the ROOT file contains a TTree named "tree".
  // It throws wgInvalidFile if the file is not found or it is not
  // valid (if the CheckExist::RootFile call has failed). It throws
  // specific ROOT exceptions if the TFile or the TTree cannot be
  // read.
  void Open();

  // Close the ROOT file. No exception is thrown.
  void Close();

  // The SetTreeFile reads a TTree into the Raw_t object passed as a
  // reference.
  // If the ROOT file was not opened a wgInvalidFile exception is thrown.
  // If there was an error in the reading of the TTree a wgElementNotFound
  // exception is thrown
  void SetTreeFile(Raw_t&);
  
  // Check if a branch exists in the tree_in TTree. Return true if it
  // exists and false otherwise.
  bool BranchExists(const std::string&);
};

#endif // WAGASCI_GET_TREE_HPP_ 
