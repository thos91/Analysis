/* ***********************************************************************
 * Online monitor program for wagasci 
 * Program : om_wagasci.C
 * Name: Naruhiro Chikuma and Giorgio Pintaudi
 * Date: 2018-10-19 22:35
 * Copyright 2018 Naruhiro Chikuma and Giorgio Pintaudi
 * This file is part of anpan
 * ********************************************************************** */

// #define __HttpServer__
// For Http server, ROOT needs to be compiled with HTTP mode.

//#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <string>
#include <TStyle.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TF1.h>
#include <TGraph.h>
#include <TGraph2D.h>
#include <TThread.h>
#include <TCanvas.h>
#include <TString.h>
#include <TApplication.h>
#include <TFrame.h>
#include <TGaxis.h>
#include <TText.h>
#include <TLine.h>
#include <TBox.h>
#include <TLegend.h>
#include <pyrame.h>
#include <X11/Xlib.h>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

using namespace std;

#ifdef __HttpServer__
#include <THttpServer.h>
#endif

#define NumHist 10              // Total number of the Histograms that are shown

#define NumSca 16               // Switched Capacitor Array (# of capacitors)
#define NumChip 20              // # of ASUs for each DIF
#define NumChipCh 32            // Number of MPPCs for each ASU
#define NumPln 8                // Number of planes
#define NumCh 80                // Numer of MPPCs for each plane
#define NumChAll NumPln*NumCh   // Total number of MPPCs (640) for each DIF

// These macros are used for the time measurement
#define OffsetTimeEven 4000.
#define OffsetTimeOdd 500.
#define RampTimeEven -1.
#define RampTimeOdd 1.
#define NormBCID 580 //ns
#define ExpectedBCIDstart 24
#define ExpectedHitTiming 100 //ns from beginning of bcid
#define ExpectedTimingWidth 100 //ns

#define NbUpdateBlock 100
#define NbSubUpdate 10
#define SpillNbUpdate 100          // Every SpillNbUpdate we update what? TO-CHECK
#define EvtDispUpdate 1            // Every EvtDispUpdate blocks we
				   // update the real-time online-monitor.
                                   // If it is 1, it means that we update every
				   // block, if 2 every two blocks, etc...
#define NbHit_Threshold 1

#define SPILL_UGLY_THRESHOLD 10    // This macros are used when calculating
#define SPILL_BAD_THRESHOLD  100   // the spill gap
#define STATUS_SPILL_GOOD 1.
#define STATUS_SPILL_UGLY 2.
#define STATUS_SPILL_BAD  3.
#define SideView 0
#define TopVeiw 1

#define WATERTANK_X 466  //mm
#define WATERTANK_Y 1256 //mm
#define SCINTI_W 25 //mm width
#define SCINTI_T 3  //mm thickness

#define DIF_1 1 // dif_id of the first DIF
#define DIF_2 2 // dif_id of the second DIF

struct map_cell 
{
  Int_t pln;
  Int_t ch;
  Float_t x;
  Float_t y;
  Float_t z;
} mapcell;

// ==================================================================

// The or_ws struct contains blocks and events parameters and the
// histograms needed for event display.

struct or_ws 
{
  // --- block&event parameters ---
  bool first_time; // Flag set at the beginning of the online monitor execution
                   // to pick the first event
  Int_t nevt;      // number of events
  Int_t nblock;    // number of blocks

  struct map_cell **mapping; // Detector mapping
  struct evloop *loop;       // Event loop (loop through the new events as they come)

  Int_t ini_spill;
  Int_t last_spill;
  Int_t lastblock_evtdisp; // Last block that was shown in the
			   // real-time event display

  Int_t mode;   // mode 0:beam, 1:pedestal
  Int_t dif_id; // can only be 1 or 2

  // --- main histograms ---
  TCanvas *canvas;
  TPad *pad[NumHist];
  TH1F *h_charge;
  TH1F *h_spillnb;
  TH1F *h_nrate;         // Each bin correspond to a channel and displays the
                         // value of the noise for that channel
  TH1F *h_gain;          // Each bin correspond to a channel and displays the
                         // value of the gain for that channel
  TH1F *h_bcid;
  TH1F *h_timing;
  TH2F *h_nrate_st;
  TH2F *h_gain_st;       // Time profile of the gain for all the channels
                         // (gain stability)
  TH2F *h_status;        // This histogram contains the instantaneus
			 // status of the detector for every block of
			 // events. It displays the following
			 // parameters: gain, noise rate, hit timing,
			 // bcid, ADC value, spill gap, hit channel,
			 // nothing.
  TGraph *g_evtdisp2D;

  // --- OM status ---
  TText *txt_time;
  TText *txt_spill;
  Int_t nb_out_nr[NumChip];
  Int_t nb_out_gain[NumChAll];
  Int_t nb_bcid;
  Int_t nb_out_bcid;
  Int_t nb_out_timing;
  Int_t mean_charge;
  Int_t nb_spillgap;
  Int_t nb_nonhit_ch;
  // --- for gain fitting ---
  TH1F *h_gain_ped[NumChAll];       // pedistal distribution
  TH1F *h_gain_hit[NumChAll];       // first hit distribution (maily dark noise)
  // --- for noise rate calc. ---
  Int_t nrate_chip[NumChip];        // # of hits for every chip
  Int_t nrate[NumChAll];            // # of hits for every channel
  // --- for event disp. ---
  Int_t nb_hits[NumSca];            // # of hits for every SCA
  Int_t hit_pln[NumSca][NumPln];    // hit flag for every SCA and plane (0 or 1)
  Int_t hit_ch[NumSca][NumCh];      // hit flag for every SCA and channel (0 or 1)

#ifdef __HttpServer__
  THttpServer *serv;
#endif
};

// ==================================================================

