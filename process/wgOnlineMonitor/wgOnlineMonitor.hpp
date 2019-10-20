#ifndef WGONLINEMONITOR_H
#define WGONLINEMONITOR_H

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
};

int wgOnlineMonitor(const char * x_pyrame_config_file, unsigned dif_id); 

#endif /* WGONLINEMONITOR_H */
