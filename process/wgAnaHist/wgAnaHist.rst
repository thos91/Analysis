=================
wgAnaHist program
=================

The wgAnaHist program is used to analyze the histograms created by the
wgMakeHist program. The result of the analysis is stored in the outputdir as
.xml files. Depending on the fit mode a different kind of analysis is performed.

Arguments
=========

- [-h] : prints an help message
- [-f] : input raw file to read
- [-i] : configuration file to read (pyrame xml file used during acquisition)
- [-o] : output directory for the xml files (if not given, the
  environment variable WAGASCI_XMLDATADIR is used instead)
- [-d] : dif number (integer starting from 1)
- [-x] : number of ASUs per DIF (default is 20)
- [-y] : number of channels per ASU (default is 36)
- [-m] : fit mode (mandatory)
- [-p] : print mode (default is true) 
- [-r] : overwrite mode (default is false)

Fit modes
=========

The -m parameter can take one of the following integer values:
1  : only dark noise
2  : only pedestal
3  : only charge_hit (low range)
4  : only charge_HG  (low range)
5  : only charge_HG  (high range)
10 : dark noise + charge_hit (low range)
11 : dark noise + pedestal + charge_hit (low range)
12 : dark noise + pedestal + charge_HG  (low range)
13 : dark noise + pedestal + charge_HG  (high range)
20 : dark noise + pedestal + charge_HG  (low range) + charge_HG (high range)

- dark noise mode : calculate the dark noise rate and its standard deviation for
  each chip and each channel.
- pedestal : calculate the position of the pedestal and its sigma using the
  charge_nohit histogram for each chip and each channel and column.
- charge_hit (low range) : calculate the position of the charge peak and its
  sigma for a low p.e. charge (ADC) distribution for each chip and each channel
  (the columns are all integrated together). The charge_hit histogram is used.
- charge_HG (low range) : same as above but consider only the high gain preamplifier. The charge_hit_HG histogram is used.
- charge_HG (high range) : not implemented yet

Print mode
==========

If the print mode (-p 1) is selected, the plot of the histograms analyzed (along
with the fitted functions) are saved in the WAGASCI_IMGDATADIR directory.