struct map_cell **load_mapping(Int_t dif_id   = 0, 
    const char *filename = "wagasci_mapping_table.txt",
    Int_t nb_chips = NumChip,
    Int_t nb_chans = NumChipCh
    ) 
{

  struct map_cell **mapping;
  FILE *f;
  Int_t i;
  Int_t dif,chip,chip_ch;
  Int_t pln,ch;
  Float_t x,y,z;
  int res;

  mapping=(struct map_cell**)malloc(sizeof(struct map_cell *)*nb_chips);
  for (i=0;i<nb_chips;i++)
    mapping[i]=(struct map_cell*)malloc(sizeof(struct map_cell)*nb_chans);

  //open mapping file
  f=fopen(filename,"r");
  if (f==NULL) {
    printf("cant open file %s\n",filename);
    return NULL;
  }

  //reading mapping
  while(!feof(f)) {
    res=fscanf(f,"%d %d %d %f %f %f %d %d",&dif,&chip,&chip_ch,&x,&y,&z,&pln,&ch);
    if ((res!=8)&&(res!=-1)) {
      printf("syntax error in file %s obtained %d items instead of 8\n",filename,res);
    }
    else if(dif==dif_id-1){
      mapping[chip][chip_ch].x   = x;
      mapping[chip][chip_ch].y   = y;
      mapping[chip][chip_ch].z   = z;
      mapping[chip][chip_ch].pln = pln;
      mapping[chip][chip_ch].ch  = ch;
    }
  } 
  fclose(f);
  return mapping;
} //load_mapping

// ==================================================================

bool isExpectedBcid(Int_t bcid)
{
  if(bcid>=ExpectedBCIDstart&&bcid<ExpectedBCIDstart+8) return true;
  else return false;
} //isExpectedBcid

// ==================================================================

bool isExpectedTiming(Int_t bcid, Float_t hittiming)
{
  Float_t offset_time = bcid%2==0 ? OffsetTimeEven : OffsetTimeOdd;
  Float_t ramp_time   = bcid%2==0 ? RampTimeEven   : RampTimeOdd; 
  Float_t exp_timing  = bcid*NormBCID + (ExpectedHitTiming-offset_time)*ramp_time;
  if(fabs(hittiming - exp_timing)*2<ExpectedTimingWidth ) return true;
  else return false;
} //isExpectedTiming

// ==================================================================

