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
  	 *                  Get directory tree and set varicables                       *
  	 ********************************************************************************/
  	
		// Get topology from input directory.
		// Get the number of dif, chip, channel, imputDAC and threshold. 
  	Topology topol(inputDir, TopologySourceType::scurve_tree );
		const unsigned 						n_inputDAC  = ListDirectories(inputDir).size();
		const unsigned						n_threshold = ListDirectories(inputDir).at(0).size();
		unsigned 									n_difs = topol.n_dfis;
		vector<unsigned> 					n_chips;
		vector<vector<unsigned>> 	n_chans;
		for(idif = 0; idif < n_difs; idif++){
			n_chips[idif] = topol.dif_map[idif+1].size();
			for(ichip = 0; ichip < n_chips[idif]; ichip++){
				n_chans[idif][ichip] = topol.dif_map[idif+1][ichip+1].size();
			}
		}

		// Define variables,
		vector<unsigned> 																inputDAC		(ndifs);
		vector<vector<unsigned>> 												threshold		(ndifs);
		vector<vector<vector<double>>>									slope1			(ndifs);
		vector<vector<vector<double>>>									slope2			(ndifs);
		vector<vector<vector<double>>>									intercept1	(ndifs);
		vector<vector<vector<double>>>									intercept1	(ndifs);
		vector<vector<vector<vector<double>>>> 					pe1					(ndifs);
		vector<vector<vector<vector<double>>>> 					pe2					(ndifs);
		vector<vector<vector<vector<vector<double>>>>> 	noise				(ndifs);
		vector<vector<vector<vector<vector<double>>>>> 	noise_sigma	(ndifs);
		//  and resize them.
		for(unsigned idif = 0; idif < n_difs; idif++){
			threshold		[idif].resize(n_chips[idif]);
			slope1			[idif].resize(n_chips[idif]);
			slope2			[idif].resize(n_chips[idif]);
			intercept1	[idif].resize(n_chips[idif]);
			intercept2	[idif].resize(n_chips[idif]);
			pe1					[idif].resize(n_chips[idif]);
			pe2					[idif].resize(n_chips[idif]);
			noise				[idif].resize(n_chips[idif]);
			noise_sigma	[idif].resize(n_chips[idif]);
			for(unsigned ichip = 0; ichip < n_chips[idif]; ichip++){
				slope1			[idif][ichip].resize(n_chans[idif][ichip]);
				slope2			[idif][ichip].resize(n_chans[idif][ichip]);
				intercept1	[idif][ichip].resize(n_chans[idif][ichip]);
				intercept2	[idif][ichip].resize(n_chans[idif][ichip]);
				pe1					[idif][ichip].resize(n_chans[idif][ichip]);
				pe2					[idif][ichip].resize(n_chans[idif][ichip]);
				noise				[idif][ichip].resize(n_chans[idif][ichip]);
				noise_sigma	[idif][ichip].resize(n_chans[idif][ichip]);
				for(unsigned ichan = 0; ichan < n_chans[idif][ichip]; ichan++){
					pe1					[idif][ichip][ichan].resize(n_inputDAC);
					pe2					[idif][ichip][ichan].resize(n_inputDAC);
					noise				[idif][ichip][ichan].resize(n_inputDAC);
					noise_sigma	[idif][ichip][ichan].resize(n_inputDAC);
					for(unsigned i_iDAC = 0; i_iDAC < n_inputDAC; i_iDAC++){
						noise				[idif][ichip][ichan][i_iDAC].resize(n_threshold);
						noise_sigma	[idif][ichip][ichan][i_iDAC].resize(n_threshold);
					}
				}
			}
		}

  	/********************************************************************************
  	 *                              Read XML files                                  *
  	 ********************************************************************************/
  	
		// input DAC
  	for (auto const & iDAC_directory : iDAC_dir_list) {
  	  unsigned i_iDAC = iDAC_directory.getIndex();
  	  vector<string> th_dir_list = ListDirectories(iDAC_directory);
  	  // threshold
  	  for (auto & th_directory : th_dir_list) {
  	    unsigned i_threshold = th_directory.getIndex();
  	    // DIF
  	    th_directory += "/wgAnaHistSummary/Xml";
  	    vector<string> dif_dir_list = ListDirectories(th_directory);
  	    for (auto const & idif_directory : dif_dir_list) {
  	      unsigned idif_id = extractIntegerFromString(GetName(idif_directory));
					unsigned idif = idif - 1;
  	      // chip
  	      vector<string> chip_xml_list = ListFilesWithExtension(idif_directory, "xml");
  	      for (auto const & ichip_xml : chip_xml_list) {
  	        unsigned ichip_id = extractIntegerFromString(GetName(ichip_xml));
						unsigned ichip = ichip_id -1;
  	    	  
  	    	  // ************* Open XML file ************* //
  	    	  try { Edit.Open(ichip_xml); }
  	    	  catch (const exception& e) {
  	    	    Log.eWrite("[wgScurve] " + string(e.what()));
  	    	    return ERR_FAILED_OPEN_XML_FILE;
  	    	  }
  	    	  // ************* Read XML file ************* //
						threshold[i_iDAC][i_threshold] = Edit.SUMMARY_GetGlobalConfigValue("trigth");
						inputDAC[i_iDAC] = Edit.SUMMARY_GetGlobalConfigValue("inputDAC");
  	    	  for(unsigned ichan = 0; ichan < nchans[idif][ichip]; ichan++) {
  	    	    unsigned ichan_id = ichan + 1;
  	    	   	// get noise rate for each channel 
							noise[idif][ichip][ichan][i_iDAC][i_threshold]  = Edit.SUMMARY_GetChFitValue("noise",ichan_id);
							noise_sigma[idif][ichip][ichan][i_iDAC][i_threshold] = Edit.SUMMARY_GetChFitValue("sigma_noise",ichan_id);
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
  	
  		for(unsigned ichip = 0; ichip < nchips[idif]; ichip++) {
  		  unsigned ichip_id = ichip + 1;

  		  for(unsigned ichan = 0; ichan < nchans[idif][ichip]; ichan++) {
  		    unsigned ichan_id = ichan + 1;
					
					for(unsigned i_iDAC = 0; i_iDAC < n_inputDAC; i_iDAC++){
  					
						TCanvas *c1 = new TCanvas("c1","c1");
						c1->SetLogy();
						
						// tentative variables for x, y and their errors 
						// which are used to draw TGraphErrors
						vector<double> gx,gy,gxe,gye;
						
						for(unsigned i_threshold=0; i_threshold < n_threshold; i_threshold++){

							gx.push_back(threshold[i_iDAC][i_threshold]);
							gy.push_back(noise[idif][ichip][ichan][i_iDAC][i_threshold]);
							gxe.push_back(0);
							gye.push_back(noise_sigma[idif][ichip][ichan][i_iDAC][i_threshold]);
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


 
