#ifndef WGONLINEMONITOR_H
#define WGONLINEMONITOR_H

// system includes
#include <vector>

// ROOT includes
#include <TH1I.h>
#include <TH2D.h>

const int FIT_WHEN_CHARGE_ENTRIES_ARE = 300;
const int FIT_WHEN_BCID_ENTRIES_ARE = 300;

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

  // charge histogram array for each chip and channel
  std::vector<std::vector<TH1I*>> h_charge; 
                                            
  // BCID histogram array for each chip  and channel
  std::vector<std::vector<std::pair<TH1I*, int>>> h_bcid;   

  TH2D *h_gain_stability; // Time profile of the gain for all the channels
  TH2D *h_dark_noise_stability; // Time profile of the dark noise for all the channels
};

int wgOnlineMonitor(const char * x_pyrame_config_file, unsigned dif_id); 

#endif /* WGONLINEMONITOR_H */
