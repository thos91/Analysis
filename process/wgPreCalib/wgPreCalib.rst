==========
wgPreCalib
==========

The wgPreCalib program


It assumes that the summary files created by the AnaHistSummary program are in
the same folder as the xml files created by the AnaHist program. Moreover it
assumes that the root folder contains in its name the inputDAC and
p.e. values. Long story short, this is the structure that the wgPreCalib program
assumes for the folder tree:

::

   inputDir
   |
   |-----> test_inputDAC121_pe2_dif_1_1_1
   |         |
   |         |-----> Summary_chip0.xml
   |         |-----> Summary_chip1.xml
   |         |-----> ...
   |
   |-----> test_inputDAC121_pe2_dif_1_1_2 ...
   |
   |-----> ...
