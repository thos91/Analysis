#ifndef WGERRORCODES_H
#define WGERRORCODES_H

const int WG_SUCCESS                     = 0;

// File system errors

const int ERR_FAILED_CREATE_DIRECTORY    = 1;
const int ERR_FAILED_OPEN_XML_FILE       = 2;
const int ERR_FAILED_OPEN_RAW_FILE       = 3;
const int ERR_FAILED_OPEN_TREE_FILE      = 4;
const int ERR_FAILED_OPEN_HIST_FILE      = 5;
const int ERR_FAILED_CREATE_XML_FILE     = 6;
const int ERR_FAILED_GET_BISTREAM        = 7;
const int ERR_FAILED_GET_FILE_LIST       = 8;

// Read - write error

const int ERR_FAILED_WRITE               = 9;
const int ERR_THRESHOLD_CARD_READ        = 10;
const int ERR_CALIBRATION_CARD_READ      = 11;
const int ERR_INPUTDAC_WRITE             = 12;
const int ERR_THRESHOLD_WRITE            = 13;

// Wrong value errors

const int ERR_WRONG_DIF_VALUE            = 14;
const int ERR_WRONG_CHIP_VALUE           = 15;
const int ERR_WRONG_CHANNEL_VALUE        = 16;
const int ERR_WRONG_INPUTDAC_VALUE       = 17;
const int ERR_WRONG_PE_VALUE             = 18;
const int ERR_WRONG_MODE                 = 19;
const int ERR_VALUE_OUT_OF_RANGE         = 20;

// File not found kind of errors

const int ERR_EMPTY_INPUT_FILE           = 21;
const int ERR_INPUT_FILE_NOT_FOUND       = 22;
const int ERR_CALIBRATION_CARD_NOT_FOUND = 23;
const int ERR_THRESHOLD_CARD_NOT_FOUND   = 24;
const int ERR_BITSTREAM_FILE_NOT_FOUND   = 25;
const int ERR_CONFIG_XML_FILE_NOT_FOUND  = 26;

// Generic errors

const int ERR_WG_ANA_HIST_SUMMARY        = 27;
const int ERR_WG_OPTIMIZE                = 28;
const int ERR_WG_SCURVE                  = 29;
const int ERR_TOPOLOGY                   = 30;
const int ERR_WG_DECODER                 = 31;

// Other errors

const int ERR_OVERWRITE_FLAG_NOT_SET     = 32;
const int ERR_NOT_IMPLEMENTED_YET        = 33;

#endif /* WGERRORCODES_H */
