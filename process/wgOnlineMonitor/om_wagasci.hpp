#ifndef OM_WAGASCI_H_
#define OM_WAGASCI_H_

#include <cstdbool>
#include <TH1F.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TText.h>

#ifdef __HttpServer__
#include <THttpServer.h>
#endif

#define BEAM_MODE      0
#define PEDESTAL_MODE  1
#define NON_BEAM_SPILL 0
#define BEAM_SPILL     1
#define HAVE_HIT       1

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

// ==================================================================

// The om_ws struct contains blocks and events parameters and the
// histograms needed for event display.

struct om_ws 
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


#endif // OM_WAGASCI_H_
