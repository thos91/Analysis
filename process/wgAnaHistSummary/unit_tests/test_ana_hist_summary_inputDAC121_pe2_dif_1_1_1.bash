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
ANAHIST="${BIN_DIR}/wgAnaHist"
ANAHISTSUMMARY="${BIN_DIR}/wgAnaHistSummary"

CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

RUN_NAME="test_inputDAC121_pe2_dif_1_1_1"
RAW_FILE="${WAGASCI_MAINDIR}/configs/unit_tests/${RUN_NAME}.raw"
CONF_FILE="${WAGASCI_MAINDIR}/configs/unit_tests/${RUN_NAME}.xml"
TREE_FILE="${CURRENT_DIR}/${RUN_NAME}_tree.root"
HIST_FILE="${CURRENT_DIR}/${RUN_NAME}_hist.root"
ANA_DIR="${CURRENT_DIR}/AnaHist"
SUMMARY_DIR="${CURRENT_DIR}/AnaHistSummary"
IMAGE_DIR="${CURRENT_DIR}/Images"

ANA_MODE=20
ANA_SUMMARY_MODE=12

DIF=1
NCHIPS=20
NCHANNELS=32

echo " wgAnaHistSummary unit test: input DAC 121 - 2 p.e."

if [ ! -f "${TREE_FILE}" ]; then
    echo ""
    echo " Decode   :  ${DECODER}  -f  ${RAW_FILE}  -o  ${CURRENT_DIR}  -x  ${NCHIPS}  -y  ${NCHANNELS}  -r"
                      "${DECODER}" -f "${RAW_FILE}" -o "${CURRENT_DIR}" -x "${NCHIPS}" -y "${NCHANNELS}" -r
fi
if [ ! -f "${HIST_FILE}" ]; then
    echo ""
    echo " MakeHist :  ${MAKEHIST}  -f  ${TREE_FILE}  -o  ${CURRENT_DIR}  -x  ${NCHIPS}  -y  ${NCHANNELS}  -r"
                      "${MAKEHIST}" -f "${TREE_FILE}" -o "${CURRENT_DIR}" -x "${NCHIPS}" -y "${NCHANNELS}" -r
fi
if [ ! -d "${ANA_DIR}" ]; then
    echo""
    echo " AnaHist  :  ${ANAHIST}  -f  ${HIST_FILE}  -o  ${ANA_DIR}  -d  ${DIF}  -i  ${CONF_FILE}  -m  ${ANA_MODE}  -r"
                      "${ANAHIST}" -f "${HIST_FILE}" -o "${ANA_DIR}" -d "${DIF}" -i "${CONF_FILE}" -m "${ANA_MODE}" -r
fi

echo ""
echo " AnaHistSummary :  ${ANAHISTSUMMARY}  -f  ${ANA_DIR}  -o  ${SUMMARY_DIR}  -i  ${IMAGE_DIR}  -m  ${ANA_SUMMARY_MODE}  -r -p"
                        "${ANAHISTSUMMARY}" -f "${ANA_DIR}" -o "${SUMMARY_DIR}" -i "${IMAGE_DIR}" -m "${ANA_SUMMARY_MODE}" -r -p
