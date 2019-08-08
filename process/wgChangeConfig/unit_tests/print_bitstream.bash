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

echo "${EXE}  -f  ${INPUT}"
     "${EXE}" -f "${INPUT}"
