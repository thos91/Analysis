#include "wgTools.h"
#include "TROOT.h"
#include "wgErrorCode.h"
#include "wgExceptions.h"
#include "Const.h"

#include <iostream>
#include <chrono>
#include <iomanip>
#include <cerrno>
#include <fstream>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

string OperateString::GetExtension(const string& str)
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

string OperateString::GetName(const string& str)
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

string OperateString::GetPath(const string& str)
{
  size_t pos1;
  pos1 = str.rfind("/");
  if(pos1 != string::npos){
    return str.substr(0,pos1+1);
  }
  return "";
}

string OperateString::GetNameBeforeLastUnderBar(const string& str)
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

Logger Log;

Logger::Logger() : Logger(string("")) {}

Logger::Logger(const string& log_dir)
{
  try {
	time_t newTime = time( NULL );
	struct tm *t_st = localtime( &newTime );
	string dir;
	// If log_dir is empty try to infer it from the environment
	if ( log_dir.empty() ) {
	  wgConst con;
	  con.GetENV();
	  dir = con.LOG_DIRECTORY;
	} else dir = log_dir;
  
	// If the log directory is not found, create it
	CheckExist check;
	if ( check.Dir(dir) == false ) {
	  const int dir_err = mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	  if (dir_err == -1) {
		string error_message("error while creating " + dir + " directory!");
		throw wgInvalidFile(error_message);
	  }
	}

	m_fileName  = dir + "/log" + to_string(t_st->tm_year + 1900) + "_" + to_string(t_st->tm_mon + 1) + "_" + to_string(t_st->tm_mday);
	m_efileName = dir + "/e_log" + to_string(t_st->tm_year + 1900) + "_" + to_string(t_st->tm_mon + 1) + "_" + to_string(t_st->tm_mday);

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
  catch (const exception& e) {
	WhereToLog = COUT;
	this->eWrite("[Logger] Error in Logger object constructor : " + string(e.what()));
	this->Write( "[Logger] The standard output (cout) will be used for logging");
  }
  catch (...) {
	WhereToLog = COUT;
	this->eWrite("[Logger] Error in Logger object constructor");
	this->Write( "[Logger] The standard output (cout) will be used for logging");
  }
}

void Logger::Write(const string& log)
{
  if ( WhereToLog == COUT )
	cout << "[ " << m_printTime() << " ]: " << log << endl;
  else if ( WhereToLog == LOGFILE )
	m_file << "[ " << m_printTime() << " ]: " << log << endl;
  else {
	cout << "[ " << m_printTime() << " ]: " << log << endl;
	m_file << "[ " << m_printTime() << " ]: " << log << endl;
  }
}

void Logger::eWrite(const string& log)
{
  if ( WhereToLog == COUT )
	cerr << "[ " << m_printTime() << " ]: " << log << endl;
  else if ( WhereToLog == LOGFILE )
	m_efile << "[ " << m_printTime() << " ]: " << log << endl;
  else {
	cerr << "[ " << m_printTime() << " ]: " << log << endl;
	m_efile << "[ " << m_printTime() << " ]: " << log << endl;
  }
}

// Prints UTC timestamp
string Logger::m_printTime() {
  chrono::time_point<chrono::system_clock> now = chrono::system_clock::now();
  time_t now_time = chrono::system_clock::to_time_t(now);
  auto gmt_time = gmtime(&now_time);
  auto timestamp = std::put_time(gmt_time, "%Y-%m-%d %H:%M:%S");
  stringstream ss;
  ss << timestamp;
  return ss.str();
}

Logger::~Logger()
{
  m_file.close();
  m_efile.close();
}

//==================== end Logger Class ====================//

