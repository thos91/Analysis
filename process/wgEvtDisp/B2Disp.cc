#include "TROOT.h"
#include "TApplication.h"
#include "TFile.h"
#include "TH1D.h"
#include "TF1.h"
#include "TGraph.h"
#include "TGraphAsymmErrors.h"
#include "TH2D.h"
#include "TStyle.h"
#include "TString.h"
#include "TSystem.h"
#include "TSpectrum.h"
#include "TTree.h"
#include "TArc.h"
#include "TBox.h"
#include "TPolyLine.h"
#include "TLine.h"
#include "TCanvas.h"
#include "TText.h"

#include <iostream>
#include <fstream>
#include "math.h"
#include <sstream>
#include <complex>

#include "DetectorDimension.hh"
#include "TwoDimReconSummary.h"
#include "ThreeDimReconSummary.h"
#include "EVENTSUMMARY.h"
#include "TwoDimRecon.hxx"
#include "Const.hh"

//#define DEBUG_DISP
//#define DEBUG_DETDIM

void sci_ing(double x,double y,double x_len,double y_len,double deg);
void sci_par(double x,double y,double x_len,double y_len,double deg);
void sci_sci(double x,double y,double x1,double y1);
void sci_veto(double x,double y,double x_len,double y_len,double deg_veto);
void iron(double x,double y,double x_len,double y_len,double deg);


void DrawINGRID(int view, double x_center, double y_center, double z_center, double deg);
void DrawProtonModule(int view, double x_center, double y_center, double z_center);
void DrawWaterModule(int view, double x_center, double y_center, double z_center);
void DrawHits(int mod, int pln, int view, int ch, double pe, double x_center, double y_center, double z_center, double deg);

void drawx(int targetmod);
void drawy(int targetmod);

void tline(double iX,double iY,double fX,double fY);
void drawdotline(double iX,double iY,double fX,double fY);
void drawtext(const char* text, double x, double y,double font);

