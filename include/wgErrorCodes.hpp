#ifndef WGERRORCODES_H
#define WGERRORCODES_H

const int WG_SUCCESS                     = 0;

// File system errors

const int ERR_FAILED_CREATE_DIRECTORY    = 1;
const int ERR_FAILED_OPEN_XML_FILE       = 2;
const int ERR_FAILED_OPEN_HIST_FILE      = 3;
const int ERR_FAILED_OPEN_RAW_FILE       = 4;
const int ERR_FAILED_CREATE_XML_FILE     = 5;
const int ERR_FAILED_GET_BISTREAM        = 6;
const int ERR_FAILED_GET_FILE_LIST       = 7;

// Read - write error

const int ERR_FAILED_WRITE               = 8;
const int ERR_THRESHOLD_CARD_READ        = 9;
const int ERR_CALIBRATION_CARD_READ      = 10;
const int ERR_INPUTDAC_WRITE             = 11;
const int ERR_THRESHOLD_WRITE            = 12;

// Wrong value errors

const int ERR_WRONG_DIF_VALUE            = 13;
const int ERR_WRONG_CHIP_VALUE           = 14;
const int ERR_WRONG_CHANNEL_VALUE        = 15;
const int ERR_WRONG_INPUTDAC_VALUE       = 16;
const int ERR_WRONG_PE_VALUE             = 17;
const int ERR_WRONG_MODE                 = 18;
const int ERR_VALUE_OUT_OF_RANGE         = 19;

// File not found kind of errors

const int ERR_EMPTY_INPUT_FILE           = 20;
const int ERR_INPUT_FILE_NOT_FOUND       = 21;
const int ERR_CALIBRATION_CARD_NOT_FOUND = 22;
const int ERR_THRESHOLD_CARD_NOT_FOUND   = 23;
const int ERR_BITSTREAM_FILE_NOT_FOUND   = 24;
const int ERR_CONFIG_XML_FILE_NOT_FOUND  = 25;

// Generic errors

const int ERR_WG_ANA_HIST_SUMMARY        = 26;
const int ERR_WG_OPTIMIZE                = 27;
const int ERR_WG_SCURVE                  = 28;
const int ERR_TOPOLOGY                   = 29;
const int ERR_WG_DECODER                 = 30;

// Other errors

const int ERR_OVERWRITE_FLAG_NOT_SET     = 31;
const int ERR_NOT_IMPLEMENTED_YET        = 32;

#endif /* WGERRORCODES_H */
