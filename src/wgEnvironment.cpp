// system includes
#include <array>
#include <exception>
#include <array>
#include <regex>
#include <sstream>

// user includes
#include "wgEnvironment.hpp"

//***************************************
wgEnvironment::wgEnvironment() : script_path("/opt/calicoes/config/wagasci_environment.sh") {
  this->GetENV();
}

//***************************************
std::string wgEnvironment::ReadENVFile(const std::string& env_var_name) {
  std::array<char, 128> buffer;
  std::string result;
  std::string command = "env -i bash -c 'source " + wgEnvironment::script_path + " && env'";
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }

  std::istringstream ss(result);
  std::string line;
  std::string delimiter("=");
  size_t pos;
  while (std::getline(ss, line)) {
    pos = line.find(delimiter);
    if (line.substr(0, pos) == env_var_name) {
      std::regex regexp("\\s+");
      return std::regex_replace(line.substr(pos + delimiter.length(), line.length()), regexp, "");
    }
  }
  return std::string("");
}

//***************************************
void wgEnvironment::GetENV(){
  if ( std::getenv("WAGASCI_RAWDATADIR") != NULL )
    RAWDATA_DIRECTORY    = std::getenv("WAGASCI_RAWDATADIR");
  else
    RAWDATA_DIRECTORY    = this->ReadENVFile("WAGASCI_RAWDATADIR");
  if ( std::getenv("WAGASCI_DECODEDIR") != NULL )
    DECODE_DIRECTORY     = std::getenv("WAGASCI_DECODEDIR");
  else
    DECODE_DIRECTORY     = this->ReadENVFile("WAGASCI_DECODEDIR");
  if ( std::getenv("WAGASCI_HISTDIR") )
    HIST_DIRECTORY       = std::getenv("WAGASCI_HISTDIR");
  else
    HIST_DIRECTORY       =  this->ReadENVFile("WAGASCI_HISTDIR");
  if ( std::getenv("WAGASCI_RECONDIR") != NULL )
    RECON_DIRECTORY      = std::getenv("WAGASCI_RECONDIR");
  else
    RECON_DIRECTORY      = this->ReadENVFile("WAGASCI_RECONDIR");
  if ( std::getenv("WAGASCI_XMLDATADIR") != NULL )
    XMLDATA_DIRECTORY    = std::getenv("WAGASCI_XMLDATADIR");
  else
    XMLDATA_DIRECTORY    =  this->ReadENVFile("WAGASCI_XMLDATADIR");
  if ( std::getenv("WAGASCI_IMGDATADIR") != NULL )
    IMGDATA_DIRECTORY    = std::getenv("WAGASCI_IMGDATADIR");
  else
    IMGDATA_DIRECTORY    = this->ReadENVFile("WAGASCI_IMGDATADIR");
  if ( std::getenv("WAGASCI_LOGDIR") != NULL )
    LOG_DIRECTORY        = std::getenv("WAGASCI_LOGDIR");
  else
    LOG_DIRECTORY        = this->ReadENVFile("WAGASCI_LOGDIR");
  if ( std::getenv("WAGASCI_MAINDIR") != NULL )
    MAIN_DIRECTORY       = std::getenv("WAGASCI_MAINDIR");
  else
    MAIN_DIRECTORY       = this->ReadENVFile("WAGASCI_MAINDIR");
  if ( std::getenv("WAGASCI_CALICOESDIR") != NULL )
    CALICOES_DIRECTORY   = std::getenv("WAGASCI_CALICOESDIR");
  else
    CALICOES_DIRECTORY   = this->ReadENVFile("WAGASCI_CALICOESDIR");
  if ( std::getenv("WAGASCI_CALIBDATADIR") != NULL )
    CALIBDATA_DIRECTORY  = std::getenv("WAGASCI_CALIBDATADIR");
  else
    CALIBDATA_DIRECTORY  = this->ReadENVFile("WAGASCI_CALIBDATADIR");
  if ( std::getenv("WAGASCI_BSDDIR") != NULL )
    BSD_DIRECTORY        = std::getenv("WAGASCI_BSDDIR");
  else
    BSD_DIRECTORY        = this->ReadENVFile("WAGASCI_BSDDIR");
  if ( std::getenv("WAGASCI_DQDIR") != NULL )
    DQ_DIRECTORY         = std::getenv("WAGASCI_DQDIR");
  else
    DQ_DIRECTORY    = this->ReadENVFile("WAGASCI_DQDIR");
  if ( std::getenv("WAGASCI_DQHISTORYDIR") != NULL )
    DQHISTORY_DIRECTORY  = std::getenv("WAGASCI_DQHISTORYDIR");
  else
    DQHISTORY_DIRECTORY  = this->ReadENVFile("WAGASCI_DQHISTORYDIR");
  if ( std::getenv("WAGASCI_CONFDIR") != NULL )
    CONF_DIRECTORY  = std::getenv("WAGASCI_CONFDIR");
  else
    CONF_DIRECTORY  = this->ReadENVFile("WAGASCI_CONFDIR");
}
