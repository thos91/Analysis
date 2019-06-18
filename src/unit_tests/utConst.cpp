#include "wgConst.hpp"
#include <iostream>

int main() {
  wgConst env; 
  env.GetENV();
  std::cout << "RAWDATA_DIRECTORY\t = " << env.RAWDATA_DIRECTORY << std::endl;
  std::cout << "DECODE_DIRECTORY\t = " << env.DECODE_DIRECTORY << std::endl;
  std::cout << "HIST_DIRECTORY\t\t = " << env.HIST_DIRECTORY << std::endl;
  std::cout << "RECON_DIRECTORY\t\t = " << env.RECON_DIRECTORY << std::endl;
  std::cout << "XMLDATA_DIRECTORY\t = " << env.XMLDATA_DIRECTORY << std::endl;
  std::cout << "IMGDATA_DIRECTORY\t = " << env.IMGDATA_DIRECTORY << std::endl;
  std::cout << "LOG_DIRECTORY\t\t = " << env.LOG_DIRECTORY << std::endl;
  std::cout << "MAIN_DIRECTORY\t\t = " << env.MAIN_DIRECTORY << std::endl;
  std::cout << "CALICOES_DIRECTORY\t = " << env.CALICOES_DIRECTORY << std::endl;
  std::cout << "CALIBDATA_DIRECTORY\t = " << env.CALIBDATA_DIRECTORY << std::endl;
  std::cout << "BSD_DIRECTORY\t\t = " << env.BSD_DIRECTORY << std::endl;
  std::cout << "DQ_DIRECTORY\t\t = " << env.DQ_DIRECTORY << std::endl;
  std::cout << "DQHISTORY_DIRECTORY\t = " << env.DQHISTORY_DIRECTORY << std::endl;
  std::cout << "CONF_DIRECTORY\t\t = " << env.CONF_DIRECTORY << std::endl; 
}
