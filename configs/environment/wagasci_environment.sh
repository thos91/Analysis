#!/bin/bash

#### Directories ####

export WAGASCI_DIR=/home/neo/Code/WAGASCI
export WAGASCI_RAWDATADIR=${WAGASCI_DIR}/Data/rawdata
export WAGASCI_DECODEDIR=${WAGASCI_DIR}/Data/decode
export WAGASCI_HISTDIR=${WAGASCI_DIR}/Data/hist
export WAGASCI_RECONDIR=${WAGASCI_DIR}/Data/recon
export WAGASCI_XMLDATADIR=${WAGASCI_DIR}/Data/xmlfile
export WAGASCI_IMGDATADIR=${WAGASCI_DIR}/Data/image
export WAGASCI_LOGDIR=${WAGASCI_DIR}/Data/log
export WAGASCI_MAINDIR=${WAGASCI_DIR}/Analysis
export WAGASCI_CALICOESDIR=/opt/calicoes
export WAGASCI_MIDASDIR=${WAGASCI_DIR}/calicoes/midas
export WAGASCI_CALIBDATADIR=${WAGASCI_DIR}/Data/calibration
export WAGASCI_BSDDIR=${WAGASCI_DIR}/Data/bsd
export WAGASCI_DQDIR=${WAGASCI_DIR}/Data/dq
export WAGASCI_DQHISTORYDIR=${WAGASCI_DIR}/Data/dq_history
export WAGASCI_DaqCommandDir=${WAGASCI_CALICOESDIR}
export WAGASCI_CONFDIR=${WAGASCI_DIR}/Configs
export WAGASCI_CARDFILESDIR=${WAGASCI_CONFDIR}

#### PATH ####

export PATH=${WAGASCI_MAINDIR}/bin:${PATH}

