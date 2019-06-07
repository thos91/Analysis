=====================
wgAnaPedestal program
=====================

The wgAnaPedestal program extracts the information contained in the xml files
created by the wgAnaHist program, necessary for the calculation of the pedestal
for each column. Its output is further analyzed by the wgAnaPedestalSummary
program that finally creates the ``pedestal_card.xml`` file.

Arguments
=========

- ``[-h]`` : prints an help message
- ``[-f]`` : input directory with xml files to read (mandatory)
- ``[-o]`` : output directory for the xml summary files (default: same as input
  directory)
- ``[-x]`` : number of chips per DIF (default is 20)
- ``[-y]`` : number of channels per chip (default is 36)
- ``[-c]`` : pre-calibration mode (default is false)
- ``[-r]`` : overwrite mode (default is false)

Operation mode
==============

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
- ``charge_nohit`` : ADC count when not hit (chip, channel, column)
- ``charge_lowHG`` : ADC count when hit and high gain preamp is selected (chip,
  channel, column)

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
- ``Noise``      : Dark noise rate (chip, channel)
- ``ped_%d``     : charge_nohit = ADC count when not hit (chip, channel, column)
- ``ped_ref_%d`` : charge_HG = ADC count when hit and high gain preamp is
  selected (chip, channel, column)
- ``gain_%d`` : Difference between the charge_HG and the charge_nohit values
  (chip, channel, column). This value is called gain but it is actually a
  multiple of the gain. Moreover it doesn't take into account the shift of the
  pedestal.

Finally in the outputIMGDir one histogram for each chip is printed. Each point
in the histogram refers to the column of a particular channel of the chip. On
the X axis the column ID, on the Y axis the pedestal value in ADC counts. The X
axis value is calculated as follows:

.. math::
    
   \textrm{ID} = \textrm{channel} * 26 + \textrm{column}

The reason for the 26 instead of 16 (number of columns) is that by adding an
empty padding of 10 to the end of each channel, it is easier to visually
distinguish where a channel ends and another begins
