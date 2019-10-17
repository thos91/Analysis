// system includes
#include <string>
#include <vector>
#include <fstream>
#include <map>

// boost includes
#include <boost/filesystem.hpp>

// ROOT includes
#include <TCanvas.h>
#include <TImage.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TF1.h>
#include <TString.h>

// user includes
#include "wgFileSystemTools.hpp"
#include "wgEditXML.hpp"
#include "wgLogger.hpp"
#include "wgTopology.hpp"
#include "wgScurve.hpp"

using namespace wagasci_tools;

bool numeric_string_compare(const std::string& s1, const std::string& s2)
{
  unsigned n1 = extractIntegerFromString(GetName(s1));
  unsigned n2 = extractIntegerFromString(GetName(s2));
  if (n1 == UINT_MAX || n2 == UINT_MAX) {
    std::string::const_iterator it1 = s1.begin(), it2 = s2.begin();
    return std::lexicographical_compare(it1, s1.end(), it2, s2.end());
  }
  if (n1 != n2) return n1 < n2;
  else return false;
}


//******************************************************************
int wgScurve(const char* x_inputDir,
             const char* x_outputXMLDir,
             const char* x_outputIMGDir,
             const bool compatibility_mode) {

  // ============================================================= //
  //                                                               //
  //                        Parse Arguments                        //
  //                                                               //
  // ============================================================= //
  
  std::string inputDir    (x_inputDir);
  std::string outputXMLDir(x_outputXMLDir);
  std::string outputIMGDir(x_outputIMGDir);

  wgEnvironment env;
  wgEditXML Edit;

  // ============ Check directories ============ //
  if (inputDir.empty() || !check_exist::Dir(inputDir)) { 
    Log.eWrite("[wgScurve] Input directory " + inputDir + " doesn't exist");
    exit(1);
  }
  if (outputXMLDir.empty()) {
    outputXMLDir = env.CALIBDATA_DIRECTORY;
  }
  if (outputIMGDir.empty()) {
    outputIMGDir = env.IMGDATA_DIRECTORY;
  }

  // ============ Create outputXMLDir ============ //
  try { MakeDir(outputXMLDir); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[wgScurve] " + std::string(e.what()));
    return ERR_FAILED_CREATE_DIRECTORY;
  }

  // ============ Create outputIMGDir ============ //
  try { MakeDir(outputIMGDir); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[wgScurve] " + std::string(e.what()));
    return ERR_FAILED_CREATE_DIRECTORY;
  }
  Log.Write("[wgScurve] *****  READING DIRECTORY      : " + inputDir + "  *****");
  Log.Write("[wgScurve] *****  OUTPUT XML DIRECTORY   : " + outputXMLDir + "  *****");
  Log.Write("[wgScurve] *****  OUTPUT IMAGE DIRECTORY : " + outputIMGDir + "  *****");

  try{

    /********************************************************************************
     *                  Get directory tree and set variables                       *
     ********************************************************************************/
        
    // Get topology from input directory.
    Topology topol(inputDir, TopologySourceType::scurve_tree);

    // Get the number imputDAC and threshold
    std::vector<std::string> list_dir = ListDirectoriesWithInteger(inputDir);
    const unsigned n_inputDAC  = list_dir.size();
    const unsigned n_threshold = HowManyDirectories(list_dir.at(0));
    unsigned n_difs = topol.n_difs;
    
    // inputDAC[] and threshold[] contain true values for each index.
    // Be careful that i_iDAC and i_threshold are only the index 
    // to get the true value in the code. We may be able to get 
    // inputDAC and threshold values from the filename, but we will
    // obtain them by reading the xml file. It is better not to depend
    // on system environment as far as possible.
    u1vector inputDAC(n_inputDAC);
    u1vector threshold(n_threshold);

    // Define variables for storing values
    d3vector slope1     (n_difs); // [dif][chip][chan] optimized threshold at 1.5 pe vs input DAC fit : slope
    d3vector slope2     (n_difs); // [dif][chip][chan] optimized threshold at 2.5 pe vs input DAC fit : slope
    d3vector intercept1 (n_difs); // [dif][chip][chan] optimized threshold at 1.5 pe vs input DAC fit : intercept
    d3vector intercept2 (n_difs); // [dif][chip][chan] optimized threshold at 2.5 pe vs input DAC fit : intercept
    d4vector pe1        (n_difs); // [dif][chip][chan][iDAC] optimized threshold at 1.5 p.e.
    d4vector pe2        (n_difs); // [dif][chip][chan][iDAC] optimized threshold at 2.5 p.e.
    d5vector noise      (n_difs); // [dif][chip][chan][iDAC][thr] dark noise count
    d5vector noise_sigma(n_difs); // [dif][chip][chan][iDAC][thr] dark noise count error
		double  mean1PE[n_inputDAC], mean2PE[n_inputDAC], sigma1PE[n_inputDAC], sigma2PE[n_inputDAC];
    //  and resize them.
    unsigned dif_counter = 0;
    u1vector dif_counter_to_id;
    for (const auto &dif : topol.dif_map) {
      dif_counter_to_id.push_back(dif.first);
      unsigned n_chips = dif.second.size();
      slope1     [dif_counter].resize(n_chips);
      slope2     [dif_counter].resize(n_chips);
      intercept1 [dif_counter].resize(n_chips);
      intercept2 [dif_counter].resize(n_chips);
      pe1        [dif_counter].resize(n_chips);
      pe2        [dif_counter].resize(n_chips);
      noise      [dif_counter].resize(n_chips);
      noise_sigma[dif_counter].resize(n_chips);
      for (const auto &asu : dif.second) {
        unsigned n_chans = asu.second;
        slope1     [dif_counter][asu.first].resize(n_chans);
        slope2     [dif_counter][asu.first].resize(n_chans);
        intercept1 [dif_counter][asu.first].resize(n_chans);
        intercept2 [dif_counter][asu.first].resize(n_chans);
        pe1        [dif_counter][asu.first].resize(n_chans);
        pe2        [dif_counter][asu.first].resize(n_chans);
        noise      [dif_counter][asu.first].resize(n_chans);
        noise_sigma[dif_counter][asu.first].resize(n_chans);
        for (unsigned ichan = 0; ichan < n_chans; ++ichan) {
          pe1        [dif_counter][asu.first][ichan].resize(n_inputDAC);
          pe2        [dif_counter][asu.first][ichan].resize(n_inputDAC);
          noise      [dif_counter][asu.first][ichan].resize(n_inputDAC);
          noise_sigma[dif_counter][asu.first][ichan].resize(n_inputDAC);
          for (unsigned i_iDAC = 0; i_iDAC < n_inputDAC; ++i_iDAC) {
            noise      [dif_counter][asu.first][ichan][i_iDAC].resize(n_threshold);
            noise_sigma[dif_counter][asu.first][ichan][i_iDAC].resize(n_threshold);
          }
        }
      }
      ++dif_counter;
    }

    /********************************************************************************
     *                              Read XML files                                  *
     ********************************************************************************/

    // input DAC
    std::vector<std::string> iDAC_dir_list = ListDirectoriesWithInteger(inputDir);
    std::sort(iDAC_dir_list.begin(), iDAC_dir_list.end(), numeric_string_compare);
    unsigned iDAC_counter = 0;  // index for inputDAC
    for (auto const & iDAC_dir : iDAC_dir_list) {
        unsigned iDAC_from_dir = extractIntegerFromString(GetName(iDAC_dir));
        
      // threshold
      std::vector<std::string> th_dir_list = ListDirectoriesWithInteger(iDAC_dir);
      std::sort(th_dir_list.begin(), th_dir_list.end(), numeric_string_compare);
      unsigned threshold_counter = 0;  // index for threshold
      for (auto & th_dir : th_dir_list) {
        unsigned th_from_dir = extractIntegerFromString(GetName(th_dir));
        
        // DIF
        th_dir += "/wgAnaHistSummary/Xml";
        std::vector<std::string> dif_dir_list = ListDirectories(th_dir);
        std::sort(dif_dir_list.begin(), dif_dir_list.end(), numeric_string_compare);
        unsigned dif_counter = 0;
        for (auto const & dif_dir : dif_dir_list) {

          // chip
          std::vector<std::string> chip_xml_list = ListFilesWithExtension(dif_dir, "xml");
          std::sort(chip_xml_list.begin(), chip_xml_list.end(), numeric_string_compare);
          for (auto const & chip_xml : chip_xml_list) {
            unsigned chip_counter = extractIntegerFromString(GetName(chip_xml));
            
            // ************* Open XML file ************* //
            try { Edit.Open(chip_xml); }
            catch (const std::exception& e) {
              Log.eWrite("[wgScurve] " + std::string(e.what()));
              return ERR_FAILED_OPEN_XML_FILE;
            }
            
            // ************* Sanity checks ************* //

            if (!compatibility_mode) {
              threshold[threshold_counter] = Edit.SUMMARY_GetGlobalConfigValue("trigth");
              if (threshold[threshold_counter] != th_from_dir)
                throw std::runtime_error("Threshold value from directory ( " +
                                         std::to_string(th_from_dir) + " ) different from the one from "
                                         "XML file : " + std::to_string(threshold[threshold_counter]));
              
              unsigned dif_id_from_xml = Edit.SUMMARY_GetGlobalConfigValue("difid");
              if (dif_id_from_xml != dif_counter_to_id[dif_counter])
                throw std::runtime_error("DIF ID value from directory ( " +
                                         std::to_string(dif_counter_to_id[dif_counter]) +
                                         " ) different from the one from XML file : "
                                         + std::to_string(dif_id_from_xml));
            } else {
              inputDAC[iDAC_counter] = iDAC_from_dir;
              threshold[threshold_counter] = th_from_dir;
            }

            // ************* Read XML file ************* //
            
            unsigned n_channels = topol.dif_map[dif_counter_to_id[dif_counter]][chip_counter];
            for (unsigned chan_counter = 0; chan_counter < n_channels; ++chan_counter) {
              // inputDAC
              if (!compatibility_mode) {
                inputDAC[iDAC_counter] = Edit.SUMMARY_GetChConfigValue("inputDAC", chan_counter);
                if (inputDAC[iDAC_counter] != iDAC_from_dir)
                  throw std::runtime_error("Threshold value from directory ( " +
                                           std::to_string(th_from_dir) + " ) different from the one from "
                                           "XML file : " + std::to_string(threshold[threshold_counter]));
              }
              // dark noise rate
              noise[dif_counter][chip_counter][chan_counter][iDAC_counter][threshold_counter] =
                  Edit.SUMMARY_GetChFitValue("noise_rate", chan_counter);
              // dark noise rate sigma
              noise_sigma[dif_counter][chip_counter][chan_counter][iDAC_counter][threshold_counter] =
                  Edit.SUMMARY_GetChFitValue("sigma_rate", chan_counter);
            }
            Edit.Close();
            
          } // chip
          ++dif_counter;
        } // dif
        ++threshold_counter;
      } // threshold
      ++iDAC_counter;
    } // inputDAC
  	Log.Write("[wgScurve] Reading Xml Files done.");

    /********************************************************************************
     *                        Draw and fit the S-curve                              *
     ********************************************************************************/

    TH1D* Pe1Hist[n_inputDAC];
    TH1D* Pe2Hist[n_inputDAC];
    TH1D* ChiHist[n_inputDAC];
    TH1D* ChiOverNdfHist[n_inputDAC];
		for(unsigned i_iDAC = 0; i_iDAC < n_inputDAC; ++i_iDAC){
			std::string name1 = "Pe1Hist_" + std::to_string(inputDAC[i_iDAC]);
			std::string name2 = "Pe2Hist_" + std::to_string(inputDAC[i_iDAC]);
			std::string name3 = "ChiSquareHist_" + std::to_string(inputDAC[i_iDAC]);
			std::string name4 = "ChiSquareOverNdfHist_" + std::to_string(inputDAC[i_iDAC]);
      Pe1Hist[i_iDAC] = new TH1D(name1.c_str(),"1.5 and 2.5 p.e. Cut Level Distribution; Threshold; # of Channels",100,100,200);
      Pe2Hist[i_iDAC] = new TH1D(name2.c_str(),"Pe2Hist",100,100,200);
      ChiHist[i_iDAC] = new TH1D(name3.c_str(),"Chi Square; Chi square; Count",100,0,1000);
      ChiOverNdfHist[i_iDAC] = new TH1D(name4.c_str(),"Chi Square / Ndf; Chi square / Ndf; Count",100,0,100);
      ChiHist[i_iDAC]->SetStats(0);
      ChiOverNdfHist[i_iDAC]->SetStats(0);
		  Pe1Hist[i_iDAC]->SetStats(0);
      Pe1Hist[i_iDAC]->SetFillColor(kRed);
      Pe1Hist[i_iDAC]->SetFillStyle(3002);
      Pe2Hist[i_iDAC]->SetFillColor(kBlue);
      Pe2Hist[i_iDAC]->SetFillStyle(3004);
		}

    for (unsigned idif = 0; idif < n_difs; ++idif) {
      for (const auto &asu : topol.dif_map[dif_counter_to_id[idif]]) {
        unsigned ichip = asu.first;
        for (unsigned ichan = 0; ichan < asu.second; ++ichan) {
          std::string image_dir = outputIMGDir +
                                  "/Dif" + std::to_string(dif_counter_to_id[idif]) +
                                  "/Chip" + std::to_string(ichip) +
                                  "/Channel" + std::to_string(ichan);
          MakeDir(image_dir);
          // If the channel does not contain the meaningful data but UNIT_MAX, skip the loop.
          if (noise[idif][ichip][ichan][0][0] == UINT_MAX) {
            pe1[idif][ichip][ichan].push_back(UINT_MAX);
            pe2[idif][ichip][ichan].push_back(UINT_MAX);
            slope1    [idif][ichip].push_back(UINT_MAX);
            intercept1[idif][ichip].push_back(UINT_MAX);
            slope2    [idif][ichip].push_back(UINT_MAX);
            intercept2[idif][ichip].push_back(UINT_MAX);
            break;
          }

          for (unsigned i_iDAC = 0; i_iDAC < n_inputDAC; ++i_iDAC) {
                                        
            TCanvas *c1 = new TCanvas("c1", "c1");
            c1->SetLogy();
            // These are temporary variables for x, y and their errors used to draw the graph.
            d1vector gx, gy, gxe, gye;

            for (unsigned i_threshold = 0; i_threshold < n_threshold; ++i_threshold) {
              gx.push_back(threshold[i_threshold]);
              gy.push_back(noise[idif][ichip][ichan][i_iDAC][i_threshold]);
              gxe.push_back(0);
              gye.push_back(noise_sigma[idif][ichip][ichan][i_iDAC][i_threshold]);
            }
                                                
            // ************* Draw S-curve Graph ************* //
            TGraphErrors* Scurve = new TGraphErrors(gx.size(), gx.data(), gy.data(),
                                                    gxe.data(), gye.data());
            TString title("Dif" + std::to_string(dif_counter_to_id[idif])
                          + "_Chip" + std::to_string(ichip)
                          + "_Channel" + std::to_string(ichan)
                          + "_InputDAC" + std::to_string(inputDAC[i_iDAC]) 
                          + ";Threshold;Noise rate");
            Scurve->SetTitle(title);
            Scurve->Draw("ap*");

            // ************* Fit S-curve ************* //
            double high = 1.0E+5;
            double low  = 1.0E+2;
            double pe1_t, pe2_t;
            fit_scurve(Scurve, pe1_t, pe2_t, idif, ichip, ichan, inputDAC[i_iDAC],
                       low, high, outputIMGDir, false);
            fit_scurve(Scurve, pe1_t, pe2_t, ChiSquare, NDF, idif, ichip, ichan, inputDAC[i_iDAC],
                       outputIMGDir, false);
            pe1[idif][ichip][ichan][i_iDAC] = pe1_t;
            pe2[idif][ichip][ichan][i_iDAC] = pe2_t;
            Pe1Hist[i_iDAC]->Fill(pe1_t);
            Pe2Hist[i_iDAC]->Fill(pe2_t);
            ChiHist[i_iDAC]->Fill(ChiSquare);
            ChiOverNdfHist[i_iDAC]->Fill(ChiSquare/(double)NDF);

            // ************* Save S-curve Graph as png ************* //
            TString image(outputIMGDir + "/Dif" + std::to_string(dif_counter_to_id[idif])
                          + "/Chip" + std::to_string(ichip) + "/Channel" + std::to_string(ichan)
                          + "/InputDAC" + std::to_string(inputDAC[i_iDAC]) + ".png");
            c1->Print(image);
                                                
            delete Scurve;
            delete c1;
          } // inputDAC

          // ************ Linear fit of 1.5pe optimized threshold vs inputDAC plot ************ //
          TCanvas *c2 = new TCanvas("c2","c2");
                                        
          // These are temporary variables for x, y used to draw the graph.
          d1vector gx, gy1, gy2;
          for (unsigned i_iDAC = 0; i_iDAC < n_inputDAC; ++i_iDAC) {
            gx.push_back(inputDAC[i_iDAC]);
            gy1.push_back(pe1[idif][ichip][ichan][i_iDAC]);
            gy2.push_back(pe2[idif][ichip][ichan][i_iDAC]);
          }
          TGraph* PELinear1 = new TGraph(gx.size(), gx.data(), gy1.data());
          TString title1("Dif" + std::to_string(dif_counter_to_id[idif])
                         + "_Chip" + std::to_string(ichip)
                         + "_Channel" + std::to_string(ichan)
                         + ";InputDAC;1.5 pe threshold");
          PELinear1->SetTitle(title1);
          PELinear1->Draw("ap*");

          TF1* fit1 = new TF1("fit1", "[0]*x + [1]", 1, 241);
          fit1->SetParameters(-0.01, 170);
          PELinear1->Fit(fit1, "rlq");
          fit1->Draw("same");
          slope1    [idif][ichip][ichan] = fit1->GetParameter(0);
          intercept1[idif][ichip][ichan] = fit1->GetParameter(1);
        
          // ************* Save plot as png ************* //
          TString image1(outputIMGDir + "/Dif" + std::to_string(dif_counter_to_id[idif])
                         + "/Chip" + std::to_string(ichip) + "/Channel" + std::to_string(ichan)
                         + "/PE1vsInputDAC.png");
          c2->Print(image1);
          c2->Clear();

          // ************ Linear fit of 2.5pe optimized threhold vs inputDAC plot ************ //
          TGraph* PELinear2 = new TGraph(gx.size(), gx.data(), gy2.data());
          TString title2("Dif" + std::to_string(dif_counter_to_id[idif])
                         + "_Chip" + std::to_string(ichip)
                         + "_Channel" + std::to_string(ichan)
                         + ";InputDAC;2.5 pe threshold");
          PELinear2->SetTitle(title2);
          PELinear2->Draw("ap*");

          TF1* fit2 = new TF1("fit2", "[0]*x + [1]", 1, 241);
          fit2->SetParameters(-0.01, 170);
          PELinear2->Fit(fit2,"rlq");
          fit2->Draw("same");
          slope2    [idif][ichip][ichan] = fit2->GetParameter(0);
          intercept2[idif][ichip][ichan] = fit2->GetParameter(1);

          // ************* Save plot as png ************* //
          TString image2(outputIMGDir + "/Dif" + std::to_string(dif_counter_to_id[idif])
                         + "/Chip" + std::to_string(ichip) + "/Channel" + std::to_string(ichan)
                         + "/PE2vsInputDAC.png");
          c2->Print(image2);
                                        
          delete PELinear1;
          delete PELinear2;
          delete fit1;
          delete fit2;
          delete c2;
        } // channel
      } // chip
  		Log.Write("[wgScurve] Fitting DIF = " +  std::to_string(idif) + " done.");
    } // dif
		
		// Draw p.e. distribution histgrams for each inputDAC and
		// save them under the inputIMGDir.
    std::string pe_dir = outputIMGDir + "/EvaluationOfFit";
    MakeDir(pe_dir);
    for(unsigned i_iDAC = 0; i_iDAC < n_inputDAC; ++i_iDAC){
      TCanvas* PECanvas = new TCanvas("PECanvas","PECanvas");
      TCanvas* ChiCanvas = new TCanvas("ChiCanvas","ChiCanvas");
      TCanvas* ChiOverNdfCanvas = new TCanvas("ChiOverNdfCanvas","ChiOverNdfCanvas");
      ChiOverNdfCanvas->SetLogy();
      // PE distribution
      TF1* Pe1Fit = new TF1("Pe1Fit","gaus",100,200);
      TF1* Pe2Fit = new TF1("Pe2Fit","gaus",100,200);
      PECanvas->cd();
      Pe1Hist[i_iDAC]->Draw();
      Pe2Hist[i_iDAC]->Draw("same");
      Pe1Hist[i_iDAC]->Fit(Pe1Fit,"rlq");
      Pe2Hist[i_iDAC]->Fit(Pe2Fit,"rlq");
      mean1PE[i_iDAC] = Pe1Fit->GetParameter(1); sigma1PE[i_iDAC] = Pe1Fit->GetParameter(2);
      mean2PE[i_iDAC] = Pe2Fit->GetParameter(1); sigma2PE[i_iDAC] = Pe2Fit->GetParameter(2);
      TString name(outputIMGDir + "/EvaluationOfFit/iDAC_" + std::to_string(inputDAC[i_iDAC]) +  ".png");
      PECanvas->Print(name);
      // Chi square distribution
      ChiCanvas->cd();
      ChiHist[i_iDAC]->Draw();
      name = outputIMGDir + "/EvaluationOfFit/Chisquare_iDAC_" + std::to_string(inputDAC[i_iDAC]) +  ".png";
      ChiCanvas->Print(name);
      // Chi square over ndf distribution
      ChiOverNdfCanvas->cd();
      ChiOverNdfHist[i_iDAC]->Draw();
      name = outputIMGDir + "/EvaluationOfFit/ChisquareOverNdf_iDAC_" + std::to_string(inputDAC[i_iDAC]) +  ".png";
      ChiOverNdfCanvas->Print(name);
      delete Pe1Fit;
      delete Pe2Fit;
      delete PECanvas;
      delete ChiCanvas;
      delete ChiOverNdfCanvas;
      delete Pe1Hist[i_iDAC];
      delete Pe2Hist[i_iDAC];
      delete ChiHist[i_iDAC];
      delete ChiOverNdfHist[i_iDAC];
    }

    /********************************************************************************
     *                           threshold_card.xml                                 *
     ********************************************************************************/

		std::ofstream fout(outputXMLDir + "/failed_channels.txt");
    std::string xmlfile(outputXMLDir + "/threshold_card.xml");

    try {
      Edit.OPT_Make(xmlfile, inputDAC, topol.dif_map);
      Edit.Open(xmlfile);
    }
    catch (const wgInvalidFile & e) {
      Log.eWrite("[wgScurve] " + xmlfile + " : " + e.what());
      return ERR_FAILED_OPEN_XML_FILE;
    }

    for (unsigned idif = 0; idif < n_difs; ++idif) {
      for (unsigned ichip = 0; ichip < topol.dif_map[idif].size(); ++ichip) {
        for (unsigned ichan = 0; ichan < topol.dif_map[idif][ichip]; ++ichan) {
          // Set the slope and intercept values for the result of fitting threshold-inputDAC plot.
          Edit.OPT_SetChanValue(std::string("slope_threshold1"),     dif_counter_to_id[idif], ichip,
                                ichan, slope1    [idif][ichip][ichan], NO_CREATE_NEW_MODE);
          Edit.OPT_SetChanValue(std::string("intercept_threshold1"), dif_counter_to_id[idif], ichip,
                                ichan, intercept1[idif][ichip][ichan], NO_CREATE_NEW_MODE);
          Edit.OPT_SetChanValue(std::string("slope_threshold2"),     dif_counter_to_id[idif], ichip,
                                ichan, slope2    [idif][ichip][ichan], NO_CREATE_NEW_MODE);
          Edit.OPT_SetChanValue(std::string("intercept_threshold2"), dif_counter_to_id[idif], ichip,
                                ichan, intercept2[idif][ichip][ichan], NO_CREATE_NEW_MODE);

          for (unsigned i_iDAC = 0; i_iDAC < n_inputDAC; ++i_iDAC) {
            // Set the 2.5 pe and 1.5 pe level after fitting the scurve.
            Edit.OPT_SetValue(std::string("threshold_1"), dif_counter_to_id[idif], ichip, ichan,
                              inputDAC[i_iDAC], pe1[idif][ichip][ichan][i_iDAC], NO_CREATE_NEW_MODE);
            Edit.OPT_SetValue(std::string("threshold_2"), dif_counter_to_id[idif], ichip, ichan,
                              inputDAC[i_iDAC], pe2[idif][ichip][ichan][i_iDAC], NO_CREATE_NEW_MODE);
            // If 1.5 or 2.5 pe level is far from the mean value by 2-sigma, 
            // it will recorded in "failed_channels.txt" with the number of 
            // DIF, CHIP, CHANNEL, InputDAC.
            if( std::abs(pe1[idif][ichip][ichan][i_iDAC] - mean1PE[i_iDAC]) > 2*sigma1PE[i_iDAC] ||
                std::abs(pe2[idif][ichip][ichan][i_iDAC] - mean2PE[i_iDAC]) > 2*sigma2PE[i_iDAC]  ){
              fout << idif << "    " << ichip << "    " << ichan << "    " << inputDAC[i_iDAC] << std::endl;
            }
          }
        }
      }
    }
		fout.close();
    Edit.Write();
    Edit.Close();
  }  // end try
  catch (const std::exception& e){
    Log.eWrite("[wgScurve][" + inputDir + "] " + std::string(e.what()));
    return ERR_WG_SCURVE;
  }
  Log.Write("[wgScurve] Writing Xml file done.");
  return WG_SUCCESS;
}

