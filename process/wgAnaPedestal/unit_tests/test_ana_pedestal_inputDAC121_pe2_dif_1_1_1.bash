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
ANAPEDESTAL="${BIN_DIR}/wgAnaPedestal"

CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

RUN_NAME="test_inputDAC121_pe2_dif_1_1_1"
RAW_FILE="${WAGASCI_MAINDIR}/configs/unit_tests/${RUN_NAME}.raw"
CONF_FILE="${WAGASCI_MAINDIR}/configs/unit_tests/${RUN_NAME}.xml"
TREE_FILE="${CURRENT_DIR}/${RUN_NAME}_tree.root"
HIST_FILE="${CURRENT_DIR}/${RUN_NAME}_hist.root"
ANA_DIR="${CURRENT_DIR}/AnaHist"
ANA_SUMMARY_DIR="${CURRENT_DIR}/AnaSummary"
ANA_PEDESTAL_DIR="${CURRENT_DIR}/AnaPedestal"
IMAGE_DIR="${CURRENT_DIR}/Images"

ANA_MODE=12 # dark noise + pedestal + charge_HG
ANA_SUMMARY_MODE=12  # dark noise + pedestal + charge_HG
DIF=1
NCHIPS_DECODE=20
NCHANNELS_DECODE=32
NCHIPS_ANA=1
NCHANNELS_ANA=32

echo " wgAnaPedestal unit test: input DAC 121 - 2 p.e."

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
    echo " AnaHistSummary : ${ANAHISTSUMMARY}  -f  ${ANA_DIR}/${RUN_NAME}  -o  ${ANA_SUMMARY_DIR}/${RUN_NAME}  -i  ${IMAGE_DIR}  -x  ${NCHIPS_ANA}  -m  ${ANA_SUMMARY_MODE}  -y  ${NCHANNELS_ANA}  -r"
                           "${ANAHISTSUMMARY}" -f "${ANA_DIR}/${RUN_NAME}" -o "${ANA_SUMMARY_DIR}/${RUN_NAME}" -i "${IMAGE_DIR}" -x "${NCHIPS_ANA}" -m "${ANA_SUMMARY_MODE}" -y "${NCHANNELS_ANA}" -r
fi

#######################################################################################################################################################

echo ""
echo " AnaPedestal : ${ANAPEDESTAL}  -f  ${ANA_SUMMARY_DIR}/${RUN_NAME}  -o  ${ANA_PEDESTAL_DIR}  -i  ${IMAGE_DIR}  -n  ${DIF}  -x  ${NCHIPS_ANA}  -y  ${NCHANNELS_ANA}"
                    "${ANAPEDESTAL}" -f "${ANA_SUMMARY_DIR}" -o "${ANA_PEDESTAL_DIR}" -i "${IMAGE_DIR}" -n "${DIF}" -x "${NCHIPS_ANA}" -y "${NCHANNELS_ANA}"
