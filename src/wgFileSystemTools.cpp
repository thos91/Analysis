// system C++ includes
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>

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

BOOST_STATIC_ASSERT(BOOST_VERSION >= 106200);

namespace get_stats {

std::string extension(const std::string& path) {
  return filesys::path(path).extension().string();
}

std::string basename(const std::string& path) {
  return filesys::path(path).stem().string();
}

std::string dirname(const std::string& path) {
  return filesys::path(path).remove_filename().string();
}

std::string name_before_last_under_bar(const std::string& str)
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

}

///////////////////////////////////////////////////////////////////////////////
//                                    list                                   //
///////////////////////////////////////////////////////////////////////////////

namespace list {

//******************************************************************
bool sort_by_integer(std::string file1, std::string file2) {
  return string::extract_integer(get_stats::basename(file1)) <
    string::extract_integer(get_stats::basename(file2));
}

std::vector<std::string> list_directories(const std::string& input_dir, bool with_integer) {
  std::vector<std::string> directory_list;

  if (filesys::exists(input_dir)) {
    if (filesys::is_directory(input_dir)) {
      for (const filesys::directory_entry& entry : filesys::directory_iterator(input_dir)) {
        if (filesys::is_directory(entry)) {
          directory_list.push_back(entry.path().string());
          if (with_integer &&
              string::extract_integer(entry.path().stem().string()) == UINT_MAX) {
            directory_list.pop_back();
          }
        }
      }
    } else {
      throw wgInvalidFile(input_dir + " exists, but is not a regular directory");
    }
  } else {
    throw wgInvalidFile(input_dir + " does not exist");
  }

  if (directory_list.size() == 0)
    throw wgInvalidFile("Nothing meaningful found in " + input_dir);

  if (with_integer)
    std::sort(directory_list.begin(), directory_list.end(), sort_by_integer);

  return directory_list;
}

//******************************************************************
unsigned how_many_directories(const std::string& input_dir, bool with_integer) {
  unsigned counter = 0;
  
  if (filesys::exists(input_dir)) {
    if (filesys::is_directory(input_dir)) {
      for (const filesys::directory_entry& entry : filesys::directory_iterator(input_dir)) {
        if (filesys::is_directory(entry)) {
          ++counter;
          if (with_integer &&
              string::extract_integer(entry.path().stem().string()) == UINT_MAX) {
            --counter;
          }
        }
      }
    } else {
      throw wgInvalidFile(input_dir + " exists, but is not a regular directory");
    }
  } else {
    throw wgInvalidFile(input_dir + " does not exist");
  }

  return counter;
} 

//******************************************************************
std::vector<std::string> list_files(const std::string& input_dir, bool with_integer,
                                    const std::string& x_extension) {
  std::vector<std::string> file_list;
  std::string extension(x_extension);
  if (!extension.empty() && extension[0] != '.')
    extension.insert(0, ".");

  if (filesys::exists(input_dir)) {
    if (filesys::is_directory(input_dir)) {
      for (const filesys::directory_entry& entry : filesys::directory_iterator(input_dir)) {
        if (filesys::is_regular_file(entry.path())) {
          file_list.push_back(entry.path().string());
          if (with_integer &&
              string::extract_integer(file_list.back()) == UINT_MAX)
            file_list.pop_back();
          if (!extension.empty() && (!entry.path().has_extension() ||
                                     entry.path().extension().string() != extension))
            file_list.pop_back();  
        }
      }
    } else {
      throw wgInvalidFile(input_dir + " exists, but is not a regular directory");
    }
  } else {
    throw wgInvalidFile(input_dir + " does not exist");
  }

  if (file_list.size() == 0)
    throw wgInvalidFile("Nothing meaningful found in " + input_dir);

  if (with_integer)
    std::sort(file_list.begin(), file_list.end(), sort_by_integer);

  return file_list;   
}

//******************************************************************
unsigned how_many_files(const std::string& input_dir, bool with_integer,
                        const std::string& x_extension) {
  unsigned counter = 0;
  
  std::string extension(x_extension);
  if (!extension.empty() && extension[0] != '.')
    extension.insert(0, ".");

  if (filesys::exists(input_dir)) {
    if (filesys::is_directory(input_dir)) {
      for (const filesys::directory_entry& entry : filesys::directory_iterator(input_dir)) {
        if (filesys::is_regular_file(entry.path())) {
          ++counter;
          if (with_integer &&
              string::extract_integer(entry.path().string()) == UINT_MAX)
            --counter;
          if (!extension.empty() && (!entry.path().has_extension() ||
                                     entry.path().extension().string() != extension))
            --counter;
        }
      }
    } else {
      throw wgInvalidFile(input_dir + " exists, but is not a regular directory");
    }
  } else {
    throw wgInvalidFile(input_dir + " does not exist");
  }
  
  return counter;
}

}

