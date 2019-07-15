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

std::string GetExtension(const std::string& str)
{
  std::string ext;
  size_t pos1 = str.rfind('.');
  if(pos1 !=std::string::npos){
    ext = str.substr(pos1+1, str.size()-pos1);
    std::string::iterator itr = ext.begin();
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

std::string GetName(const std::string& str)
{
  std::string fn;
  std::string tmp = str;
  std::string::size_type fpos;
  if( (fpos = tmp.find_last_of("/")) == tmp.size()-1){
    tmp = tmp.substr(0,tmp.size()-1);
  }

  if((fpos = tmp.find_last_of("/")) != std::string::npos){
    fn = tmp.substr(fpos+1);
  }else{
    fn = tmp;
  }
  if((fpos = fn.find_last_of(".")) != std::string::npos){
    if(fpos>1){
      fn = fn.substr(0,fpos);
    }
  }
  return fn;
}

std::string GetPath(const std::string& str)
{
  size_t pos1;
  pos1 = str.rfind("/");
  if(pos1 != std::string::npos){
    return str.substr(0,pos1+1);
  }
  return "";
}

std::string GetNameBeforeLastUnderBar(const std::string& str)
{
  std::string fn;
  std::string::size_type fpos;
  if((fpos = str.find_last_of("/")) != std::string::npos){
    fn = str.substr(fpos+1);
  }else{
    fn = str;
  }
  if((fpos = fn.find_last_of(".")) != std::string::npos){
    fn = fn.substr(0,fpos);
  }
  if((fpos = fn.find_last_of("_")) != std::string::npos){
    fn = fn.substr(0,fpos);
  }
  return fn;
}

//******************************************************************
std::vector<std::string> ListFilesWithExtension(const std::string& inputDir, const std::string& extension) {
  std::vector<std::string> file_list;

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
std::vector<std::string> ListDirectories(const std::string& inputDir) {
  std::vector<std::string> directory_list;

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
unsigned HowManyFilesWithExtension(const std::string& inputDir, const std::string& extension) {
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
unsigned HowManyDirectories(const std::string& inputDir) {
  boost::filesystem::path the_path(inputDir);
  unsigned counter = std::count_if( boost::filesystem::directory_iterator(the_path),
                                    boost::filesystem::directory_iterator(),
                                    static_cast<bool(*)(const boost::filesystem::path&)>(boost::filesystem::is_directory) );
  return counter;
} 
  
//******************************************************************
void MakeDir(const std::string& str) {
  if( !check_exist::Dir(str) ) {
    boost::filesystem::path dir(str);
    if( !boost::filesystem::create_directories(dir) ) {
      throw wgInvalidFile("Failed to create directory " + str);
    }
  }
}

//**********************************************************************
unsigned extractIntegerFromString(const std::string& str) { 
  std::size_t const n = str.find_first_of("0123456789");
  if (n != std::string::npos)
  {
    std::size_t const m = str.find_first_not_of("0123456789", n);
    return stoi(str.substr(n, m != std::string::npos ? m-n : m));
  }
  return UINT_MAX;
}

//**********************************************************************
bool findStringIC(const std::string & strHaystack, const std::string & strNeedle)
{
  auto it = std::search(
    strHaystack.begin(), strHaystack.end(),
    strNeedle.begin(),   strNeedle.end(),
    [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
  );
  return (it != strHaystack.end() );
}

namespace check_exist {

//**********************************************************************
bool GenericFile(const std::string& filename, const std::string & ext)
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
  catch(const std::exception& e) {
    Log.Write("[wgErrorCode][" + ext +"File] " + std::string(e.what()));
    return false;
  }
  return false;
}

bool RootFile(const TString& filename) {
  return RootFile(std::string(filename.Data()));
}

bool RootFile(const std::string& filename)
{
  try {
    if (GenericFile(filename, std::string("root")) == false)
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
  catch(const std::exception& e) {
    Log.Write("[wgErrorCode][RootFile] " + std::string(e.what()));
    return false;
  }
}

bool RawFile(const std::string& filename)
{
  return GenericFile(filename, std::string("raw"));
}

bool TxtFile(const std::string& filename)
{
  return GenericFile(filename, std::string("txt"));
}

bool CsvFile(const std::string& filename)
{
  return GenericFile(filename, std::string("csv"));
}

bool XmlFile(const std::string& filename)
{
  return GenericFile(filename, std::string("xml"));
}

bool LogFile(const std::string& filename)
{
  return GenericFile(filename, std::string("log"));
}

bool Dir(const std::string& filePath)
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

