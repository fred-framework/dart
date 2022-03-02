#!/bin/bash

# set it to 1 to enable these phases
compile_ips=0

if [[ -z "${XILINX_VIVADO}" ]]; then
  echo "XILINX_VIVADO is undefined"
  exit 1
fi

if [[ -z "${XILINX_HLS}" ]]; then
  echo "XILINX_HLS is undefined"
  exit 1
fi

if [[ -z "${DART_HOME}" ]]; then
  echo "DART_HOME is undefined"
  exit 1
fi

if [[ -z "${DART_HOME}/bin/dart" ]]; then
  echo "${DART_HOME}/bin/dart is not found"
  exit 1
fi

# create the IPs and link them to the DART_IP_PATH directory
if [ "${compile_ips}" == 1 ]; then
    mkdir -p compiled_ips
    export DART_IP_PATH=${PWD}/compiled_ips
    git clone https://github.com/fred-framework/dart_ips
    cd dart_ips/ips/
    make
else
    if [[ -z "${DART_IP_PATH}" ]]; then
        echo "${DART_IP_PATH} is undefined"
        exit 1
    fi
fi
# prepare for running DART
mkdir -p dart
cd dart
${DART_HOME}/bin/dart ../dart.yaml
cd fred
tar czf ../../fred.tar.gz .
cd ../..

