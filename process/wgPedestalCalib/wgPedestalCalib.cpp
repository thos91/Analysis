// system includes
#include <string>

// system C includes
#include <getopt.h>
// user includes
#include "wgFileSystemTools.hpp"

#include "wgExceptions.hpp"
#include "wgConst.hpp"
#include "wgPedestalCalib.hpp"
#include "wgLogger.hpp"

using namespace wagasci_tools;

void print_help(const char * program_name) {
  std::cout <<  program_name << " creates the pedestal_card.xml.\n"
    "  -h         : help\n"
    "  -f (char*) : input directory (mandatory)\n"
    "  -o (char*) : output directory (default: same as input directory)\n"
    "  -i (char*) : output directory for plots and images (default: WAGASCI_IMGDIR)\n";
  exit(0);
}

int main(int argc, char** argv){
  int opt;
  wgEnvironment env;
  std::string input_dir("");
  std::string output_xml_dir = env.XMLDATA_DIRECTORY;
  std::string output_img_dir = env.IMGDATA_DIRECTORY;


  while((opt = getopt(argc,argv, "f:o:i:n:x:y:h")) !=-1 ) {
    switch(opt){
    case 'f':
      input_dir = optarg;
      break;
    case 'o':
      output_xml_dir = optarg; 
      break;
    case 'i':
      output_img_dir = optarg; 
      break;
    case 'h':
      print_help(argv[0]);
      break;
      default:
        print_help(argv[0]);
    }   
  }

  int result;
  if ( (result = wgPedestalCalib(input_dir.c_str(),
                                 output_xml_dir.c_str(),
                                 output_img_dir.c_str())) != WG_SUCCESS ) {
    Log.Write("[wgPedestalCalib] Returned error code " + std::to_string(result));
  }
  return result;
}

