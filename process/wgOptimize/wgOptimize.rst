==========
wgOptimize
==========

This program is used to optimize some SPIROC2D parameters and write them into
the bitstream configuration files before data acquisition. The parameters that
are optimized are the input DAC and the trigger threshold.

There are two modes of operation:

Pre-calibration mode
====================

.. note::
   Called in the source code OP_THRESHOLD_MODE

Given a certain value of the inputDAC, the trigger threshold is usually set at
certain optimal levels corresponding to the plateaus in the S-curve graph. The
reason for this is that, if the threshold is set at the center point of these
plateaus, the natural drift of the detector parameters (like the gain) during
operation has the minimal effect on the dark noise and event rate. In other
words the centers of the plateaus are the points were a shift of the S-curve
relative to the threshold value has the least effect on the dark noise rate.

Each plateau correspond to a certain number of photo-electrons. For instance the
0.5 p.e. plateau is the one right before the 1 p.e. fall, and so on. If we set
the threshold to the center of the 0.5 p.e. plateau we will be able to detect
all the +1 p.e. events, if we set the threshold at 1.5 p.e. we will see the +2
p.e. events, etc...

Because it is computationally much simpler to deal with integers, in the
following the integer values 1,2,3,.. will refer to the 0.5, 1.5,
2.5,... p.e. thresholds respectively. Keep that in mind when reading the source
code.

In the threshold mode a threshold_card.xml file is assumed to be
present and available. This file was created when analyzing the
S-curves for every channel. For each point
(1,21,41,61,81,101,121,141,161,181,201,221,241) of the input DAC, the
0.5 p.e. threshold "threshold_1" and 1.5 p.e. threshold "threshold_2".
The program just reads the values corresponing the the given
p.e. level and edit the bitstream files accordingly.

Post-calibration mode
=====================

.. note::
   Called in the source code OP_INPUTDAC_MODE

In this mode the inputDAC vs optimal threshold and the inputDAC vs gain linear
fits are used to optimize the inputDAC and the threshold values (for a given
p.e. target). The inputDAC is set for each channel to the value such that the
gain is closest to 40 ADC counts. The threshold is set to the optimal p.e. value
(plateau of the S-curve).

Just as in the OP_THRESHOLD MODE (Pre-calibration mode), the threshold card file
is assumed to be already present. The intercept and the slope of the linear fit
of the inputDAC (x) vs optimal threshold for the given p.e. equivalend (y) are
read.  Moreover the intercept and the slope of the linear fit of the
inputDAC (x) vs Gain (y) are read from the gain card file.

Then use the gain vs inputDAC fit to select the optimal value for the inputDAC
(the one so that the gain is closest to 40 ADC counts.) If the result inputDAC
is less than 1, set it to 1. If it is greater than 250, set it to 250. If the
gain for some reason is zero, set the input DAC arbitrarily to 121. The input
DAC is fine tuned for each channel.

1 and 2 photo electrons
-----------------------
The threshold is set for the whole chip and it is calculated, once the optimal
input DAC is known, using the threshold vs input DAC fit. The reference value
for the input DAC used in the threshold calculation is the mean value over all
the chip channels.

Bitstream files
===============

This program assumes that the bitstream files for each chip are
already present and gathered together in a folder. The pattern for the
names of the bitstream files is fixed and changing it will almost
certainly result in a error, or worse a misbehavior. The files must be
in the same folder and named:

```
wagasci_bitstream_gdcc%d_dif%d_chip%d.txt
```

where each integer starts from 0.

Arguments
=========

- ``[-h]``         : print this help message
- ``[-m]`` (int)   : mode selection (default 0)
   -   ``0``       :   pre calibration (optimized threshold)
   -   ``1``       :   post calibration (optimized threshold + inputDAC)
- ``[-t]`` (char*) : threshold card to read (mandatory)
- ``[-f]`` (char*) : gain card to read (mandatory only in mode 1)
- ``[-u]`` (char*) : Pyrame configuration xml file\n"
- ``[-s]`` (char*) : spiroc2d bitstream files folder (default : "/")
- ``[-p]`` (int)   : photo electrons equivalent threshold (mandatory)
   -     ``1``     :   0.5 p.e. equivalent threshold
   -     ``2``     :   1.5 p.e. equivalent threshold
- ``[-i]`` (int)   : inputDAC (high voltage adjustment DAC) (only mode 0)
   -     ``1+20*n``:   where n is in (0,12)

C API
=====
.. code-block:: cpp

  int wgOptimize(const char * threshold_card,
                 const char * calibration_card,
                 const char * config_xml_file,
                 const char * wagasci_config_dif_dir,
                 unsigned mode      = 0,
                 unsigned pe        = 2,
                 unsigned inputDAC  = 121);

- ``threshold_card``         : threshold card to read
- ``calibration_card``       : gain card to read (only mode 1)
- ``config_xml_file``        : Pyrame configuration xml file
- ``wagasci_config_dif_dir`` : spiroc2d bitstream files folder
- ``mode``                   : mode selection (default 0)
- ``pe``                     : photo electrons equivalent threshold (default 2)
- ``inputDAC``               : inputDAC (default 121)
