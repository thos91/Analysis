#include "wgTools.h"
#include "TROOT.h"
#include "wgErrorCode.h"
#include "wgExceptions.h"
#include "Const.h"

#include <cerrno>
#include <time.h>
#include <fstream>
#include <iostream>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

string OperateString::GetExtension(string& str)
{
  OperateString::str = str;
  size_t pos1 = str.rfind('.');
  if(pos1 !=string::npos){
    OperateString::ext = str.substr(pos1+1, str.size()-pos1);
    string::iterator itr = ext.begin();
    while(itr != ext.end()){
      *itr=tolower(*itr);
      itr++;  
    }
    itr = ext.end()-1;
    while(itr != ext.begin()){
      if(*itr==0 || *itr == 32){
        ext.erase(itr--);
      }else{
        itr--;
      }
    }
  }
  return ext;
}

string OperateString::GetName(string& str)
{
  string fn;
  string::size_type fpos;
  if( (fpos = str.find_last_of("/")) == str.size()-1){
    str = str.substr(0,str.size()-1);
  }

  if((fpos = str.find_last_of("/")) != string::npos){
    fn = str.substr(fpos+1);
  }else{
    fn = str;
  }
  if((fpos = fn.find_last_of(".")) != string::npos){
    if(fpos>1){
      fn = fn.substr(0,fpos);
    }
  }
  return fn;
}

string OperateString::GetPath(string& str)
{
  size_t pos1;
  pos1 = str.rfind("/");
  if(pos1 != string::npos){
    return str.substr(0,pos1+1);
  }
  return "";
}

string OperateString::GetNameBeforeLastUnderBar(string& str)
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

//==================== Logger Class ====================//

string Logger::m_fileName = "";
string Logger::m_efileName = "";
ofstream Logger::m_file;
ofstream Logger::m_efile;

void Logger::Initialize() : Initialize("") {}
 
void Logger::Initialize(string& log_dir)
{
  time_t newTime = time( NULL );
  struct tm *t_st = localtime( &newTime );

  // If log_dir is empty try to infer it from the environment
  if ( log_dir.empty() ) {
	wgConst *con = new wgConst();
	con->GetENV();
	log_dir = con->LOG_DIRECTORY;
	delete con;
  }
  
  // If the log directory is not found, create it
  struct stat sb;
  if (stat(log_dir, &sb) != 0 || !S_ISDIR(sb.st_mode)) {
	const int dir_err = mkdir(log_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if (dir_err == -1) {
	  string error_message("error while creating " + log_dir + " directory!");
	  throw wgInvalidFile(error_message);
	}
  }
 
  Logger::m_fileName  = Form("%s/%d_%d_%d.txt",      log_dir, t_st->tm_year + 1900, t_st->tm_mon + 1, t_st->tm_mday); 
  Logger::m_efileName = Form("%s/e_log%d_%d_%d.txt", log_dir, t_st->tm_year + 1900, t_st->tm_mon + 1, t_st->tm_mday);

  m_file.open(m_fileName, ofstream::out | ofstream::app);
  // After this attempt to open a file, we can safely use strerror() only  
  // in case file.is_open() returns False.
  if (!m_file.is_open()) {
	stringstream error_message;
	error_message << "error while opening file " << m_fileName << " (" << strerror(errno) << ")";
	throw wgInvalidFile(error_message.str());
  }
  m_efile.open(m_efileName, ofstream::out | ofstream::app);
  if (!m_efile.is_open()) {
	stringstream error_message;
	error_message << "error while opening file " << m_efileName << " (" << strerror(errno) << ")";
	throw wgInvalidFile(error_message.str());
  }
}

void Logger::Write(const string& log)
{
  time_t newTime = time(NULL);
  string japtime(ctime(&newTime));
  m_file << "[ " << japtime << " ]: " << log << endl;
}

void Logger::eWrite(const string& log)
{
  time_t newTime = time(NULL);
  string japtime(ctime(&newTime));
  m_efile << "[ " << japtime << " ]: " << log << endl;
}

Logger::~Logger()
{
  m_file.close();
  m_efile.close();
}

//==================== end Logger Class ====================//