void newblock(void *workspace,struct block *b) 
{
  Int_t spillnb = (Int_t)atoi(get_block_field(b,"spill"));

  struct or_ws *ws=(struct or_ws *)workspace;
  if (ws->first_time) {
    printf("waiting for app to start\n");
    sleep(1);
    ws->nblock = 0;

    ws->ini_spill = spillnb;
    ws->last_spill = spillnb-1;
    ws->lastblock_evtdisp = 0;

    ws->first_time=false;
  }
  ws->nevt = 0;  // We still haven't filled the block.

  // ------- hist info for event disp ----------------  
  for(int isca=0;isca<NumSca;isca++){
    ws->nb_hits[isca] = 0;
    fill_n(begin(ws->hit_pln[isca]), NumPln, 0);
    fill_n(begin(ws->hit_ch [isca]), NumCh,  0);
    // memset(ws->hit_pln[isca],0,NumPln);
    // memset(ws->hit_ch [isca],0,NumCh );
  }

  if(ws->nblock==0){

    // ------- for noise rate calc. ----------------
    fill_n(begin(ws->nrate_chip), NumChip,  0);
    fill_n(begin(ws->nrate),      NumChAll, 0);
    // memset(ws->nrate_chip,0,NumChip);
    // memset(ws->nrate,0,NumChAll);

    // ------- new histograms for fit ----------------  
    for(int ich=0;ich<NumChAll;ich++){
      string tmp;
      tmp=Form("gain_ped_%05d",ich);
      ws->h_gain_ped[ich] = new TH1F(tmp.c_str(),tmp.c_str(),400,400.,800.);
      tmp=Form("gain_hit_%05d",ich);
      ws->h_gain_hit[ich] = new TH1F(tmp.c_str(),tmp.c_str(),400,400.,800.);
    }
  }

  // ------ for status pad -------
  struct tm tm;
  char *dayofweek[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
  time_t t = time(NULL);
  localtime_r(&t, &tm);
  string SpillText = Form("SPILL#: %05d", spillnb);
  string TimeText = Form("Updated: %04d/%02d/%02d %s %02d:%02d:%02d\n",
      tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
      dayofweek[tm.tm_wday], tm.tm_hour, tm.tm_min, tm.tm_sec);
  ws->txt_spill->SetText(0.82,0.95,SpillText.c_str());
  ws->txt_time ->SetText(0.82,0.91,TimeText.c_str());

  printf("new spill : %d\n",spillnb);

} //newblock

// ==================================================================

void endblock(void *workspace,struct block *b) 
{
  struct or_ws *ws=(struct or_ws *)workspace;
  Int_t spillnb = (Int_t)atoi(get_block_field(b,"spill"));
  Int_t spillflag = (Int_t)atoi(get_block_field(b,"spill_flag"));

  Int_t time_endblock = (Int_t)time(NULL);


  // ------- Filling spill number ----------------
  // If we are in beam mode and taking data during a spill
  if ( ((ws->mode==0)&&(spillflag==1))||
       // Or if we are in pedestal mode and this is the NbSubUpdate-th
       // block since the last update
       ((ws->mode==1)&&(ws->nblock % NbSubUpdate==0)) )
    {
      // How many spills since the last Spill Update? Fill the
      // h_spillnb histogram with that number.
      ws->h_spillnb->Fill( (spillnb - ws->ini_spill) % SpillNbUpdate);
    }

  // ------- Drawing event display ----------------  
  int sca_with_max_nb_hits = 0;
  int *d; // temporary variables
  int max_nbhits = NbHit_Threshold;

  // If we have at least one hit this routine finds which is the SCA with
  // The highest number of hits
  d = max_element(ws->nb_hits, ws->nb_hits + NumSca);
  if ( *d >= max_nbhits )
    sca_with_max_nb_hits = distance(begin(ws->nb_hits), d);

  // This routine creates the 2D online monitor for WAGASCI detector
  // If the hi_pln and hit_ch arrays contain a hit that point is plotted
  // TO-CHECK it is not clear if the hit_pln and hit_ch arrays contain only
  // a hit flag or total number of hits
  if(sca_with_max_nb_hits > 0){
    for(int chip=0;chip<NumChip;chip++){
      for(int chip_ch=0;chip_ch<NumChipCh;chip_ch++){
        Int_t pln = ws->mapping[chip][chip_ch].pln;
        Int_t ch  = ws->mapping[chip][chip_ch].ch;
        Int_t ip = pln*NumCh + ch;              // Point number
        if((ws->hit_pln[sca_with_max_nb_hits][pln]==1) &&
	   (ws->hit_ch[sca_with_max_nb_hits][ch]==1))
	  {
	    Float_t x   = ws->mapping[chip][chip_ch].x;
	    Float_t y   = ws->mapping[chip][chip_ch].y;
	    Float_t z   = ws->mapping[chip][chip_ch].z;

	    Float_t XX, YY;
	    XX = z;
	    if     (ws->dif_id == DIF_1){ YY = y; }
	    else if(ws->dif_id == DIF_2){ YY = x; }
	    ws->g_evtdisp2D->SetPoint(ip,XX,YY);
	  }
	// If there is a hit but it is not included in the hit_pln and hit_ch arrays
	// it means that it happened on the surface of the detector TO-CHECK
        else{
          ws->g_evtdisp2D->SetPoint(ip,-WATERTANK_X,-WATERTANK_Y);
        }
      }
    }
    // TO-CHECK what is lastblock_evtdisp? maybe is the last block that was
    // shown in the 2D online monitor
    ws->lastblock_evtdisp = ws->nblock;
  }

  // Every NbUpdateBlock blocks this routine is run
  if(ws->nblock < NbUpdateBlock-1){ ws->nblock++; }
  else{
    ws->nblock=0; 

    // ---------- noise rate per chip calculation -------------
    for(int ichip=0;ichip<NumChip;ichip++){
      Float_t noise_rate_chip; //kHz
      Float_t noise_rate_ch;   //kHz
      // Noise rate for every
      if(ws->nrate_chip[ichip]!=0){
	/* NormBCID = 580ns is the time duration for one BCID, i.e. the TCD ramp
	   NumSca = 16 is the number of Switched Capacitor Array for every channel
	   NbUpdateBlock = 100 is the number of blocks over which we are averaging
	   the noise
	   TO-CHECK */
        noise_rate_chip = 1e+6*ws->nrate_chip[ichip]/(NormBCID*NumSca*NbUpdateBlock);
      }
      // Apparently the theoretical least amount of noise is 100 Hz
      else{ noise_rate_chip = 1e-1; }
      // Fill the h_nrate histogram. Everybin refers to a chip.
      ws->h_nrate->SetBinContent(ichip+1,noise_rate_chip);
      // Do the same for every channel
      for(int ichipch=0;ichipch<NumChipCh;ichipch++){
        if(ws->nrate[ichip*NumChipCh+ichipch]==0){ noise_rate_ch = 1e-1; }
        else{ 
          noise_rate_ch = 1e+6*ws->nrate[ichip*NumChipCh+ichipch] /
	    (NormBCID*NumSca*NbUpdateBlock);
        }
        ws->h_nrate_st->Fill(time_endblock,noise_rate_ch);
      }
    }

    // ---------- noise rate & gain calculation -------------
    for(int ich=0;ich<NumChAll;ich++){
      Int_t pedestal;
      Int_t hit_pos ;
      Float_t dist;

      // If the gain histograms are empty the distance is assumed to be 1/10 of
      // an ADC count
      if(ws->h_gain_hit[ich]->GetEntries()==0||ws->h_gain_ped[ich]->GetEntries()==0){ 
        dist = 1e-1;
      }
      else{
	// Define two gaussians between 400 and 800 ADC counts (x-axis)
	// "gaus" Gaussian function with 3 parameters:
	// f(x) = p0 * exp( -0.5 * ( (x - p1) / p2 )^2 )
        TF1* fit_ped = new TF1("fit_ped","gaus",400.,800.);
        TF1* fit_hit = new TF1("fit_hit","gaus",400.,800.);
        Float_t sigma = 10.;
        Float_t lim_sigma = 2.;
        Float_t lim_pos   = 10.;
        pedestal=500; //TO-DO extract from pedestal file
        hit_pos =525; //TO-DO extract from dont know where
	// p0 = 1.
	// p1 = pedestal
	// p2 = sigma
        fit_ped->SetParameters(1.,pedestal,sigma);
	// p0 = 1.
	// p1 = hit_pos
	// p2 = sigma
        fit_hit->SetParameters(1.,hit_pos ,sigma);
	// Fix p0 = 1
        fit_ped->FixParameter(0,1.);
	// Fix p0 = 1
        fit_hit->FixParameter(0,1.);
	// The gaussian peak cannot be more than lim_pos from the assumed position
        fit_ped->SetParLimits(1,pedestal-lim_pos,pedestal+lim_pos);
        fit_hit->SetParLimits(1,hit_pos -lim_pos,hit_pos +lim_pos);
	// The gaussian sigma cannot be more than lim_sigma from the assumed value
        fit_ped->SetParLimits(2,sigma-lim_sigma,sigma+lim_sigma);
        fit_hit->SetParLimits(2,sigma-lim_sigma,sigma+lim_sigma);
	// Scale(): Multiply this histogram by a constant c1.
	// this = c1*this
	// GetMaximum(): Return maximum value smaller than
	// maxval of bins in the range, unless the value has been
	// overridden by TH1::SetMaximum, in which case it returns
	// that value.
	// These two lines normalize the histogram so that the highest bin is unity
	ws->h_gain_ped[ich]->Scale(1./ws->h_gain_ped[ich]->GetMaximum());
        ws->h_gain_hit[ich]->Scale(1./ws->h_gain_hit[ich]->GetMaximum());
        //ws->h_gain_ped[ich]->Fit("fit_ped","Q","0",400.,800.);
        //ws->h_gain_hit[ich]->Fit("fit_hit","Q","0",400.,800.);

        dist = fit_hit->GetParameter(1)-fit_ped->GetParameter(1);

        delete fit_ped;
        delete fit_hit;
      }

      ws->h_gain->SetBinContent(ich+1,dist);
      ws->h_gain_st ->Fill(time_endblock,dist);

      delete ws->h_gain_ped[ich];
      delete ws->h_gain_hit[ich];
    }
  }

  // ------- for spill gap ----------------
  // Recall that spillnb is relative to the current block.
  // When the block is created in the newblock function:
  //    ws->ini_spill  = spillnb;
  //    ws->last_spill = spillnb - 1;

  // So when the block is closed the (spillnb - ws->last_spill)
  // difference should be one. If it is more than one it may mean that
  // we didn't have any events in one spill. TO-CHECK

  // When the ws->last_spill=65535 the spill number is reset to
  // zero. If the zero value is skipped for some reason we have a
  // spillgap

  if( (spillnb - ws->last_spill > 1) ||
      ((ws->last_spill==65535)&&(spillnb!=0))) 
    {
      ws->nb_spillgap++;
    }
  // At this point we can reset the last spill to the current spill number
  ws->last_spill = spillnb;

  Float_t status_spill;
  // If we have less that SPILL_UGLY_THRESHOLD=10 skipped spills it is OK.
  // If we have less that SPILL_BAD_THRESHOLD=100 skipped spills
  // it is fishy but not impossible.
  // If we have more than 100 or negative skipped spills there is
  // something wrong.
  if     (ws->nb_spillgap < SPILL_UGLY_THRESHOLD ) status_spill = STATUS_SPILL_GOOD;
  else if(ws->nb_spillgap < SPILL_BAD_THRESHOLD  ) status_spill = STATUS_SPILL_UGLY;
  else                                             status_spill = STATUS_SPILL_BAD;

  
  // ------- Fill the OM status ----------------  
  ws->h_status->SetBinContent(1,2,2.); //gain
  ws->h_status->SetBinContent(1,1,1.); //noise rate
  ws->h_status->SetBinContent(2,2,1.); //hit timing 
  ws->h_status->SetBinContent(2,1,3.); //bcid 
  ws->h_status->SetBinContent(3,2,1.); //ADC value
  ws->h_status->SetBinContent(3,1,status_spill); //spill gap
  ws->h_status->SetBinContent(4,2,1.); //hit channel
  ws->h_status->SetBinContent(4,1,0.); //nothing

#ifdef DEBUG
  printf("time:%d \n",(Int_t)time(NULL));
  printf("  number of events: %d \n",ws->nevt);
#endif


} // endblock

// ==================================================================

// TO-CHECK maybe an empty reinit function is added for compatibility
void reinit(void *workspace) 
{
} //reinit

// ==================================================================

// TO-CHECK maybe an empty reinit function is added for compatibility
void endrun(void *workspace) 
{
} //reinit

// ==================================================================

void newevt(void *workspace,struct event *e) 
{
  struct or_ws *ws=(struct or_ws *)workspace;
  //Int_t spillnb  =(Int_t)atoi(get_event_field(e,"spill"   ));
  Int_t spillflag=(Int_t)atoi(get_event_field(e,"spill_flag")); 
  Int_t chip     =(Int_t)atoi(get_event_field(e,"roc"       )); 
  Int_t chip_ch  =(Int_t)atoi(get_event_field(e,"rocchan"   )); 
  Int_t bcid     =(Int_t)atoi(get_event_field(e,"bcid"      )); 
  Int_t sca      =(Int_t)atoi(get_event_field(e,"sca"       ));
  Int_t pln      =(Int_t)atoi(get_event_field(e,"plane"     )); 
  Int_t ch       =(Int_t)atoi(get_event_field(e,"channel"   )); 
  Int_t energy   =(Int_t)atoi(get_event_field(e,"en"        )); 
  Int_t data_time=(Int_t)atoi(get_event_field(e,"time"      )); 
  Int_t hit      =(Int_t)atoi(get_event_field(e,"hit"       )); 
  Int_t gain     =(Int_t)atoi(get_event_field(e,"gain"      ));
  //Float_t x    = (Float_t)atoi(get_event_field(e,"x"));
  //Float_t y    = (Float_t)atoi(get_event_field(e,"y"));
  //Float_t z    = (Float_t)atoi(get_event_field(e,"z"));


#ifdef DEBUG
  printf("New event %d == spillnb:%d, spillflag:%d, chip:%d, chip_ch:%d, bcid:%d, sca:%d, pln:%d, ch:%d, energy:%d, data_time:%d, hit:%d, gain:%d\n",
      ws->nevt,spillnb,spillflag,chip,chip_ch,bcid,sca,pln,ch,energy,data_time,hit,gain);
#endif

  // ------- Fill energy histogram ----------------
  // If we have a hit ...
  if(hit==1){
    // ... and are in beam mode during a spill or in pedestal mode
    // Basically, in beam mode, we are escluding hits from cosmic rays.
    if ( ((ws->mode==0)&&(spillflag==1)) || (ws->mode==1) )
    {
      ws->h_charge->Fill(energy);
    }
  }

  // ------- Counting number of hits for noise rate calcu. ----------------
  if(spillflag==0){ //internal trigger
    // If we have no hits it means that the charge collected is the pedestal
    if(hit==0){ ws->h_gain_ped[chip*NumChipCh+chip_ch]->Fill(energy);}
    else{ //if hit >= 1
      // Else if we DO have a hit, most probably it is due to noise. It may
      // be due to cosmic rays but we ignore that possibility here because
      // it is much more unlikely.
      // TO-CHECK why should BCID be less than 16. Maybe this is an error!
      if(bcid < NumSca){
        ws->nrate_chip[chip]++;
        ws->nrate[chip*NumChipCh+chip_ch]++;
      }
      // and if BCID is more than 16?? What happens??
      // If we are measuring the gain the
      if(gain==1){ ws->h_gain_hit[chip*NumChipCh+chip_ch]->Fill(energy);}
    }
  }

  // ------- Filling hit timing & bcid  ----------------
  // If we are in beam mode and during a spill
  if ( ((ws->mode==0)&&(spillflag==1))||
       // or if we are in pedestall mode (internal trigger)
       // and this is the NbSubupdate-th (=10) block
      ((ws->mode==1)&&(ws->nblock%NbSubUpdate==0)) )
  {
    Float_t offset_time = bcid%2==0 ? OffsetTimeEven : OffsetTimeOdd;
    Float_t ramp_time   = bcid%2==0 ? RampTimeEven   : RampTimeOdd;
    Float_t hittiming = bcid*NormBCID + (data_time-offset_time)*ramp_time; //TO-ASK

    ws->h_bcid   -> Fill(bcid);
    ws->h_timing -> Fill(hittiming);
  }

  // -------------- Hits info for event display ----------------
  // Every EvtDispUpdate blocks we update the real-time
  // online-monitor. If it is 1, it means that we update every block,
  // if 2 every two, etc...
  if( (ws->nblock - ws->lastblock_evtdisp >= EvtDispUpdate) || (ws->nblock == 0) )
    {
      // If we are in beam mode and during a spill
      if( ((ws->mode==0)&&(spillflag==1))||
	  // or if we are in pedestall mode (internal trigger)
	  // and this is the NbSubupdate-th (=10) block
	  ((ws->mode==1)&&(ws->nblock%NbSubUpdate==0)) )
	{
	  if(hit==1){
	    ws->nb_hits[sca]++;
	    ws->hit_pln[sca][pln]=1;
	    ws->hit_ch [sca][ ch]=1;
	  }
	}
    }

#ifdef DEBUG
  if (ws->nblock - ws->lastblock_evtdisp < EvtDispUpdate)
    {
      printf("Houston, we've had a problem here. \"ws->nblock\" = %d, "
	     "\"ws->lastblock_evtdisp\" = %d, \"EvtDispUpdate\" = %d\n",
	     &ws->nblock, &ws->lastblock_evtdisp, &EvtDispUpdate);
    }
#endif

  ws->nevt++;

  // TO-CHECK why these lines are commented?
  //ws->canvas->Modified();
  //ws->canvas->Update();

} //newevt

// ==================================================================

// If the scintillator is of the grid type the wifth and thickness are swapped
// If the grid flag "grid" is 1, the scintillator is of the grid type
bool DrawScintillator(Float_t x, Float_t y, bool grid)
{
  Float_t sci_x, sci_y;
  if(grid){ sci_x=SCINTI_W; sci_y=SCINTI_T; }
  else    { sci_x=SCINTI_T; sci_y=SCINTI_W; }

  // Draw a scintillator around the point (x,y)
  TBox *scinti = new TBox(x-sci_x/2.,y-sci_y/2.,x+sci_x/2.,y+sci_y/2.);
  scinti->SetLineColor(kSpring-1);
  scinti->SetLineWidth(1);
  scinti->Draw("l same");
  return true;
} //DrawScintillator

// ==================================================================

// Too boring to be worth reading
bool DrawModule(Int_t dif_id, struct map_cell **mapping)
{
  Double_t FrameMargin = 10.;
  TH1F *frame = gPad->DrawFrame(
      -WATERTANK_X/2.-FrameMargin,
      -WATERTANK_Y/2.-FrameMargin,
      WATERTANK_X/2. +FrameMargin,
      WATERTANK_Y/2. +FrameMargin);
  frame->GetXaxis()->SetTickLength(0.);
  frame->GetXaxis()->SetLabelSize (0.);
  frame->GetYaxis()->SetTickLength(0.);
  frame->GetYaxis()->SetLabelSize (0.);

  TBox *watertank = new TBox(
      -WATERTANK_X/2.,
      -WATERTANK_Y/2.,
      WATERTANK_X/2.,
      WATERTANK_Y/2.);
  watertank->SetFillColor(kCyan-10);
  watertank->Draw("same");

  for(int ichip=0;ichip<NumChip;ichip++){
    for(int ichipch=0;ichipch<NumChipCh;ichipch++){
      Int_t pln = mapping[ichip][ichipch].pln;
      Int_t ch  = mapping[ichip][ichipch].ch;
      if((pln<0)||(pln>7)||(ch<0)||(ch>79)){
        printf("wrong alignment of pln/ch, pln:%d ch:%d\n",pln,ch);
        return false;
      }
      bool isGrid;
      if(ch<40){isGrid=false;}else{isGrid=true;}
      Float_t XX,YY;
      XX = mapping[ichip][ichipch].z;
      if     ((dif_id-1)==SideView){ YY = mapping[ichip][ichipch].y; }
      else if((dif_id-1)==TopVeiw ){ YY = mapping[ichip][ichipch].x; }
      else{
        printf("wrong dif id\n");
        return false;
      }
#ifdef DEBUG
      printf("XX:%f, YY:%f\n",XX,YY);
#endif
      if(!DrawScintillator(XX,YY,isGrid)){ return false; }
    }
  }
  return true;
}//DrawModule

// ========================== MAIN =============================

int main(int argc, char **argv) 
{
  // Apparently the WAGASCI online-monitor must be run separately for every DIF
  // It takes two arguments: the dif number and the detector mode.

  Int_t dif_id = 0;
  Int_t mode = 0;
  string input;

  try {

    /* Description of all the options that can be passed  */
    po::options_description desc("Allowed options");
    desc.add_options()
      ("help", "produce help message")
      ("dif_id,d", po::value<Int_t>()->default_value(1), "DIF identification number")
      ("mode,m",   po::value<Int_t>()->default_value(1), "Beam mode. mode 0:beam, 1:pedestal")
      ("input,i",  po::value<string>(&input)->default_value("wagasci_mapping_table.txt"), "Input WAGASCI mapping table file path")
      ;

    /* The following three lines of code, create the object "vm" that will contain
       the command line arguments.
       These are read through the method "parse_command_line" */
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    // The --help option produces the description of all the allowed options
    if (vm.count("help")) {
      std::cout << desc << "\n";
      return 0;
    }

    /****************** Summary of all the command-line parameters ******************/
    std::cout << endl;
    if (vm.count("dif_id")) {
      std::cout << "  The DIF ID was set to " << vm["dif_id"].as<Int_t>()<<std::endl;
      dif_id = vm["dif_id"].as<Int_t>();
    } else {
      std::cout << "  The DIF ID is assumed to be " << dif_id << std::endl;
    }
    std::cout << endl;
    if (vm.count("mode")) {
      std::cout << "  The mode was set to " << vm["mode"].as<Int_t>() << std::endl;
      mode = vm["mode"].as<Int_t>();
    } else {
      std::cout << "  The mode is assumed to be " << mode << std::endl;
    }
    std::cout << endl;
    if (vm.count("input")) {
      std::cout << "  The input file name was set to " << vm["input"].as<string>() << std::endl;
    } else {
      std::cout << "  The input file is assumed to be " << input << std::endl;
    }
  }

  // If any exception is found the program is terminated with an error.
  // TO-DO add graceful termination routine
  catch(exception& e) {
    std::cerr << "  Error: " << e.what() << "\n";
    return 1;
  }

  //initialize
  struct or_ws *ws=(struct or_ws *)malloc(sizeof(struct or_ws));
  TApplication wagasciApp("App",&argc,argv);
  Int_t ini_time = (Int_t) time(NULL);
  Int_t bin_time = 1000; //ms
  //Int_t fin_time = ini_time + bin_time*20; //every 20sec
  Int_t fin_time = ini_time + bin_time; //every 1sec

  ws->dif_id = dif_id;
  ws->mapping=load_mapping(dif_id,input.c_str());
  if(!ws->mapping){
    printf("Cannot align wagasci mapping\n");
    return 1;
  }
  ws->nblock = 0;
  ws->nevt   = 0;
  //for(int ibunch=0;ibunch<8;ibunch++){
  //  Int_t   bcid = ExpectedBCIDstart + ibunch;
  //  Float_t offset_time = bcid%2==0 ? OffsetTimeEven : OffsetTimeOdd;
  //  Float_t ramp_time   = bcid%2==0 ? RampTimeEven   : RampTimeOdd;
  //  ws->timing_window[ibunch] 
  //    = bcid*NormBCID + (ExpectedHitTiming-offset_time)*ramp_time;
  //}

#ifdef __HttpServer__
  ws->serv=new THttpServer("http:8080");
#endif

  // Define the canvas margins and title size
  gStyle->SetOptStat(0);
  gStyle->SetPalette(1);
  Float_t margin_t = 0.00;
  Float_t margin_b = 0.10;
  Float_t margin_l = 0.05;
  Float_t margin_r = 0.02;
  Float_t title_size = 0.05;
  Float_t label_size = 0.04;
  gStyle->SetPadTopMargin   (margin_t);
  gStyle->SetPadBottomMargin(margin_b);
  gStyle->SetPadLeftMargin  (margin_l);
  gStyle->SetPadRightMargin (margin_r);


  /***************** SCREEN RESOLUTION DETECTION *******************/
  /* A list of display related environment variables*/
  int res_X, res_Y; // Screen resolution
  const char *env_var[2] = {"DISPLAY","WAYLAND_DISPLAY"};
  char *env_val[2];
  for(int i = 0; i < 2; i++)
    {
      env_val[i] = getenv(env_var[i]);
      /* Getting environment value if exists */
      if (env_val[i] != NULL)
	cout << endl << "  Variable = " << env_var[i] << ", Value = " << env_val[i]
	     << endl;
      else
	cout << endl << "  " << env_var[i] << " doesn't exist" << endl;
    }

  // The following routine tries to detect the screen resolution
  // If the system is running XServer
  cout << endl;
  if ( (env_val[0] != NULL) && (env_val[1] == NULL) ) {
  Display* display = XOpenDisplay(NULL);
  if (ScreenCount(display) == 0) {
    cout << "  No screen detected." << endl <<
      "  This is a GUI program and needs a screen to interact with the user.\n";
    return 1;
  } else if (ScreenCount(display) > 1) {
    cout << " Multiple screens detected. Using the default one.\n";
  }
  Screen*  screen = DefaultScreenOfDisplay(display);
  res_Y = screen->height;
  res_X = screen->width;
    // If the system is running Wayland
  } else if ( (env_val[0] == NULL) && (env_val[1] != 0) ) {
    cout << "  I am not able to detect the screen resolution if Wayland is used."
	 << endl << "  A screen resolution of 1920x1080 is assumed."
	 << endl << "  Anyway, if I were you, I would switch back to Server X."
	 << endl;
    res_X = 1920;
    res_Y = 1080;
  } else {
    cout << "  There is something wrong with screen detection. "
      "Please investigate!\n";
    return 1;
  }

  cout << "  Detected Resolution: " << res_X << "x" << res_Y << endl;

  //initialise canvas and set pad positions
  ws->canvas=new TCanvas("wagasci","Wagasci Dynamic Histogram",res_X*0.6,res_Y*0.4);
  Float_t pad_div[NumHist][4] = {
    {0.01,0.51,0.19,0.99}, // 0.Noise rate
    {0.21,0.51,0.39,0.99}, // 1.Gain
    {0.01,0.26,0.39,0.49}, // 2.Noise rate stability
    {0.01,0.01,0.39,0.24}, // 3.Gain stability
    {0.41,0.51,0.59,0.99}, // 4.BCID for nu beam
    {0.61,0.51,0.79,0.99}, // 5.Hit timing for nu beam
    {0.41,0.01,0.59,0.49}, // 6.Spill number for nu beam
    {0.61,0.01,0.79,0.49}, // 7.Charge for hit channel
    {0.81,0.01,0.99,0.80}, // 8.Event display
    {0.81,0.81,0.99,0.90}  // 9.Status display
  };
  for(int ipad=0;ipad<NumHist;ipad++){
    string name = Form("pad%d",ipad);
    ws->pad[ipad]=new TPad(name.c_str(),name.c_str(),
        pad_div[ipad][0],
        pad_div[ipad][1],
        pad_div[ipad][2],
        pad_div[ipad][3]);
    ws->canvas->cd(0);
    ws->pad[ipad]->Draw();
  }

  // define plots
  Double_t max_gain  =  50;
  Double_t min_gain  =   0;
  Double_t max_nrate = 100;
  Double_t min_nrate =   0;
  Int_t bin_gain  = (Int_t) max_gain  - min_gain;
  Int_t bin_nrate = (Int_t) max_nrate - min_nrate;
  
  ws->pad[0]->cd();
  ws->pad[0]->SetTopMargin(0.02);
  ws->pad[0]->SetLeftMargin(0.07);
  ws->h_nrate = new TH1F("NoiseRate","",NumChip,.0,(Double_t)NumChip);
  ws->h_nrate ->SetMarkerStyle(23);
  ws->h_nrate ->SetMaximum(max_nrate);
  ws->h_nrate ->SetMinimum(min_nrate);
  for(int i=0;i<NumChip;i++){
    ws->h_nrate->SetBinContent(i+1,1);
  }
  ws->h_nrate->GetXaxis()->SetTitle("chip id");
  ws->h_nrate->GetXaxis()->CenterTitle();
  ws->h_nrate->GetXaxis()->SetTitleOffset(0.8);
  ws->h_nrate->GetXaxis()->SetTitleSize(title_size);
  ws->h_nrate->GetXaxis()->SetNdivisions(4);
  ws->h_nrate->GetXaxis()->SetLabelSize(label_size);
  ws->h_nrate->GetYaxis()->SetLabelSize(label_size);
  ws->h_nrate ->Draw("HIST p");
  TLegend *leg0 = new TLegend(margin_l,0.9,1.-margin_r,0.98);
  leg0->SetFillStyle(0);
  leg0->SetBorderSize(0);
  leg0->SetHeader("Noise rate per chip [kHz]","c");
  leg0->Draw();

  ws->pad[1]->cd();
  ws->pad[1]->SetTopMargin(0.02);
  ws->pad[1]->SetGridx(1);
  ws->h_gain = new TH1F("Gain","",NumChAll,0.0,(Double_t)NumChAll);
  //ws->h_gain ->SetMarkerStyle(29);
  //ws->h_gain ->SetMarkerSize(1);
  ws->h_gain ->SetLineColor(kBlue);
  ws->h_gain ->SetMaximum(max_gain);
  ws->h_gain ->SetMinimum(min_gain);
  ws->h_gain ->GetXaxis()->SetTitle("chip_id");
  ws->h_gain ->GetXaxis()->CenterTitle();
  ws->h_gain ->GetXaxis()->SetTitleOffset(0.8);
  ws->h_gain ->GetXaxis()->SetTitleSize(title_size);
  for(int ichip=0;ichip<NumChip;ichip++){
    for(int ichipch=0;ichipch<NumChipCh;ichipch++){
      Int_t ibin = ichip*NumChipCh + ichipch + 1;
      if(ichipch==16&&ichip%5==0)
        ws->h_gain ->GetXaxis()->SetBinLabel(ibin,Form("%d",ichip));
      else
        ws->h_gain ->GetXaxis()->SetBinLabel(ibin,"");
    }
  }
  ws->h_gain ->GetXaxis()->SetLabelSize(label_size*1.5);
  ws->h_gain ->GetYaxis()->SetLabelSize(label_size);
  ws->h_gain ->GetXaxis()->SetTickLength(0);
  ws->h_gain ->GetXaxis()->SetNdivisions(NumChip);
  //ws->h_gain ->Draw("HIST p");
  ws->h_gain ->Draw("");
  TLegend *leg1 = new TLegend(margin_l+0.02,0.89,1.-margin_r,0.97);
  leg1->SetBorderSize(0);
  leg1->SetHeader("Gain for each chip/ch ","c");
  leg1->Draw();


  ws->pad[2]->cd();
  ws->pad[2]->SetGridy(1);
  ws->pad[2]->SetTopMargin(0.03);
  ws->h_nrate_st = new TH2F("NoiseStability","",
      bin_time,ini_time,fin_time,bin_nrate,min_nrate,max_nrate);
  ws->h_nrate_st ->GetXaxis()->SetTimeDisplay(1);
  //ws->h_nrate_st ->GetXaxis()->SetTimeFormat("#splitline{%M:%M}{%d/%b}");
  ws->h_nrate_st ->GetXaxis()->SetTimeFormat("%d/%b");
  ws->h_nrate_st ->GetXaxis()->SetNdivisions(510);
  ws->h_nrate_st ->GetYaxis()->SetNdivisions(205);
  ws->h_nrate_st ->GetXaxis()->SetLabelOffset(0.01);
  ws->h_nrate_st ->GetXaxis()->SetLabelSize(label_size*2);
  ws->h_nrate_st ->GetYaxis()->SetLabelSize(label_size*2);
  ws->h_nrate_st ->Draw("colz");
  TLegend *leg2 = new TLegend(margin_l,0.79,1.-margin_r,0.97);
  leg2->SetFillStyle(0);
  leg2->SetBorderSize(0);
  leg2->SetHeader("Noise rate stability","c");
  leg2->Draw();

  ws->pad[3]->cd();
  ws->pad[3]->SetGridy(1);
  ws->pad[3]->SetTopMargin(0.03);
  ws->h_gain_st = new TH2F("GainStability","",
      bin_time,ini_time,fin_time,bin_gain,min_gain,max_gain);
  ws->h_gain_st ->GetXaxis()->SetTimeDisplay(1);
  ws->h_gain_st ->GetXaxis()->SetTimeFormat("%d/%b");
  ws->h_gain_st ->GetXaxis()->SetNdivisions(510);
  ws->h_gain_st ->GetYaxis()->SetNdivisions(205);
  ws->h_gain_st ->GetXaxis()->SetLabelOffset(0.01);
  ws->h_gain_st ->GetXaxis()->SetLabelSize(label_size*2);
  ws->h_gain_st ->GetYaxis()->SetLabelSize(label_size*2);
  ws->h_gain_st ->Draw("colz");
  TLegend *leg3 = new TLegend(margin_l,0.79,1.-margin_r,0.97);
  leg3->SetFillStyle(0);
  leg3->SetBorderSize(0);
  leg3->SetHeader("Gain stability","c");
  leg3->Draw();


  ws->pad[4]->cd();
  ws->pad[4]->SetTopMargin(0.1);
  //ws->pad[4]->SetLogy(1);
  ws->h_bcid = new TH1F("Bcid","",300,0.0,100.0);
  ws->h_bcid ->SetFillColor(kBlue);
  ws->h_bcid ->SetLineColor(kBlue);
  ws->h_bcid ->Draw();
  TLine *line_bcid0
    = new TLine(ExpectedBCIDstart-0.33,0.,ExpectedBCIDstart-0.33,1e+8);
  TLine *line_bcid1
    = new TLine(ExpectedBCIDstart+8-0.33,0.,ExpectedBCIDstart+8-0.33,1e+8);
  line_bcid0->SetLineColor(kRed);
  line_bcid1->SetLineColor(kRed);
  line_bcid0->SetLineStyle(2);
  line_bcid1->SetLineStyle(2);
  line_bcid0->Draw();
  line_bcid1->Draw();
  TLegend *leg4 = new TLegend(margin_l+0.01,0.91,1.-margin_r,1.-margin_t);
  //leg4->SetFillStyle(0);
  leg4->SetBorderSize(0);
  leg4->SetHeader("Coarse trigger timing (BCID)","c");
  leg4->Draw();

  ws->pad[5]->cd();
  ws->pad[5]->SetTopMargin(0.1);
  ws->pad[5]->SetLogy(1);
  ws->h_timing = new TH1F("Hit Timing","",15000,0.0,15000.);
  ws->h_timing ->SetFillColor(kBlue);
  ws->h_timing ->SetLineColor(kBlue);
  ws->h_timing ->Draw();
  TLegend *leg5 = new TLegend(margin_l+0.01,0.91,1.-margin_r,1.-margin_t);
  //leg5->SetFillStyle(0);
  leg5->SetBorderSize(0);
  leg5->SetHeader("Hit timing","c");
  leg5->Draw();

  ws->pad[6]->cd();
  ws->h_spillnb = new TH1F("SpillNb","",SpillNbUpdate,0.0,(Double_t)SpillNbUpdate);
  ws->h_spillnb ->SetFillColor(kBlue);
  ws->h_spillnb ->Draw();
  TLegend *leg6 = new TLegend(margin_l+0.01,0.89,1.-margin_r,0.97);
  //leg6->SetFillStyle(0);
  leg6->SetBorderSize(0);
  leg6->SetHeader("Spill number recorded","c");
  leg6->Draw();

  ws->pad[7]->cd();
  ws->h_charge = new TH1F("HitCharge","",500,0.0,2000.);
  ws->h_charge ->Draw();
  TLegend *leg7 = new TLegend(margin_l+0.01,0.89,1.-margin_r,0.97);
  //leg7->SetFillStyle(0);
  leg7->SetBorderSize(0);
  leg7->SetHeader("ADC value for hits","c");
  leg7->Draw();

  ws->pad[8]->cd();
  ws->pad[8]->SetTopMargin   (0.01);
  ws->pad[8]->SetBottomMargin(0.01);
  ws->pad[8]->SetLeftMargin  (0.01);
  ws->pad[8]->SetRightMargin (0.01);
  ws->g_evtdisp2D = new TGraph(NumChAll);
  ws->g_evtdisp2D ->GetXaxis()->SetLabelSize(0);
  ws->g_evtdisp2D ->GetYaxis()->SetLabelSize(0);
  ws->g_evtdisp2D ->GetXaxis()->SetTickLength(0);
  ws->g_evtdisp2D ->GetYaxis()->SetTickLength(0);
  if(!DrawModule(dif_id,ws->mapping)){
    printf("cannot draw modules\n");
    return 1;
  }
  for(int chip=0;chip<NumChip;chip++){
    for(int chip_ch=0;chip_ch<NumChipCh;chip_ch++){
      int ip = chip*NumChipCh + chip_ch;
      ws->g_evtdisp2D->SetPoint(ip,-WATERTANK_X,-WATERTANK_Y);
    }
  }
  ws->g_evtdisp2D ->SetMarkerStyle(20); //20:circle, 29:star
  ws->g_evtdisp2D ->SetMarkerSize(1.3);
  ws->g_evtdisp2D ->SetMarkerColor(kRed);
  ws->g_evtdisp2D ->Draw("P same");
  TLegend *leg8 = new TLegend(margin_l+0.01,0.9,1.-margin_r,1.);
  leg8->SetFillStyle(0);
  leg8->SetBorderSize(0);
  leg8->SetHeader("Event display","c");
  leg8->Draw();

  ws->pad[9]->cd();
  ws->pad[9]->SetTopMargin   (0.01);
  ws->pad[9]->SetBottomMargin(0.02);
  ws->pad[9]->SetLeftMargin  (0.01);
  ws->pad[9]->SetRightMargin (0.01);
  ws->pad[9]->SetGridx(1);
  ws->pad[9]->SetGridy(1);
  ws->h_status = new TH2F("OM Status","",4,0.,4.,2,0.,2.);
  ws->h_status->GetXaxis()->SetLabelSize(0);
  ws->h_status->GetYaxis()->SetLabelSize(0);
  ws->h_status->GetXaxis()->SetTickLength(0);
  ws->h_status->GetYaxis()->SetTickLength(0);
  ws->h_status->GetXaxis()->SetNdivisions(4);
  ws->h_status->GetYaxis()->SetNdivisions(2);
  ws->h_status->GetZaxis()->SetRange(-1.,2.);
  ws->h_status ->Draw("colz");
  TText *txt9_1 = new TText(0.10,0.5,"Noise R");
  TText *txt9_2 = new TText(0.10,1.5,"Gain");
  TText *txt9_3 = new TText(1.10,0.5,"BCID");
  TText *txt9_4 = new TText(1.10,1.5,"Hit time");
  TText *txt9_5 = new TText(2.10,0.5,"spill#");
  TText *txt9_6 = new TText(2.10,1.5,"ADC");
  TText *txt9_7 = new TText(3.10,1.5,"Hit ch.");
  txt9_1->SetTextSize(0.3);
  txt9_2->SetTextSize(0.3);
  txt9_3->SetTextSize(0.3);
  txt9_4->SetTextSize(0.3);
  txt9_5->SetTextSize(0.3);
  txt9_6->SetTextSize(0.3);
  txt9_7->SetTextSize(0.3);
  txt9_1->Draw();
  txt9_2->Draw();
  txt9_3->Draw();
  txt9_4->Draw();
  txt9_5->Draw();
  txt9_6->Draw();
  txt9_7->Draw();

  fill_n(begin(ws->nb_out_nr), NumChip, 0);
  fill_n(begin(ws->nb_out_gain), NumChAll, 0);
  // memset(ws->nb_out_nr  ,0,NumChip);
  // memset(ws->nb_out_gain,0,NumChAll);
  ws->nb_bcid       = 0;
  ws->nb_out_bcid   = 0;
  ws->nb_out_timing = 0;
  ws->mean_charge   = 0;
  ws->nb_spillgap   = 0;
  ws->nb_nonhit_ch  = NumChAll;

  ws->canvas->cd(0);
  ws->txt_time  = new TText();
  ws->txt_spill = new TText();
  ws->txt_time  ->SetTextSize(0.03);
  ws->txt_spill ->SetTextSize(0.03);
  ws->txt_time  ->Draw();
  ws->txt_spill ->Draw();


#ifdef __HttpServer__
  ws->serv->Register("energy",ws->histo);
#endif

  ws->canvas->cd(0);

  /******************* RUN LOOP ********************/

  // You may want to change this for a different detector (for example BabyMIND)
  char* data_source_name = Form("converter_dif_1_1_%d",dif_id);

  cout << endl << "  ";
  if (( ws->loop=new_event_loop(data_source_name,ws,newevt,newblock,endblock,reinit,endrun,'f')) == NULL) return 1;

  ws->first_time=true;
  run_loop(ws->loop);

  //run the TApp
  wagasciApp.Run();

  //never reached
  return 0;
} //main


// ==================================================================
