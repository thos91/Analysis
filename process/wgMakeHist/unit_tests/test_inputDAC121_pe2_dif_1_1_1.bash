#!/bin/bash

set -e

if [ -z "$WAGASCI_MAINDIR" ]
then
    echo "WAGASCI environment not set"
    exit 1
fi

BIN_DIR="${WAGASCI_MAINDIR}/bin"
DECODER="${BIN_DIR}/wgDecoder"
MAKEHIST="${BIN_DIR}/wgMakeHist"
CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
RAW_FILE="${WAGASCI_MAINDIR}/configs/unit_tests/test_inputDAC121_pe2_dif_1_1_1.raw"
TREE_FILE="${CURRENT_DIR}/test_inputDAC121_pe2_dif_1_1_1_tree.root"
#TREE_FILE="/home/neo/Desktop/test/1 PE/wgDecoder/test_1pe_121iDAC_dif1_tree.root"
OUTPUT_DIR="${CURRENT_DIR}"
NCHIPS=20
NCHANNELS=32

echo " wgMakeHist unit test: input DAC 121 - 2 p.e."
echo " Command : ${DECODER}  -f  ${RAW_FILE}  -o  ${OUTPUT_DIR}  -x  ${NCHIPS}  -r -q"
                "${DECODER}" -f "${RAW_FILE}" -o "${OUTPUT_DIR}" -x "${NCHIPS}" -r -q
echo " Command : ${MAKEHIST}  -f  ${TREE_FILE}  -o  ${OUTPUT_DIR}  -x  ${NCHIPS}  -y  ${NCHANNELS}  -r"
                "${MAKEHIST}" -f "${TREE_FILE}" -o "${OUTPUT_DIR}" -x "${NCHIPS}" -y "${NCHANNELS}" -r
