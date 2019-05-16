======================
wgChangeconfig program
======================

This program is used to to check or modify the bitstream txt files for the
SPIROC2D chip. A bitstream file (ideally one for each chip) contains a single
line 1192 characters long string, made up only of '0' and '1'. Each character
represents a single bit that is sent as a bitstream during the SPIROC2D
configuration. All the internal parameters of the SPIROC2D that can be set by
the user are "packaged" in the bitstream string.  For a list of all the
parameters that can be set, see the spiroc2d.csv file.

Arguments
=========

- [-h] : prints an help message
- [-f] : selects the input bitstream txt file (mandatory)
- [-o] : selects the output bitstream txt file (default: same as the input file)
- [-r] : overwrite mode (overwrite the output file)
- [-e] : edit mode. If edit mode is not set, the changes are not written to file (useful for testing and debugging).
- [-m] : mode (see next section)
- [-b] : channel to modify 0-35 (if set to 36 all the channels are modified) (default: 36) 
- [-v] : new value (the value range depends on the mode) 
- [-t] : chip number. If a chip ID number is given, the program tries to read
  the mppc_map.csv and the arraymppc_data.root files and extract the breakdown
  voltage for every channel (every MPPC) of that chip. This information is used
  to adjust the inputDAC value. This feature is still untested and experimental.

Modes
=====

- 0 : trigger threshold    (10bit) 0-1023
  set the global threshold for all the channels
- 1 : gainselect threshold (10bit) 0-1023
  set the gain select threshold for all the channels. This thrshold is needed to
  select witch one between the high gain preamp and low gain preamp is selected
  and stored.
- 2 : inputDAC             (8bit)  0-255
  set the input DAC (high voltage adjustment). If the -t option is given the
  inputDAC is adjusted for each channel individually by using the 4-bit fine
  tuning input DAC in addition to the global 8-bit input DAC.
- 3 : HG & LG amplifier    (6bit)  0-63
  This mode modifies the value of the preamplifiers (both LG and HG) feedback
  capacitor
- 4 : threshold adjutment  (4bit)  0-15
  set the channel by channel 4-bit threshold adjustment DAC
- 5 : inputDAC reference   (1bit)  0-1
  input DAC Voltage Reference (1 = internal 4.5V   0 = internal 2.5V)
