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
ANAPEDESTAL="${BIN_DIR}/wgPedestalCalib"

CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

RUN_NAME="test_inputDAC121_pe2_dif_1_1_1"
RAW_FILE="${WAGASCI_MAINDIR}/configs/unit_tests/${RUN_NAME}.raw"
CONF_FILE="${WAGASCI_MAINDIR}/configs/unit_tests/${RUN_NAME}.xml"
TREE_FILE="${CURRENT_DIR}/${RUN_NAME}_tree.root"
HIST_FILE="${CURRENT_DIR}/${RUN_NAME}_hist.root"
ANA_DIR="${CURRENT_DIR}/AnaHist"
ANA_SUMMARY_DIR="${CURRENT_DIR}/OnePE/wgAnaHistSummary/Xml/dif1"
ANA_PEDESTAL_DIR="${CURRENT_DIR}/PedestalCalib"
IMAGE_DIR="${CURRENT_DIR}/Images"

ANA_MODE=20 # everything
ANA_SUMMARY_MODE=12  # everything
NCHIPS_DECODE=20
NCHANNELS_DECODE=32
DIF=1

echo " wgPedestalCalib unit test: input DAC 121 - 2 p.e."

if [ ! -f "${TREE_FILE}" ]; then
    echo ""
    echo " Decode   : ${DECODER}  -f  ${RAW_FILE}  -o  ${CURRENT_DIR}  -x  ${NCHIPS_DECODE}  -y  ${NCHANNELS_DECODE}  -r"
                     "${DECODER}" -f "${RAW_FILE}" -o "${CURRENT_DIR}" -x "${NCHIPS_DECODE}" -y "${NCHANNELS_DECODE}" -r
fi
if [ ! -f "${HIST_FILE}" ]; then
    echo ""
    echo " MakeHist : ${MAKEHIST}  -f  ${TREE_FILE}  -o  ${CURRENT_DIR}  -x  ${NCHIPS_DECODE}  -y  ${NCHANNELS_DECODE}  -r"
                     "${MAKEHIST}" -f "${TREE_FILE}" -o "${CURRENT_DIR}" -x "${NCHIPS_DECODE}" -y "${NCHANNELS_DECODE}" -r
fi
if [ ! -d "${ANA_DIR}" ]; then
    echo""
    echo " AnaHist  : ${ANAHIST}  -f  ${HIST_FILE}  -o  ${ANA_DIR}  -q  ${IMAGE_DIR}  -d  ${DIF}  -i  ${CONF_FILE}  -m  ${ANA_MODE}  -r"
                     "${ANAHIST}" -f "${HIST_FILE}" -o "${ANA_DIR}" -q "${IMAGE_DIR}" -d "${DIF}" -i "${CONF_FILE}" -m "${ANA_MODE}" -r
fi
if [ ! -d "${ANA_SUMMARY_DIR}" ]; then
    echo ""
    echo " AnaHistSummary : ${ANAHISTSUMMARY}  -f  ${ANA_DIR}  -o  ${ANA_SUMMARY_DIR}  -m  ${ANA_SUMMARY_MODE}  -r"
                           "${ANAHISTSUMMARY}" -f "${ANA_DIR}" -o "${ANA_SUMMARY_DIR}" -m "${ANA_SUMMARY_MODE}" -r
fi

#######################################################################################################################################################

echo ""
echo " PedestalCalib : ${ANAPEDESTAL}  -f  ${CURRENT_DIR}  -o  ${ANA_PEDESTAL_DIR}  -i  ${IMAGE_DIR}"
                      "${ANAPEDESTAL}" -f "${CURRENT_DIR}" -o "${ANA_PEDESTAL_DIR}" -i "${IMAGE_DIR}"
