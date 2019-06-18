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
CALIBRATION_CARD="${CURRENT_DIR}/calibration_card.xml"
XML_CONFIG_FILE="${CURRENT_DIR}/wagasci_config_6asu.xml"
BITSTREAMS_DIR="${CURRENT_DIR}"
PHOTO_ELECTRONS=1
INPUTDAC_MODE=1

echo " wgOptimize unit test: input DAC mode - 1 p.e."
echo " Command : ${EXE} -t ${THRESHOLD_CARD} -f ${CALIBRATION_CARD} -u ${XML_CONFIG_FILE} -s ${BITSTREAMS_DIR} -p ${PHOTO_ELECTRONS} -i ${INPUTDAC} -m ${THRESHOLD_MODE}"

${EXE} -t ${THRESHOLD_CARD} -f ${CALIBRATION_CARD} -u ${XML_CONFIG_FILE} -s ${BITSTREAMS_DIR} -p ${PHOTO_ELECTRONS} -m ${INPUTDAC_MODE}
