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
  string GetExtension(string& str);  //Get Extension from full path.
  string GetName(string& str);       //Get Name from full path.
  string GetPath(string& str);       //Get Path from full path.
  string GetNameBeforeLastUnderBar(string& str);       //Get Path from full path.
};

/* - Initialize: opens two files (one for info logging and another one for error
   logging) in the log_dir directory. If the directory is not given the environment variable
   WAGASCI_LOGDIR is used instead.
  
   - Write: logs a message to the info logging file

   - eWrite: logs a message to the error logging file
*/

class Logger
{
public:
  void Initialize();
  void Initialize(string&);
  void Write(const string&);
  void eWrite(const string&);
  ~Logger();
protected:
  static string fileName;
  static string efileName;
  static ofstream file;
  static ofstream efile;
};

#endif