int main(int argc, char** argv){

  //*****arguments*******************//
  int c = -1;
  std::stringstream readfilename;
  int mode      = -1;
  int type      = -1;
  int target    = -1;
  int evt_start = 0; 
  bool selecttype = false;
  bool SCANNING   = false;
  while ((c = getopt(argc, argv, "i:m:t:s:e:c")) != -1){
    switch(c){
      case 'i':
        readfilename << optarg;
        break;
      case 'm':
        mode = atoi(optarg);
        break;
      case 't':
        target = atoi(optarg);
        break;
      case 's':
        type = atoi(optarg);
        selecttype = true;
        break;
      case 'e':
        evt_start = atoi(optarg);
        break;
      case 'c':
        SCANNING = true;
        break;
    }
  }

  //*****open root file and get tree*******************//
  TROOT root("GUI","GUI");
  TApplication theApp("App",0,0);

  if(argc<3){
    std::cout << "-m Select mode:\n"
      << "   0 : MC true info\n"
      << "   1 : 2D reconstruction\n"
      << "   2 : 3D recon and vertex\n" << std::endl;

    std::cout << "-i Input ROOT file." << std::endl;
    std::cout << "-t Target module. <5: OnAxisWaterModule, 7:WaterModule, 8:CHModule, 9:B2BG>" << std::endl;
    std::cout << "( Optional: -s Interaction type )" << std::endl;

    exit(1);
  }

  if(mode==-1){ 
    std::cout << "-m Select mode:\n" 
      << "   0 : MC true info\n"
      << "   1 : 2D reconstruction\n"
      << "   2 : 3D recon and vertex" << std::endl;

    exit(1);
  }
  if(target!=5&&target!=7&&target!=8){
    std::cout << "-t Target module:\n"
      << "   5: OnAxisWaterModule\n" 
      << "   7: WaterModule\n" 
      << "   8: CHModule\n"
      << "   9: B2BG" << std::endl;
    exit(1);
  }


  TFile* readfile = new TFile(readfilename.str().c_str(),"read");
  if(readfile->IsZombie()){
    std::cout << "Cannot open file : " << readfilename.str().c_str() << std::endl;
    exit(1);
  }
  std::cout << "Open file : " << readfilename.str().c_str() << std::endl;
  TTree* tree = (TTree*)readfile ->Get("tree");
  TBranch* evtbr = tree->GetBranch("fDefaultReco.");

  int nevt = (int)tree -> GetEntries();
  std::cout << "Total # of events = " << nevt << std::endl; 



  // *****event loop start************ ///
  bool newcanv = true;
  TCanvas *c1;
  for(int ievt = evt_start;ievt<nevt;ievt++){

    //texts for information in picture//
    stringstream textevt,texthits,texttrk,textingtrk,textinttype;

    //get eventsummary//
    EventSummary* evtsum = new EventSummary();
    evtbr -> SetAddress(&evtsum);
    tree  -> SetBranchAddress("fDefaultReco.",&evtsum);
    tree  -> GetEntry(ievt);	
    int nhits = evtsum->NHits();

    //get simvertexsummar//
    SimVertexSummary* simvertexsum = new SimVertexSummary();
    simvertexsum  = evtsum->GetSimVertex(0);
    int inttype   = simvertexsum->inttype;
    if(selecttype==true&&inttype!=type) continue;

    std::cout << "Event # is " << ievt << std::endl;

    std::cout << "  Neutrino energy is "    << simvertexsum->nuE  << " [GeV]" 
      << std::endl 
      << "  Interaction Type # is " << inttype
      << "  Hits # is " << nhits
      << std::endl;


    //*****make canvas and histgrams and define style********///
    gROOT->SetStyle("Plain");
    if (newcanv){
      double canvas_norm = 0.8;
      c1 = new TCanvas("c1","c1",600*canvas_norm,800*canvas_norm);
      newcanv = false;			
    }

    double HistLeft  = 0.;
    double HistRight = 0.;
    int    HistBin   = 2050*100;
    if(target==7||target==8){ 
      HistLeft  = -C_B2MotherSizeZ;
      HistRight =  C_B2MotherSizeZ - 600.;
    }
    else if(target==5){
      HistLeft  = C_PMMotherPosZ   - C_PMSizeZ;
      HistRight = C_INGHMotherPosZ + C_INGSizeZ;
    }

    HistLeft  = C_PMMotherPosZ   - C_PMSizeZ + 500;
    HistRight = C_INGHMotherPosZ + C_INGSizeZ - 500;

    TH1D *h = new TH1D("","Side View",HistBin,HistLeft,HistRight);
    TH1D *v = new TH1D("","Top View" ,HistBin,HistLeft,HistRight);
    h->SetMinimum(-C_INGSizeY);
    h->SetMaximum( C_INGSizeY);
    h->GetXaxis()->SetLabelSize(0);
    h->GetYaxis()->SetLabelSize(0);
    h->SetStats(0);
    v->SetMinimum(-C_INGSizeX);
    v->SetMaximum(+C_INGSizeX);
    v->GetXaxis()->SetLabelSize(0);
    v->GetYaxis()->SetLabelSize(0);
    v->SetStats(0);
    gStyle->SetOptTitle(0);
    gStyle->SetPadTopMargin(0.01);
    gStyle->SetPadLeftMargin(0.01);
    gStyle->SetPadRightMargin(0.01);
    gStyle->SetPadBottomMargin(0.01);

    //*****make pads for sideview and topveiw****************//
    TPad *pad1 = new TPad("pad1","pad1",0.01,0.02,0.99,0.41);
    TPad *pad2 = new TPad("pad2","pad2",0.01,0.44,0.99,0.83);
    pad1->Draw();
    pad2->Draw();

    pad1->cd();
    h->Draw("AH");
    std::cout << "drawx: Side view " << std::endl;
    drawx(target); //draw module in sideview//
    pad2->cd();
    v->Draw("AH");
    std::cout << "drawy: Top view " << std::endl;
    drawy(target); //draw module in topview//

    c1->cd();
    drawtext("sideview",0.1,0.41,0.04);
    drawtext("topview" ,0.1,0.84,0.04);


    //*****Draw hits******************//
    int numhit_mod[5][2];
    for(int imod=0;imod<5;imod++) for(int iview=0;iview<2;iview++) numhit_mod[imod][iview]=0;
    for(int ihits =0;ihits< nhits;ihits++){
      HitSummary* hitsum = new HitSummary();	
      hitsum = evtsum->GetHit(ihits);

      int hitmod  = hitsum->mod;
      int hitview = hitsum->view;
      int hitpln  = hitsum->pln;
      int hitch   = hitsum->ch;
      double hitpe   = hitsum->pe;

#ifdef DEBUG_DISP
      std::cout << "Hits "
        << "\t mod:"  << hitmod
        << "\t pln:"  << hitpln
        << "\t view:" << hitview
        << "\t ch:"   << hitch
        << "\t pe:"   << hitpe
        << std::endl;
#endif

      double OFFSET[3];


      // ** from center of INGRID mod3
      if(hitmod==3){
        OFFSET[0] = 0.;
        OFFSET[1] = 0.;
        OFFSET[2] = 0.;
        numhit_mod[0][hitview]++;
      }

      else if(hitmod==20){
        OFFSET[0] = C_PMMotherPosX;
        OFFSET[1] = C_PMMotherPosY;
        OFFSET[2] = C_PMMotherPosZ;
        numhit_mod[1][hitview]++;
      }
      // ** in B2 Mother Module
      else if(hitmod==21){
        OFFSET[0] = C_B2WMPosX;
        OFFSET[1] = C_B2WMPosY;
        OFFSET[2] = C_B2WMPosZ;
        numhit_mod[2][hitview]++;
      }
      else if(hitmod==22){
        OFFSET[0] = C_B2CHPosX;
        OFFSET[1] = C_B2CHPosY;
        OFFSET[2] = C_B2CHPosZ;
        numhit_mod[3][hitview]++;
      }
      else if(hitmod==23){
        OFFSET[0] = C_B2d1INGPosX;
        OFFSET[1] = C_B2d1INGPosY;
        OFFSET[2] = C_B2d1INGPosZ;
        numhit_mod[4][hitview]++;
      }
      else continue;

      if(hitview==0) pad1->cd();
      else           pad2->cd();

      DrawHits(hitmod,hitpln,hitview,hitch,hitpe,OFFSET[0],OFFSET[1],OFFSET[2],0.);
    }
    //if(numhit_mod[0][0]<1) goto ENDEVENT;
    //if(numhit_mod[0][1]<1) goto ENDEVENT;
    //if(numhit_mod[1][0]<1) goto ENDEVENT;
    //if(numhit_mod[1][1]<1) goto ENDEVENT;

    //MC true infor
    //for fSimParticles
    if(mode == 0){

      //Get SimParticles Information//
      int nsimparticles = evtsum->NSimParticles();
      std::cout << "Simparticle # is " << nsimparticles << std::endl;

      for(int isimparticles = 0;isimparticles<nsimparticles;isimparticles++){
        SimParticleSummary* simparticlesum = new SimParticleSummary();
        simparticlesum = evtsum->GetSimParticle(isimparticles);

        // get particle type
        std::stringstream textsimpdg;
        if     (simparticlesum->pdg == 2212){ textsimpdg << isimparticles << ":" <<  "P";}
        else if(simparticlesum->pdg ==-2212){ textsimpdg << isimparticles << ":" <<  "Anti-P";}
        else if(simparticlesum->pdg ==   11){ textsimpdg << isimparticles << ":" <<  "e-";}
        else if(simparticlesum->pdg ==  -11){ textsimpdg << isimparticles << ":" <<  "e+";}
        else if(simparticlesum->pdg ==   22){ textsimpdg << isimparticles << ":" <<  "photon";}
        else if(simparticlesum->pdg == 2112){ textsimpdg << isimparticles << ":" <<  "N";}
        else if(simparticlesum->pdg ==  -13){ textsimpdg << isimparticles << ":" <<  "muon+";}
        else if(simparticlesum->pdg ==   13){ textsimpdg << isimparticles << ":" <<  "muon-";}
        else if(simparticlesum->pdg ==   14){ textsimpdg << isimparticles << ":" <<  "nu_mu";}
        else if(simparticlesum->pdg ==   12){ textsimpdg << isimparticles << ":" <<  "nu_e";}
        else if(simparticlesum->pdg == -211){ textsimpdg << isimparticles << ":" <<  "pi-";}
        else if(simparticlesum->pdg ==  211){ textsimpdg << isimparticles << ":" <<  "pi+";}
        else if(simparticlesum->pdg ==  111){ textsimpdg << isimparticles << ":" <<  "pi0";}
        else{textsimpdg << isimparticles << ":" <<  simparticlesum->pdg;}
        std::cout << " particle" << isimparticles << " is " << textsimpdg.str().c_str() << std::endl;
        std::cout << "   >> energy:" << (int)(simparticlesum->momentum[3]*1000) << "MeV" << std::endl;

        textsimpdg << " " << (int)(simparticlesum->momentum[3]*1000) << "MeV";

        c1->cd();
        //if(isimparticles<5) drawtext(textsimpdg.str().c_str(),0.03,0.20-isimparticles*0.03,0.02);

        //get the initial direction of particles and draw line
        double iX = simparticlesum->ipos[0]*10.;
        double iY = simparticlesum->ipos[1]*10.;
        double iZ = simparticlesum->ipos[2]*10.;
        double fX = simparticlesum->fpos[0]*10.;
        double fY = simparticlesum->fpos[1]*10.;
        double fZ = simparticlesum->fpos[2]*10.;


        double momX = simparticlesum->momentum[0];
        double momY = simparticlesum->momentum[1];
        double momZ = simparticlesum->momentum[2];		
        if(momZ!=0.){
          double tan1 = momX/momZ;
          double tan2 = momY/momZ;
          fZ = momZ/fabs(momZ)*fZ;
          fX = iX + tan1*(fZ-iZ);
          fY = iY + tan2*(fZ-iZ);
        }

        double OFFSET[3] = {0.,0.,0.};
        if(target==7||target==8){ 
          OFFSET[0] = C_B2MotherPosX; 
          OFFSET[1] = C_B2MotherPosY; 
          OFFSET[2] = C_B2MotherPosZ;
        }

        iX = iX - OFFSET[0];
        iY = iY - OFFSET[1];
        iZ = iZ - OFFSET[2];
        fX = fX - OFFSET[0];
        fY = fY - OFFSET[1];
        fZ = fZ - OFFSET[2];
#ifdef DEBUG_DISP
        std::cout << "SimParticle iX:" << iX << " iY:" << iY << " iZ:" << iZ << "\n"
          << "            fX:" << fX << " fY:" << fY << " fZ:" << fZ << std::endl;
#endif
        pad1->cd();  //sideview
        drawdotline(iZ,iY,fZ,fY);	
        pad2->cd(); //topview
        drawdotline(iZ,iX,fZ,fX);

        // draw a corresponding number to each particle
        /*
           std::stringstream textipar;
           textipar << isimparticles;			
           double fx_tmp,fy_tmp,fz_tmp;
           if(fZ>C_B2WMPosZ+C_WMSizeZ){
           fz_tmp = C_B2WMPosZ+C_WMSizeZ;
           fy_tmp = (fY-iY)/(fZ-iZ)*(fz_tmp-iZ)+iY;
           fx_tmp = (fX-iX)/(fZ-iZ)*(fz_tmp-iZ)+iX;
           }
           else if(fZ<C_B2WMPosZ-C_WMSizeZ){
           fz_tmp = C_B2WMPosZ-C_WMSizeZ;
           fy_tmp = (fY-iY)/(fZ-iZ)*(fz_tmp-iZ)+iY;
           fx_tmp = (fX-iX)/(fZ-iZ)*(fz_tmp-iZ)+iX;
           }
           else{ fz_tmp = fZ; y_tmp = fY; fx_tmp = fX; }
           if(fy_tmp>C_B2WMPosY+C_WMSizeY){
           fy_tmp = C_B2WMPosY+C_WMSizeY;				
           fz_tmp = (fZ-iZ)/(fY-iY)*(fy_tmp-iY)+iZ;
           }
           else if(fy_tmp<C_B2WMPosY-C_WMSizeY){
           fy_tmp = C_B2WMPosY-C_WMSizeY;				
           fz_tmp = (fZ-iZ)/(fY-iY)*(fy_tmp-iY)+iZ;
           }
           if(fx_tmp>C_B2WMPosX+C_WMSizeX){
           fx_tmp = C_B2WMPosX+C_WMSizeX;				
           fz_tmp = (fZ-iZ)/(fX-iX)*(fx_tmp-iX)+iZ;
           }
           else if(fx_tmp<C_B2WMPosX-C_WMSizeX){
           fx_tmp = C_B2WMPosX-C_WMSizeX;				
           fz_tmp = (fZ-iZ)/(fX-iX)*(fx_tmp-iX)+iZ;
           }
           pad1->cd(); //sideview
        //drawtext(textipar.str().c_str(),fz_tmp,fy_tmp,0.02);
        pad2->cd(); //topview
        //drawtext(textipar.str().c_str(),fz_tmp,fx_tmp,0.02);
        */


      }//end of SimParticle loop
    }// end of mode 0

    else if(mode==1){		
      int ntwodimrecon = evtsum->NTwoDimRecons();
      cout << "  TwoDim Recon Track # is " << ntwodimrecon <<  endl;
      //#*****Draw track*****#//
      for(int irecon = 0; irecon < ntwodimrecon ; irecon++){
        TwoDimReconSummary* reconsum = new TwoDimReconSummary();
        reconsum = evtsum->GetTwoDimRecon(irecon);

        int view     = reconsum->view;
        int hitmod   = reconsum->hitmod;
        int startpln = reconsum->startpln;
        int endpln   = reconsum->endpln;
        double startz = reconsum->startz;
        double endz   = reconsum->endz;
        double startxy= reconsum->startxy;
        double endxy  = reconsum->endxy;
        double angle  = reconsum->angle*PI/180.;
        double intcpt = reconsum->intcpt;
        double slope  = reconsum->slope;
        std::cout << "  / track" << irecon+1 << ":"
          << " view is " << view
          << " hitmod # is " << hitmod 
          << ": startpln # is " << startpln 
          << ": startz is " << startz 
          << ": startxy is " << startxy 
          << std::endl
          << ": endpln is " << endpln 
          << ": endz is " << endz 
          << ": endxy is " << endxy 
          << ": angle(deg) is " << angle*180./PI 
          << ": intcpt is " << intcpt 
          << ": slope is " << slope 
          << std::endl;
        double iX,iY,iZ,fX,fY,fZ; 
        int startpln_raw, endpln_raw, gridch_raw,grid_raw;
        DetectorDimension *detdim = new DetectorDimension();
        if(hitmod==20||hitmod==21)
          detdim->GetRawPlnChGrid(hitmod,view, startpln,0,0, &startpln_raw, &gridch_raw, &grid_raw);
        else startpln_raw = startpln;
        detdim->GetPosInMod(hitmod,startpln_raw,view,0,&iX,&iY,&iZ);
        iX = startxy; iY = startxy;

        if(hitmod==20||hitmod==21)
          detdim->GetRawPlnChGrid(hitmod,view, endpln  ,0,0, &endpln_raw  , &gridch_raw, &grid_raw);
        else endpln_raw = endpln;
        detdim->GetPosInMod(hitmod,endpln_raw,view,0,&fX,&fY,&fZ);
        fX = endxy; fY = endxy;
        delete detdim;

        //DEBUG
        iX = iY =  intcpt + slope*iZ; fX = fY = intcpt + slope*fZ;

        double OFFSET[3] = {0.,0.,0.};
        if(hitmod==21)
        { OFFSET[0] = C_B2WMPosX; OFFSET[1] = C_B2WMPosY; OFFSET[2] = C_B2WMPosZ; }
        else if(hitmod==22)
        { OFFSET[0] = C_B2CHPosX; OFFSET[1] = C_B2CHPosY; OFFSET[2] = C_B2CHPosZ; }
        else if(hitmod==23)
        { OFFSET[0] = C_B2d1INGPosX; OFFSET[1] = C_B2d1INGPosY; OFFSET[2] = C_B2d1INGPosZ; }
        else if(hitmod==16||hitmod==20)
        { OFFSET[0] = C_PMMotherPosX; OFFSET[1] = C_PMMotherPosY; OFFSET[2] = C_PMMotherPosZ; }

        iX += OFFSET[0]; iY += OFFSET[1]; iZ += OFFSET[2]; 
        fX += OFFSET[0]; fY += OFFSET[1]; fZ += OFFSET[2];



        std::cout << " iX:"<<iX << " fX:"<<fX;
        std::cout << " iY:"<<iY << " fY:"<<fY;
        std::cout << " iZ:"<<iZ << " fZ:"<<fZ;
        std::cout << std::endl;

        // drawing a reconstructed linear line
        if(view==0){  //sideview
          pad1->cd();
          tline(iZ,iY,fZ,fY);
        }
        if(view==1){  //topview
          pad2->cd();
          tline(iZ,iX,fZ,fX);
        }
      }

    }

    //  *****vertex loop start*****  ///
    else if(mode==2){
      int nvertex = evtsum->NThreeDimRecons();
      //DEBUG
      //if(nvertex==0) goto ENDEVENT;
      std::cout << "  Vertex # is " << nvertex << std::endl;


      for(int ivertex = 0;ivertex < nvertex;ivertex++){
        ThreeDimReconSummary* anasum = new ThreeDimReconSummary();
        anasum = evtsum->GetThreeDimRecon(ivertex);
        int ntrack    = anasum -> Ntrack;
        int ningtrack = anasum -> Ningtrack;
        std::cout << "  Track # is " << ntrack 
          << "(ing:" << ningtrack << ")" << std::endl;

        texttrk << "# of Track: " <<  ntrack;

        //if(ntrack!=2) goto ENDEVENT;
        c1->cd();
        //drawtext(texttrk.str().c_str(),0.03,0.25,0.02);


        //  *****Draw track***** //
        vector<bool>::iterator it_ingtrk = anasum->ing_trk.begin();
        for(int itrack = 0; itrack < ntrack ; itrack++){		
          int   startmod  = anasum->ing_startmod[itrack];
          int   endmod    = anasum->ing_endmod[itrack];
          int   startxpln = anasum->startxpln[itrack];
          int   startypln = anasum->startypln[itrack];
          double startxch  = anasum->startxch[itrack];
          double startych  = anasum->startych[itrack];
          int   ingendpln = anasum->ing_endpln[itrack];
          int   endxpln   = anasum->endxpln[itrack];
          int   endypln   = anasum->endypln[itrack];
          double endxch    = anasum->endxch[itrack];
          double endych    = anasum->endych[itrack];
          double thetax    = anasum->thetax[itrack]*PI/180.;
          double thetay    = anasum->thetay[itrack]*PI/180.;

          //DEBUG
          //if(ingendpln!=0) goto ENDEVENT;

          int   stopxpln, stopypln;
          if(*it_ingtrk==true){
            stopxpln = ingendpln;
            stopypln = ingendpln;
          }else{
            stopxpln = endxpln;
            stopypln = endypln;
          }
          if(endmod==-1){
            if(target==5)      endmod   = MOD_ONAXIS_WM;
            else if(target==7) endmod   = MOD_B2_WM;
            else if(target==8) endmod   = MOD_B2_CH;
          }
#ifdef DEBUG_DISP
          std::cout << "  / track"      << itrack+1 
            << " startmod:"     << startmod 
            << " startxpln:"    << startxpln 
            << " startypln:"    << startypln 
            << " startxch:"     << startxch 
            << " startych:"     << startych 
            << " endmod:"       << endmod
            << " stopxpln:"     << stopxpln
            << " stopypln:"     << stopypln
            << " stopxch:"      << endxch
            << " stopych:"      << endych
            << std::endl
            << "   ing_trk:"    << *it_ingtrk
            << " thetax:"       << anasum->thetax[itrack] 
            << " thetay:"       << anasum->thetay[itrack] 
            << std::endl;
#endif

          int mod_vertex = -1;
          if(target==5)      mod_vertex   = MOD_ONAXIS_WM;
          else if(target==7) mod_vertex   = MOD_B2_WM;
          else if(target==8) mod_vertex   = MOD_B2_CH;

          double iX,iY,iZ0,iZ1;
          double fX,fY,fZ0,fZ1;
          iZ0 = zposi(mod_vertex, 0, startxpln, 0);
          iZ1 = zposi(mod_vertex, 1, startypln, 0);
          fZ0 = zposi(endmod    , 0, stopxpln , 0);
          fZ1 = zposi(endmod    , 1, stopypln , 0);
          iX = startych;
          iY = startxch;
          //fX = endych;   
          //fY = endxch;   
#ifdef DEBUG_DISP
          std::cout
            << "  iX :" << iX  << "  fX :" << fX  
            << "  iY :" << iY  << "  fY :" << fY  
            << "  iZ0:" << iZ0 << "  fZ0:" << fZ0 
            << "  iZ1:" << iZ1 << "  fZ1:" << fZ1
            << std::endl;
#endif

          double OFFSET1[3] = {0.,0.,0.};
          double OFFSET2[3] = {0.,0.,0.};

          if(mod_vertex==MOD_B2_WM)
          { OFFSET1[0] = C_B2WMPosX; OFFSET1[1] = C_B2WMPosY; OFFSET1[2] = C_B2WMPosZ; }
          else if(mod_vertex==MOD_B2_CH)
          { OFFSET1[0] = C_B2CHPosX; OFFSET1[1] = C_B2CHPosY; OFFSET1[2] = C_B2CHPosZ; }
          else if(mod_vertex==MOD_B2_INGRID)
          { OFFSET1[0] = C_B2d1INGPosX; OFFSET1[1] = C_B2d1INGPosY; OFFSET1[2] = C_B2d1INGPosZ; }
          else if(mod_vertex==MOD_PM||mod_vertex==MOD_ONAXIS_WM)
          { OFFSET1[0] = C_PMMotherPosX; OFFSET1[1] = C_PMMotherPosY; OFFSET1[2] = C_PMMotherPosZ; }

          if(endmod==MOD_B2_WM)
          { OFFSET2[0] = C_B2WMPosX; OFFSET2[1] = C_B2WMPosY; OFFSET2[2] = C_B2WMPosZ; }
          else if(endmod==MOD_B2_CH)
          { OFFSET2[0] = C_B2CHPosX; OFFSET2[1] = C_B2CHPosY; OFFSET2[2] = C_B2CHPosZ; }
          else if(endmod==MOD_B2_INGRID)
          { OFFSET2[0] = C_B2d1INGPosX; OFFSET2[1] = C_B2d1INGPosY; OFFSET2[2] = C_B2d1INGPosZ; }
          else if(endmod==MOD_PM||endmod==MOD_ONAXIS_WM)
          { OFFSET2[0] = C_PMMotherPosX; OFFSET2[1] = C_PMMotherPosY; OFFSET2[2] = C_PMMotherPosZ; }

          iX  = iX  + OFFSET1[0]; fX  = fX  + OFFSET2[0];
          iY  = iY  + OFFSET1[1]; fY  = fY  + OFFSET2[1];
          iZ0 = iZ0 + OFFSET1[2]; fZ0 = fZ0 + OFFSET2[2];
          iZ1 = iZ1 + OFFSET1[2]; fZ1 = fZ1 + OFFSET2[2];

          fX = iX + tan(thetay)*(fZ0-iZ0);
          fY = iY + tan(thetax)*(fZ1-iZ1);



          for(int VIEW=0;VIEW<2;VIEW++){
            if(VIEW==0){
              pad1->cd();
              tline(iZ0,iY,fZ0,fY);
            }
            if(VIEW==1){
              pad2->cd();
              tline(iZ1,iX,fZ1,fX);
            }
          }
          it_ingtrk++;
        }
      }//Vertex loop end
    }//mode == 2 end

    c1->cd();
    textevt << "Event # is " << ievt;
    drawtext(textevt.str().c_str(),0.05,0.95,0.04);
    texthits << "# of Hits is " << nhits;
    drawtext(texthits.str().c_str(),0.1,0.9,0.03);
    drawtext(readfilename.str().c_str(),0.03,0.005,0.015); 		
    if     (fabs(inttype)==1)  {textinttype << "CCQE";}
    else if(fabs(inttype)==2)  {textinttype << "MEC2p2h";}
    else if(fabs(inttype)>=11&&
        fabs(inttype)<=13) {textinttype << "CC1Pi";}
    else if(fabs(inttype)==16) {textinttype << "CC_coh";}
    else if(fabs(inttype)==21||
        fabs(inttype)==26) {textinttype << "CC_dis";}
    else if(fabs(inttype)>=31&&
        fabs(inttype)<=35) {textinttype << "NC_1Pi";}
    else if(fabs(inttype)==36) {textinttype << "NC_coh";}
    else if(fabs(inttype)==41||
        fabs(inttype)==46) {textinttype << "NC_dis";}
    else if(fabs(inttype)==52) {textinttype << "NC_elastic";}
    else			   {textinttype << "IntType:" << inttype ;}
    drawtext(textinttype.str().c_str(),0.33,0.95,0.04);

    c1->cd(0);
    c1->Update();
    if(SCANNING){
      std::cout << " =========== Next. ========== \n" << std::endl;
    }else{
      printf("  Type \' n\' to move to next event.\n");
      printf("  Type \' s\' to save the event display.\n");
      printf("  Type \' q\' to quit.\n");
      printf("  Type \' e\' to get a event you want.\n");
      printf("  Type \' m\' to change mode.\n");
      printf("  Type any other key to go to the next event.\n");

      while(1){
        char ans[8];
        cin >> ans;
        if(*ans == 'n'){
          std::cout << ">> Next." << std::endl;
          break;
        }
        else if(*ans == 's'){
          std::stringstream filename;
          filename << "event" << ievt << textinttype.str().c_str()<<"_"<<mode<<".eps";
          c1->Print(filename.str().c_str());
          //break;
        }
        else if(*ans == 'q') exit(0);
        else if(*ans == 'e'){
          int eventnumber;
          std::cout << "  Type the number of event :";
          cin >> eventnumber;
          ievt = eventnumber - 1;
          break;
        }
        else if(*ans == 'm'){
          std::cout << "  Type the mode :";
          cin >> mode;
          ievt = ievt - 1;
          break;
        }
        //else{
        printf("  Type \' n\' to move to next event.\n");
        printf("  Type \' s\' to save the event display.\n");
        printf("  Type \' q\' to quit.\n");
        printf("  Type \' e\' to get a event you want.\n");
        printf("  Type \' m\' to change mode.\n");
        printf("  Type any other key to go to the next event.\n");
        //}
      }
    }

ENDEVENT:
    c1->Clear();
  } // Event Loop end

  return 0;
}

