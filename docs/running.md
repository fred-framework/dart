

## Running DART

Once DART is installed and the [DART IPs](https://gitlab.retis.santannapisa.it/a.amory/dart_ips) are available, the next is step is to test DART with some simple example designs. 

**We are assuming that DART and DART IPs are already setup !!!**

### DART input file format

DART input file format is based on YAML, making it easier to extend and add more features.
The YAML format depends on the compilation parameters `PARTITIONING_MODE` for DART.

Be aware the YAML depends on tabulation and spacing. We suggest using an [online YAML parser](https://codebeautify.org/yaml-parser-online) in case of doubts when writing your own YAML for DART.

Finally, all IPs referenced in this section are taken from [DART IPs](https://gitlab.retis.santannapisa.it/a.amory/dart_ips). So, the information provided to DART about each IP must obviously match the IP design. For example, the user must know the name of IP, the top module name, the number and size of the buffers used, and so on. Please, scan the DART IPs documentation to learn more about the already tested IPs.

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

DART has **three phases**: IP synthesis, static part synthesis, and implementation. Sometimes it is useful to redo some of these steps, but not all of them, saving time and CPU usage. Use the above-mentioned attributes to turn on/off the first or the second DART phases. When these attributes are not defined, the default is used, i.e. the attributes are on.

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

Similar to the previous mode, at the top of the hierarchy there are system-wide definitions. Under `dart` tag, the definitions for the FPGA design are slightly different because the user also needs to define the FPGA partitioning. So, under `dart` tag there is a list of `partitions` and each partition has a list of `hw_ips` and an optional `debug` attribute. An IP description is the same as in the previous mode, except that the attributes `slack_time` and `wcet` are not present. The `debug` attribute enables automatic insertion of [Xilinx Integrated Logic Analyzer (ILA) IPs](https://www.xilinx.com/products/intellectual-property/ila.html) to debug the FPGA partition. This attribute has sub-attributes such as `data_depth`, which defines the ILA internal buffer size. The `debug` attribute is not supported in the partitioning mode.

This current version of DART does not support a user-defined static part. Thus, `static_top_module` and `static_dcp_file` are ignored. This is left for future work.

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

<!---
The following report includes three memcpy IPs and it considers the board PYNQ-Z1 
(xc7z020clg400-1). 

![DART block design with 3 RRs](docs/images/FRED-static-3rr.png)


|          Site Type         | Used | Fixed | Available | Util% |
|----------------------------|-----:|------:|----------:|------:|
| Slice LUTs*                | 6009 |     0 |     53200 | 11.30 |
|   LUT as Logic             | 5548 |     0 |     53200 | 10.43 |
|   LUT as Memory            |  461 |     0 |     17400 |  2.65 |
|     LUT as Distributed RAM |   10 |     0 |           |       |
|     LUT as Shift Register  |  451 |     0 |           |       |
| Slice Registers            | 7986 |     0 |    106400 |  7.51 |
|   Register as Flip Flop    | 7986 |     0 |    106400 |  7.51 |
|   Register as Latch        |    0 |     0 |    106400 |  0.00 |
| F7 Muxes                   |    0 |     0 |     26600 |  0.00 |
| F8 Muxes                   |    0 |     0 |     13300 |  0.00 |
-->

#### DART/FRED Interface

DART automates only the hardware design of the hardware/software system based on Zynq.
As seen before, it generates two sets of outputs: the bitstreams and the FRED configuration files.

This section shows the DART outputs assuming a design with a single `memcpy` IP.
Under the generated DART design directory, there is a `fred` directory with the following structure:

```bash
$ cd fred
$ tree
.
├── arch.csv
├── hw_tasks.csv
├── dart_dev_tree.dts
├── dart_fred
    └── bits
        ├── p0
        │   └── memcpy_s0.bin
        └── static.bin
```

The FRED configuration files are `arch.csv` and `hw_tasks.csv`, and 
the next blocks of code show an example of both files:

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

The example above shows that the generated design has a single partition called `p0` and it has a single slot (i.e. IP) assigned to it. In case of multiple partitions, each partition is reported in a new line of the CSV file. Although FRED supports multiple slots per partition, currently **DART supports only a single slot per partition**. So, in this case, DART will always have the second value of this CSV equal to 1.


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
 
 memcpy, 100, p0, dart_fred/bits, 32768, 32768
```

The CSV file presented above shows the corresponding `hw_tasks.csv` file for the same example design. It shows the name of the IP, its FRED Hw ID, the partition it is assigned, the bitstream location, and the sizes of the input and output buffers. If the design has multiple IPs, each one appears in a different line. DART assumes the following conventions:
 - a partition name *p<int>* for every partition;
 - the FRED Hw ID starts with 100 and it increments for each new IP specified in the design;
 - bitstream location is fixed, i.e. it is always *dart_fred/bits*;
 - the number and sizes of the buffers correspond to the information provided in the YAML file.

Finally, the `dart_dev_tree.dts` file represents the device tree to be included in the Linux OS so that the new partitions are recognized by Linux. For this example with one partition, this file must be like this:

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
};
```

### Example with two partitions and FRED

In this example we show how to run DART with two partitions where each partition has only one slot. Two IPs from *DART IPs* are used: `sub_vec` and `sum_vec`. This step-by-step tutorial assumes partitioning mode OFF and FPGA=pynq. A [Pynq board](https://digilent.com/reference/programmable-logic/pynq-z1/start) is required to run the example in the FPGA. We are also assuming that the FRED server is already installed in the FPGA.

#### Running DART

Let's follow these steps to create a DART design integrated with FRED.


```bash
$ mkdir -p ~/2ips/dart
cd ~/2ips
nano 2ips.yaml
```

Write the YAML file with the following content, meaning that the `sub_vec` IP is mapped to partition 0 and the `sum_vec` IP is mapped to partition 1.

```yaml
dart:
    partitions: 
      - hw_ips:
        - ip_name: "sub_vec"
          top_name: "sub_vec_top"
          timeout: 100000
          buffers: [32768, 32768, 32768]
    partitions: 
      - hw_ips:
        - ip_name: "sum_vec"
          top_name: "sum_vec_top"
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
├── dart_dev_tree.dts
├── dart_fred
    └── bits
        ├── p0
        │   └── sub_vec_s0.bin
        ├── p1
        │   └── sum_vec_s0.bin
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
the IP `sub_vec` has FRED ID `100` and it is assigned to partition `p0`.

```bash
$ cat hw_tasks.csv
...
sub_vec, 100, p0, dart_fred/bits, 32768, 32768, 32768 
sum_vec, 101, p1, dart_fred/bits, 32768, 32768, 32768
```

For learning purposes, it's recommended to explore the generated Vivado design located in the **static_hw** directory. 

Now, let's prepare the FPGA to receive the newly generated hardware design. In the FPGA terminal, create the directory for this example. 

```bash
$ mkdir -p ~/dart/2ips/
```

In the host computer, let's export the design to the FPGA board running the following commands in the terminal.

```bash
$ cd ~/2ips/dart/fred
$ tar czf ../fred.tar.gz .
$ scp ../fred.tar.gz username@fpga_ip:~/dart/2ips/
```

Reboot the FPGA board. It's important to reboot the FPGA before loading a new hardware design for FRED.

```bash
$ sudo shutdown -r now
```

Back in the FPGA terminal, the file `fred.tar.gz` must be found. Run the following script that extracts the file exported from DART and apply it to the FRED folders.

```bash
$ cd ~/dart/2ips/
$ sudo su
$ /fredsys/launch_fred_server.sh .
   - Updating the FRED design ...
   - Launching the FRED device drivers ...
   - Loading the FPGA static part ...
   - Launching the FRED server ...
   - FRED server is up! Ready to run FRED applications ...
```

If the FRED server is not launched successfully, a red error message will appear instead of the messages showed above in green. Up to this part, we successfully loaded a new FRED design generated by DART in the FPGA. The next step is to run a FRED test application to check whether the design is actually running.

#### Running the example in the FPGA


Let's prepare the FPGA for the software execution:

```bash
$ cd ~/dart/
$ mkdir -p 2ips/code/sub
$ mkdir -p 2ips/code/sum
```

In the host computer side, copy the example software provided by DART IPs:


```bash
$ cd dart_ips/ips/sub_vec
$ scp -r sw/ username@fpga_ip:~/dart/2ips/code/sub
$ cd dart_ips/ips/sum_vec
$ scp -r sw/ username@fpga_ip:~/dart/2ips/code/sum
```

Back to the FPGA terminal, let's compile the example applications.
But, before, it's important to do a minor change in the example software for the `sum_vec` IP.

```bash
$ cd ~/dart/2ips/code/sum/src
$ nano sum.c
```

Replace the line **const int hw_id = 100;** by **101**. Note that this value matches with the FRED hardware IP showed before in the **hw_tasks.csv** file.

```bash
$ cd ~/dart/2ips/code/sum/
$ mkdir build; cd build
$ cmake ..
$ make
Scanning dependencies of target synthetic
[ 50%] Building C object CMakeFiles/synthetic.dir/src/sum.c.o
[100%] Linking C executable ../synthetic
[100%] Built target synthetic
```

Now let's run the sum application with FRED. This example sums 0 + 1.
Feel free to change the input values to check the results generated by the FPGA.

```bash
$ ../synthetic
 starting vector sum 
fred_lib: connected to fred server!
buff: buffer mapped at addresses: 0x36f5a000, length:32768 
buff: buffer mapped at addresses: 0x36f24000, length:32768 
buff: buffer mapped at addresses: 0x36f1c000, length:32768 
Match!
Content of A[0:9]:
[ 0 0 0 0 0 0 0 0 0 0 ] 
Content of B[0:9]:
[ 1 1 1 1 1 1 1 1 1 1 ] 
Content of C[0:9]:
[ 1 1 1 1 1 1 1 1 1 1 ] 
Fred finished
```

The message **Match!** indicates that the test was successful. Otherwise, the message **Mismatch!** will appear.

Follow similar steps to test the `sub_vec` IP. In this case, the IP already has the correct FRED **hw_id**. So, no change is required. The expected output is:

```bash
$ ../synthetic
 starting vector sub 
fred_lib: connected to fred server!
buff: buffer mapped at addresses: 0x36fe7000, length:32768 
buff: buffer mapped at addresses: 0x36fb1000, length:32768 
buff: buffer mapped at addresses: 0x36fa9000, length:32768 
Match!
Content of A[0:9]:
[ 2 2 2 2 2 2 2 2 2 2 ] 
Content of B[0:9]:
[ 1 1 1 1 1 1 1 1 1 1 ] 
Content of C[0:9]:
[ 1 1 1 1 1 1 1 1 1 1 ] 
Fred finished
```

When FRED is running, it logs some performance data in the `\fredsys\log.txt`. It might be useful to explore this file to learn the kind of runtime information provided by FRED.

So, this is the end of this initial tutorial. Feel free to play with DART, adding more IPs and configuring more partitions.