///////////////////////////////////////////////////////////////////////////////
//                                    make                                   //
///////////////////////////////////////////////////////////////////////////////

namespace make {

//******************************************************************
void directory(const std::string& str) {
  if( !check_exist::directory(str) ) {
    filesys::path dir(str);
    if( !filesys::create_directories(dir) ) {
      throw wgInvalidFile("_failed to create directory " + str);
    }
  }
}

}

namespace string {

//**********************************************************************
unsigned extract_integer(const std::string& str) { 
  std::size_t const n = str.find_first_of("0123456789");
  if (n != std::string::npos)
  {
    std::size_t const m = str.find_first_not_of("0123456789", n);
    return std::stoi(str.substr(n, m != std::string::npos ? m-n : m));
  }
  return UINT_MAX;
}

//**********************************************************************
bool find_string_ic(const std::string & str_haystack, const std::string & str_needle)
{
  auto it = std::search(
      str_haystack.begin(), str_haystack.end(),
      str_needle.begin(),   str_needle.end(),
      [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
                        );
  return (it != str_haystack.end() );
}

//**********************************************************************
int max_depth(std::string str)
{ 
  int current_max = 0; // current count 
  int max = 0;    // overall maximum count 
  int n = str.length(); 
  
  // _traverse the input string 
  for (int i = 0; i< n; i++) { 
    if (str[i] == '{')  { 
      current_max++; 
  
      // update max if required 
      if (current_max> max) 
        max = current_max; 
    } 
    else if (str[i] == '}') { 
      if (current_max>0) 
        current_max--; 
      else
        return -1; 
    } 
  } 
  
  // finally check for unbalanced string 
  if (current_max != 0) 
    return -1; 
  
  return max; 
}

}

///////////////////////////////////////////////////////////////////////////////
//                                check_exist                                //
///////////////////////////////////////////////////////////////////////////////

namespace check_exist {

//**********************************************************************
bool generic_file(const std::string& filename, const std::string & ext)
{
  try {
    // check for correct extension
    if(get_stats::extension(filename) != ext)
      throw wgInvalidFile(filename + " has not ." + ext + " extension");
    // _create a _path object from given path string
    filesys::path path_obj(filename);
    // _check if path exists and is of a regular file
    if (filesys::exists(path_obj) && filesys::is_regular_file(path_obj))
      return true;
  }
  catch(const std::exception& e) {
    Log.Write("[wg_error_code][" + ext +"_file] " + std::string(e.what()));
    return false;
  }
  return false;
}

bool root_file(const TString& filename) {
  return root_file(std::string(filename.Data()));
}

bool root_file(const std::string& filename) {
  try {
    if (generic_file(filename, std::string("root")) == false)
      return false;
    // _check if the _r_o_o_t file is zombie
    TFile file(filename.c_str());
    if (file.IsZombie())
      throw wgInvalidFile(filename + " is zombie");
    // _check if the _r_o_o_t file was successfully recovered
    if (file.TestBit(TFile::kRecovered))
      Log.Write("[wg_error_code][_root_file] " + filename + " was successfully recovered");
    // _if everything is fine return true
    return true;
  }
  catch(const std::exception& e) {
    Log.Write("[wg_error_code][_root_file] " + std::string(e.what()));
    return false;
  }
}

bool raw_file(const std::string& filename)
{
  return generic_file(filename, std::string("raw"));
}

bool txt_file(const std::string& filename)
{
  return generic_file(filename, std::string("txt"));
}

bool csv_file(const std::string& filename)
{
  return generic_file(filename, std::string("csv"));
}

bool xml_file(const std::string& filename)
{
  return generic_file(filename, std::string("xml"));
}

bool log_file(const std::string& filename)
{
  return generic_file(filename, std::string("log"));
}

bool directory(const std::string& file_path)
{
  try {
    // create a path object from given path string
    filesys::path path_obj(file_path);
    // _check if path exists and is of a directory file
    if (filesys::exists(path_obj) && filesys::is_directory(path_obj))
      return true;
  }
  catch (filesys::filesystem_error &e) {
    Log.eWrite(e.what());
  }
  return false;
}

}  // check_exist

///////////////////////////////////////////////////////////////////////////////
//                                  datetime                                 //
///////////////////////////////////////////////////////////////////////////////

namespace datetime {

int datetime_to_seconds(const std::string & datetime) {
  std::tm t = {};
  std::istringstream ss(datetime);
  if (ss >> std::get_time(&t, "%Y/%m/%d %H:%M:%S"))
    return std::mktime(&t);
  else
    return -1;
}

} // datetime

} // namespace wagasci_tools

