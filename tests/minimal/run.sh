#!/bin/bash


############################################################
# Check vivado version                                     #
############################################################
if [[ -z "$XILINX_VIVADO" ]]
then
    echo
    echo "ERROR: Expecting vivado 2020.2. The process is not tested with other versions..."
    echo
    exit 1
fi

############################################################
# Check if the sum_vec IP has been compiled                #
############################################################
ip_path=$DART_IP_PATH
if [[ ! -d "$ip_path/sum_vec" ]]
then
    echo
    echo "ERROR: sum_vec IP not found in ${ip_path}/sum_vec or DART_IP_PATH environment variable not set."
    echo
fi

# Expected format of configuration.cfg: PARTITIONING_MODE, FPGA
# Note that us_96 and zcu_102 boards are not implemented in partitioning mode.

for n in $(cat ../configuration.cfg)
do
 PART_MODE=$(echo $n | cut -d "," -f 1)
 FPGA_BOARD=$(echo $n | cut -d "," -f 2)
 DIR_NAME="build_${PART_MODE}_${FPGA_BOARD}"
 mkdir -p ${DIR_NAME}
 cd ${DIR_NAME}
 DART_PATH="../../build/${DIR_NAME}/dart"
 if [[ ! -f ${DART_PATH} ]]
 then
    echo "DART '${DART_PATH}' does not exist!"
    exit 1
 fi
 YAML_FILE=""
 if [[ "$PART_MODE" == "ON" ]]
 then
    YAML_FILE="../part-on.yaml"
 else
    YAML_FILE="../part-off.yaml"
 fi
 CMD="${DART_PATH} ${YAML_FILE}"
 echo
 echo
 echo "Executing DART: '${CMD}'"
 echo "PWD '${PWD}'"
 echo
 echo
#  eval "nice -n 19 ${CMD} &"
 eval "nice -n 19 ${CMD}"
 status=$?
 echo
 echo
 if [ $status -eq 0 ]
 then
    echo "DART execution successful"
 else
    echo "DART execution error"
    exit 1
 fi
 cd ..
done
wait
echo "DART execution completed"
exit 0