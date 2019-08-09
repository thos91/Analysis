#!/bin/bash

set -e

if [ -z "$WAGASCI_MAINDIR" ]
then
    echo "WAGASCI environment not set"
    exit 1
fi

RUN_NAME="test_inputDAC121_pe2_dif_0"

BIN_DIR="${WAGASCI_MAINDIR}/bin"
DECODER="${BIN_DIR}/wgDecoder"
MAKEHIST="${BIN_DIR}/wgMakeHist"
ANAHIST="${BIN_DIR}/wgAnaHist"

CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
RAW_FILE="${WAGASCI_MAINDIR}/configs/unit_tests/${RUN_NAME}.raw"
XML_CONFIG_FILE="${WAGASCI_MAINDIR}/configs/unit_tests/${RUN_NAME}.xml"
TREE_FILE="${CURRENT_DIR}/${RUN_NAME}_tree.root"
HIST_FILE="${CURRENT_DIR}/${RUN_NAME}_hist.root"
EVERYTHING_MODE=20

OUTPUT_XMLDIR="${CURRENT_DIR}/xml"
OUTPUT_IMGDIR="${CURRENT_DIR}/img"
DIF=0
NCHIPS=20

echo " wgAnaHist unit test: input DAC 121 - 2 p.e."

if [ ! -f "${TREE_FILE}" ]; then
	echo " Decode   :  ${DECODER}  -f  ${RAW_FILE}  -o  ${CURRENT_DIR}  -x  ${NCHIPS}  -r -q"
	                  "${DECODER}" -f "${RAW_FILE}" -o "${CURRENT_DIR}" -x "${NCHIPS}" -r -q
fi
if [ ! -f "${HIST_FILE}" ]; then
	echo " MakeHist :  ${MAKEHIST}  -f  ${TREE_FILE}  -o  ${CURRENT_DIR}  -x  ${NCHIPS}  -r"
                          "${MAKEHIST}" -f "${TREE_FILE}" -o "${CURRENT_DIR}" -x "${NCHIPS}" -r
fi

echo " AnaHist  :  ${ANAHIST}  -f  ${HIST_FILE}  -o  ${OUTPUT_XMLDIR}  -q  ${OUTPUT_IMGDIR}  -d  ${DIF}  -i  ${XML_CONFIG_FILE}  -m  ${EVERYTHING_MODE}  -r"
                  "${ANAHIST}" -f "${HIST_FILE}" -o "${OUTPUT_XMLDIR}" -q "${OUTPUT_IMGDIR}" -d "${DIF}" -i "${XML_CONFIG_FILE}" -m "${EVERYTHING_MODE}" -r
