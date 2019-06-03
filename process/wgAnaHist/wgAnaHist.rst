=================
wgAnaHist program
=================

The wgAnaHist program is used to analyze the histograms created by the
wgMakeHist program. The result of the analysis is stored in the outputdir as
.xml files. Depending on the fit mode a different kind of analysis is performed.

Arguments
=========

- ``[-h]`` : prints an help message
- ``[-f]`` : input raw file to read
- ``[-i]`` : configuration file to read (pyrame xml file used during acquisition)
- ``[-o]`` : output directory for the xml files (if not given, the
  environment variable WAGASCI_XMLDATADIR is used instead)
- ``[-q]`` : output directory for the png image files (if not given, the
  environment variable WAGASCI_IMGDATADIR is used instead) 
- ``[-d]`` : dif number (integer starting from 1)
- ``[-x]`` : number of ASUs per DIF (default is 20)
- ``[-y]`` : number of channels per ASU (default is 36)
- ``[-m]`` : fit mode (mandatory)
- ``[-p]`` : print mode (default is false) 
- ``[-r]`` : overwrite mode (default is false)

Modes
=====

Fit modes
---------

The -m parameter can take one of the following integer values:

- ``1`` : only dark noise
- ``2`` : only pedestal
- ``3`` : only charge_hit (low range)
- ``4`` : only charge_HG  (low range)
- ``5`` : only charge_HG  (high range)
- ``10`` : dark noise + charge_hit (low range)
- ``11`` : dark noise + pedestal + charge_hit (low range)
- ``12`` : dark noise + pedestal + charge_HG  (low range)
- ``13`` : dark noise + pedestal + charge_HG  (high range)
- ``20`` : everything

Fit modes: simple explanation
-----------------------------

+------------------+-----------------------------------------------------------------------------------------+
| dark noise       | calculate the dark noise rate and its standard deviation for each chip and each channel.|
+------------------+-----------------------------------------------------------------------------------------+
| pedestal         | calculate the position of the pedestal and its sigma using the charge_nohit histogram   |
|                  | for each chip and each channel and column.                                              |
+------------------+-----------------------------------------------------------------------------------------+
| charge_hit       | calculate the position of the charge peak and its sigma for a low p.e. charge (ADC)     |
| (low range)      | distribution for each chip and each channel  (the columns are all integrated together). |
|                  | The charge_hit histogram is used.                                                       |
+------------------+-----------------------------------------------------------------------------------------+
| charge_HG        | same as above but consider only the high gain preamplifier.                             |
| (low range)      | The charge_hit_HG histogram is used.                                                    |
+------------------+-----------------------------------------------------------------------------------------+
| charge_HG        | not implemented yet                                                                     |
| (high range)     |                                                                                         |
+------------------+-----------------------------------------------------------------------------------------+

Print mode
----------

If the print mode (-p 1) is selected, the plot of the histograms analyzed (along
with the fitted functions) are saved in the WAGASCI_IMGDATADIR directory.

C API
=====
.. code-block:: cpp
				
				int AnaHist(const char * inputFileName,
				            const char * configFileName,
				            const char * outputDir,
				            const char * outputIMGDir,
				            unsigned long flags_ulong,
				            unsigned idif    = 1,
				            unsigned n_chips = NCHIPS,
				            unsigned n_chans = NCHANNELS);

- ``inputFileName``  : complete path to the ``_hist.root`` ROOT file
- ``configFileName`` : complete path to the Pyrame XML configuration file
- ``outputDir``      : output directory where all the XML files are written
- ``outputIMGDir``   : output directory for the PNG graphs
- ``flags_ulong``    : <unsigned long> containing all the flags (see next sections)
- ``idif``           : DIF to analyze (from 1 to NDIFS)
- ``n_chips``        : number of chips for each DIF
- ``n_channels``     : number of channels for each chip
  
Flags
=====

The C API of the wgAnaHist library ("wgAnaHist" function in the "libwgAnaHist.cc" source file) has one argument of type <unsigned long> called "flags_ulong". This argument is decoded by the "wgAnaHist" function into a set of flags. Each flag occupies a well definite place in the binary represetation of that number.

.. code-block:: cpp

				#define M 8

				#define SELECT_OVERWRITE       0
				#define SELECT_CONFIG          1
				#define SELECT_PRINT           2
				#define SELECT_DARK_NOISE      3
				#define SELECT_CHARGE_LOW      4
				#define SELECT_PEDESTAL        5
				#define SELECT_CHARGE_HG_LOW   6
				#define SELECT_CHARGE_HG_HIGH  7

				...
				
				bitset<M> flags(flags_ulong);
				
				if( flags[SELECT_OVERWRITE] )
				// something

If you need an introduction to the bitset class template take a look `here <https://en.cppreference.com/w/cpp/utility/bitsets>`_. It is used mainly to handle arrays of booleans and so it fits our needs perfectly. When the wgAnaHist function is called through the CLI the flags are automatically set according to the selected mode. When calling the C API the user is free to set the flags at will.

- ``flags[SELECT_OVERWRITE]``      : overwrite the XML files in the output folder if present
- ``flags[SELECT_CONFIG]``         : read the acquisition start time, stop time, global 10-bit discriminator threshold, global 10-bit gain selection discriminator threshold, adjustable input 8-bit DAC, adjustable 6-bit high gain (HG) preamp feedback capacitance, adjustable 4-bit discriminator threshold from the Pyrame XML configuration file.
- ``flags[SELECT_PRINT]``          : print graphs. If false no image is printed and only the XML files are filled.
- ``flags[SELECT_DARK_NOISE]``     : calculate dark noise for each chip and channel. The ``wgFit`` class ``NoiseRate`` method is used to calculate the dark noise. Prints the ``bcid_hit[chip][chan]`` histogram if the print flag is set.
- ``flags[SELECT_CHARGE_LOW]``     : calculate the ADC count of the first peak when there is a hit using the ``charge_hit[chip][chan]`` histogram. Print the histogram if the print flag is set.
- ``flags[SELECT_PEDESTAL]``       : calculate the ADC count of the first peak when there is no hit using the ``charge_nohit[chip][chan]`` histogram. Print the histogram if the print flag is set.
- ``flags[SELECT_CHARGE_HG_LOW]``  : calculate the ADC count of the first peak when there is no hit in the high gain preamp using the ``charge_nohit[chip][chan]`` histogram. Print the histogram if the print flag is set.
- ``flags[SELECT_CHARGE_HG_HIGH]`` : not implemented yet
