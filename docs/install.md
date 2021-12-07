
## Dependencies

DART has been tested with:

- Ubuntu 18.4 LTS;
- Gurobi 8.1. If you dont have Gurobi, download and request an academic license [here](https://www.gurobi.com/downloads/). It's known that DART gives compilation version with later versions of Gurobi. So, please, stick with Gurobi 8.1;
- [YAML-CPP](https://github.com/jbeder/yaml-cpp/releases/tag/yaml-cpp-0.7.0) v0.7.0;
- Vivado 2018.3 or later. The most tested and recommended version is 2019.2;
- CMake 3.0 or later;
- It is recommended to use [CCMake](https://askubuntu.com/questions/121797/how-do-i-install-ccmake) for compilation configuration;
- DART is programmed with C++17, tested with GCC v8.4.0.;

## Supported FPGAs

The supported FPGAs are:

 - [Pynq board](https://store.digilentinc.com/pynq-z1-python-productivity-for-zynq-7000-arm-fpga-soc/): with device XC7Z020-1CLG400C;
 - [Zybo board](https://reference.digilentinc.com/programmable-logic/zybo/start): with the device XC7Z010-1CLG400C; 
 - [ZCU102 board](https://www.xilinx.com/products/boards-and-kits/ek-u1-zcu102-g.html): with Zynq UltraScale+ XCZU9EG-2FFVB1156 MPSoC;
 - [Ultra96v2 board](https://www.avnet.com/wps/portal/us/products/new-product-introductions/npi/aes-ultra96-v2/)


## Setting Up DART

DART requires the following environment variables:

 - XILINX_VIVADO: Points to the Vivado directory;
 - DART_HOME: The source dir for DART;
 - DART_IP_PATH: The directory where the IPs are stores;
 - GUROBI_HOME: The home dir for gurobi installation;
 - GRB_LICENSE_FILE: Gurobi's license file.

Moreover, *vivado* must be in the PATH. For convenience, it's also recommended to add DART_HOME/bin in the PATH.

Before running DART, we need a set of DART compliant IPs. [DART IPs](https://gitlab.retis.santannapisa.it/a.amory/dart_ips) is a set of ready-to-use IPs in DART designs. Please refer to its documentation for installation and usage procedure.

## Downloading and Compiling

Please follow these steps to compile DART. The compilation have to be AFTER setting up the environment variables mentioned before. In this example we are creating the DART executable for the `pr` tool with support to the `pynq` FPGA and partitioning mode.

```bash
$ git clone https://repo.retis.sssup.it/pr_tool
$ cd pr_tool
$ mkdir build
$ cd build
$ cmake .. FPGA=pynq PARTITIONING_MODE=ON
$ make -j 8
```

where `FPGA` is: pynq, zynq, us, or us_96. Check `ccmake` to see other configuration parameters.


## Setting Up the FPGA

**LINK HERE TO SOME READY TO USE FRED-ENABLED LINUX IMAGE**


