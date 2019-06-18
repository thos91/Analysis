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
XML_CONFIG_FILE="${CURRENT_DIR}/wagasci_config_6asu.xml"
PHOTO_ELECTRONS=2
INPUTDAC=121
THRESHOLD_MODE=0

echo " wgOptimize unit test: threshold mode - 121 inputDAC - 2 p.e."
echo "${EXE} -t ${THRESHOLD_CARD} -f ${CALIBRATION_CARD} -s ${BITSTREAMS_DIR} -u ${XML_CONFIG_FILE} -p ${PHOTO_ELECTRONS} -i ${INPUTDAC} -m ${THRESHOLD_MODE}"

${EXE} -t ${THRESHOLD_CARD} -s ${BITSTREAMS_DIR} -u ${XML_CONFIG_FILE} -p ${PHOTO_ELECTRONS} -i ${INPUTDAC} -m ${THRESHOLD_MODE}
