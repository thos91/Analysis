#!/bin/sh

runid=69
NumFile=6

for i in `seq 0 5`
do
  echo $i
  rm -f /home/data/recon/run_000${runid}_00${i}_cosmic_hittiming.root
  sleep 1
  process="/usr/local/src/wagasci_software/Analysis/bin/wgHitTimeCheck -f run_000${runid}_00${i}_cosmic >> /dev/null 2>&1 &"
  echo ${process}
  ${process}
done

while true
do
  num_file=0
  for i in `seq 0 5`
  do
    if [ -f /home/data/recon/run_000${runid}_00${i}_cosmic_hittiming.root ]
    then
      num_file=`expr ${num_file} + 1`
    fi
  done
  echo ${num_file}
  if [ ${num_file} -eq ${NumFile} ]
  then
    rm -f /home/users/nchikuma/rootfile/run_000${runid}_hittiming.root
    sleep 1
    hadd /home/users/nchikuma/rootfile/run_000${runid}_hittiming.root /home/data/recon/run_000${runid}*_hittiming.root
    break
  else
    echo "checking..."
    sleep 1
  fi
done
