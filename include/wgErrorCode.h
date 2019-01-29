#ifndef ERROR_CODE_H_INCLUDE
#define ERROR_CODE_H_INCLUDE

#include <string>
using namespace std;

//=== error list :open
#define XXX_FILE_NOT_FOUND 0

//=== error list :decoder
#define XXX_FILE_EXIST 100


class CheckExist
{
  private:
    string filename;
  public:
    bool RootFile(string& filename);            //check the existence of file.
    bool RawFile(string& filename);             //check the existence of file.
    bool TxtFile(string& filename);             //check the existence of file.
    bool XmlFile(string& filename);             //check the existence of file.
    bool LogFile(string& filename);             //check the existence of file.
    bool Dir(string& filename);                 //check the existence of dir.
};



#endif
