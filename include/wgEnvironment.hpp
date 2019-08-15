#ifndef WGENVIRONMENT_H
#define WGENVIRONMENT_H

#include <string>

///////////////////////////////////////////////////////////////////////////////
// The wgEnvironment class is used to fetch the paths to the various
// directories needed by the WAGASCI analysis software.
//
// If one or more of the environment variables are defined in the
// shell from which the program is launched, then it is simply
// imported from there.
//
// The remaining variables are retrieved from a certain bash script
// file. This very script can also be used by any user to import these
// variables into their shell.
//
// The script path must be "/opt/calicoes/config/wagasci_environment.sh"
///////////////////////////////////////////////////////////////////////////////

class wgEnvironment{
public:
  wgEnvironment();
  
  std::string RAWDATA_DIRECTORY;
  std::string DECODE_DIRECTORY;
  std::string HIST_DIRECTORY;
  std::string RECON_DIRECTORY;
  std::string XMLDATA_DIRECTORY;
  std::string IMGDATA_DIRECTORY;
  std::string LOG_DIRECTORY;
  std::string MAIN_DIRECTORY;
  std::string CALICOES_DIRECTORY;
  std::string CALIBDATA_DIRECTORY;
  std::string BSD_DIRECTORY;
  std::string DQ_DIRECTORY;
  std::string DQHISTORY_DIRECTORY;
  std::string CONF_DIRECTORY;

 private:
  const std::string script_path;
  void GetENV();
  std::string ReadENVFile(const std::string &env_var_name);
};

#endif /* WGENVIRONMENT_H */