//Graphic
double PEpara  = 10.;
double PEth    = 0.1;
double LineWid  = 0.05; 
double LineWid2 = 1.5; 


void sci_ing(double x,double y,double x_len,double y_len,double deg_ing=0.){
  std::complex<double> pos_tmp[5] = {
    std::complex<double>(-x_len/2.,-y_len/2.),
    std::complex<double>( x_len/2.,-y_len/2.),
    std::complex<double>( x_len/2., y_len/2.),
    std::complex<double>(-x_len/2., y_len/2.),
    std::complex<double>(-x_len/2.,-y_len/2.)
  };
  Double_t x_tmp[5], y_tmp[5];
  for(int i=0;i<5;i++){
    pos_tmp[i] *= exp(std::complex<double>(0.,-deg_ing));
    x_tmp[i] = x + pos_tmp[i].real();
    y_tmp[i] = y + pos_tmp[i].imag();
  }

  TPolyLine *pline = new TPolyLine(5,x_tmp,y_tmp);
  pline->SetLineColor(kGreen);
  pline->SetFillStyle(0);
  pline->SetLineWidth(LineWid);
  pline->Draw("SAME");
};

void sci_par(double x,double y,double x_len,double y_len,double deg_par=0.){
  std::complex<double> pos_tmp[5] = {
    std::complex<double>(-x_len/2.,-y_len/2.),
    std::complex<double>( x_len/2.,-y_len/2.),
    std::complex<double>( x_len/2., y_len/2.),
    std::complex<double>(-x_len/2., y_len/2.),
    std::complex<double>(-x_len/2.,-y_len/2.)
  };
  Double_t x_tmp[5], y_tmp[5];
  for(int i=0;i<5;i++){
    pos_tmp[i] *= exp(std::complex<double>(0.,-deg_par));
    x_tmp[i] = x + pos_tmp[i].real();
    y_tmp[i] = y + pos_tmp[i].imag();
  }

  TPolyLine *pline = new TPolyLine(5,x_tmp,y_tmp);
  pline->SetLineColor(kYellow);
  pline->SetFillStyle(0);
  pline->SetLineWidth(LineWid);
  pline->Draw("SAME");
};

