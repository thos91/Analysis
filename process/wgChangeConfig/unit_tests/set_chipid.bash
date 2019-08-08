#!/bin/bash

if [ -z "$WAGASCI_MAINDIR" ]
then
    echo "WAGASCI environment not set"
    exit 1
fi

BIN_DIR="${WAGASCI_MAINDIR}/bin"
CHANGE_CONFIG="${BIN_DIR}/wgChangeConfig"
CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
INPUT="${CURRENT_DIR}/test_bitstream.txt"
OUTPUT="${CURRENT_DIR}/output.txt"
CHIP_ID_MODE=5
CHIP_ID_VALUE=10

"${CHANGE_CONFIG}" -f "${INPUT}" -o "${OUTPUT}" -m "${CHIP_ID_MODE}" -v "${CHIP_ID_VALUE}" -e -r
