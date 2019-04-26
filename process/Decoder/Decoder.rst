===============
Decoder program
===============

The Decoder program takes the raw data from the WAGASCI detector and extracts
(decodes) all the information that it can, organizing it into a ROOT tree.  The
ROOT tree is then written in a ROOT file.

Arguments
=========

- [-h] : prints an help message
- [-f] : selects the input raw file to read
- [-i] : selects the calibration file to read
- [-o] : selects the output directory for the ROOT file (if not given, the
  environment variable WAGASCI_DECODEDIR is used instead)
- [-r] : overwrite mode (overwrite the ROOT file if present)

Histograms
==========

