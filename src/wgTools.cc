#include "wgTools.h"
#include "TROOT.h"
#include "wgErrorCode.h"
#include "Const.h"

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

string Logger::fileName = "";
string Logger::efileName = "";
FILE * Logger::file = NULL;
FILE * Logger::efile = NULL;

void Logger::Initialize()
{
  time_t newTime = time( NULL );
  struct tm *t_st = localtime( &newTime );
  wgConst *con = new wgConst();
  con->GetENV();
  Logger::fileName = Form("%s/%d_%d_%d.txt",con->LOG_DIRECTORY,t_st->tm_year+1900,t_st->tm_mon+1,t_st->tm_mday); 
  Logger::efileName = Form("%s/e_log%d_%d_%d.txt",con->LOG_DIRECTORY,t_st->tm_year+1900,t_st->tm_mon+1,t_st->tm_mday); 
  delete con;
  file = fopen( fileName.data(), "a" );
  if ( file == NULL )
  {
    printf("Failed to open log file for writing.\n");
    exit(1);
  }
  efile = fopen( efileName.data(), "a" );
  if ( file == NULL )
  {
    printf("Failed to open elog file for writing.\n");
    exit(1);
  }
}

void Logger::Write(const std::string& log)
{
  time_t newTime = time(NULL);
  char* japtime;
  japtime = ctime(&newTime);
  japtime[strlen(japtime)-1] = '\0';

  fprintf( file, "[%s]:%s\n",japtime,log.c_str());
  fflush( file );
}

void Logger::eWrite(const std::string& log)
{
  time_t newTime = time(NULL);
  char* japtime;
  japtime = ctime(&newTime);
  japtime[strlen(japtime)-1] = '\0';

  fprintf( efile, "[ %s ]: %s\n",japtime,log.c_str());
  fflush( file );
}

Logger::~Logger()
{
  fclose(file);
}

//======end Logger Class//

