# DART

## Dependencies

DART uses Gurobi 8.1, Vivado 2018.3, and C++ 2017.

DART requires the following environment variables:

 - XILINX_VIVADO: points to the Vivado directory;
 - DART_HOME: the source dir for DART. It must be gurobi version 8.1.1;
 - DART_IP_PATH: the directory where the IPs are stores;
 - GUROBI_HOME: the home dir for gurobi installation;
 - GRB_LICENSE_FILE: Gurobi's license file.

Moreover, *vivado* must be in the PATH.

If you dont have Gurobi, download and request a license [here](https://www.gurobi.com/downloads/).

## DART Modes

DART can be configured with four different ways:

 - flora_with_partitioning: floorplanner with partitioning
 - flora_without_partitioning: only floorplanner without partitioning
 - pr_tool_with_part: run the PR flow with including floorplanning and partitioning'
 - pr_tool_without_part: run the PR flow including only the floorplanning 


**Biruk, please detail each of them !!!**

## Supported FPGAs

The supported FPGAs are:

 - [PYNQ](https://www.xilinx.com/support/university/boards-portfolio/xup-boards/XUPPYNQ.html): with device XC7Z020-1CLG400C;
 - [Biruk, put here the board name and link]: with the device xc7z010clg400-1.

## Compiling

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

## Input File

Biruk, write here the input format and an example for each mode.
I suggest that each example is also available at the dir 'inputs'

### Input File for Mode ABCDEF

### Input File for Mode ABCDEFG

### Input File for Mode ABCDEFGH

### Input File for Mode ABCDEFGHI

## Example IPs

For the user convenience, there are ready-to-use IPs that can be downloaded from [here]()
for testing purposes.

Biruk, please point to the examples IPs

## Running DART

## Running in the FPGA

## Analysing the Outputs

## Feedback

## Papers

Biruk, please point to the main papers related to DART/FLORA/FRED

## Acknoledgments

put here the list of the brains behind it, contributors, funding projects.

