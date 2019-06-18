#ifndef WGTOOLS_H_INCLUDE
#define WGTOOLS_H_INCLUDE

// system includes
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

namespace wagasci_tools {

  string GetExtension(const string& str);
  string GetName(const string& str);
  string GetPath(const string& str);
  string GetNameBeforeLastUnderBar(const string& str);
  vector<string> ListFilesWithExtension(const string& inputDir,
                                        const string& extension = "");
  void MakeDir(const string& str);

}
#endif
