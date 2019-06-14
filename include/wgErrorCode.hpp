#ifndef ERROR_CODE_H_INCLUDE
#define ERROR_CODE_H_INCLUDE

#include <string>

using namespace std;

//=== error list :open
#define XXX_FILE_NOT_FOUND 0

//=== error list :decoder
#define XXX_FILE_EXIST 100

// All the methods of the CheckExist class check for existance and validity of
// the passed file path. They should not throw any exception (all the exceptions
// are handled and reported internally).

class CheckExist
{
private:
  string filename;
public:
  bool GenericFile(const string& filename, const string & ext);
  bool RootFile(const string& filename);   //check the existence of root file.
  bool RawFile(const string& filename);    //check the existence of raw file.
  bool TxtFile(const string& filename);    //check the existence of txt file.
  bool CsvFile(const string& filename);    //check the existence of csv file.
  bool XmlFile(const string& filename);    //check the existence of xml file.
  bool LogFile(const string& filename);    //check the existence of log file.
  bool Dir(const string& filename);        //check the existence of dir.
};



#endif
