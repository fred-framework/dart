
## Dependencies

DART has been tested in Ubuntu 18.4 LTS. It uses Gurobi 8.1, Vivado 2018.3 or 2019.2, and C++ 2017 (GCC v8.4.0). If you dont have Gurobi, download and request an academic license [here](https://www.gurobi.com/downloads/).

## Supported FPGAs

The supported FPGAs are:

 - [PYNQ](https://www.xilinx.com/support/university/boards-portfolio/xup-boards/XUPPYNQ.html): with device XC7Z020-1CLG400C;
 - [Biruk, put here the board name and link]: with the device xc7z010clg400-1.


## Downloading and Compiling


Please follow these steps to compile DART. In this example we are creating the DART executable for the pr tool with support to the Pynq FPGA.

```bash
$ git clone https://repo.retis.sssup.it/pr_tool
$ cd pr_tool
$ make pr_tool_with_part FPGA=PYNQ
```

Go to DART_HOME and run:

```
make <dart_mode> FPGA=<fpga>
```

where *dart_mode* is: flora_with_partitioning, flora_without_partitioning, pr_tool_with_part, or pr_tool_without_part.

where *fpga* is: ZYNQ or PYNQ.

Example:

```
make pr_tool_with_part FPGA=PYNQ
```


Note that DART has several compilation modes, depending on the tool to be compile (pr or flora), the tool mode (with or without partitioning,) and also on the target FPGA. These are all compilation options:


```
$ make 
Please run the make file using the following format

make target FPGA=type_of_FPGA

please use a specific target 
'flora_with_partitioning'  ---> floorplanner with partitioning
'flora_without_partitioning' ---> only floorplanner without partitioning
'pr_tool_with_part' ---> run the PR flow with including floorplanning and partitioning
'pr_tool_without_part' ---> run the PR flow including only the floorplanning
'pr_all' ---> compile both PR flow executables: floorplanning and partitioning; floorplanning only
'flora_all' ---> compile both floorplanner executables: floorplanning and partitioning; floorplanning only
'all_all' ---> compile the four executables: two for flora and two for the PR flow
 
for type of FPGA please use ZYNQ or PYNQ
 
For example make pr_tool_with_part FPGA=PYNQ
```

## Setting Up DART

DART requires the following environment variables:

 - XILINX_VIVADO: Points to the Vivado directory;
 - DART_HOME: The source dir for DART;
 - DART_IP_PATH: The directory where the IPs are stores;
 - GUROBI_HOME: The home dir for gurobi installation;
 - GRB_LICENSE_FILE: Gurobi's license file.

Moreover, *vivado* must be in the PATH. For convenience, it's also recommended to add DART_HOME/bin in the PATH.

Before running DART, we need a set of DART compliant IPs. [DART IPs](https://gitlab.retis.santannapisa.it/a.amory/dart_ips) is a set of ready-to-use IPs in DART designs. Please refer to its documentation for installation and usage procedure.

## Setting Up the FPGA

**LINK HERE TO SOME READY TO USE FRED-ENABLED LINUX IMAGE**


