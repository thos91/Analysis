#ifndef WGTOOLS_H_INCLUDE
#define WGTOOLS_H_INCLUDE

// system includes
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>

#include "TString.h"

namespace wagasci_tools {

// extract the extension from a path string
// GetExtension("/path/to/file.txt") -> "txt"
std::string GetExtension(const std::string& str);

// extract the top-level file or directory name without extension
// GetName("/path/to/file.txt") -> "file"
std::string GetName(const std::string& str);

// extract the lower-level path
// GetPath("/path/to/file.txt") -> "/path/to"
std::string GetPath(const std::string& str);

// extract the name before the last underscore
// GetPath("/path/to/my_file.txt") -> "my"
std::string GetNameBeforeLastUnderBar(const std::string& str);

// List all the files with extension "extension" present in the folder
// "inputDir". Returns the absolute path to the files as a vector of
// std::strings.
std::vector<std::string> ListFilesWithExtension(const std::string& inputDir,
                                                const std::string& extension = "");

// List all the directories in the folder "inputDir". Returns the
// absolute path to the directories as a vector of std::strings.
std::vector<std::string> ListDirectories(const std::string& inputDir);

// count all the files with extension "extension" present in the
// folder "inputDir"
unsigned HowManyFilesWithExtension(const std::string& inputDir,
                                   const std::string& extension);

// count all the directories present in the folder "inputDir"
unsigned HowManyDirectories(const std::string& inputDir);

// Create the directory whose absolute path is "str"
void MakeDir(const std::string& str);

// extract the first unsigned integer from a std::string. If not found
// returns UINT_MAX
unsigned extractIntegerFromString(const std::string& str);

// Try to find in the Haystack the Needle - ignore case
bool findStringIC(const std::string & strHaystack, const std::string & strNeedle);

// Helper function that takes a string and returns the maximum depth
// nested parenthesis
int maxDepth(std::string str);

namespace check_exist {

// return true if the file exists, otherwise false

// All the methods of the CheckExist class check for existance and validity of
// the passed file path. They should not throw any exception (all the exceptions
// are handled and reported internally).

bool GenericFile(const std::string& filename, const std::string & ext);
bool RootFile   (const std::string& filename);   //check the existence of root file.
bool RootFile   (const     TString& filename);   //check the existence of root file.
bool RawFile    (const std::string& filename);   //check the existence of raw file.
bool TxtFile    (const std::string& filename);   //check the existence of txt file.
bool CsvFile    (const std::string& filename);   //check the existence of csv file.
bool XmlFile    (const std::string& filename);   //check the existence of xml file.
bool LogFile    (const std::string& filename);   //check the existence of log file.
bool Dir        (const std::string& filename);   //check the existence of dir.

}  // check_exist

namespace datetime {
// convert localtime datetime string formated as "%Y/%m/%d %H:%M:%S"
// into seconds elapsed from epoch. If the string could not be parse
// -1 is returned.
int DatetimeToSeconds(const std::string & datetime);

} // datetime

} // wagasci_tools
#endif
