#ifndef WGONLINEMONITOR_H
#define WGONLINEMONITOR_H

// system includes
#include <vector>

// ROOT includes
#include "TH1I.h"

const int FIT_WHEN_CHARGE_ENTRIES_IS = 1000;

struct om_ws 
{
  int dif_id;

  struct evloop *loop;

  int num_events;        // number of events
  int num_blocks;        // number of blocks
  int lastblock_evtdisp; // Last block that was shown in the
                         // real-time event display
  int last_spill;        // spill number of the previous block
  int num_spill_gaps;    // number of times a spill number gap has bee detected

  std::vector<std::vector<TH1I*>> h_charge;  // charge histogram array for each
                                             // dif and channel

  std::vector<std::vector<TH1I*>> h_bcid;    // BCID histogram array for each
                                             // dif and channel

  TH2D *h_gain_st;       // Time profile of the gain for all the channels
                         // (gain stability)
};

int wgOnlineMonitor(const char * x_pyrame_config_file, unsigned dif_id); 

#endif /* WGONLINEMONITOR_H */
