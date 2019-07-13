#ifndef WGLOGGER_HPP_INCLUDE_
#define WGLOGGER_HPP_INCLUDE_

// system includes
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

/* - Initialize: opens two files (one for info logging and another one for error
                 logging) in the log_dir directory. If the directory is not
                 given the environment variable WAGASCI_LOGDIR is used instead.
  
   - Write: logs a message to the info logging file

   - eWrite: logs a message to the error logging file

   - LogToCout and LogToCerr: if set tu true the Logger will not log to file but
                              will redirect every message to std::cout and
                              std::cerr respectively
*/

typedef enum {
   LOGFILE  = 0,
   COUT     = 1,
   BOTH     = 2
} TriState;

class wgLogger
{
public:
  TriState WhereToLog = BOTH;
  wgLogger();
  wgLogger(const std::string & log_dir);
  void Write(const std::string & log);
  void eWrite(const std::string & log);
  ~wgLogger();
protected:
  std::string m_printTime();
  std::string m_fileName;
  std::string m_efileName;
  std::ofstream m_file;
  std::ofstream m_efile;
};

extern wgLogger Log;

#endif
