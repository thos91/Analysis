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
XML_CONFIG_FILE="${CURRENT_DIR}/wagasci_config.xml"
PHOTO_ELECTRONS=2
INPUTDAC=121
MODE=0 # threshold mode 0 ; inputdac mode 1

echo " wgOptimize unit test: threshold mode"
echo "Command : ${EXE} -t ${THRESHOLD_CARD} -s ${BITSTREAMS_DIR} -u ${XML_CONFIG_FILE} -p ${PHOTO_ELECTRONS} -i ${INPUTDAC} -m ${MODE}"
                ${EXE} -t ${THRESHOLD_CARD} -s ${BITSTREAMS_DIR} -u ${XML_CONFIG_FILE} -p ${PHOTO_ELECTRONS} -i ${INPUTDAC} -m ${MODE}
