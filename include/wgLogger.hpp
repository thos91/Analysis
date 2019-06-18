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

using namespace std;

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
  wgLogger(const string & log_dir);
  void Write(const string & log);
  void eWrite(const string & log);
  ~wgLogger();
protected:
  string m_printTime();
  string m_fileName;
  string m_efileName;
  ofstream m_file;
  ofstream m_efile;
};

extern wgLogger Log;

#endif