void sci_sci(double x,double y,double x1,double y1){
  TBox *b1=new TBox(x,y,x1,y1);
  b1->SetLineColor(kGreen+2);
  b1->SetFillStyle(0);
  b1->SetLineWidth(LineWid);
  b1->Draw("SAME");
};

void sci_veto(double x,double y,double x_len,double y_len,double deg_veto=0.){

  std::complex<double> pos_tmp[5] = {
    std::complex<double>(-x_len/2.,-y_len/2.),
    std::complex<double>( x_len/2.,-y_len/2.),
    std::complex<double>( x_len/2., y_len/2.),
    std::complex<double>(-x_len/2., y_len/2.),
    std::complex<double>(-x_len/2.,-y_len/2.)
  };
  Double_t x_tmp[5], y_tmp[5];
  for(int i=0;i<5;i++){
    pos_tmp[i] *= exp(std::complex<double>(0.,-deg_veto));
    x_tmp[i] = x + pos_tmp[i].real();
    y_tmp[i] = y + pos_tmp[i].imag();
  }

  TPolyLine *pline = new TPolyLine(5,x_tmp,y_tmp);
  pline->SetLineColor(kBlue);
  pline->SetLineWidth(LineWid);
  pline->Draw("SAME");

};

void iron(double x,double y,double x_len,double y_len,double deg_iron=0.){
  std::complex<double> pos_tmp[5] = {
    std::complex<double>(-x_len/2.,-y_len/2.),
    std::complex<double>( x_len/2.,-y_len/2.),
    std::complex<double>( x_len/2., y_len/2.),
    std::complex<double>(-x_len/2., y_len/2.),
    std::complex<double>(-x_len/2.,-y_len/2.)
  };
  Double_t x_tmp[5], y_tmp[5];
  for(int i=0;i<5;i++){
    pos_tmp[i] *= exp(std::complex<double>(0.,-deg_iron));
    x_tmp[i] = x + pos_tmp[i].real();
    y_tmp[i] = y + pos_tmp[i].imag();
  }

  TPolyLine *pline = new TPolyLine(5,x_tmp,y_tmp);
  pline->SetFillColor(17);
  pline->SetLineWidth(LineWid);
  pline->Draw("f SAME");
};

