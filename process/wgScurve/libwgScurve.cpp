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
             const char* x_outputIMGDir){

  // ============================================================= //
  //                                                               //
  //                        Parse Arguments                        //
  //                                                               //
  // ============================================================= //
  
  string inputDir    (x_inputDir);
  string outputXMLDir(x_outputXMLDir);
  string outputIMGDir(x_outputIMGDir);

  CheckExist check;
  wgConst con;
	wgEditXML Edit;
	wgFit Fit;

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
		unsigned const						n_inputDAC = run_directory_tree.size();
		unsigned									n_threshold = run_directpry_tree[0].size();
		unsigned 									n_difs = topol.n_dfis;
		vector<unsigned> 					n_chips;
		vector<vector<unsigned>> 	n_chans;
		for(idif = 0; idif < n_difs; idif++){
			n_chips[idif] = topol.dif_map[idif].size();
			for(ichip = 0; ichip < n_chips[idif]; ichip++){
				n_chans[idif][ichip] = topol.dif_map[idif][ichip].size();
			}
		}


  	// make output xml files under the outputXMLdirectory
  	MakeScurveXML(outputXMLDir,n_difs,n_chips,n_chans);
		// define variables
		vector<vector<vector<vector<vector<double>>>>> 	noise;
		vector<vector<vector<vector<vector<double>>>>> 	noise_sigma;
		vector<vector<unsigned>> 												threshold;
		vector<unsigned> 																inputDAC;
		vector<vector<vector<vector<double>>>> 					pe1(ndifs);
		vector<vector<vector<vector<double>>>> 					pe2(ndifs);
		vector<vector<vector<double>>>									slope1(ndifs);
		vector<vector<vector<double>>>									slope2(ndifs);
		vector<vector<vector<double>>>									intercept1(ndifs);
		vector<vector<vector<double>>>									intercept1(ndifs);

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
  	  
  	    	for(unsigned ichip = 0; ichip < nchips[idif]; ichip++) {
  	    	  unsigned ichip_id = ichip + 1;
  	    	  
  	    	  // ************* Open XML file ************* //
  	    	  string xmlfile(idif_directory + "/Summary_chip" + to_string(ichip_id) + ".xml");
  	    	  try { Edit.Open(xmlfile); }
  	    	  catch (const exception& e) {
  	    	    Log.eWrite("[wgScurve] " + string(e.what()));
  	    	    return ERR_FAILED_OPEN_XML_FILE;
  	    	  }

  	    	  // ************* Read XML file ************* //
  	    	  for(unsigned ichan = 0; ichan < nchans[idif][ichip]; ichan++) {
  	    	    unsigned ichan_id = ichan + 1;
  	    	   	// get noise rate for each channel 
							noise[i_iDAC][i_threshold][idif][ichip][ichan] = Edit.SUMMARY_GetChFitValue("noise",ichan_id);
							noise_sigma[i_iDAC][i_threshold][idif][ichip][ichan] = Edit.SUMMARY_GetChFitValue("sigma_noise",ichan_id);
							threshold[i_iDAC][i_threshold] = Edit.SUMMARY_GetGlobalConfigValue("trigth");
							inputDAC[i_iDAC] = Edit.SUMMARY_GetGlobalConfigValue("inputDAC");
  	    	  }
  	    	  Edit.Close();
  	    	}  	// chip
				}  	// dif
  	  }  	// threshold
  	} 	// inputDAC
		

  	/********************************************************************************
  	 *                        Draw and fit the S-curve                              *
  	 ********************************************************************************/
		
		for(unsigned idif = 0; idif < n_difs; idif++){
  		unsigned idif_id = idif + 1;
			pe1[idif].reserve(nchips[idif]);
			pe2[idif].reserve(nchips[idif]);
			slope1[idif].reserve(nchips[idif]);
			slope2[idif].reserve(nchips[idif]);
			intercept1[idif].reserve(nchips[idif]);
			intercept2[idif].reserve(nchips[idif]);
  	
  		for(unsigned ichip = 0; ichip < nchips[idif]; ichip++) {
  		  unsigned ichip_id = ichip + 1;
  		  pe1[idif][ichip].reserve(nchans[idif][ichip]);
  		  pe2[idif][ichip].reserve(nchans[idif][ichip]);

  		  for(unsigned ichan = 0; ichan < nchans[idif][ichip]; ichan++) {
  		    unsigned ichan_id = ichan + 1;
					
  				for (auto const & x : run_directory_tree) {
  	  			unsigned i_iDAC = x.first;
						std::map iDAC_map = x.second;
  					
						TCanvas *c1 = new TCanvas("c1","c1");
						c1->SetLogy();
						
						// tentative variables for x, y and their errors
						vector<double> gx,gy,gxe,gye;

						for(auto const & y : iDAC_map) {
							unsigned i_threshold = y.first;

							gx.push_back(threshold[i_iDAC][i_threshold]);
							gy.push_back(noise[i_iDAC][i_threshold][idif][ichip][ichan]);
							gxe.push_back(0);
							gye.push_back(noise_sigma[i_iDAC][i_threshold][idif][ichip][ichan]);
						}
						
 						// ************* Draw S-curve Graph ************* //
						TGraphErrors* Scurve  = new TGraphErrors(gx.size(),&gx[0],&gy[0],&gxe[0],&gye[0]);
						string title;
						title = "Dif" + to_string(idif_id)
										+ "_Chip" + to_string(ichip_id)
										+ "_Channel" + to_string(ichan_id)
										+ "_InputDAC" + to_string(inputDAC[i_iDAC]) 
										+ ";Threshold;Noise rate";
						Scurve->SetTitle(title);
						Scurve->Draw("ap*");

 						// ************* Fit S-curve ************* //
						double pe1_t, pe2__t;
						Fit.scurve(Scurve,pe1_t,pe2_t);
						pe1[idif][ichip][ichan].push_back(pe1_t);
						pe2[idif][ichip][ichan].push_back(pe2_t);

 						// ************* Save S-curve Graph as png ************* //
						string image;
						image = outputIMGDir + "/Dif" + to_string(idif_id)
										+ "/Chip" + to_string(ichip_id) + "/Channel" + to_string(ichan_id)
										+ "/InputDAC" + to_string(inputDAC[i_iDAC]) + ".png";
						c1->Print(image);
						
						delete Scurve;
						delete c1;
  	    	} 	// inputDAC

					// ************ Linear fit of 1.5pe threshold vs inputDAC plot ************ //
					TCanvas *c2 = new TCanvas("c2","c2");

					TGraph* PELinear1 = new TGraph(	n_inputDAC,&inputDAC[0],&pe1[idif][ichip][ichan][0]);
					string title;
					title = "Dif" + to_string(idif_id)
									+ "_Chip" + to_string(ichip_id)
									+ "_Channel" + to_string(ichan_id)
									+ ";InputDAC;1.5 pe threshold";
					PELinear1->SetTitle(title);
					PELinear1->Draw("ap*");

					TF1* fit1 = new TF1("fit1", "[0]*x + [1]", 1, 241);
					fit1->SetParameters(-0.01, 170);
					PELinear1->Fit(fit1,"rl");
					fit1->Draw("same");
					slope1   [idif][ichip].push_back(fit1->GetParameter(0));
					inercept1[idif][ichip].push_back(fit1->GetParameter(1));
	
 					// ************* Save plot as png ************* //
					string image;
					image = outputIMGDir + "/Dif" + to_string(idif_id)
									+ "/Chip" + to_string(ichip_id) + "/Channel" + to_string(ichan_id)
									+ "/PE1vsInputDAC.png";
					c2->Print(image);
					c2->Clear();

					// ************ Linear fit of 2.5pe threshold vs inputDAC plot ************ //
					TGraph* PELinear2 = new TGraph(	n_inputDAC,&inputDAC[0],&pe2[idif][ichip][ichan][0]);
					string title;
					title = "Dif" + to_string(idif_id)
									+ "_Chip" + to_string(ichip_id)
									+ "_Channel" + to_string(ichan_id)
									+ ";InputDAC;2.5 pe threshold";
					PELinear2->SetTitle(title);
					PELinear2->Draw("ap*");

					TF1* fit2 = new TF1("fit2", "[0]*x + [1]", 1, 241);
					fit2->SetParameters(-0.01, 170);
					PELinear2->Fit(fit2,"rl");
					fit2->Draw("same");
					slope2   [idif][ichip].push_back(fit2->GetParameter(0));
					inercept2[idif][ichip].push_back(fit2->GetParameter(1));

 					// ************* Save plot as png ************* //
					image = outputIMGDir + "/Dif" + to_string(idif_id)
									+ "/Chip" + to_string(ichip_id) + "/Channel" + to_string(ichan_id)
									+ "/PE2vsInputDAC.png";
					c2->Print(image);
					
					delete PELinear1, PELinear2;
					delete fit1,fit2;
					delete c2;

				}  	// channel
  	  }  	// chip
  	} 	// dif


  	/********************************************************************************
  	 *                           threshold_card.xml                                 *
  	 ********************************************************************************/

  	string xmlfile(outputXMLDir + "/threshold_card.xml");
  	try { Edit.Open(xmlfile); }
  	catch (const wgInvalidFile & e) {
  	  Log.eWrite("[wgScurve] " + xmlfile + " : " + string(e.what()));
  	  return ERR_FAILED_OPEN_XML_FILE;
  	}

  	wgEditXML Edit;
		Edit.OPT_Make(xmlfile,inputDAC,n_difs,n_chips,n_chans);

		for(unsigned idif = 0; idif < n_difs; idif++){
  		unsigned idif_id = idif + 1;
  	
  		for(unsigned ichip = 0; ichip < nchips[idif]; ichip++) {
  		  unsigned ichip_id = ichip + 1;

  			for(unsigned ichan = 0; ichan < n_chans[idif][ichip]; ichan++) {
  			  unsigned ichan_id = ichan + 1;
  			  // Set the slope and intercept values for the result of fitting threshold-inputDAC plot.
					Edit.OPT_SetChanValue(string("s_th1"), idif_id, ichip_id, ichan_id, slope1[idif][ichip][ichan], NO_CREATE_NEW_MODE);
					Edit.OPT_SetChanValue(string("i_th1"), idif_id, ichip_id, ichan_id, intercept1[idif][ichip][ichan], NO_CREATE_NEW_MODE);
					Edit.OPT_SetChanValue(string("s_th2"), idif_id, ichip_id, ichan_id, slope2[idif][ichip][ichan], NO_CREATE_NEW_MODE);
					Edit.OPT_SetChanValue(string("i_th2"), idif_id, ichip_id, ichan_id, intercept2[idif][ichip][ichan], NO_CREATE_NEW_MODE);

						for(unsigned i_iDAC = 0; i_iDAC < inputDAC.size(); i_iDAC++){
							// Set the 2.5 pe and 1.5 pe level of ftting the scurve.
  			  		Edit.OPT_SetValue(string("threshold_1"), idif_id, ichip_id, ichan_id, i_iDAC, pe1[idif][ichip][ichan][i_iDAC], NO_CREATE_NEW_MODE);
  			  		Edit.OPT_SetValue(string("threshold_2"), idif_id, ichip_id, ichan_id, i_iDAC, pe2[idif][ichip][ichan][i_iDAC], NO_CREATE_NEW_MODE);
  			  	}
					}
  			}
  		}
		}
  	Edit.Write();
  	Edit.Close();
  	
		cout << "[wgScurve] Finish!" << endl;
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


 
