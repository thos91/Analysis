// system includes
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>

// boost includes
#include <boost/filesystem.hpp>

// system C includes
#include <cstdbool>
#include <bits/stdc++.h>

// ROOT includes
#include <TSystem.h>
#include <THStack.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TImage.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TMultiGraph.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TF1.h>
#include <TSpectrum.h>

// user includes
#include "wgFileSystemTools.hpp"
#include "wgErrorCode.hpp"
#include "wgEditXML.hpp"
#include "wgColor.hpp"
#include "wgFit.hpp"
#include "wgFitConst.hpp"
#include "wgScurve.hpp"
#include "wgGetHist.hpp"
#include "wgLogger.hpp"
#include "wgTopology.hpp"

using namespace std;
using namespace wagasci_tools;

//******************************************************************
int wgScurve(const char* x_inputDir,
             const char* x_outputXMLDir,
             const char* x_outputIMGDir,
             const char* x_topologyString,
						 DirectoryTreeMap run_directory_tree) {

  // ============================================================= //
  //                                                               //
  //                        Parse Arguments                        //
  //                                                               //
  // ============================================================= //
  
  string inputDir    (x_inputDir);
  string outputXMLDir(x_outputXMLDir);
  string outputIMGDir(x_outputIMGDir);
  string topologyString(x_topologyString);

  CheckExist check;
  wgConst con;
	wgEditXML Edit;

  // ============ Check directories ============ //
  if( inputDir.empty() || !check.Dir(inputDir) ) { 
    Log.eWrite("[wgScurve] Input directory " + inputDir + " doesn't exist");
    exit(1);
  }
  if( outputXMLDir.empty() ) {
    outputXMLDir = con.CALIBDATA_DIRECTORY;
  }
  if( outputIMGDir.empty() ) {
    outputIMGDir = con.IMGDATA_DIRECTORY;
  }
  outputIMGDir = outputIMGDir + "/" + GetName(inputDir);

  // ============ Create outputXMLDir ============ //
  try { MakeDir(outputXMLDir); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[wgScurve] " + string(e.what()));
    return ERR_CANNOT_CREATE_DIRECTORY;
  }

  // ============ Create outputIMGDir ============ //
  try { MakeDir(outputIMGDir); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[wgScurve] " + string(e.what()));
    return ERR_CANNOT_CREATE_DIRECTORY;
  }

  Log.Write(" *****  READING DIRECTORY      : " + inputDir + "  *****");
  Log.Write(" *****  OUTPUT XML DIRECTORY   : " + outputXMLDir + "  *****");
  Log.Write(" *****  OUTPUT IMAGE DIRECTORY : " + outputIMGDir + "  *****");
  

  try{

  	/********************************************************************************
  	 *                         Include directory tree                               *
  	 ********************************************************************************/
  	
		// get topology from directory tree 
		// get the number of dif, chip, channel 
  	Topology topol(inputDir, run_directory_tree);
		unsigned 									n_difs = topol.n_dfis;
		vector<unsigned> 					n_chips;
		vector<vector<unsigned>> 	n_chans;
		for(idif = 0; idif < n_difs; idif++){
			n_chips[idif] = topol.dif_map[idif].size();
			for(ichip = 0; ichip < n_chips[idif]; ichip++){
				n_chans[idif][ichips] = topol.dif_map[idif][ichip].size();
			}
		}

  	// make output xml files under the outputXMLdirectory
  	MakeScurveXML(outputXMLDir,n_difs,n_chips,n_chans);
		// define variables
		vector<vector<vector<vector<vector<unsigned>>>>> 	noise;
		vector<vector<vector<vector<vector<unsigned>>>>> 	noise_sigma;

  	/********************************************************************************
  	 *                              Read XML files                                  *
  	 ********************************************************************************/

  	for (auto const & x : run_directory_tree) {
  	  unsigned i_iDAC = x.first;
			std::map iDAC_map = x.second;

  	  for(auto const & y : iDAC_map) {
				unsigned i_threshold = y.first;
				string threshold_directory = y.second;

				for(unsigned idif = 0; idif < n_difs; idif++){
  	    	unsigned idif_id = idif + 1;
  	    	string idif_directory(input_run_dir + threshold_directory + "/dif" + to_string(idif_id));
  	  
  	    	for(unsigned ichip = 0; ichip < topol.dif_map[idif].size(); ichip++) {
  	    	  unsigned ichip_id = ichip + 1;
  	    	  
  	    	  // ************* Open XML file ************* //
  	    	  string xmlfile(idif_directory + "/Summary_chip" + to_string(ichip_id) + ".xml");
  	    	  try { Edit.Open(xmlfile); }
  	    	  catch (const exception& e) {
  	    	    Log.eWrite("[wgScurve] " + string(e.what()));
  	    	    return ERR_FAILED_OPEN_XML_FILE;
  	    	  }

  	    	  // ************* Read XML file ************* //
  	    	  for(unsigned ichan = 0; ichan < topol.dif_map[idif][ichip]; ichan++) {
  	    	    unsigned ichan_id = ichan + 1;
  	    	   	// get noise rate for each channel 
							noise[i_iDAC][i_threshold][idif][ichip][ichan] = Edit.SUMMARY_GetChFitValue("noise",ichan_id);
							noise_sigma[i_iDAC][i_threshold][idif][ichip][ichan] = Edit.SUMMARY_GetChFitValue("sigma_noise",ichan_id);
  	    	  }
  	    	  Edit.Close();
  	    	}  	// end loop for chip
				}  	// end loop for dif
  	  }  	// end loop for threshold
  	} 	// end loop for inputDAC
		

  	/********************************************************************************
  	 *                              Draw S-curve                                    *
  	 ********************************************************************************/


  	/********************************************************************************
  	 *                              Fit  S-curve                                    *
  	 ********************************************************************************/


		cout << "Finish!" << endl;
  	Log.Write("end wgScurve ... " );
	}  // end try

	catch (const exception& e){
  	Log.eWrite("[wgScurve][" + inputDir + "] " + string(e.what()));
  	return ERR_WG_SCURVE;
	}
	return SCURVE_SUCCESS;
}

