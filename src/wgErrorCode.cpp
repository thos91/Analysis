// system includes
#include <string>
#include <sstream>

// boost includes
#include <boost/filesystem.hpp>

// ROOT includes
#include "TFile.h"

// user includes
#include "wgErrorCode.h"
#include "wgExceptions.h"
#include "wgTools.h"
#include "Const.h"

namespace filesys = boost::filesystem;

bool CheckExist::GenericFile(const string& filename, const string & ext)
{
  OperateString OpStr;

  try {
	// Check for correct extension
    if(OpStr.GetExtension(filename) != ext)
	  throw wgInvalidFile(filename + " has not ." + ext + " extension");
	// Create a Path object from given path string
	filesys::path pathObj(filename);
	// Check if path exists and is of a regular file
	if (filesys::exists(pathObj) && filesys::is_regular_file(pathObj))
	  return true;
  }
  catch(const exception& e) {
	Log.Write("[wgErrorCode][CheckExist::" + ext +"File] " + string(e.what()));
    return false;
  }
  return false;
}

bool CheckExist::RootFile(const string& filename)
{
  try {
	if (this->GenericFile(filename, string("root")) == false)
	  return false;
	// Check if the ROOT file is zombie
	TFile file(filename.c_str());
	if (file.IsZombie())
	  throw wgInvalidFile(filename + " is zombie");
	// Check if the ROOT file was successfully recovered
	if (file.TestBit(TFile::kRecovered))
	  Log.Write("[wgErrorCode][CheckExist::RootFile] " + filename + " was successfully recovered");
	// If everything is fine return true
    return true;
  }
  catch(const exception& e) {
	Log.Write("[wgErrorCode][CheckExist::RootFile] " + string(e.what()));
    return false;
  }
}

bool CheckExist::RawFile(const string& filename)
{
  return this->GenericFile(filename, string("raw"));
}

bool CheckExist::TxtFile(const string& filename)
{
  return this->GenericFile(filename, string("txt"));
}

bool CheckExist::CsvFile(const string& filename)
{
 return this->GenericFile(filename, string("csv"));
}

bool CheckExist::XmlFile(const string& filename)
{
 return this->GenericFile(filename, string("xml"));
}

bool CheckExist::LogFile(const string& filename)
{
 return this->GenericFile(filename, string("log"));
}

bool CheckExist::Dir(const string& filePath)
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

