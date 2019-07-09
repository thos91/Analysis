===============
Decoder program
===============

The Decoder program takes the raw data from the WAGASCI detector and extracts
(decodes) all the information that it can, organizing it into a ROOT tree. The
ROOT tree is then written in a ROOT file.

Arguments
=========

- ``[-h]`` : prints an help message
- ``[-f]`` : input raw file to read (mandatory)
- ``[-c]`` : directory containing the calibration card files (default = WAGASCI_CONFDIR)
- ``[-o]`` : output directory for the ROOT file (default = WAGASCI_DECODEDIR)
- ``[-n]`` : DIF number 1-8 (default 1-8)\n"
- ``[-x]`` : number of ASU chips per DIF 1-20. By default the decoder automatically detects the number of ASU chips.\n"
- ``[-r]`` : overwrite mode (default = false). Overwrite the ROOT tree file.
- ``[-q]`` : compatibility mode for old data (default = false). Use for raw data files acquired before the first half of 2018. Even if not set, the decoder tries to detect the old raw data format automatically.
- ``[-b]`` : silent mode (default = false). If set, nothing is printed on the stardard output. The output is still printed on the log file.

Histograms
==========

The following histograms are directly extracted from the raw data. All the other histograms are calculated from the calibration data.

- ``spill_number`` : spill number (for each event)
- ``spill_mode`` : 0 for non-beam spill, 1 for beam spill (for each event)
- ``spill_count`` : number of spill acquired during this acquisition (until that
  particular event)
- ``bcid[<n_chips>][<n_columns>]`` : the bunch crossing identification time (16 bit integer)
- ``charge[<n_chips>][<n_channels>][<n_columns>]`` : ADC counts
- ``time[<n_chips>][<n_channels>][<n_columns>]`` : TDC counts
- ``gain[<n_chips>][<n_channels>][<n_columns>]`` : if the gain bit in the raw data (for each chip and each channel) is
  set to one the high gain preamplifier is used, if it set to zero the low gain
- ``hit[<n_chips>][<n_channels>][<n_columns>]`` : if the hit bit in the raw data (for each chip and each channel) is set to
  one we have a hit, if it set to zero we don't have a hit

  preamplifier is used.

- ``debug_chip[<n_chips>]`` : for each chip DEBUG macro defined in wgDecoderReader.hpp that bin is increased by one
- ``debug_spill`` : for each spill DEBUG macro defined in wgDecoderReader.hpp that bin is increased by one
  every time that the relative error occurres
- ``chipid[<n_chips>]`` : chip ID tag as it is recorded in the chip trailer.
