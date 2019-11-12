// system includes
#include <string>
#include <getopt.h>

// user includes
#include "wgErrorCodes.hpp"
#include "wgLogger.hpp"
#include "wgDQCheck.hpp"

void print_help(const char * program_name) {
  std::cout << program_name << " creates several plots to check the data quality\n"
      "  -h         : help\n"
      "  -f (char*) : directory containing the summary xml files (mandatory)\n"
      "  -c (char*) : directory containing the calibration card files\n"
      "                 (default = WAGASCI_CONFDIR)\n"
      "  -o (char*) : output directory (default = WAGASCI_DQDIR)\n";
  exit(0);
}

int main(int argc, char** argv) {
  
  int opt;
  std::string summary_dir("");
  std::string calib_dir("");
  std::string output_dir("");
  
  while ((opt = getopt(argc, argv, "h")) != -1) {
    switch(opt) {
      case 'f':
        summary_dir = optarg;
        break;
      case 'c':
        calib_dir = optarg; 
        break;
      case 'o':
        output_dir = optarg; 
        break;
      case 'h':      
      default:
        print_help(argv[0]);
    }   
  }

  int retcode;
  if ((retcode = wgDQCheck(summary_dir.c_str(),
                           calib_dir.c_str(),
                           output_dir.c_str())) != WG_SUCCESS) {
    Log.eWrite("[wgDGCheck] failed with code " +
               std::to_string(retcode));
  }
  exit(retcode);
}