void watertank(double x,double y,double x1,double y1){
  TBox *b1=new TBox(x,y,x1,y1);
  b1->SetFillColor(kBlack);
  b1->SetFillStyle(0);
  b1->SetLineWidth(LineWid);
  b1->Draw("SAME");
};

void DrawProtonModule(int view, double x_center, double y_center, double z_center){
#ifdef DEBUG_DISP
  std::cout << "DrawProtonModule" << std::endl;
#endif
  int pln,ch,ch_num;
  double x,y,z,xy,x_tmp,z_tmp,z_length,xy_length; 
  DetectorDimension *detdim = new DetectorDimension();

  for(int pln=0;pln<22;pln++){
    if(pln==0) ch_num = 24;
    else if(pln<18) ch_num = 32;
    else if(pln<22) ch_num = 17;
    for(ch=0;ch<ch_num;ch++){

      detdim->GetPosPM(pln,view,ch,&x,&y,&z);
      x = x + x_center;
      y = y + y_center;
      z = z + z_center;
      if(view==0) xy = y;
      else        xy = x;
      if (pln<18){
        if(pln==0||ch<8||ch>=24) sci_ing(z,xy,C_INGScintiThick,C_INGScintiWidth);
        else                     sci_ing(z,xy,C_PMScintiThick ,C_PMScintiWidth);
      }
      else if((view==0&&(pln==18||pln==20))||
          (view==1&&(pln==19||pln==21)))
        sci_veto(z,xy,C_INGScintiWidth,C_INGScintiThick);
    }
    if(pln<18){

      detdim->GetPosPM(pln,1-view,0,&x,&y,&z);
      x = x + x_center;
      y = y + y_center;
      z = z + z_center;	  
      if(view==0) xy = y;
      else        xy = x;
      sci_par(z,xy,C_PMScintiThick,C_PMScintiLength);
    }
  }
}

