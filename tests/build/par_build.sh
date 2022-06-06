#!/bin/bash

#expected format: PARTITIONING_MODE, FPGA

for n in $(cat ./configuration.cfg)
do
 PART_MODE=$(echo $n | cut -d "," -f 1)
 FPGA_BOARD=$(echo $n | cut -d "," -f 2)
 DIR_NAME="build_${PART_MODE}_${FPGA_BOARD}"
 mkdir -p ${DIR_NAME}
 cd ${DIR_NAME}
 CMD="cmake -DCMAKE_BUILD_TYPE=Release -DPARTITIONING_MODE=${PART_MODE} -DFPGA=${FPGA_BOARD} ../../.."
 echo
 echo
 echo "Configuring '${CMD}'"
 echo "PWD '${PWD}'"
 echo
 echo
 eval "${CMD}"
 CMD="nice -n 19 make -j 4 &"
 echo
 echo
 echo "Compiling '${CMD}'"
 echo
 echo
 eval "${CMD}"
 status=$?
 [ $status -eq 0 ] && echo "Compilation successful" || echo "Compilation error"; exit 1
 cd ..
 exit 0
done
wait
echo "Compilation execution completed"
exit 0