#ifndef WGTOOLS_H_INCLUDE
#define WGTOOLS_H_INCLUDE

#include <string>
#include <sstream>
#include <iostream>

using namespace std;

class OperateString
{
private:
  string str;
  string ext;
public:  
  string GetExtension(const string& str);  //Get Extension from full path.
  string GetName(const string& str);       //Get Name from full path.
  string GetPath(const string& str);       //Get Path from full path.
  string GetNameBeforeLastUnderBar(const string& str);       //Get Path from full path.
};

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

class Logger
{
public:
  TriState WhereToLog = BOTH;
  Logger();
  Logger(const string&);
  void Write(const string&);
  void eWrite(const string&);
  ~Logger();
protected:
  string m_printTime();
  static string m_fileName;
  static string m_efileName;
  static ofstream m_file;
  static ofstream m_efile;
};

#endif
