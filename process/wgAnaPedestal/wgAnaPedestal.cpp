// system includes
#include <string>

// system C includes
#include <getopt.h>
// user includes
#include "wgFileSystemTools.hpp"

#include "wgExceptions.hpp"
#include "wgConst.hpp"
#include "wgAnaPedestal.hpp"
#include "wgLogger.hpp"

using namespace wagasci_tools;

void print_help(const char * program_name) {
  cout <<  program_name << " creates the pedestal_card.xml.\n"
    "  -h         : help\n"
    "  -f (char*) : input directory (mandatory)\n"
    "  -o (char*) : output directory (default: same as input directory)\n"
    "  -i (char*) : output directory for plots and images (default: WAGASCI_IMGDIR)\n";
  exit(0);
}

int main(int argc, char** argv){
  int opt;
  wgConst con;
  string inputDir("");
  string outputXMLDir = con.XMLDATA_DIRECTORY;
  string outputIMGDir = con.IMGDATA_DIRECTORY;


  while((opt = getopt(argc,argv, "f:o:i:n:x:y:h")) !=-1 ) {
    switch(opt){
    case 'f':
      inputDir = optarg;
      break;
    case 'o':
      outputXMLDir = optarg; 
      break;
    case 'i':
      outputIMGDir = optarg; 
      break;
    case 'h':
      print_help(argv[0]);
      break;
    default:
      print_help(argv[0]);
    }   
  }

  int result;
  if ( (result = wgAnaPedestal(inputDir.c_str(),
                               outputXMLDir.c_str(),
                               outputIMGDir.c_str())) != WG_SUCCESS ) {
    Log.Write("[wgAnaPedestal] Returned error code " + to_string(result));
  }

}

