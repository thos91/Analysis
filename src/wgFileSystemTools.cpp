// system C++ includes
#include <string>
#include <sstream>
#include <algorithm>

// system C includes
#include <dirent.h>

// boost includes
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/lambda/bind.hpp>

// ROOT includes
#include "TFile.h"
#include "TString.h"

// user includes
#include "wgExceptions.hpp"
#include "wgConst.hpp"
#include "wgFileSystemTools.hpp"
#include "wgLogger.hpp"

namespace filesys = boost::filesystem;

namespace wagasci_tools {

string GetExtension(const string& str)
{
  string ext;
  size_t pos1 = str.rfind('.');
  if(pos1 !=string::npos){
    ext = str.substr(pos1+1, str.size()-pos1);
    string::iterator itr = ext.begin();
    while(itr != ext.end()){
      *itr=tolower(*itr);
      itr++;  
    }
    itr = ext.end()-1;
    while(itr != ext.begin()){
      if(*itr == 0 || *itr == 32){
        ext.erase(itr--);
      }else{
        itr--;
      }
    }
  }
  return ext;
}

string GetName(const string& str)
{
  string fn;
  string tmp = str;
  string::size_type fpos;
  if( (fpos = tmp.find_last_of("/")) == tmp.size()-1){
    tmp = tmp.substr(0,tmp.size()-1);
  }

  if((fpos = tmp.find_last_of("/")) != string::npos){
    fn = tmp.substr(fpos+1);
  }else{
    fn = tmp;
  }
  if((fpos = fn.find_last_of(".")) != string::npos){
    if(fpos>1){
      fn = fn.substr(0,fpos);
    }
  }
  return fn;
}

string GetPath(const string& str)
{
  size_t pos1;
  pos1 = str.rfind("/");
  if(pos1 != string::npos){
    return str.substr(0,pos1+1);
  }
  return "";
}

string GetNameBeforeLastUnderBar(const string& str)
{
  string fn;
  string::size_type fpos;
  if((fpos = str.find_last_of("/")) != string::npos){
    fn = str.substr(fpos+1);
  }else{
    fn = str;
  }
  if((fpos = fn.find_last_of(".")) != string::npos){
    fn = fn.substr(0,fpos);
  }
  if((fpos = fn.find_last_of("_")) != string::npos){
    fn = fn.substr(0,fpos);
  }
  return fn;
}

//******************************************************************
vector<string> ListFilesWithExtension(const string& inputDir, const string& extension) {
  vector<string> file_list;

  if (boost::filesystem::exists(inputDir)) {
    if (boost::filesystem::is_directory(inputDir)) {
      for (const boost::filesystem::directory_entry& entry : boost::filesystem::directory_iterator(inputDir))
        if (GetExtension(entry.path().string()) == extension || extension.empty()) {
          file_list.push_back(entry.path().string());
        }
    } else {
      throw wgInvalidFile(inputDir + " exists, but is not a regular directory");
    }
  } else {
    throw wgInvalidFile(inputDir + " does not exist");
  }
  return file_list;   
}
  
//******************************************************************
vector<string> ListDirectories(const string& inputDir) {
  vector<string> directory_list;

  if (boost::filesystem::exists(inputDir)) {
    if (boost::filesystem::is_directory(inputDir)) {
      for (const boost::filesystem::directory_entry& entry : boost::filesystem::directory_iterator(inputDir))
        if (boost::filesystem::is_directory(entry))
          directory_list.push_back(entry.path().string());
    } else {
      throw wgInvalidFile(inputDir + " exists, but is not a regular directory");
    }
  } else {
    throw wgInvalidFile(inputDir + " does not exist");
  }
  return directory_list;
}
  
//******************************************************************
unsigned HowManyFilesWithExtension(const string& inputDir, const string& extension) {
  unsigned counter = 0;

  if (boost::filesystem::exists(inputDir)) {
    if (boost::filesystem::is_directory(inputDir)) {
      for (const boost::filesystem::directory_entry& entry : boost::filesystem::directory_iterator(inputDir))
        if (GetExtension(entry.path().string()) == extension || extension.empty())
          counter++;
    }
    else throw wgInvalidFile(inputDir + " exists, but is not a regular directory");
  }
  else throw wgInvalidFile(inputDir + " does not exist");

  return counter;
}

//******************************************************************
unsigned HowManyDirectories(const string& inputDir) {
  boost::filesystem::path the_path(inputDir);
  unsigned counter = std::count_if( boost::filesystem::directory_iterator(the_path),
                                    boost::filesystem::directory_iterator(),
                                    static_cast<bool(*)(const boost::filesystem::path&)>(boost::filesystem::is_directory) );
  return counter;
} 
  
//******************************************************************
void MakeDir(const string& str) {
  if( !check_exist::Dir(str) ) {
    boost::filesystem::path dir(str);
    if( !boost::filesystem::create_directories(dir) ) {
      throw wgInvalidFile("Failed to create directory " + str);
    }
  }
}

//**********************************************************************
unsigned extractIntegerFromString(const string& str) { 
  std::size_t const n = str.find_first_of("0123456789");
  if (n != std::string::npos)
  {
    std::size_t const m = str.find_first_not_of("0123456789", n);
    return stoi(str.substr(n, m != std::string::npos ? m-n : m));
  }
  return UINT_MAX;
}

namespace check_exist {



//**********************************************************************
bool GenericFile(const string& filename, const string & ext)
{
  try {
    // Check for correct extension
    if(GetExtension(filename) != ext)
      throw wgInvalidFile(filename + " has not ." + ext + " extension");
    // Create a Path object from given path string
    filesys::path pathObj(filename);
    // Check if path exists and is of a regular file
    if (filesys::exists(pathObj) && filesys::is_regular_file(pathObj))
      return true;
  }
  catch(const exception& e) {
    Log.Write("[wgErrorCode][" + ext +"File] " + string(e.what()));
    return false;
  }
  return false;
}

bool RootFile(const TString& filename) {
  return RootFile(string(filename.Data()));
}

bool RootFile(const string& filename)
{
  try {
    if (GenericFile(filename, string("root")) == false)
      return false;
    // Check if the ROOT file is zombie
    TFile file(filename.c_str());
    if (file.IsZombie())
      throw wgInvalidFile(filename + " is zombie");
    // Check if the ROOT file was successfully recovered
    if (file.TestBit(TFile::kRecovered))
      Log.Write("[wgErrorCode][RootFile] " + filename + " was successfully recovered");
    // If everything is fine return true
    return true;
  }
  catch(const exception& e) {
    Log.Write("[wgErrorCode][RootFile] " + string(e.what()));
    return false;
  }
}

bool RawFile(const string& filename)
{
  return GenericFile(filename, string("raw"));
}

bool TxtFile(const string& filename)
{
  return GenericFile(filename, string("txt"));
}

bool CsvFile(const string& filename)
{
  return GenericFile(filename, string("csv"));
}

bool XmlFile(const string& filename)
{
  return GenericFile(filename, string("xml"));
}

bool LogFile(const string& filename)
{
  return GenericFile(filename, string("log"));
}

bool Dir(const string& filePath)
{
  try {
    // Create a Path object from given path string
    filesys::path pathObj(filePath);
    // Check if path exists and is of a directory file
    if (filesys::exists(pathObj) && filesys::is_directory(pathObj))
      return true;
  }
  catch (filesys::filesystem_error & e) {
    Log.eWrite(e.what());
  }
  return false;
}

}  // check_exist

} // namespace wagasci_tools

