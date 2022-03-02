

## Running DART

Once DART is installed and the [DART IPs](https://github.com/fred-framework/dart_ips) are available, the next is step is to test DART with some simple example designs. 

**We are assuming that DART and DART IPs are already setup !!!**

### DART input file format

DART input file format is based on YAML, making it easier to extend and add more features. The YAML format depends on the compilation parameters `PARTITIONING_MODE` for DART.

Be aware the YAML depends on tabulation and spacing. We suggest using an [online YAML parser](https://codebeautify.org/yaml-parser-online) in case of doubts when writing your own YAML for DART.

Finally, all IPs referenced in this section are taken from [DART IPs](https://github.com/fred-framework/dart_ips). So, the information provided to DART about each IP must obviously match the IP design. For example, the user must know the name of IP, the top module name, the number and size of the buffers used, and so on. Please, scan the DART IPs documentation to learn more about the already tested IPs.

#### DART YAML file with partitioning mode ON

In this mode, the user provides a list of hardware IPs and their timing requirements plus the desired number of partitions and DART checks if the desired partition is feasible and implements it.

The following is an example of YAML for DART with partitioning mode ON.

```yaml
dart:   
  num_partitions: 1
  hw_ips:
    - ip_name: "sum_vec"
      top_name: "sum_vec_top"
      timeout: 100000
      slack_time: 10000
      wcet: 100
      buffers: [32768, 32768, 32768]
    - ip_name: "sub_vec"
      top_name: "sub_vec_top"
      timeout: 100000
      slack_time: 10000
      wcet: 101    
      buffers: [32768, 32768, 32768]
skip_ip_synthesis: False
skip_static_synthesis: False
```
At the top of the hierarchy there are system wide definitions like:
 - `skip_ip_synthesis`: True/False
 - `skip_static_synthesis`: True/False
 - `skip_implementation`: True/False

DART has **three phases**: IP synthesis, static part synthesis, and implementation. Sometimes it is useful to redo some of these steps, but not all of them, saving time and CPU usage. Use the above-mentioned attributes to turn on/off the first or the second DART phases. The default value for these attributes is False.

Under `dart` tag, there are the definitions for the FPGA design. In partitioning mode, they are:
 - `num_partitions`: \<unsigned char\>
 - `hw_ips`: a list of hardware IP cores

An IP description, in the partitioning mode, has the following attributes:
 - `ip_name`: \<string\>, following this regular expression `[a-zA-Z_][a-zA-Z0-9]*`
 - `top_name`: \<string\>, following this regular expression `[a-zA-Z_][a-zA-Z0-9]*`
 - `slack_time`:  \<unsigned int\>,representing microseconds
 - `timeout`:  \<unsigned int\>,representing microseconds (used only by FRED)
 - `wcet`: \<unsigned int\>,representing microseconds
 - `buffers`: a list of \<unsigned int\>,representing FRED buffer sizes in bytes

#### DART YAML file with partitioning mode OFF

Unlike the previous mode, the user does not provide timing information for a set IPs in this mode. Thus, DART does not test timing feasibility. In this mode, the user is responsible for providing the FPGA partitioning and the set of IPs for each partition. DART only implements the specified configuration. 

The following is an example of YAML for DART with partitioning mode OFF.

```yaml
dart:
    partitions: 
      - hw_ips:
        - ip_name: "sum_vec"
          top_name: "sum_vec_top"
          timeout: 100000
          buffers: [32768, 32768, 32768]
        - ip_name: "sub_vec"
          top_name: "sub_vec_top"
          timeout: 100000
          buffers: [32768, 32768, 32769]
        debug:
          data_depth: 2048
      - hw_ips:
        - ip_name: "xor_vec"
          top_name: "xor_vec_top"
          timeout: 100000
          buffers: [32768, 32768, 32770]
    static_top_module: static_top
    static_dcp_file: ./dcp/static.dcp
skip_ip_synthesis: False
skip_static_synthesis: False
```

Similar to the previous mode, at the top of the hierarchy there are system-wide definitions. Under `dart` tag, the definitions for the FPGA design are slightly different because the user also needs to define the FPGA partitioning. So, under `dart` tag there is a list of `partitions` and each partition has a list of `hw_ips` and an optional `debug` attribute. An IP description is the same as in the previous mode, except that the attributes `slack_time` and `wcet` are not present. The `debug` attribute enables automatic insertion of [Xilinx Integrated Logic Analyzer (ILA) IPs](https://www.xilinx.com/products/intellectual-property/ila.html) to debug the FPGA partition. This attribute has sub-attributes such as `data_depth`, which defines the ILA internal buffer size. The `debug` attribute is [not supported in the partitioning mode](https://github.com/fred-framework/dart/issues/9).

This current version of DART does not support [a user-defined static part](https://github.com/fred-framework/dart/issues/11). Thus, `static_top_module` and `static_dcp_file` are ignored. This is left for future work.

### Example of a DART design with the `memcpy` IP

This is an image of the block design automatically generated by DART with one reconfigurable region and one `memcpy` IP.

This is the input YAML for DART assuming partitioning mode OFF and FPGA=pynq. This file specifies a single FPGA partition with a single IP code. DART will generate a Vivado design such as the one presented in the figure below.

```yaml
dart:
    partitions: 
      - hw_ips:
        - ip_name: "memcpy"
          top_name: "memcpy_top"
          timeout: 100000
          buffers: [32768, 32768]
```

![DART block design with 1 RR](docs/images/FRED-static-1rr.png)


The following report includes one memcpy IP and it considers the board PYNQ-Z1 (xc7z020clg400-1). 

|          Site Type         | Used | Fixed | Available | Util% |
|----------------------------|------|-------|-----------|-------|
| Slice LUTs                 | 1983 |     0 |     53200 |  3.73 |
|   LUT as Logic             | 1869 |     0 |     53200 |  3.51 |
|   LUT as Memory            |  114 |     0 |     17400 |  0.66 |
|     LUT as Distributed RAM |   10 |     0 |           |       |
|     LUT as Shift Register  |  104 |     0 |           |       |
| Slice Registers            | 3040 |     0 |    106400 |  2.86 |
|   Register as Flip Flop    | 3040 |     0 |    106400 |  2.86 |
|   Register as Latch        |    0 |     0 |    106400 |  0.00 |
| F7 Muxes                   |    0 |     0 |     26600 |  0.00 |
| F8 Muxes                   |    0 |     0 |     13300 |  0.00 |

#### DART/FRED Interface

DART outputs consists of a set of bitstreams and FRED runtime configuration files. This section shows the DART outputs assuming a design with a single `memcpy` IP. Under the generated DART design directory, there is a `fred` directory with the following structure:

```bash
$ cd fred
$ tree
.
├── arch.csv
├── hw_tasks.csv
├── static.dts
├── dart_fred
    └── bits
        ├── p0
        │   └── memcpy_s0.bin
        └── static.bin
```

The FRED configuration files are `arch.csv` and `hw_tasks.csv`, and the next blocks of code show an example of both files:

```bash
$ cat arch.csv 
# FRED Architectural description file. 
# Warning: This file must match synthesized hardware! 
 
# Each line defines a partition, syntax: 
# <partition name>, <num slots> 
 
# example: 
# "ex_partition, 3" 
# defines a partion named "ex_partition" containing 3 slots
p0, 1
```

The example above shows that the generated design has a single partition called `p0` and it has a single slot (i.e. IP) assigned to it. In case of multiple partitions, each partition is reported in a new line of the CSV file. Although FRED supports multiple slots per partition, currently [DART supports only a single slot per partition](https://github.com/fred-framework/dart/issues/2). So, in this case, DART will always have the second value of this CSV equal to 1.


```bash
$ cat hw_tasks.csv
# FRED hw-tasks description file. 
# Warning: This file must match synthesized hardware! 

# Each line defines a HW-Tasks: 
# <name>, <hw_id>, <partition>, <bistream_path>, <buff_0_size>, ... <buff_7_size> 
# Note: the association between a hw-task and its partition 
# it's defined during the synthesis flow! Here is specified only 
# to guess the number of bistreams and their length. 

# example: 
# "ex_hw_task, 64, ex_partition, bits, 1024, 1024, 1024" 
# defines a hw-task named "ex_hw_task", with id 64, allocated on a 
# partition named "ex_partition", whose bitstreams are located in 
# the "/bits" folder, and uses three input/output buffers of size 1024 bytes. 
 
memcpy, 100, 1000, p0, dart_fred/bits, 32768, 32768
```

The CSV file presented above shows the corresponding `hw_tasks.csv` file for the same example design. It shows the name of the IP, its FRED Hw ID, its timeout, the partition it is assigned, the bitstream location, and the sizes of the input and output buffers. If the design has multiple IPs, each one appears in a different line. DART assumes the following conventions:
 - a partition name *p<int>* for every partition;
 - the FRED Hw ID starts with 100 and it increments for each new IP specified in the design;
 - timeout in us;
 - bitstream location is fixed, i.e. it is always *dart_fred/bits*;
 - the number and sizes of the buffers correspond to the information provided in the YAML file.

Finally, the `static.dts` file represents the devicetree to be included in the Linux OS so that the new partitions are recognized by Linux. For this example with one partition, the devicetree  file would be like this, depending on the target board:

```
	amba {

		slot_p0_s0@43c00000 {
			compatible = "generic-uio";
			reg = <0x43c00000 0x10000>;
			interrupt-parent = <0x4>;
			interrupts = <0x0 0x1d 0x4>;
		};

		pr_decoupler_p0_s0@43c10000 {
			compatible = "generic-uio";
			reg = <0x43c10000 0x10000>;
		};
	};
```

### Example with two partitions and FRED

In this example we show how to run DART with two partitions where each partition has only one slot. Two IPs from *DART IPs* are used: `sum_vec` and `sub_vec`. This step-by-step tutorial assumes partitioning mode OFF and FPGA=pynq. A [Pynq board](https://digilent.com/reference/programmable-logic/pynq-z1/start) is required to run the example in the FPGA. We are also assuming that the FRED server is already installed in the FPGA.

#### Running DART

Let's follow these steps to create a DART design integrated with FRED.


```bash
$ mkdir -p ~/2ips/dart
$ cd ~/2ips
$ nano 2ips.yaml
```

Write the YAML file with the following content, meaning that the `sum_vec` IP is mapped to partition 0 and the `sub_vec` IP is mapped to partition 1.

```yaml
dart:
    partitions: 
      - hw_ips:
        - ip_name: "sum_vec"
          top_name: "sum_vec_top"
          timeout: 100000
          buffers: [32768, 32768, 32768]
      - hw_ips:
        - ip_name: "sub_vec"
          top_name: "sub_vec_top"
          timeout: 100000
          buffers: [32768, 32768, 32768]
```

Now let's run DART:

```bash
$ cd ~/2ips/dart
$ dart ../2ips.yaml
```

The execution time is about 10 minutes, depending on the computer. The expected output is:

```
...
INFO: [Common 17-206] Exiting Vivado at Sun Jul 11 10:07:13 2021...
PR_TOOL: creating FRED files 
PR_TOOL: destruction of PR_tool
```

Now let's explore the generated design. The part that matters for FRED integration is under the `fred` folder. You should see something like this structure:

```bash
$ cd fred
$ tree
.
├── arch.csv
├── hw_tasks.csv
├── static.dts
├── dart_fred
    └── bits
        ├── p0
        │   └── sum_vec_s0.bin
        ├── p1
        │   └── sub_vec_s0.bin
        └── static.bin
```

This next file tells us that the generated design has two partitions p0 and p1, each one with a single slot.

```bash
$ cd ~/2ips/dart/fred
$ cat arch.csv 
p0, 1
p1, 1
```

This file maps the IPs to their partitions. So, for instance, 
the IP `sum_vec` has FRED ID `100` and it is assigned to partition `p0`.

```bash
$ cat hw_tasks.csv
...
sum_vec, 100, 1000, p0, dart_fred/bits, 32768, 32768, 32768 
sub_vec, 101, 1000, p1, dart_fred/bits, 32768, 32768, 32768
```

For learning purposes, it's recommended to explore the generated Vivado design located in the **static_hw** directory. 

Finally, let's package the resulting design so that later we can import it into FRED runtime:

```bash
$ cd ~/2ips/dart/fred
$ tar czf ../fred.tar.gz .
```

The next step would be to setup an adequate Linux image with FRED Framwork to run the designs generated by DART. For this purpose, please refer to [FRED Runtime documentation](https://fred-framework-docs.readthedocs.io/en/latest/docs/03_runtime/index.html) for further instructions. A ready to use Linux image is available `here <>`_ if you prefer to skip the Linux image generation and FRED Framework compilation processes.

Now, assuming your are already running a FRED compatible Linux distribution, let's import the DART design to the FPGA board and start fred-server by running the following commands in the terminal:

```bash
$ update_hw <user> <ip> <path>
$ load_hw
$ fred-server &
fred_sys: building hw-tasks
fred_sys: pars: parsing file: /opt/fredsys/hw_tasks.csv
buff: buffer mapped at addresses: 0xffffa65f1000, length:1942168 
fred_sys: loaded slot 0 bitstream for hw-task sum_vec, size: 1942168
fred_sys: creating data buffer 0 of size 32768 for HW-task sum_vec
fred_sys: creating data buffer 1 of size 32768 for HW-task sum_vec
fred_sys: creating data buffer 2 of size 32768 for HW-task sum_vec
```

The use of `update_hw` and `load_hw` is documented [here](https://github.com/fred-framework/meta-fred#updating-the-hardware-desgin). The parameters `update_hw <user> <ip> <path>` refer to the username, IP address, and full path to the `fred.tar.gz` file.

In another terminal of the board, run:

```
$ sum-vec
sum-vec 
 starting vector sum 
fred_lib: connected to fred server!
buff: buffer mapped at addresses: 0xffff89bc1000, length:32768 
buff: buffer mapped at addresses: 0xffff89a15000, length:32768 
buff: buffer mapped at addresses: 0xffff89a0d000, length:32768 
Match!
Content of A[0:9]:
[ 0 0 0 0 0 0 0 0 0 0 ] 
Content of B[0:9]:
[ 1 1 1 1 1 1 1 1 1 1 ] 
Content of C[0:9]:
[ 1 1 1 1 1 1 1 1 1 1 ] 
Fred finished
```

This example [application](https://github.com/fred-framework/meta-fred/tree/main/recipes-example/sum-vec) issues an FPGA offloading to perform the sum of two vectors. The message **Match!** indicates that the test was successful. Otherwise, the message **Mismatch!** will appear.

Every time the hardware design is changed, it is important to reboot the board before loading the new device tree and fred-server. 
