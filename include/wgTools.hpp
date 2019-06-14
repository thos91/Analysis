#ifndef WGTOOLS_H_INCLUDE
#define WGTOOLS_H_INCLUDE

// system includes
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

using namespace std;

/* class to extract the information contained in a generic path. The members and
   methods names are pretty self-explanatory */

class OperateString
{
private:
  string str;
  string ext;
public:  
  string GetExtension(const string& str);                //Get Extension from full path.
  string GetName(const string& str);                     //Get Name from full path.
  string GetPath(const string& str);                     //Get Path from full path.
  string GetNameBeforeLastUnderBar(const string& str);   //Get Path from full path.
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
  string m_fileName;
  string m_efileName;
  ofstream m_file;
  ofstream m_efile;
};

extern Logger Log;

#endif
