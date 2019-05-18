#!/bin/bash

set -e

if [ -z "$WAGASCI_MAINDIR" ]
then
	echo "WAGASCI environment not set"
	exit 1
fi

BIN_DIR="${WAGASCI_MAINDIR}/bin"
DECODER="${BIN_DIR}/wgDecoder"
EXE="${BIN_DIR}/wgMakeHist"
CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
INPUT_RAW="${WAGASCI_MAINDIR}/configs/unit_tests/test_inputDAC121_pe2_dif_1_1_1.raw"
INPUT_TREE="${CURRENT_DIR}/test_inputDAC121_pe2_dif_1_1_1_tree.root"
OUTPUT_DIR="${CURRENT_DIR}"
NCHIPS=20
NCHANNELS=32

echo " wgMakeHist unit test: input DAC 121 - 2 p.e."
echo " Command : ${DECODER} -f ${INPUT_RAW} -o ${OUTPUT_DIR} -x ${NCHIPS} -y ${NCHANNELS} -r"
echo " Command : ${EXE} -f ${INPUT_TREE} -o ${OUTPUT_DIR} -x ${NCHIPS} -y ${NCHANNELS} -r"

${DECODER} -f ${INPUT_RAW} -o ${OUTPUT_DIR} -x ${NCHIPS} -y ${NCHANNELS} -r
${EXE} -f ${INPUT_TREE} -o ${OUTPUT_DIR} -x ${NCHIPS} -y ${NCHANNELS} -r
