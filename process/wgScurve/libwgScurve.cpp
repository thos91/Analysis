// system includes
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <time.h>

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
#include <TGaxis.h>

// user includes
#include "wgFileSystemTools.hpp"
#include "wgErrorCodes.hpp"
#include "wgEditXML.hpp"
#include "wgLogger.hpp"
#include "wgTopology.hpp"
#include "wgScurve.hpp"

using namespace wagasci_tools;

//#define LOG_SCURVE 1 // comment out when not using log-scale

//#define LOG_SCURVE 1 // comment out when not using log-scale

//******************************************************************
int wgScurve(const char* x_input_dir,
             const char* x_output_xml_dir,
             const char* x_output_img_dir,
             const bool compatibility_mode) {

  // ============================================================= //
  //                                                               //
  //                        Parse Arguments                        //
  //                                                               //
  // ============================================================= //
  
  std::string input_dir    (x_input_dir);
  std::string output_xml_dir(x_output_xml_dir);
  std::string output_img_dir(x_output_img_dir);

  wgEnvironment env;
  wgEditXML Edit;

  // ============ Check directories ============ //
  if (input_dir.empty() || !check_exist::directory(input_dir)) { 
    Log.eWrite("[wgScurve] Input directory " + input_dir + " doesn't exist");
    exit(1);
  }
  if (output_xml_dir.empty()) {
    output_xml_dir = env.CALIBDATA_DIRECTORY;
  }
  if (output_img_dir.empty()) {
    output_img_dir = env.IMGDATA_DIRECTORY;
  }

  // ============ Create output_xml_dir ============ //
  try { make::directory(output_xml_dir); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[wgScurve] " + std::string(e.what()));
    return ERR_FAILED_CREATE_DIRECTORY;
  }

  // ============ Create output_img_dir ============ //
  try { make::directory(output_img_dir); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[wgScurve] " + std::string(e.what()));
    return ERR_FAILED_CREATE_DIRECTORY;
  }
  Log.Write("[wgScurve] *****  READING DIRECTORY      : " + input_dir + "  *****");
  Log.Write("[wgScurve] *****  OUTPUT XML DIRECTORY   : " + output_xml_dir + "  *****");
  Log.Write("[wgScurve] *****  OUTPUT IMAGE DIRECTORY : " + output_img_dir + "  *****");

  try{

    /********************************************************************************
     *                  Get directory tree and set variables                       *
     ********************************************************************************/
        
    // Get topology from input directory.
    Topology topol(input_dir, TopologySourceType::scurve_tree);

    // Get the number imputDAC and threshold
    std::vector<std::string> list_dir = list::list_directories(input_dir, true);
    const unsigned n_inputDAC  = list_dir.size();
    const unsigned n_threshold = list::how_many_directories(list_dir.at(0), true);
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
    d3vector slope1     (n_difs); // [dif][chip][chan] optimized threshold at 0.5 pe vs input DAC fit : slope
    d3vector slope2     (n_difs); // [dif][chip][chan] optimized threshold at 1.5 pe vs input DAC fit : slope
    d3vector slope3     (n_difs); // [dif][chip][chan] optimized threshold at 2.5 pe vs input DAC fit : slope
    d3vector intercept1 (n_difs); // [dif][chip][chan] optimized threshold at 0.5 pe vs input DAC fit : intercept
    d3vector intercept2 (n_difs); // [dif][chip][chan] optimized threshold at 1.5 pe vs input DAC fit : intercept
    d3vector intercept3 (n_difs); // [dif][chip][chan] optimized threshold at 2.5 pe vs input DAC fit : intercept
    d4vector pe1        (n_difs); // [dif][chip][chan][iDAC] optimized threshold at 0.5 p.e.
    d4vector pe2        (n_difs); // [dif][chip][chan][iDAC] optimized threshold at 1.5 p.e.
    d4vector pe3        (n_difs); // [dif][chip][chan][iDAC] optimized threshold at 2.5 p.e.
    d5vector noise      (n_difs); // [dif][chip][chan][iDAC][thr] dark noise count
    d5vector noise_sigma(n_difs); // [dif][chip][chan][iDAC][thr] dark noise count error
    double meanPE[n_inputDAC][3], mean1PE[n_inputDAC][3], mean2PE[n_inputDAC][3], mean3PE[n_inputDAC][3];
    double sigmaPE[n_inputDAC][3], sigma1PE[n_inputDAC][3], sigma2PE[n_inputDAC][3], sigma3PE[n_inputDAC][3];
    double meanSlope1, meanSlope2, meanSlope3;
    double meanIntercept1, meanIntercept2, meanIntercept3;
    s4vector bestFit    (n_difs);
    //  and resize them.
    unsigned dif_counter = 0;
    u1vector dif_counter_to_id;
    for (const auto &dif : topol.dif_map) {
      dif_counter_to_id.push_back(dif.first);
      unsigned n_chips = dif.second.size();
      slope1     [dif_counter].resize(n_chips);
      slope2     [dif_counter].resize(n_chips);
      slope3     [dif_counter].resize(n_chips);
      intercept1 [dif_counter].resize(n_chips);
      intercept2 [dif_counter].resize(n_chips);
      intercept3 [dif_counter].resize(n_chips);
      pe1        [dif_counter].resize(n_chips);
      pe2        [dif_counter].resize(n_chips);
      pe3        [dif_counter].resize(n_chips);
      noise      [dif_counter].resize(n_chips);
      noise_sigma[dif_counter].resize(n_chips);
      bestFit    [dif_counter].resize(n_chips);
      for (const auto &asu : dif.second) {
        unsigned n_chans = asu.second;
        slope1     [dif_counter][asu.first].resize(n_chans);
        slope2     [dif_counter][asu.first].resize(n_chans);
        slope3     [dif_counter][asu.first].resize(n_chans);
        intercept1 [dif_counter][asu.first].resize(n_chans);
        intercept2 [dif_counter][asu.first].resize(n_chans);
        intercept3 [dif_counter][asu.first].resize(n_chans);
        pe1        [dif_counter][asu.first].resize(n_chans);
        pe2        [dif_counter][asu.first].resize(n_chans);
        pe3        [dif_counter][asu.first].resize(n_chans);
        noise      [dif_counter][asu.first].resize(n_chans);
        noise_sigma[dif_counter][asu.first].resize(n_chans);
        bestFit    [dif_counter][asu.first].resize(n_chans);
        for (unsigned ichan = 0; ichan < n_chans; ++ichan) {
          pe1        [dif_counter][asu.first][ichan].resize(n_inputDAC);
          pe2        [dif_counter][asu.first][ichan].resize(n_inputDAC);
          pe3        [dif_counter][asu.first][ichan].resize(n_inputDAC);
          noise      [dif_counter][asu.first][ichan].resize(n_inputDAC);
          noise_sigma[dif_counter][asu.first][ichan].resize(n_inputDAC);
          bestFit    [dif_counter][asu.first][ichan].resize(n_inputDAC);
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
    std::vector<std::string> iDAC_dir_list = list::list_directories(input_dir, true);
    unsigned iDAC_counter = 0;  // index for inputDAC
    for (auto const & iDAC_dir : iDAC_dir_list) {
      unsigned iDAC_from_dir = string::extract_integer(get_stats::basename(iDAC_dir));

      // threshold
      std::vector<std::string> th_dir_list = list::list_directories(iDAC_dir, true);
      unsigned threshold_counter = 0;  // index for threshold
      for (auto & th_dir : th_dir_list) {
        unsigned th_from_dir = string::extract_integer(get_stats::basename(th_dir));
        
        // DIF
        th_dir += "/wgAnaHistSummary/Xml";
        std::vector<std::string> dif_dir_list = list::list_directories(th_dir, true);
        unsigned dif_counter = 0;
        for (auto const & dif_dir : dif_dir_list) {

          // chip
          std::vector<std::string> chip_xml_list = list::list_files(dif_dir, true, ".xml");
          for (auto const & chip_xml : chip_xml_list) {
            unsigned chip_counter = string::extract_integer(get_stats::basename(chip_xml));
            
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

              unsigned noiserate = Edit.SUMMARY_GetChFitValue("noise_rate", chan_counter);
              unsigned noiseratesigma = Edit.SUMMARY_GetChFitValue("sigma_rate", chan_counter);
              if(noiserate == UINT_MAX || noiserate < 0 || std::isnan(noiserate)){
                noise[dif_counter][chip_counter][chan_counter][iDAC_counter][threshold_counter] = UINT_MAX;
                noise_sigma[dif_counter][chip_counter][chan_counter][iDAC_counter][threshold_counter] = UINT_MAX;
              }else{
#ifdef LOG_SCURVE
                /* Log-scaled version of Scurve */
                // log of dark noise rate
                noise[dif_counter][chip_counter][chan_counter][iDAC_counter][threshold_counter] =
                    std::log(noiserate);
                // log of dark noise rate sigma
                noise_sigma[dif_counter][chip_counter][chan_counter][iDAC_counter][threshold_counter] = 
                    (std::log(noiserate+noiseratesigma) - std::log(noiserate-noiseratesigma))/2;
#else 
                /* Not log-scaled version of Scurve */
                // dark noise rate
                noise[dif_counter][chip_counter][chan_counter][iDAC_counter][threshold_counter] =
                    Edit.SUMMARY_GetChFitValue("noise_rate", chan_counter);
                // dark noise rate sigma
                noise_sigma[dif_counter][chip_counter][chan_counter][iDAC_counter][threshold_counter] =
                    Edit.SUMMARY_GetChFitValue("sigma_rate", chan_counter);
#endif
              }
            } // chan
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

    // Define histgrams for drawing p.e. distribution and for chi-square distribution.
    // One for sum of all channels, and others(3) for each fitting patterns.
    // For chi-square histgram, one is just a chi-square and the other is chi-square/NDF.
    // NDF means "Number of Degrees of Freedom".
    TH1D* AllPeHist[n_inputDAC][3];
    TH1D* Pe1Hist[n_inputDAC][3];
    TH1D* Pe2Hist[n_inputDAC][3];
    TH1D* Pe3Hist[n_inputDAC][3];
    TH1D* ChiHist[n_inputDAC];
    TH1D* ChiOverNdfHist[n_inputDAC];
    for(unsigned i_iDAC = 0; i_iDAC < n_inputDAC; ++i_iDAC){
      std::string name1 = "Pe1Hist_" + std::to_string(inputDAC[i_iDAC]);
      std::string name2 = "Pe2Hist_" + std::to_string(inputDAC[i_iDAC]);
      std::string name3 = "Pe3Hist_" + std::to_string(inputDAC[i_iDAC]);
      std::string name4 = "ChiSquareHist_" + std::to_string(inputDAC[i_iDAC]);
      std::string name5 = "ChiSquareOverNdfHist_" + std::to_string(inputDAC[i_iDAC]);
      for(size_t i=0; i<3; i++){
        name1 = name1 + "_" + std::to_string(i);
        name2 = name2 + "_" + std::to_string(i);
        name3 = name3 + "_" + std::to_string(i);
        TString pe_hist_name = "P.E. Distribution (fit pattern : " + std::to_string(i) + "); Threshold; # of Channels";
        Pe1Hist[i_iDAC][i] = new TH1D(name1.c_str(),pe_hist_name,100,100,200);
        Pe2Hist[i_iDAC][i] = new TH1D(name2.c_str(),"Pe2Hist",100,100,200);
        Pe3Hist[i_iDAC][i] = new TH1D(name3.c_str(),"Pe3Hist",100,100,200);
        name1 = "AllPeHist_" + std::to_string(inputDAC[i_iDAC]) + "_" + std::to_string(i);
        AllPeHist[i_iDAC][i] = new TH1D(name1.c_str(),"P.E. Distribution; Threshold; # of Channels",100,100,200);
      }
      ChiHist[i_iDAC] = new TH1D(name4.c_str(),"Chi Square; Chi square; Count",100,0,500000);
      ChiOverNdfHist[i_iDAC] = new TH1D(name5.c_str(),"Chi Square / Ndf; Chi square / Ndf; Count",100,0,500000);
      ChiHist[i_iDAC]->SetStats(0);
      ChiOverNdfHist[i_iDAC]->SetStats(0);
      for(size_t i=0; i<3; i++){
        Pe1Hist[i_iDAC][i]->SetStats(0);
        Pe1Hist[i_iDAC][i]->SetFillColor(kRed);
        Pe1Hist[i_iDAC][i]->SetFillStyle(3002);
        Pe2Hist[i_iDAC][i]->SetFillColor(kBlue);
        Pe2Hist[i_iDAC][i]->SetFillStyle(3005);
        Pe3Hist[i_iDAC][i]->SetFillColor(kGreen);
        Pe3Hist[i_iDAC][i]->SetFillStyle(3006);
      }
      AllPeHist[i_iDAC][0]->SetStats(0);
      AllPeHist[i_iDAC][0]->SetFillColor(kRed);
      AllPeHist[i_iDAC][0]->SetFillStyle(3002);
      AllPeHist[i_iDAC][1]->SetFillColor(kBlue);
      AllPeHist[i_iDAC][1]->SetFillStyle(3005);
      AllPeHist[i_iDAC][2]->SetFillColor(kGreen);
      AllPeHist[i_iDAC][2]->SetFillStyle(3006);
    }
    
    // Draw s-curve, fit them, fill parameters and histgrams. 
    for (unsigned idif = 0; idif < n_difs; ++idif) {
      clock_t start = clock();
      for (const auto &asu : topol.dif_map[dif_counter_to_id[idif]]) {
        unsigned ichip = asu.first;
        for (unsigned ichan = 0; ichan < asu.second; ++ichan) {
          std::string image_dir = output_img_dir +
                                  "/Dif" + std::to_string(dif_counter_to_id[idif]) +
                                  "/Chip" + std::to_string(ichip) +
                                  "/Channel" + std::to_string(ichan);
          make::directory(image_dir);
          // If the channel does not contain the meaningful data but UNIT_MAX, skip the loop.
          if (noise[idif][ichip][ichan][0][0] == UINT_MAX) {
            for (unsigned i_iDAC = 0; i_iDAC < n_inputDAC; ++i_iDAC) {
              pe1[idif][ichip][ichan][i_iDAC] = UINT_MAX;
              pe2[idif][ichip][ichan][i_iDAC] = UINT_MAX;
              pe3[idif][ichip][ichan][i_iDAC] = UINT_MAX;
            }
            slope1    [idif][ichip][ichan] = UINT_MAX;
            intercept1[idif][ichip][ichan] = UINT_MAX;
            slope2    [idif][ichip][ichan] = UINT_MAX;
            intercept2[idif][ichip][ichan] = UINT_MAX;
            continue;
          }

          for (unsigned i_iDAC = 0; i_iDAC < n_inputDAC; ++i_iDAC) {
                                        
            TCanvas *c1 = new TCanvas("c1", "c1");
            c1->SetGrid(1,1);
#ifndef LOG_SCURVE
            c1->SetLogy();
#endif
            // These are temporary variables for x, y and their errors used to draw the graph.
            d1vector gx, gy, gxe, gye;
            unsigned max_bin_counter = 0;
            unsigned under10_counter = 0;
            unsigned under100_counter = 0;
            for (unsigned i_threshold = 0; i_threshold < n_threshold; ++i_threshold) {
              gx.push_back(threshold[i_threshold]);
              gy.push_back(noise[idif][ichip][ichan][i_iDAC][i_threshold]);
              gxe.push_back(0);
              gye.push_back(noise_sigma[idif][ichip][ichan][i_iDAC][i_threshold]);
              if(0.0 < noise[idif][ichip][ichan][i_iDAC][i_threshold] && 
                 noise[idif][ichip][ichan][i_iDAC][i_threshold] < 2.0E+5 && 
                 max_bin_counter < threshold[i_threshold]){
                max_bin_counter = threshold[i_threshold];
              }
              if(0.0 < noise[idif][ichip][ichan][i_iDAC][i_threshold] && 
                 noise[idif][ichip][ichan][i_iDAC][i_threshold] <= 10){ 
                under10_counter++;
              }
              if(0.0 < noise[idif][ichip][ichan][i_iDAC][i_threshold] && 
                 noise[idif][ichip][ichan][i_iDAC][i_threshold] <= 100){ 
                under100_counter++;
              }
            }
                                         
            // ************* Draw S-curve Graph ************* //
            TGraphErrors* Scurve = new TGraphErrors(gx.size(), gx.data(), gy.data(),
                                                    gxe.data(), gye.data());
            TGraphErrors* ScurveToDraw = new TGraphErrors(gx.size(), gx.data(), gy.data(),
                                                          gxe.data(), gye.data());
#ifdef LOG_SCURVE
            Scurve->GetHistogram()->SetMaximum(12);
            Scurve->GetHistogram()->SetMinimum(0.0);
#else
            Scurve->GetHistogram()->SetMaximum(2.0E+5);
            Scurve->GetHistogram()->SetMinimum(1.0);
            ScurveToDraw->GetHistogram()->SetMaximum(2.0E+5);
            ScurveToDraw->GetHistogram()->SetMinimum(1.0);
#endif
            TString title("Dif" + std::to_string(dif_counter_to_id[idif])
                          + "_Chip" + std::to_string(ichip)
                          + "_Channel" + std::to_string(ichan)
                          + "_InputDAC" + std::to_string(inputDAC[i_iDAC]) 
                          + ";Threshold;Noise rate [Hz]");
            ScurveToDraw->SetTitle(title);
            ScurveToDraw->Draw("ap*");
            
            if(dif_counter_to_id[idif] < 4) {
              TString image(output_img_dir + "/Dif" + std::to_string(dif_counter_to_id[idif])
                            + "/Chip" + std::to_string(ichip) + "/Channel" + std::to_string(ichan)
                            + "/InputDAC" + std::to_string(inputDAC[i_iDAC]) + ".png");
              c1->Print(image);
              pe1[idif][ichip][ichan][i_iDAC] = 0.0;
              pe2[idif][ichip][ichan][i_iDAC] = 0.0;
              pe3[idif][ichip][ichan][i_iDAC] = 0.0;
              delete Scurve;
              delete ScurveToDraw;
              delete c1;
            }else{
              // ************* Fit S-curve ************* //
              TF1* fit_func[3];
              const char* sigmoid_double = "[0]/(1+exp(-[1]*(x-[2]))) + [3]/(1+exp(-[4]*(x-[5]))) + [6]";
              const char* sigmoid_triple = "[0]/(1+exp(-[1]*(x-[2]))) + [3]/(1+exp(-[4]*(x-[5]))) + [6]/(1+exp(-[7]*(x-[8]))) + [9]";
              fit_func[0] = new TF1("fit_scurve1", sigmoid_double, 120, 170);
              fit_func[1] = new TF1("fit_scurve2", sigmoid_triple, 120, 170);
              fit_func[2] = new TF1("fit_scurve3", sigmoid_triple, 120, 170);
              double pe1_t[3], pe2_t[3], pe3_t[3];
              double ChiSquare[3];
              int NDF[3];
              fit_scurve1(Scurve, fit_func[0], pe1_t[0], pe2_t[0], pe3_t[0], ChiSquare[0], NDF[0], idif, ichip, ichan, inputDAC[i_iDAC],
                          output_img_dir, false);
              fit_scurve2(Scurve, fit_func[1], pe1_t[1], pe2_t[1], pe3_t[1], ChiSquare[1], NDF[1], idif, ichip, ichan, inputDAC[i_iDAC],
                          output_img_dir, false);
              fit_scurve3(Scurve, fit_func[2], pe1_t[2], pe2_t[2], pe3_t[2], ChiSquare[2], NDF[2], idif, ichip, ichan, inputDAC[i_iDAC],
                          output_img_dir, false);
              std::vector<double> goodness;
              std::vector<size_t> gn_index = {0, 1, 2};
              for(size_t i=0; i<3; i++){
                goodness.push_back(std::abs(1.0 - ChiSquare[i]/(double)NDF[i]));
              }
              if(under10_counter > 5 || under100_counter > 2){
              	goodness[0] = 999999.9;
              }
              std::sort(gn_index.begin(), gn_index.end(), [&goodness](size_t i1, size_t i2){
                return goodness[i1] < goodness[i2];
              });
              bestFit[idif][ichip][ichan][i_iDAC] = gn_index[0];
              size_t bestFit_t = bestFit[idif][ichip][ichan][i_iDAC];
              for(size_t i=0; i<3; i++){
                if(i != bestFit_t){
                  fit_func[i]->SetLineColor(kBlue);
                  fit_func[i]->SetLineStyle(2);
                  fit_func[i]->Draw("same");
                }
              }
              fit_func[bestFit_t]->Draw("same");
              pe1[idif][ichip][ichan][i_iDAC] = max_bin_counter;
              pe2[idif][ichip][ichan][i_iDAC] = pe2_t[bestFit_t];
              pe3[idif][ichip][ichan][i_iDAC] = pe3_t[bestFit_t];
              Pe1Hist[i_iDAC][bestFit_t]->Fill(max_bin_counter);
              Pe2Hist[i_iDAC][bestFit_t]->Fill(pe2_t[bestFit_t]);
              Pe3Hist[i_iDAC][bestFit_t]->Fill(pe3_t[bestFit_t]);
              AllPeHist[i_iDAC][0]->Fill(max_bin_counter);
              AllPeHist[i_iDAC][1]->Fill(pe2_t[bestFit_t]);
              AllPeHist[i_iDAC][2]->Fill(pe3_t[bestFit_t]);
              ChiHist[i_iDAC]->Fill(ChiSquare[bestFit_t]);
              ChiOverNdfHist[i_iDAC]->Fill(goodness[bestFit_t]);
              // Show each p.e. level line
              TGaxis *a1 = new TGaxis(max_bin_counter,1.0E+3,max_bin_counter,2.0E+5,0,0,0,"");
              TGaxis *a2 = new TGaxis(pe2_t[bestFit_t],1.0E+2,pe2_t[bestFit_t],2.0E+4,0,0,0,"");
              TGaxis *a3 = new TGaxis(pe3_t[bestFit_t],10,pe3_t[bestFit_t],1.0E+3,0,0,0,"");
              a1->SetLineColor(kGreen+1);
              a1->SetLineWidth(2);
              a2->SetLineColor(kGreen+1);
              a2->SetLineWidth(2);
              a3->SetLineColor(kGreen+1);
              a3->SetLineWidth(2);
              a1->Draw();
              a2->Draw();
              a3->Draw();
              // ************* Save S-curve Graph as png ************* //
              TString image(output_img_dir + "/Dif" + std::to_string(dif_counter_to_id[idif])
                            + "/Chip" + std::to_string(ichip) + "/Channel" + std::to_string(ichan)
                            + "/InputDAC" + std::to_string(inputDAC[i_iDAC]) + ".png");
              c1->Print(image);
                                                  
              delete Scurve;
              delete ScurveToDraw;
              for(size_t i=0; i<3; i++){
                delete fit_func[i];
              }
              delete c1;
            }
          } // inputDAC
          
        } // channel
      } // chip
      clock_t end = clock();
      const double time = static_cast<double>(end-start)/CLOCKS_PER_SEC;
      Log.Write("[wgScurve] Fitting DIF = " +  std::to_string(idif) + " done. (time = " + std::to_string(time) + " s)");
    } // dif
		
    // Draw p.e. distribution histgrams for each inputDAC and save them under the inputIMGDir.
    std::string pe_dir = output_img_dir + "/EvaluationOfFit";
    make::directory(pe_dir);
    for(unsigned i_iDAC = 0; i_iDAC < n_inputDAC; ++i_iDAC){
      // p.e. distribution for all channel.
      TCanvas* PeSumCanvas = new TCanvas("PeSumCanvas","PeSumCanvas");
      PeSumCanvas->cd();
      AllPeHist[i_iDAC][0]->Draw();
      AllPeHist[i_iDAC][1]->Draw("same");
      AllPeHist[i_iDAC][2]->Draw("same");
      TF1* AllPeFit[3];
      for(size_t i=0; i<3; i++){
        AllPeFit[i] = new TF1("AllPeFit","gaus",100,200);
        AllPeHist[i_iDAC][i]->Fit(AllPeFit[i],"rlq");
        meanPE[i_iDAC][i] = AllPeFit[i]->GetParameter(1); sigmaPE[i_iDAC][i] = AllPeFit[i]->GetParameter(2);
      }
      TString nameall = output_img_dir + "/EvaluationOfFit/PE_Distribution_All_iDAC" + std::to_string(inputDAC[i_iDAC]) +  ".png";
      PeSumCanvas->Print(nameall);


      // p.e. distribution for each fitting patterns.
      TCanvas* PECanvas[3];
      std::string name_pe = "PECanvas_inputDAC" + std::to_string(inputDAC[i_iDAC]);
      for(size_t i=0; i<3; i++){
        name_pe = name_pe + "_" + std::to_string(i);
        PECanvas[i] = new TCanvas(name_pe.c_str(),"PECanvas");
      }
      // p.e. distribution fitting and drawing.
      TF1* Pe1Fit[3];
      TF1* Pe2Fit[3];
      TF1* Pe3Fit[3];
      for(size_t i=0; i<3; i++){
        Pe1Fit[i] = new TF1("Pe1Fit","gaus",100,200);
        Pe2Fit[i] = new TF1("Pe2Fit","gaus",100,200);
        Pe3Fit[i] = new TF1("Pe3Fit","gaus",100,200);
        PECanvas[i]->cd();
        Pe1Hist[i_iDAC][i]->Draw();
        Pe2Hist[i_iDAC][i]->Draw("same");
        Pe3Hist[i_iDAC][i]->Draw("same");
        Pe1Hist[i_iDAC][i]->Fit(Pe1Fit[i],"rlq");
        Pe2Hist[i_iDAC][i]->Fit(Pe2Fit[i],"rlq");
        Pe3Hist[i_iDAC][i]->Fit(Pe3Fit[i],"rlq");
        mean1PE[i_iDAC][i] = Pe1Fit[i]->GetParameter(1); sigma1PE[i_iDAC][i] = Pe1Fit[i]->GetParameter(2);
        mean2PE[i_iDAC][i] = Pe2Fit[i]->GetParameter(1); sigma2PE[i_iDAC][i] = Pe2Fit[i]->GetParameter(2);
        mean3PE[i_iDAC][i] = Pe3Fit[i]->GetParameter(1); sigma3PE[i_iDAC][i] = Pe3Fit[i]->GetParameter(2);
      }
      TString name(output_img_dir + "/EvaluationOfFit/iDAC_" + std::to_string(inputDAC[i_iDAC]));
      for(size_t i=0; i<3; i++){
        TString name_pe_c = output_img_dir + "/EvaluationOfFit/iDAC_" + std::to_string(inputDAC[i_iDAC]) + "_fit" + std::to_string(i) + ".png";
        PECanvas[i]->Print(name_pe_c);
      }
      // Chi-square and chi-square/NDF distribution.
      TCanvas* ChiCanvas = new TCanvas("ChiCanvas","ChiCanvas");
      TCanvas* ChiOverNdfCanvas = new TCanvas("ChiOverNdfCanvas","ChiOverNdfCanvas");
      ChiCanvas->SetLogy();
      ChiOverNdfCanvas->SetLogy();
      ChiCanvas->cd();
      ChiHist[i_iDAC]->Draw();
      name = output_img_dir + "/EvaluationOfFit/Chisquare_iDAC_" + std::to_string(inputDAC[i_iDAC]) +  ".png";
      ChiCanvas->Print(name);
      ChiOverNdfCanvas->cd();
      ChiOverNdfHist[i_iDAC]->Draw();
      name = output_img_dir + "/EvaluationOfFit/ChisquareOverNdf_iDAC_" + std::to_string(inputDAC[i_iDAC]) +  ".png";
      ChiOverNdfCanvas->Print(name);
      // Delete all objects
      for(size_t i=0; i<3; i++){
        delete Pe1Fit[i];
        delete Pe2Fit[i];
        delete Pe3Fit[i];
        delete Pe1Hist[i_iDAC][i];
        delete Pe2Hist[i_iDAC][i];
        delete Pe3Hist[i_iDAC][i];
        delete PECanvas[i];
        delete AllPeHist[i_iDAC][i];
        delete AllPeFit[i];
      }
      delete PeSumCanvas;
      delete ChiCanvas;
      delete ChiOverNdfCanvas;
      delete ChiHist[i_iDAC];
      delete ChiOverNdfHist[i_iDAC];
    }

    // Calculate slope and intercept
    for (unsigned idif = 0; idif < n_difs; ++idif) {
      for (const auto &asu : topol.dif_map[dif_counter_to_id[idif]]) {
        unsigned ichip = asu.first;
        for (unsigned ichan = 0; ichan < asu.second; ++ichan) {
          if(dif_counter_to_id[idif] < 4){
            slope1    [idif][ichip][ichan] = 0.0;
            slope2    [idif][ichip][ichan] = 0.0;
            slope3    [idif][ichip][ichan] = 0.0;
            intercept1[idif][ichip][ichan] = 0.0;
            intercept2[idif][ichip][ichan] = 0.0;
            intercept3[idif][ichip][ichan] = 0.0;
          }else{
            TCanvas *c2 = new TCanvas("c2","c2");
            // These are temporary variables for x, y used to draw the graph.
            d1vector gx, gy1, gy2, gy3;
            for (unsigned i_iDAC = 0; i_iDAC < n_inputDAC; ++i_iDAC) {
              size_t bestFit_t = bestFit[idif][ichip][ichan][i_iDAC];
              gx.push_back(inputDAC[i_iDAC]);
              if( std::abs(pe1[idif][ichip][ichan][i_iDAC] - mean1PE[i_iDAC][bestFit_t]) > 2*sigma1PE[i_iDAC][bestFit_t] ||
                  std::abs(pe2[idif][ichip][ichan][i_iDAC] - mean2PE[i_iDAC][bestFit_t]) > 2*sigma2PE[i_iDAC][bestFit_t] ||
                  std::abs(pe3[idif][ichip][ichan][i_iDAC] - mean3PE[i_iDAC][bestFit_t]) > 2*sigma3PE[i_iDAC][bestFit_t] ){
                gy1.push_back(mean1PE[i_iDAC][bestFit_t]);
                gy2.push_back(mean2PE[i_iDAC][bestFit_t]);
                gy3.push_back(mean3PE[i_iDAC][bestFit_t]);
              }else{
                gy1.push_back(pe1[idif][ichip][ichan][i_iDAC]);
                gy2.push_back(pe2[idif][ichip][ichan][i_iDAC]);
                gy3.push_back(pe3[idif][ichip][ichan][i_iDAC]);
              }
            }
                                          
            // ************ Linear fit of 0.5pe optimized threshold vs inputDAC plot ************ //
            TGraph* PELinear1 = new TGraph(gx.size(), gx.data(), gy1.data());
            TString title1("Dif" + std::to_string(dif_counter_to_id[idif])
                           + "_Chip" + std::to_string(ichip)
                           + "_Channel" + std::to_string(ichan)
                           + ";InputDAC;0.5 pe threshold");
            PELinear1->SetTitle(title1);
            PELinear1->Draw("ap*");

            TF1* fit1 = new TF1("fit1", "pol1", 1, 241);
            fit1->SetParameters(170,-0.01);
            PELinear1->Fit(fit1, "rlq");
            fit1->Draw("same");
            slope1    [idif][ichip][ichan] = fit1->GetParameter(1);
            intercept1[idif][ichip][ichan] = fit1->GetParameter(0);
        
            // ************* Save plot as png ************* //
            TString image1(output_img_dir + "/Dif" + std::to_string(dif_counter_to_id[idif])
                           + "/Chip" + std::to_string(ichip) + "/Channel" + std::to_string(ichan)
                           + "/PE1vsInputDAC.png");
            c2->Print(image1);
            c2->Clear();

            // ************ Linear fit of 1.5pe optimized threshold vs inputDAC plot ************ //
            TGraph* PELinear2 = new TGraph(gx.size(), gx.data(), gy2.data());
            TString title2("Dif" + std::to_string(dif_counter_to_id[idif])
                           + "_Chip" + std::to_string(ichip)
                           + "_Channel" + std::to_string(ichan)
                           + ";InputDAC;1.5 pe threshold");
            PELinear2->SetTitle(title2);
            PELinear2->Draw("ap*");

            TF1* fit2 = new TF1("fit2", "pol1", 1, 241);
            fit2->SetParameters(170,-0.01);
            PELinear2->Fit(fit2, "rlq");
            fit2->Draw("same");
            slope2    [idif][ichip][ichan] = fit2->GetParameter(1);
            intercept2[idif][ichip][ichan] = fit2->GetParameter(0);
        
            // ************* Save plot as png ************* //
            TString image2(output_img_dir + "/Dif" + std::to_string(dif_counter_to_id[idif])
                           + "/Chip" + std::to_string(ichip) + "/Channel" + std::to_string(ichan)
                           + "/PE2vsInputDAC.png");
            c2->Print(image2);
            c2->Clear();

            // ************ Linear fit of 2.5pe optimized threhold vs inputDAC plot ************ //
            TGraph* PELinear3 = new TGraph(gx.size(), gx.data(), gy3.data());
            TString title3("Dif" + std::to_string(dif_counter_to_id[idif])
                           + "_Chip" + std::to_string(ichip)
                           + "_Channel" + std::to_string(ichan)
                           + ";InputDAC;2.5 pe threshold");
            PELinear3->SetTitle(title3);
            PELinear3->Draw("ap*");

            TF1* fit3 = new TF1("fit3", "pol1", 1, 241);
            fit3->SetParameters(170,-0.01);
            PELinear3->Fit(fit3,"rlq");
            fit3->Draw("same");
            slope3    [idif][ichip][ichan] = fit3->GetParameter(1);
            intercept3[idif][ichip][ichan] = fit3->GetParameter(0);

            // ************* Save plot as png ************* //
            TString image3(output_img_dir + "/Dif" + std::to_string(dif_counter_to_id[idif])
                           + "/Chip" + std::to_string(ichip) + "/Channel" + std::to_string(ichan)
                           + "/PE3vsInputDAC.png");
            c2->Print(image3);
                                          
            delete PELinear1;
            delete PELinear2;
            delete PELinear3;
            delete fit1;
            delete fit2;
            delete fit3;
            delete c2;
          }
        }
      }
    }

    d1vector gx, gy1, gy2, gy3;
    for (unsigned i_iDAC = 0; i_iDAC < n_inputDAC; ++i_iDAC) {
      gx.push_back(inputDAC[i_iDAC]);
      gy1.push_back(meanPE[i_iDAC][0]);
      gy2.push_back(meanPE[i_iDAC][1]);
      gy3.push_back(meanPE[i_iDAC][2]);
    }
    TGraph* PELinear1 = new TGraph(gx.size(), gx.data(), gy1.data());
    TF1* fit1 = new TF1("fit1", "pol1", 1, 241);
    fit1->SetParameters(170,-0.01);
    PELinear1->Fit(fit1, "rlq");
    meanSlope1     = fit1->GetParameter(1);
    meanIntercept1 = fit1->GetParameter(0);
    
    TGraph* PELinear2 = new TGraph(gx.size(), gx.data(), gy2.data());
    TF1* fit2 = new TF1("fit2", "pol1", 1, 241);
    fit2->SetParameters(170,-0.01);
    PELinear2->Fit(fit2, "rlq");
    meanSlope2     = fit2->GetParameter(1);
    meanIntercept2 = fit2->GetParameter(0);

    TGraph* PELinear3 = new TGraph(gx.size(), gx.data(), gy3.data());
    TF1* fit3 = new TF1("fit3", "pol1", 1, 241);
    fit3->SetParameters(170,-0.01);
    PELinear3->Fit(fit3, "rlq");
    meanSlope3     = fit3->GetParameter(1);
    meanIntercept3 = fit3->GetParameter(0);
    delete PELinear1;
    delete PELinear2;
    delete PELinear3;
    delete fit1;
    delete fit2;
    delete fit3;

    /********************************************************************************
     *                           threshold_card.xml                                 *
     ********************************************************************************/

    std::ofstream fout(output_xml_dir + "/failed_channels.txt");
    std::string xmlfile(output_xml_dir + "/threshold_card.xml");

    try {
      Edit.OPT_Make(xmlfile, inputDAC, topol.dif_map);
      Edit.Open(xmlfile);
    }
    catch (const wgInvalidFile & e) {
      Log.eWrite("[wgScurve] " + xmlfile + " : " + e.what());
      return ERR_FAILED_OPEN_XML_FILE;
    }

    for (unsigned idif = 0; idif < 4; ++idif) {
      for (unsigned ichip = 0; ichip < topol.dif_map[dif_counter_to_id[idif]].size(); ++ichip) {
        for (unsigned ichan = 0; ichan < topol.dif_map[dif_counter_to_id[idif]][ichip]; ++ichan) {
          Edit.OPT_SetChanValue(std::string("slope_threshold1"),     dif_counter_to_id[idif], ichip,
                                ichan, meanSlope1, NO_CREATE_NEW_MODE);
          Edit.OPT_SetChanValue(std::string("intercept_threshold1"), dif_counter_to_id[idif], ichip,
                                ichan, meanIntercept1, NO_CREATE_NEW_MODE);
          Edit.OPT_SetChanValue(std::string("slope_threshold2"),     dif_counter_to_id[idif], ichip,
                                ichan, meanSlope2, NO_CREATE_NEW_MODE);
          Edit.OPT_SetChanValue(std::string("intercept_threshold2"), dif_counter_to_id[idif], ichip,
                                ichan, meanIntercept2, NO_CREATE_NEW_MODE);
          Edit.OPT_SetChanValue(std::string("slope_threshold3"),     dif_counter_to_id[idif], ichip,
                                ichan, meanSlope3, NO_CREATE_NEW_MODE);
          Edit.OPT_SetChanValue(std::string("intercept_threshold3"), dif_counter_to_id[idif], ichip,
                                ichan, meanIntercept3, NO_CREATE_NEW_MODE);
          for (unsigned i_iDAC = 0; i_iDAC < n_inputDAC; ++i_iDAC) {
              Edit.OPT_SetValue(std::string("threshold_1"), dif_counter_to_id[idif], ichip, ichan,
                                inputDAC[i_iDAC], meanPE[i_iDAC][0], NO_CREATE_NEW_MODE);
              Edit.OPT_SetValue(std::string("threshold_2"), dif_counter_to_id[idif], ichip, ichan,
                                inputDAC[i_iDAC], meanPE[i_iDAC][1], NO_CREATE_NEW_MODE);
              Edit.OPT_SetValue(std::string("threshold_3"), dif_counter_to_id[idif], ichip, ichan,
                                inputDAC[i_iDAC], meanPE[i_iDAC][2], NO_CREATE_NEW_MODE);
          } // iDAC
        } // chan
      } // chip
    } // dif

    for (unsigned idif = 4; idif < n_difs; ++idif) {
      for (unsigned ichip = 0; ichip < topol.dif_map[dif_counter_to_id[idif]].size(); ++ichip) {
        for (unsigned ichan = 0; ichan < topol.dif_map[dif_counter_to_id[idif]][ichip]; ++ichan) {
          // Set the slope and intercept values for the result of fitting threshold-inputDAC plot.
          Edit.OPT_SetChanValue(std::string("slope_threshold1"),     dif_counter_to_id[idif], ichip,
                                ichan, slope1    [idif][ichip][ichan], NO_CREATE_NEW_MODE);
          Edit.OPT_SetChanValue(std::string("intercept_threshold1"), dif_counter_to_id[idif], ichip,
                                ichan, intercept1[idif][ichip][ichan], NO_CREATE_NEW_MODE);
          Edit.OPT_SetChanValue(std::string("slope_threshold2"),     dif_counter_to_id[idif], ichip,
                                ichan, slope2    [idif][ichip][ichan], NO_CREATE_NEW_MODE);
          Edit.OPT_SetChanValue(std::string("intercept_threshold2"), dif_counter_to_id[idif], ichip,
                                ichan, intercept2[idif][ichip][ichan], NO_CREATE_NEW_MODE);
          Edit.OPT_SetChanValue(std::string("slope_threshold3"),     dif_counter_to_id[idif], ichip,
                                ichan, slope3    [idif][ichip][ichan], NO_CREATE_NEW_MODE);
          Edit.OPT_SetChanValue(std::string("intercept_threshold3"), dif_counter_to_id[idif], ichip,
                                ichan, intercept3[idif][ichip][ichan], NO_CREATE_NEW_MODE);

          for (unsigned i_iDAC = 0; i_iDAC < n_inputDAC; ++i_iDAC) {
            size_t bestFit_t = bestFit[idif][ichip][ichan][i_iDAC];
            if( pe1[idif][ichip][ichan][i_iDAC] == UINT_MAX ||
                pe2[idif][ichip][ichan][i_iDAC] == UINT_MAX ||
                pe3[idif][ichip][ichan][i_iDAC] == UINT_MAX ){
              fout << dif_counter_to_id[idif] << "    " << ichip << "    " << ichan << "    " << inputDAC[i_iDAC] << "   !!! channel broken !!!" << std::endl;
              Edit.OPT_SetValue(std::string("threshold_1"), dif_counter_to_id[idif], ichip, ichan,
                                inputDAC[i_iDAC], UINT_MAX, NO_CREATE_NEW_MODE);
              Edit.OPT_SetValue(std::string("threshold_2"), dif_counter_to_id[idif], ichip, ichan,
                                inputDAC[i_iDAC], UINT_MAX, NO_CREATE_NEW_MODE);
              Edit.OPT_SetValue(std::string("threshold_3"), dif_counter_to_id[idif], ichip, ichan,
                                inputDAC[i_iDAC], UINT_MAX, NO_CREATE_NEW_MODE);
            }else if( std::abs(pe1[idif][ichip][ichan][i_iDAC] - mean1PE[i_iDAC][bestFit_t]) > 2*sigma1PE[i_iDAC][bestFit_t] ||
                      std::abs(pe2[idif][ichip][ichan][i_iDAC] - mean2PE[i_iDAC][bestFit_t]) > 2*sigma2PE[i_iDAC][bestFit_t] ||
                      std::abs(pe3[idif][ichip][ichan][i_iDAC] - mean3PE[i_iDAC][bestFit_t]) > 2*sigma3PE[i_iDAC][bestFit_t] ){
              // If 0.5, 1.5 or 2.5 pe level is far from the mean value by 2-sigma, 
              // it will recorded in "failed_channels.txt" with the number of 
              // DIF, CHIP, CHANNEL, InputDAC. Also mean threshold value will 
              // be recorded in threshold_card.xml, instead.
              fout << dif_counter_to_id[idif] << "    " << ichip << "    " << ichan << "    " << inputDAC[i_iDAC] << std::endl;
              Edit.OPT_SetValue(std::string("threshold_1"), dif_counter_to_id[idif], ichip, ichan,
                                inputDAC[i_iDAC], mean1PE[i_iDAC][bestFit_t], NO_CREATE_NEW_MODE);
              Edit.OPT_SetValue(std::string("threshold_2"), dif_counter_to_id[idif], ichip, ichan,
                                inputDAC[i_iDAC], mean2PE[i_iDAC][bestFit_t], NO_CREATE_NEW_MODE);
              Edit.OPT_SetValue(std::string("threshold_3"), dif_counter_to_id[idif], ichip, ichan,
                                inputDAC[i_iDAC], mean3PE[i_iDAC][bestFit_t], NO_CREATE_NEW_MODE);
            }else{
              // Set the 0.5 pe, 1.5 pe and 2.5 pe level after fitting the scurve.
              Edit.OPT_SetValue(std::string("threshold_1"), dif_counter_to_id[idif], ichip, ichan,
                                inputDAC[i_iDAC], pe1[idif][ichip][ichan][i_iDAC], NO_CREATE_NEW_MODE);
              Edit.OPT_SetValue(std::string("threshold_2"), dif_counter_to_id[idif], ichip, ichan,
                                inputDAC[i_iDAC], pe2[idif][ichip][ichan][i_iDAC], NO_CREATE_NEW_MODE);
              Edit.OPT_SetValue(std::string("threshold_3"), dif_counter_to_id[idif], ichip, ichan,
                                inputDAC[i_iDAC], pe3[idif][ichip][ichan][i_iDAC], NO_CREATE_NEW_MODE);
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
    Log.eWrite("[wgScurve][" + input_dir + "] " + std::string(e.what()));
    return ERR_WG_SCURVE;
  }
  Log.Write("[wgScurve] Writing Xml file done.");
  return WG_SUCCESS;
}

//**********************************************************************
void fit_scurve1(TGraphErrors* Scurve, 
                TF1* fit_scurve,
                double& pe1_t, 
                double& pe2_t, 
                double& pe3_t, 
                double& ChiSquare,
                int&    NDF,
                unsigned idif, 
                unsigned ichip, 
                unsigned ichan, 
                unsigned inputDAC,
                std::string output_img_dir, 
                bool print_flag) {

#ifdef LOG_SCURVE
  /* for log-scaled Scurve */
  double c0 = 3.0, c1 = 0.5, c2 = 155, c3 = 2.5, c4 = 0.5, c5 = 135, c6 = 5.0;
  fit_scurve->SetParameters(c0, c1, c2, c3, c4, c5, c6);
  fit_scurve->SetParLimits(0, c0-0.5, c0+0.5);  
  fit_scurve->SetParLimits(3, c3-0.5, c3+0.5); 
  fit_scurve->SetParLimits(6, c6-0.5, c6+0.5);
#else 
  /* for not log-scaled Scurve */
  double c0 = 1.0E+5, c1 = 0.5, c2 = 155, c3 = 3.0E+3, c4 = 0.5, c5 = 135, c6 = 1.0E+2;
  fit_scurve->SetParameters(c0, c1, c2, c3, c4, c5, c6);
  fit_scurve->SetParLimits(0, c0/2.0, c0*1.5);  
  fit_scurve->SetParLimits(3, c3/3.0, c3*1.5); 
  fit_scurve->SetParLimits(6, c6/2.0, c6*4.0);
  fit_scurve->SetParLimits(2, c2-5.0, c2+5.0);
  fit_scurve->SetParLimits(5, c5-5.0, c5+5.0);
#endif
  
  Scurve->Fit(fit_scurve, "q");

  // From the fitting parameters, calcurate each p.e. level.
  // Here, pe1 -> 0.5 pe, pe2 -> 1.5 pe and pe3 -> 2.5 pe threshold.
  // Variables a and b indicate the center point of each sigmoid function.
  double a = fit_scurve->GetParameter(2);
  double b = fit_scurve->GetParameter(5);
  ChiSquare = fit_scurve->GetChisquare();
  NDF = fit_scurve->GetNDF();
  pe1_t = (3*a - b) / 2;
  pe2_t = (a + 2*b) / 3;
  pe3_t = (5*b - 2*a) / 3;

  if( print_flag && (!output_img_dir.empty()) ) {
    TString image(output_img_dir + "/Dif" + std::to_string(idif) + "/Chip" + std::to_string(ichip) +
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
}

//**********************************************************************
void fit_scurve2(TGraphErrors* Scurve, 
                TF1* fit_scurve,
                 double& pe1_t, 
                 double& pe2_t, 
                 double& pe3_t, 
                 double& ChiSquare,
                 int&    NDF,
                 unsigned idif, 
                 unsigned ichip, 
                 unsigned ichan, 
                 unsigned inputDAC,
                 std::string output_img_dir, 
                 bool print_flag) {

#ifdef LOG_SCURVE
	/* for log-scaled Scurve */
	double c0 = 3.0, c1 = 0.5, c2 = 155, c3 = 2.5, c4 = 0.5, c5 = 135, c6 = 5.0;
  fit_scurve->SetParameters(c0, c1, c2, c3, c4, c5, c6);
  fit_scurve->SetParLimits(0, c0-0.5, c0+0.5);  
  fit_scurve->SetParLimits(3, c3-0.5, c3+0.5); 
  fit_scurve->SetParLimits(6, c6-0.5, c6+0.5);
#else 
  /* for not log-scaled Scurve */
  double c0 = 1.0E+5, c1 = 0.5, c2 = 160, c3 = 3.0E+3, c4 = 0.5, c5 = 147, c6 = 2.0E+2, c7 = 0.5, c8 = 135, c9 = 10;
  fit_scurve->SetParameters(c0, c1, c2, c3, c4, c5, c6, c7, c8, c9);
  fit_scurve->SetParLimits(0, c0/2.0, c0*1.5);  
  fit_scurve->SetParLimits(3, c3/3.0, c3*1.5); 
  fit_scurve->SetParLimits(6, c6/2.0, c6*2.0);
  fit_scurve->SetParLimits(9, c9/4.0, c9*2.0);
  fit_scurve->SetParLimits(2, c2-5.0, c2+10.0);
  fit_scurve->SetParLimits(5, c5-5.0, c5+5.0);
  fit_scurve->SetParLimits(8, c8-5.0, c8+5.0);
#endif
  
  Scurve->Fit(fit_scurve, "q");

  // From the fitting parameters, calcurate each p.e. level.
  // Here, pe1 -> 0.5 pe, pe2 -> 1.5 pe and pe3 -> 2.5 pe threshold.
  // Variables a,b and c indicate the center point of each sigmoid function.
  double a = fit_scurve->GetParameter(2);
  double b = fit_scurve->GetParameter(5);
  double c = fit_scurve->GetParameter(8);
  ChiSquare = fit_scurve->GetChisquare();
  NDF = fit_scurve->GetNDF();
  pe1_t = (3*a - b) / 2;
  pe2_t = (a + 2*b) / 3;
  pe3_t = (b + 2*c) / 3;

  if( print_flag && (!output_img_dir.empty()) ) {
    TString image(output_img_dir + "/Dif" + std::to_string(idif) + "/Chip" + std::to_string(ichip) +
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
}

//**********************************************************************
void fit_scurve3(TGraphErrors* Scurve, 
                TF1* fit_scurve,
                 double& pe1_t, 
                 double& pe2_t, 
                 double& pe3_t, 
                 double& ChiSquare,
                 int&    NDF,
                 unsigned idif, 
                 unsigned ichip, 
                 unsigned ichan, 
                 unsigned inputDAC,
                 std::string output_img_dir, 
                 bool print_flag) {

#ifdef LOG_SCURVE
	/* for log-scaled Scurve */
	double c0 = 3.0, c1 = 0.5, c2 = 155, c3 = 2.5, c4 = 0.5, c5 = 135, c6 = 5.0;
  fit_scurve->SetParameters(c0, c1, c2, c3, c4, c5, c6);
  fit_scurve->SetParLimits(0, c0-0.5, c0+0.5);  
  fit_scurve->SetParLimits(3, c3-0.5, c3+0.5); 
  fit_scurve->SetParLimits(6, c6-0.5, c6+0.5);
#else 
  /* for not log-scaled Scurve */
  double c0 = 1.0E+5, c1 = 0.5, c2 = 157, c3 = 3.0E+3, c4 = 0.5, c5 = 140, c6 = 2.0E+2, c7 = 0.5, c8 = 127, c9 = 10;
  fit_scurve->SetParameters(c0, c1, c2, c3, c4, c5, c6, c7, c8, c9);
  fit_scurve->SetParLimits(0, c0/2.0, c0*1.5);  
  fit_scurve->SetParLimits(3, c3/3.0, c3*1.5); 
  fit_scurve->SetParLimits(6, c6/2.0, c6*2.0);
  fit_scurve->SetParLimits(9, c9/4.0, c9*2.0);
  fit_scurve->SetParLimits(2, c2-5.0, c2+5.0);
  fit_scurve->SetParLimits(5, c5-5.0, c5+5.0);
  fit_scurve->SetParLimits(8, c8-8.0, c8+8.0);
#endif
  
  Scurve->Fit(fit_scurve, "q");

  // From the fitting parameters, calcurate each p.e. level.
  // Here, pe1 -> 0.5 pe, pe2 -> 1.5 pe and pe3 -> 2.5 pe threshold.
  // Variables a,b and c indicate the center point of each sigmoid function.
  double a = fit_scurve->GetParameter(2);
  double b = fit_scurve->GetParameter(5);
  double c = fit_scurve->GetParameter(8);
  ChiSquare = fit_scurve->GetChisquare();
  NDF = fit_scurve->GetNDF();
  pe1_t = (3*a - b) / 2;
  pe2_t = (a + 2*b) / 3;
  pe3_t = (b + 2*c) / 3;

  if( print_flag && (!output_img_dir.empty()) ) {
    TString image(output_img_dir + "/Dif" + std::to_string(idif) + "/Chip" + std::to_string(ichip) +
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
}
