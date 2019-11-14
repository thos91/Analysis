#ifndef WGFILESYSTEMTOOLS_H
#define WGFILESYSTEMTOOLS_H

// system includes
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>

// ROOT includes
#include "TString.h"

// user includes
#include "wgGainCalib.hpp"
#include "wgGainCalibUgly.hpp"

namespace wagasci_tools {

///////////////////////////////////////////////////////////////////////////////
//                                 get_stats                                 //
///////////////////////////////////////////////////////////////////////////////

namespace get_stats {

// extract the extension from a path string
// extension("/path/to/file.txt") -> ".txt"
std::string extension(const std::string& str);

// extract the file or directory name
// filename("/path/to/file.txt") -> "file.txt"
std::string filename(const std::string& str);

// extract the top-level file or directory name without extension
// basename("/path/to/file.txt") -> "file"
std::string basename(const std::string& str);

// extract the lower-level path
// dirname("/path/to/file.txt") -> "/path/to"
std::string dirname(const std::string& str);

// extract the name before the last underscore
// name_before_last_under_bar("/path/to/my_file.txt") -> "my"
std::string name_before_last_under_bar(const std::string& str);

// extract the name after the last underscore
// name_after_last_under_bar("/path/to/my_file.txt") -> "file"
std::string name_after_last_under_bar(const std::string& str);

} // get_stats

///////////////////////////////////////////////////////////////////////////////
//                                    list                                   //
///////////////////////////////////////////////////////////////////////////////

namespace list {

// list all the files with extension "extension" present in the folder
// "input_dir". returns the absolute path to the files as a vector of
// std::strings.
std::vector<std::string> list_files(const std::string& input_dir,
                                    bool with_integer = false,
                                    const std::string& extension = "");

// list all the directories in the folder "input_dir". _returns the
// absolute path to the directories as a vector of std::strings.
std::vector<std::string> list_directories(const std::string& input_dir,
                                          bool with_integer = false);

// count all the files with extension "extension" present in the
// folder "input_dir"
unsigned how_many_files(const std::string& input_dir,
                        bool with_integer = false,
                        const std::string& extension = "");

// count all the directories present in the folder "input_dir"
unsigned how_many_directories(const std::string& input_dir,
                              bool with_integer = false);

} // list

///////////////////////////////////////////////////////////////////////////////
//                                    make                                   //
///////////////////////////////////////////////////////////////////////////////

namespace make {

// _create the directory whose absolute path is "str"
void directory(const std::string& str);

// Print the bad_channels map to a CVS file. The file format is as
// follows:
//
// "DIF"  "CHIP" "CHANNEL" "ADC 1PEU for each iDAC" "ADC 2PEU for each iDAC"
void bad_channels_file(const gain_calib::BadChannels& bad_channels,
                       gain_calib::Charge& charge,
                       const std::vector<unsigned>& v_idac,
                       const std::string &cvs_file_path);
// "DIF"  "CHIP" "CHANNEL" "FAIN for each iDAC"
void bad_channels_file(const gain_calib::BadChannels& bad_channels,
                       gain_calib::ugly::Gain& gain,
                       const std::vector<unsigned>& v_idac,
                       const std::string &cvs_file_path);

} // make

///////////////////////////////////////////////////////////////////////////////
//                                   string                                  //
///////////////////////////////////////////////////////////////////////////////

namespace string {

// extract the first unsigned integer from a std::string. If not found
// returns -1
int extract_integer(const std::string& str);

// extract the DIF ID from a filename or path. The DIF ID must come
// after the "ecal_dif_" string. If not found -1 is returned
int extract_dif_id(const std::string& str);

// _try to find in the haystack the needle - ignore case
bool find_string_ic(const std::string & str_haystack,
                    const std::string & str_needle);

// _helper function that takes a string and returns the maximum depth
// nested parenthesis
int max_depth(std::string str);

} // string

///////////////////////////////////////////////////////////////////////////////
//                                check_exist                                //
///////////////////////////////////////////////////////////////////////////////

namespace check_exist {

// return true if the file exists, otherwise false

// all the methods of the check_exist namespace check for existance and validity of
// the passed file path. They should not throw any exception (all the exceptions
// are handled and reported internally).

bool generic_file(const std::string& filename, const std::string & ext);
bool root_file   (const std::string& filename);   //check the existence of root file.
bool root_file   (const TString&     filename);   //check the existence of root file.
bool raw_file    (const std::string& filename);   //check the existence of raw file.
bool txt_file    (const std::string& filename);   //check the existence of txt file.
bool csv_file    (const std::string& filename);   //check the existence of csv file.
bool xml_file    (const std::string& filename);   //check the existence of xml file.
bool log_file    (const std::string& filename);   //check the existence of log file.
bool directory   (const std::string& filename);   //check the existence of dir.

}  // check_exist

///////////////////////////////////////////////////////////////////////////////
//                                  datetime                                 //
///////////////////////////////////////////////////////////////////////////////

namespace datetime {
// convert localtime datetime string formated as "%Y/%m/%d %H:%M:%S"
// into seconds elapsed from epoch. If the string could not be parse
// -1 is returned.
int datetime_to_seconds(const std::string & datetime);

} // datetime

} // wagasci_tools

#endif /* WGFILESYSTEMTOOLS_H */
