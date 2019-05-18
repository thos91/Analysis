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
CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
INPUT_RAW="${WAGASCI_MAINDIR}/configs/unit_tests/test_inputDAC121_pe2_dif_1_1_1.raw"
INPUT_XML="${WAGASCI_MAINDIR}/configs/unit_tests/test_inputDAC121_pe2_dif_1_1_1.xml"
INPUT_TREE="${CURRENT_DIR}/test_inputDAC121_pe2_dif_1_1_1_tree.root"
INPUT_HIST="${CURRENT_DIR}/test_inputDAC121_pe2_dif_1_1_1_hist.root"
EVERYTHING_MODE=20

OUTPUT_DIR="${CURRENT_DIR}"
DIF=1
NCHIPS=20
NCHANNELS=32

echo " wgAnaHist unit test: input DAC 121 - 2 p.e."

if [ ! -f "${INPUT_TREE}" ]; then
	echo " Decode   : ${DECODER}  -f ${INPUT_RAW}  -o ${OUTPUT_DIR} -x ${NCHIPS} -y ${NCHANNELS} -r"
	${DECODER}  -f ${INPUT_RAW}  -o ${OUTPUT_DIR} -x ${NCHIPS} -y ${NCHANNELS} -r
fi
if [ ! -f "${INPUT_HIST}" ]; then
	echo " MakeHist : ${MAKEHIST} -f ${INPUT_TREE} -o ${OUTPUT_DIR} -x ${NCHIPS} -y ${NCHANNELS} -r"
	${MAKEHIST} -f ${INPUT_TREE} -o ${OUTPUT_DIR} -x ${NCHIPS} -y ${NCHANNELS} -r
fi

echo " AnaHist  : ${ANAHIST}  -f ${INPUT_HIST} -o ${OUTPUT_DIR} -x ${NCHIPS} -y ${NCHANNELS} -d ${DIF} -i ${INPUT_XML} -m ${EVERYTHING_MODE} -r"
${ANAHIST}  -f ${INPUT_HIST} -o ${OUTPUT_DIR} -x ${NCHIPS} -y ${NCHANNELS} -d ${DIF} -i ${INPUT_XML} -m ${EVERYTHING_MODE} -r
