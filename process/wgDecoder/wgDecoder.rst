===============
Decoder program
===============

The Decoder program takes the raw data from the WAGASCI detector and extracts
(decodes) all the information that it can, organizing it into a ROOT tree. The
ROOT tree is then written in a ROOT file.

Arguments
=========

- ``[-h]`` : prints an help message
- ``[-f]`` : selects the input raw file to read
- ``[-i]`` : selects the calibration file to read
- ``[-o]`` : selects the output directory for the ROOT file (if not given, the
  environment variable WAGASCI_DECODEDIR is used instead)
- ``[-r]`` : overwrite mode (overwrite the ROOT file if present)

Histograms
==========

- ``spill`` : spill number (for each event)
- ``spill_mode`` : 0 for non-beam spill, 1 for beam spill (for each event)
- ``spill_count`` : number of spill acquired during this acquisition (until that
  particular event)
- ``spill_flag`` : for each event counts the number of chips that were correctly read

- ``bcid`` : the bunch crossing identification time (16 bit integer)
- ``hit`` : if the hit bit in the raw data (for each chip and each channel) is set to
  one we have a hit, if it set to zero we don't have a hit
- ``gain`` : if the gain bit in the raw data (for each chip and each channel) is
  set to one the high gain preamplifier is used, if it set to zero the low gain
  preamplifier is used.
- ``debug`` : for each DEBUG macro defined in Decoder.cc that bin is increased by one
  every time that the relative error occurres
- ``chipid`` : chip ID tag as it is recorded in the chip trailer.
- ``chipid_tag`` : chip ID tag as it is recorded in the chip header (if not found the software tries to guess it)