//**********************************************************************
void fit_scurve(TGraphErrors* Scurve, 
                double& pe1_t, 
                double& pe2_t, 
                double& ChiSquare,
                int&    NDF,
                unsigned idif, 
                unsigned ichip, 
                unsigned ichan, 
                unsigned inputDAC,
                double low,
                double high,
                std::string outputIMGDir, 
                bool print_flag) {

  // Fitting function for Scurve is summation of two sigmoid functions (and a constant).
  const char * fit_function = "[0]/(1+exp(-[1]*(x-[2]))) + [3]/(1+exp(-[4]*(x-[5]))) + [6]";
  double middle = exp((log(low)+log(high))/2);
  double c0 = high, c1 = 0.5, c2 = 155, c3 = middle, c4 = 0.5, c5 = 135, c6 = low;

  TF1* fit_scurve = new TF1("fit_scurve", fit_function, 120, 170);
  fit_scurve->SetParameters(c0, c1, c2, c3, c4, c5, c6);
  fit_scurve->SetParLimits(0, high/1.5,      high*1.5);  
  //fit_scurve->SetParLimits(1, 0.35,        1.0);
  //fit_scurve->SetParLimits(2, 145,         165);
  fit_scurve->SetParLimits(3, middle/1.5,  middle*1.5); 
  //fit_scurve->SetParLimits(4, 0.35,        1.0);
  //fit_scurve->SetParLimits(5, 125,         145); 
  fit_scurve->SetParLimits(6, low/1.5,     low*4.0);
  
  Scurve->Fit(fit_scurve, "q");
        
  // From the fitting parameters, calcurate each p.e. level.
  // Here, pe1 -> 1.5 pe threshold, pe2 -> 2.5 pe threshold.
  // variables a and b indicates the center point of each sigmoid function.
  // Set 1.5 pe as the middle point between two sigmoid functions' center.
  double a = fit_scurve->GetParameter(5);
  double b = fit_scurve->GetParameter(2);
  ChiSquare = fit_scurve->GetChisquare();
  NDF = fit_scurve->GetNDF();
  pe1_t =  (  a + b) / 2;
  pe2_t =  (3*a - b) / 2;

  if( print_flag && (!outputIMGDir.empty()) ) {
    TString image(outputIMGDir + "/Dif" + std::to_string(idif) + "/Chip" + std::to_string(ichip) +
                  "/Channel" + std::to_string(ichan) + "/InputDAC" + std::to_string(inputDAC) +
                  "_fit.png");
    TCanvas * canvas = new TCanvas("canvas", "canvas");
    canvas->SetCanvasSize(1024, 768);
    canvas->SetLogy();
    Scurve->Draw("ap*");
    fit_scurve->Draw("same");
    canvas->Print(image);
    delete canvas; 
  }
  delete fit_scurve;
}
