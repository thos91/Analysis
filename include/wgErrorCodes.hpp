#ifndef WGERRORCODES_H
#define WGERRORCODES_H

enum WG_ERROR_CODES {
  WG_SUCCESS                     = 0,

  // File system errors

  ERR_FAILED_CREATE_DIRECTORY    = 1,
  ERR_FAILED_OPEN_XML_FILE       = 2,
  ERR_FAILED_OPEN_RAW_FILE       = 3,
  ERR_FAILED_OPEN_TREE_FILE      = 4,
  ERR_FAILED_OPEN_HIST_FILE      = 5,
  ERR_FAILED_CREATE_XML_FILE     = 6,
  ERR_FAILED_GET_BISTREAM        = 7,
  ERR_FAILED_GET_FILE_LIST       = 8,

  // Read - write error

  ERR_FAILED_WRITE               = 9,
  ERR_THRESHOLD_CARD_READ        = 10,
  ERR_GAIN_CARD_READ             = 11,
  ERR_INPUTDAC_WRITE             = 12,
  ERR_THRESHOLD_WRITE            = 13,

  // Wrong value errors

  ERR_WRONG_DIF_VALUE            = 14,
  ERR_WRONG_CHIP_VALUE           = 15,
  ERR_WRONG_CHANNEL_VALUE        = 16,
  ERR_WRONG_INPUTDAC_VALUE       = 17,
  ERR_WRONG_PE_VALUE             = 18,
  ERR_WRONG_MODE                 = 19,
  ERR_VALUE_OUT_OF_RANGE         = 20,

  // File not found kind of errors

  ERR_EMPTY_INPUT_FILE           = 21,
  ERR_INPUT_FILE_NOT_FOUND       = 22,
  ERR_GAIN_CARD_NOT_FOUND        = 23,
  ERR_THRESHOLD_CARD_NOT_FOUND   = 24,
  ERR_BITSTREAM_FILE_NOT_FOUND   = 25,
  ERR_CONFIG_XML_FILE_NOT_FOUND  = 26,

  // Generic errors

  ERR_WG_ANA_HIST_SUMMARY        = 27,
  ERR_WG_OPTIMIZE                = 28,
  ERR_WG_SCURVE                  = 29,
  ERR_TOPOLOGY                   = 30,
  ERR_WG_DECODER                 = 31,

  // Other errors

  ERR_OVERWRITE_FLAG_NOT_SET     = 32,
  ERR_NOT_IMPLEMENTED_YET        = 33,
  ERR_EVENT_LOOP                 = 34,
  ERR_WORKSPACE_INITIALIZATION   = 35,

  N_WG_ERROR_CODES
};

#endif /* WGERRORCODES_H */