void DrawINGRID(int view, double x_center, double y_center, double z_center, double deg=0.){
#ifdef DEBUG_DISP
  std::cout << "DrawINGRID" << std::endl;
#endif
  int pln,ch,ch_num;
  double x,y,z,xy,x_tmp,z_tmp,z_length,xy_length,rotate; 
  DetectorDimension *detdim = new DetectorDimension();
  rotate = deg*PI/180.;

  for(int pln=0;pln<15;pln++){
    if(pln<11) ch_num = 24;
    else if(pln<15) ch_num = 22;
    for(ch=0;ch<ch_num;ch++){

      detdim->GetPosING(8,pln,view,ch,&x,&y,&z);
      x_tmp = cos(rotate)*x - sin(rotate)*z; 
      z_tmp = sin(rotate)*x + cos(rotate)*z; 

      x = x_tmp + x_center;
      y = y     + y_center;
      z = z_tmp + z_center;
      if(view==0) xy = y;
      else        xy = x;
      z_length  = cos(rotate)*C_INGScintiThick/2. - sin(rotate)*C_INGScintiWidth/2.;
      xy_length = sin(rotate)*C_INGScintiThick/2. + cos(rotate)*C_INGScintiWidth/2.;
      if  (pln<11){
        sci_ing(z,xy,C_INGScintiThick,C_INGScintiWidth,rotate);
      }
      else if((view==0&&(pln==13||pln==14))||
          (view==1&&(pln==11||pln==12)))
        sci_veto(z,xy,C_INGScintiWidth,C_INGScintiThick,rotate);
    }

    if(pln<11){

      detdim->GetPosING(8,pln,1-view,0,&x,&y,&z);
      x_tmp = cos(rotate)*x - sin(rotate)*z; 
      z_tmp = sin(rotate)*x + cos(rotate)*z; 

      x = x_tmp + x_center;
      y = y     + y_center;
      z = z_tmp + z_center;	  
      if(view==0) xy = y;
      else        xy = x;

      sci_par(z,xy,C_INGScintiThick,C_INGScintiLength,rotate);

      if(pln<9){
        detdim->GetPosING(8,pln,1-view,0,&x,&y,&z);
        x_tmp = cos(rotate)*x - sin(rotate)*z; 
        z_tmp = sin(rotate)*x + cos(rotate)*z; 

        x_tmp = x_tmp - sin(rotate)*(C_INGIronStart - C_INGPlnStart);
        z_tmp = z_tmp + cos(rotate)*(C_INGIronStart - C_INGPlnStart);

        x = x_tmp + x_center;
        y = y     + y_center;
        z = z_tmp + z_center;	  
        if(view==0) xy = y;
        else        xy = x;

        iron(z,xy,C_INGIronThick,C_INGIronXY,rotate);
      }
    }
  }
}

