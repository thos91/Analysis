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
  vector<string> ListDirectories(const string& inputDir);
  unsigned HowManyFilesWithExtension(const string& inputDir, const string& extension);
  unsigned HowManyDirectories(const string& inputDir);
  void MakeDir(const string& str);

  // extract the first unsigned integer from a string. If not found
  // returns UINT_MAX
  unsigned extractIntegerFromString(const string& str);

}
#endif
