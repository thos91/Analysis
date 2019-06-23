// system C++ includes
#include <string>

// system C includes
#include <dirent.h>

// boost includes
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>

// user includes
#include "wgErrorCode.hpp"
#include "wgExceptions.hpp"
#include "wgConst.hpp"
#include "wgFileSystemTools.hpp"

using namespace std;

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
          if (GetExtension(entry.path().string()) == extension || extension.empty())
            file_list.push_back(entry.path().string());
      }
      else throw wgInvalidFile(inputDir + " exists, but is not a regular file or directory");
    }
    else throw wgInvalidFile(inputDir + " does not exist");

    return file_list;   
  }

  void MakeDir(const string& str) {
    CheckExist Check;
    if( !Check.Dir(str) ) {
      boost::filesystem::path dir(str);
      if( !boost::filesystem::create_directories(dir) ) {
        throw wgInvalidFile("Failed to create directory " + str);
      }
    }
  }

} // namespace wagasci_tools
