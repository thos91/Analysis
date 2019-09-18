#!/bin/sh
. /opt/pyrame/ports.sh
if test $# -lt 1
then
echo "usage $0 data_folder"
exit 1
fi

#sudo systemctl stop pyrame
#sleep 1s
#sudo systemctl start pyrame
#sleep 2s

cd $1
for data_file in *.raw
do
    #source_name=`echo ${data_file}|sed -r "s/.*([0-9][0-9][0-9]_dif.*)\.raw/\1/"`
	source_name=`echo ${data_file}|sed -r "s/.*(dif.*)\.raw/\1/"`
	# CMD_CONVERTER
	#   1) ds_name            : dif_1_1_X
	#   2) port               : undef
	#   3) uri                : ~/Code/WAGASCI/Data/online-monirot-test/run_00062
	#   4) convert_plugin     : /opt/pyrame/spiroc2d_decoder.so
	# SPIROC2D_DECODER arguments
	#   1) dif_id             : 1
	#   2) ref                : 0
	#   3) cut_non_hit        : 1
	#   4) min_energy         : 0
	#   5) mapping_filename   : /home/neo/Code/WAGASCI/Configs/wagasci_mapping_table.txt
	#   6) offset_x           : 0
	#   7) offset_y           : 0
	#   9) max_roc            : 20
	cmdmod /opt/pyrame/cmd_converter.xml -a $source_name undef file://$PWD/${data_file} /opt/pyrame/spiroc2d_decoder.so 1 0 1 0 /home/neo/Code/WAGASCI/Configs/wagasci_mapping_table.txt 0 0 20 &
done

sleep 1s
for data_file in *.raw
do
    #source_name=`echo ${data_file}|sed -r "s/.*([0-9][0-9][0-9]_dif.*)\.raw/\1/"`
	source_name=`echo ${data_file}|sed -r "s/.*(dif.*)\.raw/\1/"`
    cntds.py converter_${source_name} start_acq_converter
done
