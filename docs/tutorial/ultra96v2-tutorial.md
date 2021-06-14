# DART-FRED tutorial

This document describes a step-by-step tutorial on how to use *DART & Fred* on Zynq UltraScale+ MPSoCs. More specifically, this tutorial targets the Ultra96v2 board as the reference platform.

The tutorial covers an FPGA-based design flow where DART does the design-time optimization based on real-time
constraints and the bitstream generations, while Fred manages the dinamic partial reconfiguration in runtime, also respecting the same real-time constraints used for DART. At the end of the tutorial, a real-time hw/sw real-time application is executed in the board. The figure presented next illustrates the design flow.

![DART-Fred design flow](docs/tutorial/imgs/design-flow.png)

*TO BE DONE: decide the board to be used*

## Software Requirements

This tutorial is assuming Vivado 2019.2. The use of other versions of Vivado may require changes in the scripts provided for the tutorial.


```
Version 0.3 ZynqUS+ / Ultra96v2
```

## Background Requirements

To execute this tutorial, it is highly advisable to read first the theoreatical and installation aspects of [DART](docs/index.md) and [Fred](https://gitlab.retis.sssup.it/m.pagani/fred-docs). 
We are assuming that the reader is familiar with the concepts presented in this document.

For a more detailed description of the DART and FRED, please refer to their papers: 

- [Biruk Seyoum, Alessandro Biondi, Marco Pagani, and Giorgio Buttazzo, "Automating the Design Flow under Dynamic Partial Reconfiguration for Hardware-Software Co-design in FPGA SoC", In Symposium on Applied Computing (SAC), 2021.](https://retis.sssup.it/~a.biondi/papers/DART.pdf) 
- [Alessandro Biondi, Björn B. Brandenburg and Alexander Wieder, "A Blocking Bound for Nested FIFO Spin Locks", In IEEE Real-Time Systems Symposium (RTSS), 2016.](https://retis.sssup.it/~a.biondi/papers/FRED_RTSS16.pdf).


It is also recommended to have familiarity with the following Xilinx tools and design flows:

*to be discussed: what is the minimal background on vivado we need to execute this tutorial ?!?!*

- *Vivado Design Suite Partial Reconfiguration Guide - UG909*.
- what else?

## Demo image
This tutorial describes how to build a Fred-Linux demo application on the Ultra96v2 board. You can download a prebuild SD-card image including all sources used in this tutorial from [here](https://owncloud.retis.sssup.it/index.php/s/BF41kgz6rbddBUH).

If you just want to try Fred-Linux on the Ultra96v2 board, copy the demo image on a 16 GB or larger SD card:

```console
$ unzip fred-ultra96v2-demo.zip
$ sudo dd if=fred-ultra96v2-demo.img of=/dev/<sd-card> bs=8M conv=fsync
```

Then, follow these [instructions](#starting-the-demo) for starting the demo.


## Project directory structure

This is the proposed directory structure for this tutorial:

```
tutorial/
├── dart.csv  --> dart input file
├── dart      --> where the dart desgin project will be generated
├── dart_ips  --> the source code of the IPs used in this tutorial
└── ips       --> the list of IPs after compilation
```

## DART inputs

DART requires two sets of inputs: the first one is a set of IP cores designed incompliance with DART design principles (see the document about [IP design](xxxx)); the second one is a CSV file with the timing requiremts for each DART IP used in the design. The next sections detail each set of DART inputs.

### DART IPs

[DART IPs](https://gitlab.retis.sssup.it/a.amory/dart_ips) is a repository with a set of ready-to-use IPs for DART. In this tutorial we are using the following five IPs:

- *sum_vec*: It performs vector sum of two vectors of 1024 integers;
- *mul_vec*: It performs vector multiplication of two vectors of 1024 integers;
- *sub_vec*: It performs vector subtraction of two vectors of 1024 integers;
- *nor_vec*: It performs vector bitwise NOR of two vectors of 1024 integers;
- *xor_vec*: It performs vector bitwise XOR of two vectors of 1024 integers.

To have access to the source code of these IPs, please execute the following command inside the `tutorial` directory:

```console
$ git clone https://gitlab.retis.santannapisa.it/a.amory/dart_ips.git dart_ips
```

For the next step, execute the following commands to compile the IPs to be used:

```console
$ cd dart_ips/ips/sum_vec/hw
$ vivado_hls build.tcl
```

The generated IP is located in the directory `dart_ips/ips/sum_vec/hw/sum_vec/solution_0/impl/ip`. This is the directory relevant for DART.

Repeat the same commands for the other four IPs mentioned above. Once all the IPs were compiled,
now we have to set the environment variable **DART_IP_PATH** and set the symbolic links to the IPs.

```console
$ export DART_IP_PATH=<absolute-path>/tutorial/ips
$ cd $DART_IP_PATH
$ ln -s ../dart_ips/ips/sum_vec/hw/sum_vec/solution_0/impl/ip sum_vec
$ ln -s ../dart_ips/ips/mul_vec/hw/mul_vec/solution_0/impl/ip mul_vec
$ ln -s ../dart_ips/ips/sub_vec/hw/sub_vec/solution_0/impl/ip sub_vec
$ ln -s ../dart_ips/ips/nor_vec/hw/nor_vec/solution_0/impl/ip nor_vec
$ ln -s ../dart_ips/ips/xor_vec/hw/xor_vec/solution_0/impl/ip xor_vec
```

The resulting directory structure under the `ips` must be:

```console
$ tree -dl -L 2
.
├── mul_vec -> ../dart_ips/ips/mul_vec/hw/mul_vec/solution_0/impl/ip
│   ├── bd
│   ├── constraints
│   ├── doc
│   ├── drivers
│   ├── example
│   ├── hdl
│   ├── misc
│   ├── subcore
│   └── xgui
├── nor_vec -> ../dart_ips/ips/nor_vec/hw/nor_vec/solution_0/impl/ip
│   ├── bd
│   ├── constraints
│   ├── doc
│   ├── drivers
│   ├── example
│   ├── hdl
│   ├── misc
│   ├── subcore
│   └── xgui
├── sub_vec -> ../dart_ips/ips/sub_vec/hw/sub_vec/solution_0/impl/ip
│   ├── bd
│   ├── constraints
│   ├── doc
│   ├── drivers
│   ├── example
│   ├── hdl
│   ├── misc
│   ├── subcore
│   └── xgui
├── sum_vec -> ../dart_ips/ips/sum_vec/hw/sum_vec/solution_0/impl/ip
│   ├── bd
│   ├── constraints
│   ├── doc
│   ├── drivers
│   ├── example
│   ├── hdl
│   ├── misc
│   ├── subcore
│   └── xgui
└── xor_vec -> ../dart_ips/ips/xor_vec/hw/xor_vec/solution_0/impl/ip
    ├── bd
    ├── constraints
    ├── doc
    ├── drivers
    ├── example
    ├── hdl
    ├── misc
    ├── subcore
    └── xgui
```


### DART input CSV file

The `dart.csv` file should have the following content:

```
300,1000,sum_vec,sum_vec_top
300,1000,mul_vec,mul_vec_top
300,1000,sub_vec,sub_vec_top
300,1000,nor_vec,nor_vec_top
200,1000,xor_vec,xor_vec_top
```

The runtime and deadline information, respectively the 1st and the 2nd information in each line,
were chosen such that when DART is run with two partitions, it will assign three IPs to the first partition and two IPs for the second partition. If DART is run with only one partition, 
it is expected to have an unfeasible solution considering these runtime and deadline information.

## Executing DART

In this section we are assuming that DART is already installed in the desgin computer. If this is not the case, please refer to the [DART installation procedure](docs/install/index.md).

Execute the following commands to run DART. 

```console
$ cd tutorial/dart
$ run_pr_tool_with_part_pynq 2 ../dart.csv
```

## Analysing the results from DART

This section describes the main outputs generated by DART like the generated design and the files to be used by Fred.

### The generated design

Show what DART built.

![xilinx bd](docs/tutorial/imgs/block-diagram.png)


![slots](docs/tutorial/imgs/slots.png)

The physical layout of the slots can be defined at this design stage using a set of `pblocks`. For more details, you can refer to [this](https://xilinx-wiki.atlassian.net/wiki/spaces/A/pages/18841851/XAPP1231+-+Partial+Reconfiguration+of+a+Hardware+Accelerator+with+Vivado+Design+Suite) tutorial. While floorplanning the design, remember that the FRED frameworks require that all slots of a partition must contain the same amount of resources.

![pblocks](docs/tutorial/imgs/slots-blocks-ultra96v2.png)

It is worth noting that the floorplanning (i.e., the mapping between a module and a specific geometrical region of the FPGA) of the slots (i.e., the placeholder accelerators) is defined through a `xdc` constraint file describing the geometry of the slots.


### DART outputs

Show the files in the fred folder.


The HW-tasks are allocated in this way:

Partition       | HW-tasks
---             | ---
P0 (2 slots)    | V. Mul, V. Add, V. Sub
P1 (1 slots)    | V. XOR, V. NOR


In this section we are assuming that the OS, the Fred-server and its kernel modules were already installed in the target FPGA. If this is not the case, please refer to the [Fred documentation](blabla).

The Fred-Linux runtime requires a set of configuration and data files, including:

- Two comma-separated values files named `arch.csv` and `hw-tasks.csv` describing the system layout.
- The bitstreams generated using the [partial reconfiguration flow](#partial-reconfiguration-flow);
- Optionally, a device tree overlay describing the reconfigurable portion of the static design.


```text
fred
├── arch.csv
├── hw_tasks.csv
└── bits
    ├── static.bin
    ├── static.dtbo
    ├── p0
    │   ├── <hw-task_0>_s0.bin
    │   ├── <hw-task_0>_s1.bin
    │   ├── <hw-task_1>_s0.bin
    │   ├── <hw-task_1>_s1.bin
    └── p1
        ├── <hw-task_2>_s0.bin
        └── <hw-task_3>_s0.bin
```


### Layout files
The layout files describe the system support design layout and the HW-tasks available on the system. The fred-server reads them during the initialization phase to get to know the underlying hardware design. Both files are written in the comma-separated value format. Lines beginning with the `#` symbol are interpreted as comments.

The `arch.csv` file describes the layout of the hardware support design in terms of partition and slots according to the following example:

```text
# Fred-Linux static desing layout description file.
# Warning: This file must match synthesized hardware!

# Each line defines a partition with the following syntax: 
# <partition name>, <num slots>

# example:
# "example-partition, 3"
# defines a partion named "example-partition" containing 3 slots.

p0_arithmetic,  2
p1_bitwise,     1
```

The `hw-tasks.csv` file containing a description of the HW-tasks available on the system according to the following example:

```text
# Fred-Linux hw-tasks description file.
# Warning: This file must match synthesized hardware!
#
# Each line defines a HW-Tasks with the following syntax:
# <name>, <id>, <timeout_ms>, <partition>, <bits_path>, <buff_0_size>, ... <buff_7_size>
#
# For each hw-task, the field:
# - <name> is the name of the hw-task and must match the name of the bitstream files;
# - <id> is the hw-id of the hw-task;
# - <timeout_ms> defines an execution timeout for the hw-task in milliseconds;
# - <partition> is the name of the partition to which it belongs;
# - <bits_path> define the relative path of the bitstreams directory for the hw-task;
# - <buff_0_size>, ... <buff_7_size> define the number and sizes fo the data buffers.
#
# Example:
# "ex-hw-task, 64, ex-partition, bitstreams, 100, 1024, 1024, 1024"
# defines a hw-task named "ex-hw-task"
# - having id 64,
# - belonging to a partition named "ex-partition",
# - whose bitstreams are located in the "./bitstreams" directory,
# - having an execution timeout of 100 ms,
# - using three input/output buffers of size 1024 bytes each.

sum,    100,    100,    p0_arithmetic,  bitstreams, 4096, 4096, 4096
sub,    101,    100,    p0_arithmetic,  bitstreams, 4096, 4096, 4096
mul,    102,    100,    p0_arithmetic,  bitstreams, 4096, 4096, 4096
xor,    200,    100,    p1_bitwise,     bitstreams, 4096, 4096, 4096
nor,    201,    100,    p1_bitwise,     bitstreams, 4096, 4096, 4096
```

### Bistreams
The bitstreams generated using the [partial reconfiguration flow](#partial-reconfiguration-flow) must be renamed and copied to the `fredsys/<bitstreams-dir-name>` directory according to a specific convention. First, rename all bitstreams generated by the partial reconfiguration flow according to this convention: `<hw_task_name>_s<N>.bin` where `hw_task_name` is the name of the HW-task and `N` is the name of the slot targeted by that specific bitstream. E.g., `sum_s1.bin` denotes the bitstream that implements the `sum` HW-task on the slot number `1` of its partition. Then, for each partition, create a directory having the same name and move into that directory all bitstreams of the HW-tasks belonging to that partition. For instance, the bitstream folder for the demo design must be organized as follows:

```text
bitstreams/
├── p0_arithmetic
│   ├── mul_s0.bin
│   ├── mul_s1.bin
│   ├── sub_s0.bin
│   ├── sub_s1.bin
│   ├── sum_s0.bin
│   └── sum_s1.bin
├── p1_bitwise
│   ├── nor_s0.bin
│   └── xor_s0.bin
├── static.bin
├── static.dtbo
└── static.dts

```

### Device tree overlay
In order to make the slots (placeholders for HW-tasks) and the decouplers visible to the Fred-Linux runtime, it is necessary to patch the device tree with a device tree overlay. The overlay must describe all slots, decouplers, and eventually other static IPs present in the support design. For instance, the overlay for the demo design can be written as follows:

```text
/dts-v1/;
/plugin/;
/ {
	/* FRED static support design */
	fragment@0 {
		target = <&fpga_full>;
		overlay0: __overlay__ {
			#address-cells = <2>;
			#size-cells = <2>;
			firmware-name = "static.bin";
			resets = <&zynqmp_reset 116>;
		};
	};

	/* Disable PYNQ base */
	fragment@1 {
		target-path = "/amba/fabric@A0000000";
		overlay1: __overlay__ {
			status = "disabled";
		};
	};
	
	/* PL base configuration */
	fragment@2 {
		target = <&amba>;
		overlay2: __overlay__ {
			afi0: afi0 {
				compatible = "xlnx,afi-fpga";
				config-afi = < 0 0>, <1 0>, <2 0>, <3 0>, <4 0>, <5 0>, <6 0>, <7 0>, <8 0>, <9 0>, <10 0>, <11 0>, <12 0>, <13 0>, <14 0xa00>, <15 0x000>;
			};
			clocking0: clocking0 {
				#clock-cells = <0>;
				assigned-clock-rates = <100000000>;
				assigned-clocks = <&zynqmp_clk 71>;
				clock-output-names = "fabric_clk";
				clocks = <&zynqmp_clk 71>;
				compatible = "xlnx,fclk";
			};
		};
	};

	/* FRED slots layout */
	fragment@3 {
		target = <&amba>;
		overlay3: __overlay__ {
			#address-cells = <2>;
			#size-cells = <2>;
			pr_decoupler_p0_s0@a0030000 {
				clock-names = "aclk";
				clocks = <&zynqmp_clk 71>;
				compatible = "generic-uio";
				reg = <0x0 0xa0030000 0x0 0x10000>;
			};
			pr_decoupler_p0_s1@a0040000 {
				clock-names = "aclk";
				clocks = <&zynqmp_clk 71>;
				compatible = "generic-uio";
				reg = <0x0 0xa0040000 0x0 0x10000>;
			};
			pr_decoupler_p1_s0@a0050000 {
				clock-names = "aclk";
				clocks = <&zynqmp_clk 71>;
				compatible = "generic-uio";
				reg = <0x0 0xa0050000 0x0 0x10000>;
			};
			slot_p0_s0@a0000000 {
				clock-names = "ap_clk";
				clocks = <&zynqmp_clk 71>;
				compatible = "generic-uio";
				reg = <0x0 0xa0000000 0x0 0x10000>;
				xlnx,s-axi-ctrl-bus-addr-width = <0x8>;
				xlnx,s-axi-ctrl-bus-data-width = <0x20>;
				interrupt-parent = <&gic>;
				interrupts = <0 89 4>;
			};
			slot_p0_s1@a0020000 {
				clock-names = "ap_clk";
				clocks = <&zynqmp_clk 71>;
				compatible = "generic-uio";
				reg = <0x0 0xa0020000 0x0 0x10000>;
				xlnx,s-axi-ctrl-bus-addr-width = <0x8>;
				xlnx,s-axi-ctrl-bus-data-width = <0x20>;
				interrupt-parent = <&gic>;
				interrupts = <0 90 4>;
			};
			slot_p1_s0@a0010000 {
				clock-names = "ap_clk";
				clocks = <&zynqmp_clk 71>;
				compatible = "generic-uio";
				reg = <0x0 0xa0010000 0x0 0x10000>;
				xlnx,s-axi-ctrl-bus-addr-width = <0x8>;
				xlnx,s-axi-ctrl-bus-data-width = <0x20>;
				interrupt-parent = <&gic>;
				interrupts = <0 91 4>;
			};
		};
	};
	
};
```

The overlay can then be compiled using the device tree compiler on the host computer.

```console
$ dtc -O dtb -o static.dtbo -b 0 -@ static.dts
```

Then, the compiled overlay must be copied in the `fredsys/<bitstreams-dir-name>` directory alongside the static bitstream of the support design generated by the partial reconfiguration flow:

```text
bitstreams/
├── p0_arithmetic
│   └── <...>
├── p1_bitwise
│   └── <...>
├── static.bin
├── static.dtbo
└── static.dts

```

## Setting up Fred runtime

By default, these files are located in the `/fredsys` directory and organized with the following structure:

The path of the `fredsys` directory can be customized at compile time by changing the `FRED_PATH` macro in the source  file `parameters.h`of the fred-linux server. The following sections describe how to customize the single components within the `fredsys` directory:


## Writing Fred-compatible software

In this section we are assuming that you are already familiar with [Fred client API](blabla).

The demo design presented in this tutorial includes a test application comprising five SW-tasks. Each SW-task call one of the HW-task included in the demo set and performs the same operation in software comparing the result. The test application can be downloaded and compiled with the following commands:

```console
$ ssh fpga-username@fpga-ip-address
$ cd /opt/fredsys/build
$ git clone https://github.com/SSSA-ampere/fred-linux-test-client.git
$ cd fred-linux-test-client
$ make
```

The build process will generate a static library for the client support library and the executable of the demo application named `fred-test-cli`.


## Running Fred

Before starting the fred-server and the demo client application, it is necessary to:

- Load the two kernel support modules;
- Reconfigure the PL with the static bitstream;
- Apply the device tree overlay.

These operations can be performed with the following commands to load the kernel modules:

```console
# cd /opt/fredsys
# insmod ./build/fred-kmods/fred_buffctl/fred-buffctl.ko
# insmod ./build/fred-kmods/fpga_mgr_zynqmp_drv/zynqmp-fpga-fmod.ko
```

and then reconfigure the static design and apply its device tree overaly:

```console
# mkdir -p /lib/firmware
# cp ./bitstreams/static.bin /lib/firmware/
# cp ./bitstreams/static.dtbo /lib/firmware/
# mkdir -p /sys/kernel/config/device-tree/overlays/fred-static
# echo 0 > /sys/class/fpga_manager/fpga0/flags
# echo -n "static.dtbo" > /sys/kernel/config/device-tree/overlays/fred-static/path
```

Alternatively, the [demo image](demo-image) contains a script named `fred-set.sh` that performs these operations.

```console
# cd /opt/fredsys
# . fred-set.sh
```

Once the enviroiment has been set, the fred-server and the test application can executed.

```console
# cd /opt/fredsys/build/fred-linux
# ./fred-server &
# cd ../fred-linux-test-client
# ./fred-test-cli
```

When reqired, you can stop the client application with a `Ctrl+c` and check the fred-server execution log in `/opt/fredsys/log.txt`.

## Analysing the results from Fred




### Partial reconfiguration flow
When the support design and the HW-tasks are ready to be synthesized, they can be plugged into the partial reconfiguration (PR) flow for generating the bitstreams. The PR flow is script-based and requires all input sources to be organized in a specific directory structure as described in this [tutorial](https://xilinx-wiki.atlassian.net/wiki/spaces/A/pages/18841851/XAPP1231+-+Partial+Reconfiguration+of+a+Hardware+Accelerator+with+Vivado+Design+Suite).

An example PR flow allocating the demo HW-tasks on top of the demo support design can be downloaded [here](https://owncloud.retis.sssup.it/index.php/s/P9ZKPxzEPKarw4f). 
The PR flow can be customized following these steps:

- Rename the synthesis checkpoint resulting from the [support design synthesis](#pynq-support-design) as `top_synth.dpc` and copy it into the `Synth/Static` directory.

- Copy the top-level wrappers of the slots' placeholder HW-tasks, e.g., `design_1_hw_task_0_0_0.vhd`, etc., into the `Sources/hdl` directory without the black box attribute.

- Copy the design constraint file, e.g., `top.xdc`, and the slots constraints file, e.g., `pblocks.xdc`, into the `Sources/xdc` folder.

Then, for each HW-task, say `my_hw_task`:

- Copy the HLS implementation output into a new `Sources/ip/my_hw_task` directory. Note that the top function name must match the slots name. E.g., for a partition containing two slots (`hw_task_0` and `hw_task_1`), two copies of the same HW-task with different top level functions (`my_hw_task_0` and `my_hw_task_1`) must be provided in two different directories.

- Generate a `my_hw_task_N.prj` (where N is the slot number) project file specifying the HDL / LogiCore sources of the HW-tasks. The `load_prj.py` script can be used to facilitate this process.

Finally, customize the `design.tcl` script accordingly to the changes you made and launch the PR synthesis process with the following command.

```console
$ vivado -mode batch  -source design.tcl
```


## Troubleshooting
Please consider that Fred-Linux is not a self-contained "download and run" piece of software but rather a system support software that relies on a custom hardware design and two custom kernel modules. For this reason, building a fully functional design is a somewhat tricky and time-consuming process. As a general rule, it worth testing all programmable hardware components in isolation before proceeding with the software deployment. In case you encounter some troubles, please check the following steps: 

1. Make sure that the hardware design works correctly. It's highly recommended to (i) design the static part using the reference design as a starting point and (ii) test all Hw-tasks in isolation using a bare-metal testbench. Typical issues with Hw-task are (i) spurious memory accesses, like overwriting memory starting from physical address 0 with disruptive consequences for the OS kernel, (ii) spurious interrupt signalling, and (iii) internal logic stalls.

2. Always check that the device tree has been correctly patched. In particular, always double-check that interrupt numbers and slots and decouplers registers maps match the values specified in the Vivado hardware design.

3. Before starting the fred-server remember to (i) load the kernel modules and (ii) reconfigure the PL with the correct static bitstream.

## Contributors

- John Dow: did nothing very relevant in the project;
- Jane Dow: the true brain behind the project, although her name came in 2nd in the papers;
- ...


## Funding

These tools have been partially developed in the context of the [AMPERE project](<https://ampere-euproject.eu/>).
This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement No 871669.

## References

- **[1]** [Biruk Seyoum, Alessandro Biondi, Marco Pagani, and Giorgio Buttazzo, "Automating the Design Flow under Dynamic Partial Reconfiguration for Hardware-Software Co-design in FPGA SoC", In Symposium on Applied Computing (SAC), 2021.](https://retis.sssup.it/~a.biondi/papers/DART.pdf) 
- **[2]** [Alessandro Biondi, Björn B. Brandenburg and Alexander Wieder, "A Blocking Bound for Nested FIFO Spin Locks", In IEEE Real-Time Systems Symposium (RTSS), 2016.](https://retis.sssup.it/~a.biondi/papers/FRED_RTSS16.pdf).