void DrawWaterModule(int view, double x_center, double y_center, double z_center){
#ifdef DEBUG_DISP
  std::cout << "DrawWaterModule" << std::endl;
#endif
  int pln,ch,ch_num;
  double x,y,z,xy; 
  double z1,z2,xy1,xy2; 
  DetectorDimension *detdim = new DetectorDimension();

  z1 = z_center-C_WMWaterTargetSizeZ/2.;
  z2 = z_center+C_WMWaterTargetSizeZ/2.;
  if(view==0){
    xy1 = y_center-C_WMWaterTargetSizeY/2.;
    xy2 = y_center+C_WMWaterTargetSizeY/2.;
  }
  else{
    xy1 = x_center-C_WMWaterTargetSizeX/2.;
    xy2 = x_center+C_WMWaterTargetSizeX/2.;
  }
  watertank(z1,xy1,z2,xy2);

  for(int pln=0;pln<8;pln++){
    ch_num = 40;
    for(ch=0;ch<ch_num;ch++){

      detdim->GetPosWM(pln,view,ch,0,&x,&y,&z);
      x = x + x_center;
      y = y + y_center;
      z = z + z_center;
      if(view==0) xy = y;
      else        xy = x;

      sci_ing(z,xy,C_WMScintiThick,C_WMScintiWidth,0.);

      if(ch<20){
        detdim->GetPosWM(pln,view,ch+40,1,&x,&y,&z);
        x = x + x_center;
        y = y + y_center;
        z = z + z_center;
        if(view==0) xy = y;
        else        xy = x;
        sci_ing(z,xy,C_WMScintiWidth,C_WMScintiThick,0.);

        detdim->GetPosWM(pln,view,ch+60,2,&x,&y,&z);
        x = x + x_center;
        y = y + y_center;
        z = z + z_center;
        if(view==0) xy = y;
        else        xy = x;
        sci_ing(z,xy,C_WMScintiWidth,C_WMScintiThick,0.);
      }
    }

    detdim->GetPosWM(pln,1-view,0,0,&x,&y,&z);
    x = x + x_center;
    y = y + y_center;
    z = z + z_center;	  
    if(view==0) xy = y;
    else        xy = x;

    sci_par(z,xy,C_WMScintiThick,C_WMScintiLength,0.);
  }

  delete detdim;
}

