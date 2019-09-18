#ifndef WGONLINEMONITOR_H
#define WGONLINEMONITOR_H

// ROOT includes
// #include <TROOT.h>
// #include <TH1F.h>
// #include <TH2F.h>
// #include <TCanvas.h>
// #include <TGraph.h>
// #include <TText.h>

#define NHIST   10    // Total number of the Histograms that are shown
#define NPLANES 8     // Number of planes

struct om_ws 
{
  int dif_id;
  struct evloop *loop;
};

int wgOnlineMonitor(const char * x_pyrame_config_file, unsigned dif_id); 

#endif /* WGONLINEMONITOR_H */
