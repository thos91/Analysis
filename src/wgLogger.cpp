// system C++ includes
#include <iostream>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <string>

// system C includes
#include <cerrno>

// boost includes
#include <boost/filesystem.hpp>

// user includes
#include "wgErrorCode.hpp"
#include "wgExceptions.hpp"
#include "wgConst.hpp"
#include "wgLogger.hpp"

//==================== wgLogger Class ====================//

wgLogger Log;

wgLogger::wgLogger() : wgLogger(string("")) {}

wgLogger::wgLogger(const string& log_dir)
{
  try {
	time_t newTime = time( NULL );
	struct tm *t_st = localtime( &newTime );
	string dir;
	// If log_dir is empty try to infer it from the environment
	if ( log_dir.empty() ) {
	  wgConst con;
	  dir = con.LOG_DIRECTORY;
	} else dir = log_dir;
  
	// If the log directory is not found, create it
	CheckExist check;
	if ( !check.Dir(dir) ) {
	  boost::filesystem::path dir_path(dir);
	  if( !boost::filesystem::create_directories(dir_path) ) {
		string error_message("[wgTools][" + dir + "] failed to create directory");
		Log.eWrite(error_message);
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
	this->eWrite("[wgLogger] Error in wgLogger object constructor : " + string(e.what()));
	this->Write( "[wgLogger] The standard output (cout) will be used for logging");
  }
  catch (...) {
	WhereToLog = COUT;
	this->eWrite("[wgLogger] Error in wgLogger object constructor");
	this->Write( "[wgLogger] The standard output (cout) will be used for logging");
  }
}

void wgLogger::Write(const string& log)
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

void wgLogger::eWrite(const string& log)
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
string wgLogger::m_printTime() {
  chrono::time_point<chrono::system_clock> now = chrono::system_clock::now();
  time_t now_time = chrono::system_clock::to_time_t(now);
  auto gmt_time = gmtime(&now_time);
  auto timestamp = std::put_time(gmt_time, "%Y-%m-%d %H:%M:%S");
  stringstream ss;
  ss << timestamp;
  return ss.str();
}

wgLogger::~wgLogger()
{
  m_file.close();
  m_efile.close();
}

//==================== end wgLogger Class ====================//
