// system includes
#include <string>
#include <exception>

// system C includes
#include <cstdio>

// boost includes
#include <boost/program_options.hpp>

// ROOT includes
// #include <TROOT.h>
// #include <TH1F.h>
// #include <TH2F.h>
// #include <TCanvas.h>
// #include <TGraph.h>
// #include <TText.h>

// pyrame includes
#include "pyrame.h"

// user includes
#include "wgErrorCodes.hpp"
#include "wgConst.hpp"
#include "wgLogger.hpp"
#include "wgTopology.hpp"
#include "wgOnlineMonitor.hpp"

void newevt(void   *workspace, struct event *e);
void newblock(void *workspace, struct block *b);
void endblock(void *workspace, struct block *b);
void reinit(void   *workspace);
void endrun(void   *workspace);

void newblock(void *workspace,struct block *block)
{
  struct om_ws *ws = (struct om_ws *) workspace;

  int spillnb = std::stoi(get_block_field(block, "spill"));

  std::printf("\nNew block\n\tID: %d | Spill number: %d\n", block->id, spillnb);
  
} //newblock

// // ==================================================================

// void newevt(void *workspace,struct event *e) 
// {
//   struct om_ws *ws=(struct om_ws *)workspace;
  
//   Int_t spillnb  =(Int_t)atoi(get_event_field(e,"spill"   ));
//   Int_t spillflag=(Int_t)atoi(get_event_field(e,"spill_flag")); 
//   Int_t chip     =(Int_t)atoi(get_event_field(e,"roc"       )); 
//   Int_t chip_ch  =(Int_t)atoi(get_event_field(e,"rocchan"   )); 
//   Int_t bcid     =(Int_t)atoi(get_event_field(e,"bcid"      )); 
//   Int_t sca      =(Int_t)atoi(get_event_field(e,"sca"       ));
//   Int_t pln      =(Int_t)atoi(get_event_field(e,"plane"     )); 
//   Int_t ch       =(Int_t)atoi(get_event_field(e,"channel"   )); 
//   Int_t energy   =(Int_t)atoi(get_event_field(e,"en"        )); 
//   Int_t data_time=(Int_t)atoi(get_event_field(e,"time"      )); 
//   Int_t hit      =(Int_t)atoi(get_event_field(e,"hit"       )); 
//   Int_t gain     =(Int_t)atoi(get_event_field(e,"gain"      ));
//   Float_t x    = (Float_t)atoi(get_event_field(e,"x"));
//   Float_t y    = (Float_t)atoi(get_event_field(e,"y"));
//   Float_t z    = (Float_t)atoi(get_event_field(e,"z"));

//   std::printf("\tNew event %d\n\t\tspillnb:%d | spillflag:%d | chip:%d | chip_ch:%d | bcid:%d | sca:%d | pln:%d | ch:%d | energy:%d | data_time:%d | hit:%d | gain:%d | x:%.1f | y:%.1f | z:%.1f\n", ws->nevt,spillnb,spillflag,chip,chip_ch,bcid,sca,pln,ch,energy,data_time,hit,gain,x,y,z);

//   Float_t offset_time = bcid%2==0 ? OffsetTimeEven : OffsetTimeOdd;
//   Float_t ramp_time   = bcid%2==0 ? RampTimeEven   : RampTimeOdd;
//   Float_t hittiming = bcid*NormBCID + (data_time-offset_time)*ramp_time;

//   std::printf("\t\toffset_time:%.0f | ramp_time:%.0f | hittiming:%.0f\n", offset_time, ramp_time, hittiming);
  
//   if (ws->nblock - ws->lastblock_evtdisp != 1)
//     {
//       printf("\t\tHouston, we've had a problem here. \"ws->nblock\" = %d | \"ws->lastblock_evtdisp\" = %d\n", ws->nblock, ws->lastblock_evtdisp);
//     }

//   ws->nevt++;

// } //newevt

// // ==================================================================

// void endblock(void *workspace,struct block *b)
// {
//   struct om_ws *ws=(struct om_ws *)workspace;

//   Int_t spillnb = (Int_t)atoi(get_block_field(b,"spill"));
//   Int_t spillflag = (Int_t)atoi(get_block_field(b,"spill_flag"));
  
//   std::printf("End block\n\tID = %d | Spill number: %d | Spill flag: %d\n", b->id, spillnb, spillflag);

//   // ------- for spill gap ----------------
//   // Recall that spillnb is relative to the current block.
//   // When the block is created in the newblock function:
//   //    ws->ini_spill  = spillnb;
//   //    ws->last_spill = spillnb - 1;

//   // So when the block is closed the (spillnb - ws->last_spill)
//   // difference should be one. If it is more than one it means that
//   // one or more spills were skipped

//   // When the ws->last_spill=65535 the spill number is reset to
//   // zero.

//   if( (spillnb - ws->last_spill > 1) || ((ws->last_spill == 65535) && (spillnb != 0))) {
// 	ws->nb_spillgap++;
//   }
//   // At this point we can reset the last spill to the current spill number
//   ws->last_spill = spillnb;

//   Float_t status_spill;
//   // If we have less that SPILL_UGLY_THRESHOLD=10 skipped spills it is OK.
//   // If we have less that SPILL_BAD_THRESHOLD=100 skipped spills
//   // it is fishy but not impossible.
//   // If we have more than 100 or negative skipped spills there is
//   // something wrong.
//   if     (ws->nb_spillgap < SPILL_UGLY_THRESHOLD ) status_spill = STATUS_SPILL_GOOD;
//   else if(ws->nb_spillgap < SPILL_BAD_THRESHOLD  ) status_spill = STATUS_SPILL_UGLY;
//   else                                             status_spill = STATUS_SPILL_BAD;

//   std::printf("\tSpill gap:%d | Status spill:%.0f\n", ws->nb_spillgap, status_spill);
//   std::printf("\tNumber of events: %d \n",ws->nevt);

// } // endblock

// // ==================================================================

// void reinit(void *workspace) 
// {
// } //reinit

// // ==================================================================

// void endrun(void *workspace) 
// {
// } //reinit


void initialize_work_space(struct om_ws &ws) {
    ws.dif_id = -1;
}

int wgOnlineMonitor(const char * x_pyrame_config_file, unsigned dif_id) {

  std::string pyrame_config_file(x_pyrame_config_file);

  // =========== Topology =========== //

  Topology * topol;
  try { topol = new Topology(pyrame_config_file); }
  catch (const std::exception& e) {
    Log.eWrite("[wgAnaHist] " + std::string(e.what()));
    return ERR_TOPOLOGY;
  }
  unsigned n_chips = topol->dif_map[dif_id].size();

  if ( n_chips == 0 || n_chips > NCHIPS ) {
    Log.eWrite("[wgAnaHist] wrong number of chips : " + std::to_string(n_chips) );
    return ERR_WRONG_CHIP_VALUE;
  }
  
  // =========== Event loop =========== //
  
  struct om_ws *ws = new struct om_ws;
  initialize_work_space(*ws);
  ws->dif_id = dif_id;

  char data_source_name[64];
  std::snprintf(data_source_name, 64, "converter_dif_0_0_%d", dif_id);

  if ((ws->loop = new_event_loop(data_source_name,
                                 ws,
                                 newevt,
                                 newblock,
                                 endblock,
                                 reinit,
                                 endrun,
                                 'f')) == NULL)
    return ERR_EVENT_LOOP;

  run_loop(ws->loop);

  return WG_SUCCESS;
}
