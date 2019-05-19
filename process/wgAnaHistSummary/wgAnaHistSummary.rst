========================
wgAnaHistSummary program
========================

The wgAnaHistSummay program is used to summarize the information contained in the xml
files created by the wgAnaHist program.

Arguments
=========

- [-h] : prints an help message
- [-f] : input directory with xml files to read (mandatory)
- [-o] : output directory for the xml summary files (default: same as input directory)
- [-i] : output directory for plots and images (default: WAGASCI_IMGDIR)
- [-x] : number of chips per DIF (default is 20)
- [-y] : number of channels per chip (default is 36)
- [-p] : print plots and images
- [-r] : overwrite mode (default is false)
- [-m] : mode (default:10)

Modes
=====

The -m parameter can take one of the following integer values:
- 1  : Noise Rate
- 2  : Gain
- 3  : Pedestal
- 4  : Raw Charge
- 10 : Noise Rate + Gain
- 11 : Noise Rate + Gain + Pedestal
- 12 : Noise Rate + Gain + Pedestal + Raw Charge

- dark noise mode : record the dark noise rate
- gain mode       : record the gain
- pedestal        : record the pedestal position and its sigma
- raw charge      : record the 1 p.e. peak or 2 p.e. peak and its sigma