//******************************************************************
vector<string> GetIncludeFileName(const string& inputDir){
  DIR *dp;
  struct dirent *entry;
  vector<string> openxmlfile;

  dp = opendir(inputDir.c_str());
  if(dp==NULL){
    cout << " !! WARNING !! no data is in "<< inputDir << endl;
    return openxmlfile;
  }

  while( (entry = readdir(dp))!=NULL ){
    if((entry->d_name[0])!='.'){
      openxmlfile.push_back(Form("%s/%s",inputDir.c_str(),entry->d_name));
      cout << "ReadFile : " << inputDir << "/" << entry->d_name << endl;
    }
  }
  closedir(dp);
  return openxmlfile;
} 

//******************************************************************
// make output xml files below the outputXMLdirectory
void MakeScurveXML(string& outputXMLDir, 
		               unsigned n_difs, 
									 vector<unsigned> n_chips, 
									 vector<vector<unsigned>> n_chans) {
  wgEditXML Edit;
  for(unsigned idif_id = 1; idif_id < n_difs+1; idif_id++) {
    for(unsigned ichip_id = 1; ichip_id < n_chips[n_difs]+1; ichip_id++) {
      MakeDir(outputXMLDir + "/dif" + to_string(idif_id) + "/chip" + to_string(ichip_id));
      for(unsigned ichan_id = 1; ichan_id < n_chans[n_difs][n_chips]+1; ichan_id++) {
        Edit.SCURVE_Make(outputXMLDir + "/dif" + to_string(idif_id) + "/chip" + to_string(ichip_id) + "/chan" + to_string(ichan_id) + ".xml");
      }
    }
  }
}

//******************************************************************
double Calcurate_Mean(vector<double> v){
  double mean=0;
  double size=v.size();
  double max=0.;
  double min=0.;
  double col[(int)size];
  for(unsigned int i=0;i<v.size();i++){col[i]=0.;}
  if(v.size()==0){return 0.;}
  if(v.size()==1){return v[0];}
  for(unsigned int i=0;i<v.size();i++){
    int rank=0;
    for(unsigned int i2=0;i2<v.size();i2++){
      if(v[i]>v[i2])rank++;
    }
    col[rank]=v[i];
  }
  for(unsigned int i=0;i<v.size()-1;i++){
    if(col[i+1] > col[i]+2){
      size=i;
    }
  }
  max=v[(int)size-1];
  min=v[0];
  mean=((max+min)/2.);
  return mean;
}

//******************************************************************
double Calcurate_Sigma(vector<double> v){
  double mean=0;
  double size=v.size();
  double max=0.;
  double min=0.;
  double col[(int)size];
  for(unsigned int i=0;i<v.size();i++){col[i]=0.;}
  if(v.size()==0){return 0.;}
  if(v.size()==1){return v[0];}
  for(unsigned int i=0;i<v.size();i++){
    int rank=0;
    for(unsigned int i2=0;i2<v.size();i2++){
      if(v[i]>v[i2])rank++;
    }
    col[rank]=v[i];
  }
  for(unsigned int i=0;i<v.size()-1;i++){
    if(col[i+1] > col[i]+2){
      size=i;
    }
  }
  max=v[(int)size-1];
  min=v[0];
  mean=((max-min)/2.);
  return mean;
}


 