void DrawHits(int mod, int pln, int view, int ch, double pe, double x_center, double y_center, double z_center, double deg){
  double X=0.,Y=0.,Z=0.,R=0., XY=0.;
  double X_tmp,Z_tmp;
  double rotate = deg*PI/180.;
  DetectorDimension *detdim = new DetectorDimension();
  detdim->GetPosInMod(mod,pln,view,ch,&X,&Y,&Z);
  delete detdim;
  X_tmp = cos(rotate)*X - sin(rotate)*Z; 
  Z_tmp = sin(rotate)*X + cos(rotate)*Z; 

  X = X_tmp + x_center;
  Y = Y     + y_center;
  Z = Z_tmp + z_center;	  
  if(view==0) XY=Y;
  else        XY=X;

  /*
#ifdef DEBUG_DISP
std::cout << "             HitPos X:" << X << " Y:" << Y << " Z:" << Z << std::endl;
#endif
*/
  if(pe<PEth)R=0.;
  else R=sqrt(pe-PEth)*PEpara;

  //R = PEpara;

  TArc *arc=new TArc(Z,XY,R);
  arc->SetFillColor(kRed);
  arc->SetLineColor(kRed);
  arc->SetLineWidth(LineWid);
  arc->Draw("SAME");
}


//sideview
void drawx(int targetmod){
  int modules[3];
  int pln,ch,ch_num;
  double x,y,z;
  double offset_x,offset_y,offset_z;

  if(targetmod==7||targetmod==8){
    modules[0] = 21;
    modules[1] = 22;
    modules[2] = 23;
  }
  else if(targetmod==5){
    modules[0] = 20;
    modules[1] = 3;
    modules[2] = -1;
  }

  for(int mod=0;mod<3;mod++){
    if(modules[mod]==3){
      offset_x = C_INGHMotherPosX;
      offset_y = C_INGHMotherPosY;
      offset_z = C_INGHMotherPosZ;
      DrawINGRID(0,offset_x,offset_y,offset_z);
    }
    if(modules[mod]==20){
      offset_x = C_PMMotherPosX;
      offset_y = C_PMMotherPosY;
      offset_z = C_PMMotherPosZ;
      DrawWaterModule(0,offset_x,offset_y,offset_z);
    }
    if(modules[mod]==21){
      offset_x = C_B2WMPosX;
      offset_y = C_B2WMPosY;
      offset_z = C_B2WMPosZ;
      DrawWaterModule(0,offset_x,offset_y,offset_z);
    }
    if(modules[mod]==22){
      offset_x = C_B2CHPosX;
      offset_y = C_B2CHPosY;
      offset_z = C_B2CHPosZ;
      DrawProtonModule(0,offset_x,offset_y,offset_z);
    }
    if(modules[mod]==23){
      offset_x = C_B2d1INGPosX;
      offset_y = C_B2d1INGPosY;
      offset_z = C_B2d1INGPosZ;
      DrawINGRID(0,offset_x,offset_y,offset_z);
    }
  }
};

//topview
void drawy(int targetmod){
  int modules[3];
  int pln,ch,ch_num;
  double x,y,z;
  double offset_x,offset_y,offset_z;

  if(targetmod==7||targetmod==8){
    modules[0] = 21;
    modules[1] = 22;
    modules[2] = 23;
  }
  else if(targetmod==5){
    modules[0] = 20;
    modules[1] = 3;
    modules[2] = -1;
  }

  for(int mod=0;mod<3;mod++){
    if(modules[mod]==3){
      offset_x = C_INGHMotherPosX;
      offset_y = C_INGHMotherPosY;
      offset_z = C_INGHMotherPosZ;
      DrawINGRID(1,offset_x,offset_y,offset_z);
    }
    if(modules[mod]==20){
      offset_x = C_PMMotherPosX;
      offset_y = C_PMMotherPosY;
      offset_z = C_PMMotherPosZ;
      DrawWaterModule(1,offset_x,offset_y,offset_z);
    }
    if(modules[mod]==21){
      offset_x = C_B2WMPosX;
      offset_y = C_B2WMPosY;
      offset_z = C_B2WMPosZ;
      DrawWaterModule(1,offset_x,offset_y,offset_z);
    }
    if(modules[mod]==22){
      offset_x = C_B2CHPosX;
      offset_y = C_B2CHPosY;
      offset_z = C_B2CHPosZ;
      DrawProtonModule(1,offset_x,offset_y,offset_z);
    }
    if(modules[mod]==23){
      offset_x = C_B2d1INGPosX;
      offset_y = C_B2d1INGPosY;
      offset_z = C_B2d1INGPosZ;
      DrawINGRID(1,offset_x,offset_y,offset_z);
    }
  }
};

void tline(double iX,double iY,double fX,double fY){
  TLine *l1=new TLine(iX,iY,fX,fY);
  l1->SetLineWidth(LineWid2);
  l1->Draw("SAME");
};

void drawdotline(double iX,double iY,double fX,double fY){
  TLine *l1=new TLine(iX,iY,fX,fY);
  l1->SetLineStyle(2);
  l1->SetLineColor(kBlue);
  l1->SetLineWidth(LineWid2);
  l1->Draw("SAME");
};

void drawtext(const char* text, double x, double y,double font){
  TText *t1 = new TText(x,y,text);
  t1->SetTextSize(font);  
  t1->Draw("SAME");
}
