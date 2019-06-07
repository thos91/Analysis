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

Charge fit
----------

All the charge* histograms are fitted by a single 3-parameter gaussian: it is
assumed that only one peak is present corresponding to low p.e. events (mainly
dark noise). If a second smaller peak is present it is ignored (only the highest
peak is fitted). If two peaks whose highest bin has exactly the same height are
present, only the most left one is fitted.

The lower limit of the gaussian peak is 0.9 times the maximum bin height. The
upper limit is 1.1 times the maximum bin height. The mean value is limited to
the value of the maximum bin (its x) +/- three times the max_sigma variable.

.. figure:: ../nohit_example.png
			:width: 600px
					
			Sample picture of the gaussian fit of the `charge_nohit`
			histogram. This histogram is filled using the values of the charge
			ADC when there is no hit in the corresponding channel and
			column. All the SPIROC chips show a small drift with respect to the
			real pedestal for the channels without any hit. I was told by
			Stéphane that it is probably due to some coupling because all
			channels without hit switch from Track to Hold at the same. When an
			external trigger signal is used then the pedestal value is more
			accurate (barely shifted).

.. figure:: ../HG_example.png
			:width: 600px
					
			Sample picture of the gaussian fit of the `charge_lowHG` histogram.
			This histogram is filled using the values of the charge ADC when
			there is a hit in the high gain preamp of the corresponding channel
			and column. Depending on the threshold value the peak can correspond
			to 1 p.e. or 2 p.e. The 3 p.e. peak is rarely used because of the
			considerable time needed to acquire enough statistics. In this
			example picture the 2 p.e. peak is fitted but, as you can see, the
			statistics is barely enough. *Please update the picture*

Dark noise fit
--------------

The `NoiseRate` histogram is filled with the BCID values recorded only when a certain channel is hit, regardless of the column. If you think about it, integrating over this histogram from zero to a certain BCID, say T, will give use the number of hits over the whole acquisition period. More precicely, if the length of each BCID is 580 ns, the integral of this histogram is equal to the number of hits over a time equal to:

.. math::

   \textrm{total time (ns)} = \textrm{number of spills} * T * 580 \textrm{ns}

But one must also take into account that the number of columns is limited to 16
and the gate window cannot be arbitrarily wide (the chip saturate). In the
example picture below, we can see that, when the gate window is too wide, the
number of hits start to decrease at the right end of the histogram.

Once there is a hit in a channel, the SPIROC chip cannot record another hit in
the same channel for at least another BCID period (580ns). This means that we
have to deal with an unavoidable 580ns dead-time when measuring the dark noise
rate. **To learn how this issue is addressed in the code, please refer to the
WAGASCI PDF documentation (Chapter 4).**

.. figure:: ../NoiseRate_example.png	
			:width: 600px
	
			Sample picture of the `NoiseRate` histogram with fitted with a step
			function of unit height. The purpose of this fit is just to measure
			the "length" of the histogram. We could use the value of the last non
			zero bin as a measure of the histogram "non zero range" but, that
			way, a single corrupted hit could spoil the whole measurement and we
			want to avoid that. Better to make the code a little slower (more
			computational heavy) than to make it a little more unreliable.

Print mode
----------

If the print mode (-p) is selected, the plot of the histograms analyzed (along
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

The C API of the wgAnaHist library ("wgAnaHist" function in the "libwgAnaHist.cpp" source file) has one argument of type <unsigned long> called "flags_ulong". This argument is decoded by the "wgAnaHist" function into a set of flags. Each flag occupies a well definite place in the binary represetation of that number.

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
