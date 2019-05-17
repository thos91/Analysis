#!/bin/bash

if [ -z "$WAGASCI_MAINDIR" ]
then
	echo "WAGASCI environment not set"
	exit 1
fi

BIN_DIR="${WAGASCI_MAINDIR}/bin"
EXE="${BIN_DIR}/wgOptimize"
CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
THRESHOLD_CARD="${CURRENT_DIR}/threshold_card.xml"
BITSTREAMS_DIR="${CURRENT_DIR}"
NDIFS=1
NCHIPS=2
NCHANNELS=32
PHOTO_ELECTRONS=2
INPUTDAC=121
THRESHOLD_MODE=0

echo " wgOptimize unit test: threshold mode - 121 inputDAC - 2 p.e."
echo "${EXE} -t ${THRESHOLD_CARD} -f ${CALIBRATION_CARD} -s ${BITSTREAMS_DIR} -d ${NDIFS} -c ${NCHIPS} -e ${NCHANNELS} -p ${PHOTO_ELECTRONS} -i ${INPUTDAC} -m ${THRESHOLD_MODE}"
echo "The threshold should be 154 (0010011010) for the chip_1 and 157 (0010011101) for the chip_2"

${EXE} -t ${THRESHOLD_CARD} -s ${BITSTREAMS_DIR} -d ${NDIFS} -c ${NCHIPS} -e ${NCHANNELS} -p ${PHOTO_ELECTRONS} -i ${INPUTDAC} -m ${THRESHOLD_MODE}
