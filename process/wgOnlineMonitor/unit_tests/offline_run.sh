#!/bin/sh

. /opt/pyrame/ports.sh

BIN_DIR="${WAGASCI_MAINDIR}/bin"
CONFIG_DIR="${WAGASCI_MAINDIR}/configs"
DIF_ID=6

data_file="/home/neo/Desktop/test_ecal_dif_6.raw"
#data_file="${CONFIG_DIR}/unit_tests/test_inputDAC121_pe2_dif_0.raw"

pyrame_config=$(echo "${data_file}" | sed -E "s/_ecal_dif_[0-9]+\.raw/\.xml/")
    
source_name=$(echo "${data_file}" | sed -E "s/.*(dif.*)\.raw/\1/")
# CMD_CONVERTER
#   1) ds_name            : dif_X
#   2) port               : undef
#   3) uri                : data_file_path
#   4) convert_plugin     : /opt/pyrame/spiroc2d_decoder.so
# SPIROC2D_DECODER arguments
#   1) dif_id             : DIF_ID
#   3) cut_non_hit        : 1
#   4) min_energy         : 0
#   5) mapping_filename   : /home/neo/Code/WAGASCI/Configs/wagasci_mapping_table.txt
#   6) offset_x           : 0
#   7) offset_y           : 0
#   8) offset_z           : 0
#   9) max_roc            : n_chips
echo "cmdmod /opt/pyrame/cmd_converter.xml -a \"$source_name\" undef \"file://${data_file}\" /opt/pyrame/spiroc2d_decoder.so ${DIF_ID} 1 0 \"${CONFIG_DIR}/mapping/wagasci_mapping_table.txt\" 0 0 0 20 > /dev/null 2>&1 &"
cmdmod /opt/pyrame/cmd_converter.xml -a "$source_name" undef "file://${data_file}" /opt/pyrame/spiroc2d_decoder.so ${DIF_ID} 1 0 "${CONFIG_DIR}/mapping/wagasci_mapping_table.txt" 0 0 0 20 &
    
sleep 1s
cntds.py "converter_${source_name}" start_acq_converter

echo "\"${BIN_DIR}/wgOnlineMonitor\" --dif_id ${DIF_ID} --pyrame_config \"${pyrame_config}\""
"${BIN_DIR}/wgOnlineMonitor" --dif_id ${DIF_ID} --pyrame_config "${pyrame_config}"
