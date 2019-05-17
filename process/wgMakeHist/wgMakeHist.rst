==========
wgOptimize
==========

This program reads the _tree.root file created by the wgDecoder program and fill
many histograms with that data. These histograms are then written to a a
_hist.root file.

Histograms
==========

- TH1D start_time
  Start time of the acquisition
- TH1D stop_time
  Stop time of the acquisition
- TH1D nb_data_pkts
  Number of packages acquired
- TH1D nb_lost_pkts
  Number of packages lost
- TH1D h_spill
  Spill number
- TH1D h_bcid_hit      [n_chips][n_channels]
  BCID time (only when there is a hit)
- TH1D h_charge_hit    [n_chips][n_channels][MEMDEPTH]
  ADC charge (only when there is a hit)
- TH1D h_charge_hit_HG [n_chips][n_channels][MEMDEPTH]
  ADC charge (only when there is a hit and the high gain preamp is selected)
- TH1D h_charge_hit_LG [n_chips][n_channels][MEMDEPTH]
  ADC charge (only when there is a hit and the low gain preamp is selected)
- TH1D h_pe_hit        [n_chips][n_channels][MEMDEPTH]
  Number of photo electrons (meaningful only if the detector has been calibrated)
- TH1D h_charge_nohit  [n_chips][n_channels][MEMDEPTH]
  ADC charge (only when there is no hit)
- TH1D h_time_hit      [n_chips][n_channels][MEMDEPTH]
  TDC time (only when there is a hit)
- TH1D h_time_nohit    [n_chips][n_channels][MEMDEPTH]
  TDC time (only when there is no hit)

Arguments
=========

- [-h] : help
- [-f] : input ROOT file (mandatory)
- [-o] : output directory (default = WAGASCI_HISTDIR)
- [-r] : overwrite mode
- [-x] : number of ASU chips per DIF (must be 1-20)
- [-y] : number of channels per chip (must be 1-36)
