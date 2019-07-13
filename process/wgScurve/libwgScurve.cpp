// system includes
#include <string>
#include <vector>

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

//******************************************************************
int wgScurve(const char* x_inputDir,
             const char* x_outputXMLDir,
             const char* x_outputIMGDir) {

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
  outputIMGDir = outputIMGDir + "/" + GetName(inputDir);

  // ============ Create outputXMLDir ============ //
  try { MakeDir(outputXMLDir); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[wgScurve] " + string(e.what()));
    return ERR_FAILED_CREATE_DIRECTORY;
  }

  // ============ Create outputIMGDir ============ //
  try { MakeDir(outputIMGDir); }
  catch (const wgInvalidFile& e) {
    Log.eWrite("[wgScurve] " + string(e.what()));
    return ERR_FAILED_CREATE_DIRECTORY;
  }

  Log.Write(" *****  READING DIRECTORY      : " + inputDir + "  *****");
  Log.Write(" *****  OUTPUT XML DIRECTORY   : " + outputXMLDir + "  *****");
  Log.Write(" *****  OUTPUT IMAGE DIRECTORY : " + outputIMGDir + "  *****");

  try{

    /********************************************************************************
     *                  Get directory tree and set variables                       *
     ********************************************************************************/
        
    // Get topology from input directory.
    // Get the number of dif, chip, channel, imputDAC and threshold. 
    Topology topol(inputDir, TopologySourceType::scurve_tree);
    const unsigned n_inputDAC  = ListDirectories(inputDir).size();
    const unsigned n_threshold = ListDirectories(inputDir).at(0).size(); // FIXME: maybe wrong
    unsigned n_difs = topol.n_difs;
    u1vector n_chips;
    u2vector n_chans;
    for(unsigned idif = 0; idif < n_difs; ++idif) {
      unsigned idif_id = idif + 1;
      n_chips[idif] = topol.dif_map[idif_id].size();
      for(unsigned ichip = 0; ichip < n_chips[idif]; ++ichip) {
        unsigned ichip_id = ichip + 1;
        n_chans[idif][ichip] = topol.dif_map[idif_id][ichip_id];
      }
    }

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
    //  and resize them.
    for (unsigned idif = 0; idif < n_difs; ++idif) {
      slope1     [idif].resize(n_chips[idif]);
      slope2     [idif].resize(n_chips[idif]);
      intercept1 [idif].resize(n_chips[idif]);
      intercept2 [idif].resize(n_chips[idif]);
      pe1        [idif].resize(n_chips[idif]);
      pe2        [idif].resize(n_chips[idif]);
      noise      [idif].resize(n_chips[idif]);
      noise_sigma[idif].resize(n_chips[idif]);
      for (unsigned ichip = 0; ichip < n_chips[idif]; ++ichip) {
        slope1     [idif][ichip].resize(n_chans[idif][ichip]);
        slope2     [idif][ichip].resize(n_chans[idif][ichip]);
        intercept1 [idif][ichip].resize(n_chans[idif][ichip]);
        intercept2 [idif][ichip].resize(n_chans[idif][ichip]);
        pe1        [idif][ichip].resize(n_chans[idif][ichip]);
        pe2        [idif][ichip].resize(n_chans[idif][ichip]);
        noise      [idif][ichip].resize(n_chans[idif][ichip]);
        noise_sigma[idif][ichip].resize(n_chans[idif][ichip]);
        for (unsigned ichan = 0; ichan < n_chans[idif][ichip]; ++ichan) {
          pe1        [idif][ichip][ichan].resize(n_inputDAC);
          pe2        [idif][ichip][ichan].resize(n_inputDAC);
          noise      [idif][ichip][ichan].resize(n_inputDAC);
          noise_sigma[idif][ichip][ichan].resize(n_inputDAC);
          for (unsigned i_iDAC = 0; i_iDAC < n_inputDAC; ++i_iDAC) {
            noise      [idif][ichip][ichan][i_iDAC].resize(n_threshold);
            noise_sigma[idif][ichip][ichan][i_iDAC].resize(n_threshold);
          }
        }
      }
    }

    /********************************************************************************
     *                              Read XML files                                  *
     ********************************************************************************/
        
    unsigned i_iDAC = 0;  // index for inputDAC
    // input DAC
    std::vector<std::string> iDAC_dir_list = ListDirectories(inputDir);
    for (auto const & iDAC_directory : iDAC_dir_list) {
      std::vector<std::string> th_dir_list = ListDirectories(iDAC_directory);
      unsigned i_threshold = 0;  // index for threshold
      // threshold
      for (auto & th_directory : th_dir_list) {
        // DIF
        th_directory += "/wgAnaHistSummary/Xml";
        std::vector<std::string> dif_dir_list = ListDirectories(th_directory);
        for (auto const & idif_directory : dif_dir_list) {
          unsigned idif = extractIntegerFromString(GetName(idif_directory)) - 1;
          // chip
          std::vector<std::string> chip_xml_list = ListFilesWithExtension(idif_directory, "xml");
          for (auto const & ichip_xml : chip_xml_list) {
            unsigned ichip_id = extractIntegerFromString(GetName(ichip_xml));
            unsigned ichip = ichip_id - 1;
                  
            // ************* Open XML file ************* //
            try { Edit.Open(ichip_xml); }
            catch (const exception& e) {
              Log.eWrite("[wgScurve] " + string(e.what()));
              return ERR_FAILED_OPEN_XML_FILE;
            }
            // ************* Read XML file ************* //
            threshold[i_threshold] = Edit.SUMMARY_GetGlobalConfigValue("trigth");
            inputDAC[i_iDAC]       = Edit.SUMMARY_GetGlobalConfigValue("inputDAC");
            // channel
            for (unsigned ichan = 0; ichan < n_chans[idif][ichip]; ++ichan) {
              unsigned ichan_id = ichan + 1;
              // get noise rate for each channel 
              noise      [idif][ichip][ichan][i_iDAC][i_threshold] = Edit.SUMMARY_GetChFitValue("noise",       ichan_id);
              noise_sigma[idif][ichip][ichan][i_iDAC][i_threshold] = Edit.SUMMARY_GetChFitValue("sigma_noise", ichan_id);
            }  // channel
            Edit.Close();
          } // chip
        } // dif
        ++i_threshold;
      } // threshold
      ++i_iDAC;
    } // inputDAC

    /********************************************************************************
     *                        Draw and fit the S-curve                              *
     ********************************************************************************/
        
    // DIF
    for (unsigned idif = 0; idif < n_difs; ++idif){
      unsigned idif_id = idif + 1;
      // chip
      for (unsigned ichip = 0; ichip < n_chips[idif]; ++ichip) {
        unsigned ichip_id = ichip + 1;
        // channel
        for (unsigned ichan = 0; ichan < n_chans[idif][ichip]; ++ichan) {
          unsigned ichan_id = ichan + 1;
          // input DAC
          for (unsigned i_iDAC = 0; i_iDAC < n_inputDAC; ++i_iDAC) {
                                        
            TCanvas *c1 = new TCanvas("c1", "c1");
            c1->SetLogy();
            // These are temporary variables for x, y and their errors used to draw the graph.
            d1vector gx, gy, gxe, gye;

            // threshold
            for (unsigned i_threshold = 0; i_threshold < n_threshold; ++i_threshold) {
              gx.push_back(threshold[i_threshold]);
              gy.push_back(noise[idif][ichip][ichan][i_iDAC][i_threshold]);
              gxe.push_back(0);
              gye.push_back(noise_sigma[idif][ichip][ichan][i_iDAC][i_threshold]);
            }
                                                
            // ************* Draw S-curve Graph ************* //
            TGraphErrors* Scurve = new TGraphErrors(gx.size(), gx.data(), gy.data(), gxe.data(), gye.data());
            TString title("Dif" + std::to_string(idif_id)
                          + "_Chip" + std::to_string(ichip_id)
                          + "_Channel" + std::to_string(ichan_id)
                          + "_InputDAC" + std::to_string(inputDAC[i_iDAC]) 
                          + ";Threshold;Noise rate");
            Scurve->SetTitle(title);
            Scurve->Draw("ap*");

            // ************* Fit S-curve ************* //
            double pe1_t, pe2_t;
            fit_scurve(Scurve, pe1_t, pe2_t, idif_id, ichip_id, ichan_id, inputDAC[i_iDAC], outputIMGDir, true);
            pe1[idif][ichip][ichan].push_back(pe1_t);
            pe2[idif][ichip][ichan].push_back(pe2_t);

            // ************* Save S-curve Graph as png ************* //
            TString image(outputIMGDir + "/Dif" + std::to_string(idif_id)
                          + "/Chip" + std::to_string(ichip_id) + "/Channel" + std::to_string(ichan_id)
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
          TString title1("Dif" + std::to_string(idif_id)
                        + "_Chip" + std::to_string(ichip_id)
                        + "_Channel" + std::to_string(ichan_id)
                        + ";InputDAC;1.5 pe threshold");
          PELinear1->SetTitle(title1);
          PELinear1->Draw("ap*");

          TF1* fit1 = new TF1("fit1", "[0]*x + [1]", 1, 241);
          fit1->SetParameters(-0.01, 170);
          PELinear1->Fit(fit1, "rl");
          fit1->Draw("same");
          slope1    [idif][ichip].push_back(fit1->GetParameter(0));
          intercept1[idif][ichip].push_back(fit1->GetParameter(1));
        
          // ************* Save plot as png ************* //
          TString image1(outputIMGDir + "/Dif" + std::to_string(idif_id)
                        + "/Chip" + std::to_string(ichip_id) + "/Channel" + std::to_string(ichan_id)
                        + "/PE1vsInputDAC.png");
          c2->Print(image1);
          c2->Clear();

          // ************ Linear fit of 2.5pe optimized threhold vs inputDAC plot ************ //
          TGraph* PELinear2 = new TGraph(gx.size(), gx.data(), gy2.data());
          TString title2("Dif" + std::to_string(idif_id)
                         + "_Chip" + std::to_string(ichip_id)
                         + "_Channel" + std::to_string(ichan_id)
                         + ";InputDAC;2.5 pe threshold");
          PELinear2->SetTitle(title2);
          PELinear2->Draw("ap*");

          TF1* fit2 = new TF1("fit2", "[0]*x + [1]", 1, 241);
          fit2->SetParameters(-0.01, 170);
          PELinear2->Fit(fit2,"rl");
          fit2->Draw("same");
          slope2    [idif][ichip].push_back(fit2->GetParameter(0));
          intercept2[idif][ichip].push_back(fit2->GetParameter(1));

          // ************* Save plot as png ************* //
          TString image2(outputIMGDir + "/Dif" + std::to_string(idif_id)
                         + "/Chip" + std::to_string(ichip_id) + "/Channel" + std::to_string(ichan_id)
                         + "/PE2vsInputDAC.png");
          c2->Print(image2);
                                        
          delete PELinear1;
          delete PELinear2;
          delete fit1;
          delete fit2;
          delete c2;
        } // channel
      } // chip
    } // dif

    /********************************************************************************
     *                           threshold_card.xml                                 *
     ********************************************************************************/

    std::string xmlfile(outputXMLDir + "/threshold_card.xml");
    try { Edit.Open(xmlfile); }
    catch (const wgInvalidFile & e) {
      Log.eWrite("[wgScurve] " + xmlfile + " : " + e.what());
      return ERR_FAILED_OPEN_XML_FILE;
    }

    Edit.OPT_Make(xmlfile, inputDAC, n_difs, n_chips, n_chans);

    for (unsigned idif = 0; idif < n_difs; ++idif) {
      unsigned idif_id = idif + 1;
        
      for (unsigned ichip = 0; ichip < n_chips[idif]; ++ichip) {
        unsigned ichip_id = ichip + 1;

        for (unsigned ichan = 0; ichan < n_chans[idif][ichip]; ++ichan) {
          unsigned ichan_id = ichan + 1;
          // Set the slope and intercept values for the result of fitting threshold-inputDAC plot.
          Edit.OPT_SetChanValue(string("s_th1"), idif_id, ichip_id, ichan_id, slope1[idif][ichip][ichan],     NO_CREATE_NEW_MODE);
          Edit.OPT_SetChanValue(string("i_th1"), idif_id, ichip_id, ichan_id, intercept1[idif][ichip][ichan], NO_CREATE_NEW_MODE);
          Edit.OPT_SetChanValue(string("s_th2"), idif_id, ichip_id, ichan_id, slope2[idif][ichip][ichan],     NO_CREATE_NEW_MODE);
          Edit.OPT_SetChanValue(string("i_th2"), idif_id, ichip_id, ichan_id, intercept2[idif][ichip][ichan], NO_CREATE_NEW_MODE);

          for (unsigned i_iDAC = 0; i_iDAC < n_inputDAC; ++i_iDAC) {
            // Set the 2.5 pe and 1.5 pe level of ftting the scurve.
            Edit.OPT_SetValue(string("threshold_1"), idif_id, ichip_id, ichan_id, i_iDAC, pe1[idif][ichip][ichan][i_iDAC], NO_CREATE_NEW_MODE);
            Edit.OPT_SetValue(string("threshold_2"), idif_id, ichip_id, ichan_id, i_iDAC, pe2[idif][ichip][ichan][i_iDAC], NO_CREATE_NEW_MODE);
          }
        }
      }
    }
    Edit.Write();
    Edit.Close();
  }  // end try
  catch (const exception& e){
    Log.eWrite("[wgScurve][" + inputDir + "] " + string(e.what()));
    return ERR_WG_SCURVE;
  }
  return WG_SUCCESS;
}

//**********************************************************************
void fit_scurve(TGraphErrors* Scurve, 
                double& pe1_t, 
                double& pe2_t, 
                unsigned idif_id, 
                unsigned ichip_id, 
                unsigned ichan_id, 
                unsigned inputDAC,
                std::string outputIMGDir, 
                bool print_flag) {

  // Fitting function for Scurve is summation of two sigmoid functions (and a constant).
  const char * fit_function = "[0]/(1+exp(-[1]*(x-[2]))) + [3]/(1+exp(-[4]*(x-[5]))) + [6]";
  double c0 = 50000, c1 = 1, c2 = 155, c3 = 5000, c4 = 1, c5 = 135, c6 = 300;

  TF1* fit_scurve = new TF1("fit_scurve", fit_function, 120, 170);
  fit_scurve->SetParameters(c0, c1, c2, c3, c4, c5, c6);
  fit_scurve->SetParLimits(0, 30000, 70000);
  fit_scurve->SetParLimits(2, 145,   160);
  fit_scurve->SetParLimits(3, 2000,  7000);
  fit_scurve->SetParLimits(5, 130,   145);
  fit_scurve->SetParLimits(6, 100,   500);
        
  Scurve->Fit(fit_scurve, "");
        
  // From the fitting parameters, calcurate each p.e. level.
  // Here, pe1 -> 1.5 pe threshold, pe2 -> 2.5 pe threshold.
  // variables a and b indicates the center point of each sigmoid function.
  // Set 1.5 pe as the middle point between two sigmoid functions' center.
  double a = fit_scurve->GetParameter(5);
  double b = fit_scurve->GetParameter(2);
  pe1_t =  (  a + b) / 2;
  pe2_t =  (3*a + b) / 2;

  if( print_flag && (!outputIMGDir.empty()) ) {
    TString image(outputIMGDir + "/Dif" + std::to_string(idif_id) + "/Chip" + std::to_string(ichip_id) +
                  "/Channel" + std::to_string(ichan_id) + "/InputDAC" + std::to_string(inputDAC) + "_fit.png");
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
