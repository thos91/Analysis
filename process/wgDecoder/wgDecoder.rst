=========
wgDecoder
=========

The Decoder program takes as an input a raw data file *.raw* and
extracts (decodes) the information contained in it, organizing it into
a ROOT tree. The ROOT tree is then written to a *_tree.root* file.

The wgDecoder can decode even raw data where some headers and trailers
are corrupted or missing. Be aware that this does not mean that the
wgDecoder tampers or modify the physical data in any way. Only the
header and trailer sections are recovered or skipped in case of
corruption. This means that if there is any corruption in the SPIROC2D raw
data itself it is read and stored "as is" : no assumption is made
about the physical content of the raw data.

For example, if a certain chip trailer is missing/corrupted the
wgDecoder would not crash, but try to recover the missing/corrupted
trailer. The chip ID in the trailer is then compared with the chip ID
in the SPIROC2D raw data and if they match, everything is good,
otherwise the corrupted one is ignored and so on so forth.

For a more in-depth explanation about how the wgDecoder works
internally, refer to the comments contained in the wgDecoder*.hpp
headers.

Arguments
=========

- ``[-h]`` : prints an help message
- ``[-f]`` : input raw file to read (mandatory)
- ``[-c]`` : directory containing the calibration card files (default = WAGASCI_CONFDIR)
- ``[-o]`` : output directory for the ROOT file (default = WAGASCI_DECODEDIR)
- ``[-n]`` : DIF number 1-8. Useful only for the channel mapping (default = 1)
- ``[-x]`` : number of ASU chips per DIF 1-20 (default = automatically detected)
- ``[-r]`` : overwrite mode : overwrite the output ROOT tree file (default = false)
- ``[-q]`` : compatibility mode. Set this for raw data files acquired before the first half of 2018. Even if not set, the decoder tries to detect the old raw data format automatically (default = false)
- ``[-b]`` : silent mode : nothing is printed to the stardard output (default = false)

Raw DATA
========

The following variables are directly extracted from the raw data. All the other variables are calculated from the calibration data.

+----------------------------------------------------+--------------------------------------------------------+-----------------------------+
| **Variable**                                       | **Description**                                        | **Raw data field**          |
+----------------------------------------------------+--------------------------------------------------------+-----------------------------+
| ``spill_number``                                   | spill number                                           | <Spill number>              |
+----------------------------------------------------+--------------------------------------------------------+-----------------------------+
| ``spill_mode``                                     | 0 for non-beam spill, 1 for beam spill                 | <Spill mode>                |
+----------------------------------------------------+--------------------------------------------------------+-----------------------------+
| ``spill_count``                                    | number of spill acquired during this acquisition run   | <ACQid>                     |
+----------------------------------------------------+--------------------------------------------------------+-----------------------------+
| ``bcid[<n_chips>][<n_columns>]``                   | the bunch crossing identification time (16 bit integer)| <Bunch Crossing ID>         |
+----------------------------------------------------+--------------------------------------------------------+-----------------------------+
| ``charge[<n_chips>][<n_channels>][<n_columns>]``   | ADC counts                                             | <Charge measure>            |
+----------------------------------------------------+--------------------------------------------------------+-----------------------------+
| ``time[<n_chips>][<n_channels>][<n_columns>]``     | TDC counts                                             | <Time measure>              |
+----------------------------------------------------+--------------------------------------------------------+-----------------------------+
| ``gain[<n_chips>][<n_channels>][<n_columns>]``     | if the gain bit in the raw data (for each chip and     | <Gain bit>                  |
|                                                    | each channel) is set to one the high gain preamplifier |                             |
|                                                    | is used, if it set to zero the low gain one is used    |                             |
+----------------------------------------------------+--------------------------------------------------------+-----------------------------+
| ``hit[<n_chips>][<n_channels>][<n_columns>]``      | if the hit bit in the raw data (for each chip and each | <Hit bit>                   |
|                                                    | channel) is set to one we have a hit, if it set to zero|                             |
|                                                    | we don't have a hit preamplifier is used.              |                             |
+----------------------------------------------------+--------------------------------------------------------+-----------------------------+
| ``chipid[<n_chips>]``                              | chip ID tag as it is recorded in the chip trailer.     | <CHIP ID>                   |
+----------------------------------------------------+--------------------------------------------------------+-----------------------------+
| ``debug_chip[<n_chips>]``                          | for each chip DEBUG macro defined in                   |                             |
|                                                    | wgDecoderReader.hpp that bin is increased by one every |                             |
|                                                    | time that the relative error occurres.                 |                             |
+----------------------------------------------------+--------------------------------------------------------+-----------------------------+
| ``debug_spill``                                    | for each spill DEBUG macro defined in                  |                             |
|                                                    | wgDecoderReader.hpp that bin is increased by one every |                             |
|                                                    | time that the relative error occurres.                 |                             |
+----------------------------------------------------+--------------------------------------------------------+-----------------------------+


.. image:: ../images/DIF_data_format.png
            :width: 600px

.. image:: ../images/SPIROC2D_data_format.png
            :width: 600px                   
