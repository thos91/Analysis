========================
wgAnaHistSummary program
========================

The wgAnaHistSummay program is used to summarize the information contained in the xml
files created by the wgAnaHist program.

Arguments
=========

- ``[-h]`` : prints an help message
- ``[-f]`` : input directory with xml files to read (mandatory)
- ``[-o]`` : output directory for the xml summary files (default: same as input directory)
- ``[-i]`` : output directory for plots and images (default: WAGASCI_IMGDIR)
- ``[-p]`` : print plots and images (default is false)
- ``[-r]`` : overwrite mode (default is false)
- ``[-m]`` : mode (default:10)

Modes
=====

The -m parameter can take one of the following integer values:

- ``1``  : Noise Rate
- ``2``  : Gain
- ``3``  : Pedestal
- ``4``  : Raw Charge
- ``10`` : Noise Rate + Gain
- ``11`` : Noise Rate + Gain + Pedestal
- ``12`` : Noise Rate + Gain + Pedestal + Raw Charge

Where:
  
- ``dark noise mode`` : record the dark noise rate
- ``gain mode``       : record the gain
- ``pedestal``        : record the pedestal position and its sigma
- ``raw charge``      : record the 1 p.e. peak or 2 p.e. peak and its sigma

  
Functional behavior
===================

It is assumed that the thresholds have been optimized for a certain
p.e. level. This means that the wgAnaPedestal must be run after TO-DO

The ``chip%d/ch%d.xml`` files that were produced by the wgAnaHist program are
read. You need to run that program at in a mode that has at least "Dark noise",
"pedestal" and "charge_HG". Mode 12 is recommended but mode 20 is fine too.

The following info are extracted by those files:

- ``trigth``       : Trigger threshold (chip)
- ``gainth``       : Gain select threshold (chip)
- ``inputDAC``     : Input DAC (chip, channel)
- ``HG``           :  High gain preamp feedback capacitance (chip, channel)
- ``trig_adj``     : Trigger threshold adjustment (chip, channel)
- ``NoiseRate``    : Dark noise rate (chip, channel)
- ``NoiseRate_e``  : Dark noise rate error (chip, channel) 
- ``charge_nohit`` : ADC count when not hit (chip, channel, column)
- ``sigma_nohit``  : error on the ADC count when not hit (chip, channel, column) 
- ``charge_lowHG`` : ADC count when hit and high gain preamp is selected (chip,
  channel, column)
- ``sigma_lowHG``  : error on the ADC count when hit and high gain
  preamp is selected (chip, channel, column)

``charge_lowHG`` may refer to the 1 p.e., 2 p.e., etc depending on the value of
the threshold. ``NoiseRate`` is used to guess the value of the threshold in
p.e. equivalent (1 p.e. threshold, 2 p.e. threshold, etc), because that the
noise rate is strongly dependent on the threshold.

Then in the output directory a ``Summary_chip%d.xml`` template file is created
for every chip and then filled with the following info:

- ``trigth``     : Trigger threshold
- ``gainth``     : Gain select threshold (chip)
- ``inputDAC``   : Input DAC (chip, channel)
- ``ampDAC``     : High gain preamp feedback capacitance (chip, channel)
- ``adjDAC``     : trig_adj : trigger threshold adjustment (chip, channel)
- ``pe_level``   : p.e. equivalent for the trigger threshold (chip, channel)
- ``noise``      : Dark noise rate (chip, channel)
- ``enoise``     : Dark noise rate error (chip, channel)
- ``ped_%d``     : charge_nohit = ADC count when not hit (chip, channel, column)
- ``eped_%d``    : charge_nohit = error on the ADC count when not hit (chip, channel, column)
- ``raw_%d``     : charge_HG = ADC count when hit and high gain preamp is
  selected (chip, channel, column)
- ``eraw_%d``    : charge_HG = error on the ADC count when hit and high
  gain preamp is selected (chip, channel, column)
- ``gain_%d``    : Difference between the charge_HG and the charge_nohit values
  (chip, channel, column). This value is called gain but it is actually a
  multiple of the gain. Moreover it doesn't take into account the shift of the
  pedestal.
- ``egain_%d``  : error on the gain (chip, channel, column).

Finally in the outputIMGDir one histogram for each chip is printed. Each point
in the histogram refers to the column of a particular channel of the chip. On
the X axis the column ID, on the Y axis the pedestal value in ADC counts. The X
axis value is calculated as follows:

.. math::
    
   \textrm{ID} = \textrm{channel} * 26 + \textrm{column}

The reason for the 26 instead of 16 (number of columns) is that by adding an
empty padding of 10 to the end of each channel, it is easier to visually
distinguish where a channel ends and another begins

Ouput graphs
============

Along with the .xml files, a graphical representation of their content is
created using ROOT. Four plots for each chip are created.

.. figure:: ../images/Summary_Pedestal_example.png
            :width: 600px

            The `charge_nohit` peak value for each channel and column is
            plotted. The channels are separated by dotted lines.

.. figure:: ../images/Summary_Npe_example.png
            :width: 600px

            The `charge_lowHG` peak value for each channel and column is
            plotted. The channels are separated by dotted lines. The peak may
            refer to 1 p.e., 2 p.e. or more rarely to 3 p.e. depending on the
            threshold set during acquisition.d

.. figure:: ../images/Summary_Gain_example.png
            :width: 600px

            The difference between the `charge_lowHG` peak and the
            `charge_nohit` peak for each channel and column is plotted. The
            channels are separated by dotted lines. This difference is roughly
            an integer multiple of the gain. I say roughly because, for the
            SPIROC chip family, the `charge_nohit` peak value is slightly
            shifted with respect to the actual pedestal. *Please update the
            picture*

.. figure:: ../images/Summary_Noise_example.png
            :width: 600px

            The `DarkNoise` value for each channel is plotted. This histogram is
            not essential for the pedestal analysis. The dark noise rate is only
            used to guess if the `charge_lowHG` peak is the 1 p.e. peak or the 2
            p.e. peak, etc.

C API
=====
.. code-block:: cpp
                
   int wgAnaHistSummary(const char * inputDir,
                        const char * outputXMLDir,
                        const char * outputIMGDir,
                        int mode,
                        bool overwrite = false,
                        bool print = false,
                        unsigned n_chips = NCHIPS,
                        unsigned n_chans = NCHANNELS);


- ``inputDir``       : complete path to the directory containing the XML files
  generated by the wgAnaHist program (at least mode 12).
- ``outputXMLDir``   : output directory where all the summery XML files are written
- ``outputIMGDir``   : output directory for the PNG graphs
- ``mode``           : mode
- ``overwrite``      : if set to true all the output files can be overwritten
- ``print``          : if set to true all the histograms are printed as png images
- ``n_chips``        : number of chips for each DIF (default NCHIPS = 20)
- ``n_channels``     : number of channels for each chip (default NCHANNELS = 32)
