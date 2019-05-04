#include "wgErrorCode.h"
#include "wgExceptions.h"
#include "wgTools.h"
#include "TFile.h"
#include "Const.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>

bool CheckExist::GenericFile(const string& filename, const string & ext)
{
  OperateString OpStr;
  struct stat st; 
  try {
	// Check if the file exists
    if(stat(filename.c_str(),&st) != 0)
	  throw wgInvalidFile(filename + " not found");
	// Check if the file is a directory
    mode_t m = st.st_mode;
    if(S_ISDIR(m))
	  throw wgInvalidFile(filename + " is a directory and not a file");
	// Check for the correct extension
    if(OpStr.GetExtension(filename) != ext)
	  throw wgInvalidFile(filename + " has not ." + ext + " extension");
	// If everything is fine return true
    return true;
  }
  catch(const exception& e) {
	Log.Write("[wgErrorCode][CheckExist::" + ext +"File] " + string(e.what()));
    return false;
  }
  catch(int e) {
	Log.Write("[wgErrorCode][CheckExist::" + ext + "File] error code = " + to_string(e));
    return false;
  }
  catch(...) {
    return false;
  }
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
  catch(int e) {
	Log.Write("[wgErrorCode][CheckExist::RootFile] caught error number " + e);
    return false;
  }
  catch(...) {
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

bool CheckExist::XmlFile(const string& filename)
{
 return this->GenericFile(filename, string("xml"));
}

bool CheckExist::LogFile(const string& filename)
{
 return this->GenericFile(filename, string("log"));
}

bool CheckExist::Dir(const string& dirname)
{
  struct stat st; 
  if(stat(dirname.c_str(),&st) != 0){ 
    return false;
  }else{
    mode_t m = st.st_mode;
    if(S_ISDIR(m)){
      return true;
    }else{
      return false;
    }   
  }
}

