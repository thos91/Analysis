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

class Logger
{
  public:
    void Initialize();         //make log txt
    void Write(const string&);   //input log to ~.txt
    void eWrite(const string&);   //input elog to ~.txt
    ~Logger();
  protected:
    static string fileName;
    static string efileName;
    static FILE* file;
    static FILE* efile;
};

#endif
