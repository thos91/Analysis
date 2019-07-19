#!/bin/bash

set -e

if [ -z "$WAGASCI_MAINDIR" ]
then
    echo "WAGASCI environment not set"
    exit 1
fi

BIN_DIR="${WAGASCI_MAINDIR}/bin"
DECODER="${BIN_DIR}/wgDecoder"
CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
#INPUT_RAW="/home/neo/Downloads/190626_tdccheck2_dif_1_1_1.raw"  # offset 2
INPUT_RAW="/home/neo/Code/WAGASCI/Analysis/configs/unit_tests/test_inputDAC121_pe2_dif_1_1_1.raw" # offset 1
OUTPUT_DIR="${CURRENT_DIR}"
NCHIPS=20
DIF=1

echo " wgDecoder unit test:"
echo " Command : ${DECODER}  -f  ${INPUT_RAW}  -c \"\" -o  ${OUTPUT_DIR}  -x  ${NCHIPS}  -n  ${DIF}  -r -q"
                "${DECODER}" -f "${INPUT_RAW}" -c ""   -o "${OUTPUT_DIR}" -x "${NCHIPS}" -n "${DIF}" -r -q
