#!/bin/bash

if [ -z "$WAGASCI_MAINDIR" ]
then
	echo "WAGASCI environment not set"
	exit 1
fi

BIN_DIR="${WAGASCI_MAINDIR}/bin"
EXE="${BIN_DIR}/wgChangeConfig"
CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
INPUT="${CURRENT_DIR}/test_bitstream.txt"
OUTPUT="${CURRENT_DIR}/output.txt"
PREAMP_MODE=3
PREAMP_VALUE=0
ALL_CHANNELS=36

${EXE} -f ${INPUT} -o ${OUTPUT} -m ${PREAMP_MODE} -b ${ALL_CHANNELS} -v ${PREAMP_VALUE} -e -r
