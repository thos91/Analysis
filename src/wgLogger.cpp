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
#include "wgFileSystemTools.hpp"
#include "wgExceptions.hpp"
#include "wgConst.hpp"
#include "wgLogger.hpp"

//==================== wgLogger Class ====================//

wgLogger Log;

wgLogger::wgLogger() : wgLogger(std::string("")) {}

wgLogger::wgLogger(const std::string& log_dir)
{
  try {
    time_t newTime = time( NULL );
    struct tm *t_st = localtime( &newTime );
    std::string dir;
    // If log_dir is empty try to infer it from the environment
    if ( log_dir.empty() ) {
      wgEnvironment env;
      dir = env.LOG_DIRECTORY;
    } else dir = log_dir;
  
    // If the log directory is not found, create it
        
    if ( !wagasci_tools::check_exist::directory(dir) ) {
      boost::filesystem::path dir_path(dir);
      if( !boost::filesystem::create_directories(dir_path) ) {
        std::string error_message("[wgTools][" + dir + "] failed to create directory");
        Log.eWrite(error_message);
        throw wgInvalidFile(error_message);
      }
    }

    m_fileName  = dir + "/log" + std::to_string(t_st->tm_year + 1900) + "_" + std::to_string(t_st->tm_mon + 1) + "_" + std::to_string(t_st->tm_mday);
    m_efileName = dir + "/e_log" + std::to_string(t_st->tm_year + 1900) + "_" + std::to_string(t_st->tm_mon + 1) + "_" + std::to_string(t_st->tm_mday);

    m_file.open(m_fileName, std::ofstream::out | std::ofstream::app);
    // After this attempt to open a file, we can safely use strerror() only  
    // in case file.is_open() returns False.
    if (!m_file.is_open()) {
      std::stringstream error_message;
      error_message << "error while opening file " << m_fileName << " (" << strerror(errno) << ")";
      throw wgInvalidFile(error_message.str());
    }
    m_efile.open(m_efileName, std::ofstream::out | std::ofstream::app);
    if (!m_efile.is_open()) {
      std::stringstream error_message;
      error_message << "error while opening file " << m_efileName << " (" << strerror(errno) << ")";
      throw wgInvalidFile(error_message.str());
    }
  }
  catch (const std::exception& e) {
    WhereToLog = COUT;
    this->eWrite("[wgLogger] Error in wgLogger object constructor : " + std::string(e.what()));
    this->Write( "[wgLogger] The standard output (cout) will be used for logging");
  }
}

void wgLogger::Write(const std::string& log)
{
  if ( WhereToLog == COUT )
    std::cout << "[ " << m_printTime() << " ]: " << log << std::endl;
  else if ( WhereToLog == LOGFILE )
    m_file << "[ " << m_printTime() << " ]: " << log << std::endl;
  else {
    std::cout << "[ " << m_printTime() << " ]: " << log << std::endl;
    m_file << "[ " << m_printTime() << " ]: " << log << std::endl;
  }
}

void wgLogger::eWrite(const std::string& log)
{
  if ( WhereToLog == COUT )
    std::cerr << "[ " << m_printTime() << " ]: " << log << std::endl;
  else if ( WhereToLog == LOGFILE )
    m_efile << "[ " << m_printTime() << " ]: " << log << std::endl;
  else {
    std::cerr << "[ " << m_printTime() << " ]: " << log << std::endl;
    m_efile << "[ " << m_printTime() << " ]: " << log << std::endl;
  }
}

// Prints UTC timestamp
std::string wgLogger::m_printTime() {
  std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
  time_t now_time = std::chrono::system_clock::to_time_t(now);
  auto gmt_time = gmtime(&now_time);
  auto timestamp = std::put_time(gmt_time, "%Y-%m-%d %H:%M:%S");
  std::stringstream ss;
  ss << timestamp;
  return ss.str();
}

wgLogger::~wgLogger()
{
  m_file.close();
  m_efile.close();
}

//==================== end wgLogger Class ====================//
