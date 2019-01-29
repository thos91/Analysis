#include "wgErrorCode.h"
#include "wgTools.h"
#include "Const.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>

bool CheckExist::RootFile(string &filename)
{
  OperateString* OpStr =new OperateString;
  struct stat st; 
  try {
    if(stat(filename.c_str(),&st) != 0) throw 0;
    mode_t m = st.st_mode;
    if(S_ISDIR(m)) throw 1;
    if(OpStr->GetExtension(filename)!="root") throw 2;
    delete OpStr;
    return true;
  }
  catch(int fError) {
    delete OpStr;
    return false;
  }
}

bool CheckExist::RawFile(string &filename)
{
  OperateString* OpStr =new OperateString;
  struct stat st; 
  try {
    if(stat(filename.c_str(),&st) != 0) throw 0;
    mode_t m = st.st_mode;
    if(S_ISDIR(m)) throw 1;
    if(OpStr->GetExtension(filename)!="raw") throw 2;
    delete OpStr;
    return true;
  }
  catch(int fError) {
    delete OpStr;
    return false;
  }
}

bool CheckExist::TxtFile(string &filename)
{
  OperateString* OpStr =new OperateString;
  struct stat st; 
  try {
    if(stat(filename.c_str(),&st) != 0) throw 0;
    mode_t m = st.st_mode;
    if(S_ISDIR(m)) throw 1;
    if(OpStr->GetExtension(filename)!="txt") throw 2;
    delete OpStr;
    return true;
  }
  catch(int fError) {
    delete OpStr;
    return false;
  }
}

bool CheckExist::XmlFile(string &filename)
{
  OperateString* OpStr =new OperateString;
  struct stat st; 
  try {
    if(stat(filename.c_str(),&st) != 0) throw 0;
    mode_t m = st.st_mode;
    if(S_ISDIR(m)) throw 1;
    if(OpStr->GetExtension(filename)!="xml") throw 2;
    delete OpStr;
    return true;
  }
  catch(int fError) {
    delete OpStr;
    return false;
  }
}

bool CheckExist::LogFile(string &filename)
{
  OperateString* OpStr =new OperateString;
  struct stat st; 
  try {
    if(stat(filename.c_str(),&st) != 0) throw 0;
    mode_t m = st.st_mode;
    if(S_ISDIR(m)) throw 1;
    if(OpStr->GetExtension(filename)!="log") throw 2;
    delete OpStr;
    return true;
  }
  catch(int fError) {
    delete OpStr;
    return false;
  }
}


bool CheckExist::Dir(string &dirname)
{
  struct stat st; 
  if(stat(dirname.c_str(),&st) != 0){ 
    return false;
  }else{
    mode_t m = st.st_mode;
    if(S_ISDIR(m)){
      return true;
    }else{
      return false;
    }   
  }
}

