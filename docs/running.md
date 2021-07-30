

## Running DART

Once DART is installed and the DART IPs are available, the next is step
is to test DART with some simple example designs. 

**We are assuming that DART and DART IPs are already setup !!!**

### Example of a DART design with the `memcpy` IP

This is an image of the block design automatically generated by DART with one reconfigurable reagion and one `memcpy` IP.

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
Under the generated DART design directory there is a `fred` directory with the following structure:

```bash
$ cd fred
$ tree
.
├── arch.csv
├── hw_tasks.csv
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

The example above shows that the generated design has a single partition called `p0`
and it has a single slot (i.e. IP) assigned to it. In case of multiple partitions, 
each partition is reported in a new line of the CSV file.


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
 
 memcpy, 100, p0, dart_fred/bits, 1048576, 32768
```

The block above shows the corresponding `hw_tasks.csv` file for the same example design. It shows the name of the IP, its FRED Hw ID, the partition it is assigned, 
the bitstream location, and the sizes of the input and output buffers.
If the design has multiple IPs, each one appears in a different line.


### Example with two partitions

In this example we show how to run DART with two partitions where each partition has only one slot. Two IPs from *DART IPs* are used: `sub_vec` and `sum_vec`.

#### Running DART

In 


```bash
$ mkdir -p ~/2ips/dart
cd ~/2ips
nano dart.csv
```

Fill dart.csv with the following content, meaning that the `sub_vec` IP is mapped to the partition 0 and the `sum_vec` IP is mapped to partition 1.

```
0,sub_vec,sub_vec_top
1,sum_vec,sum_vec_top
```

Now let's run DART:

```bash
$ cd ~/2ips/dart
$ run_pr_tool_without_part_pynq 2 ../dart.csv
```

The execution time is about 10 minutes, depending on the computer.
The expected output is:

```
...
INFO: [Common 17-206] Exiting Vivado at Sun Jul 11 10:07:13 2021...
PR_TOOL: creating FRED files 
PR_TOOL: destruction of PR_tool
```

Now let's explore the generated design and send later send it to the FPGA.

```bash
$ cd ~/2ips/dart/fred
$ cat arch.csv 
p0, 1
p1, 1
```

This last file tells us that the generated design has two partitions p0 and p1, each one with a single slot, i.e., just one core assigned to the partition.

```bash
$ cat hw_tasks.csv
...
sub_vec, 100, p0, dart_fred/bits, 1048576, 32768
sum_vec, 101, p1, dart_fred/bits, 1048576, 32768
```

This file gives information related to the IPs for each slot. So, for instance, 
the IP `sub_vec` has FRED ID `100` and it is assigned to partition `p0`.

One current DART limitation is that it supports only two FRED buffers.
However, the IPs we are using in this example use three buffers. Thus,
we have to **manually fix this in the hw_tasks.csv** file. The expected result is:

```
sub_vec, 100, p0, dart_fred/bits, 32768, 32768, 32768
sum_vec, 101, p1, dart_fred/bits, 32768, 32768, 32768
```

For learning purposes, it's recommended to explored the generated Vivado design located in the **static_hw** directory. 

Now, let's prepare the FPGA to receive the newly generated hardware design. In the FPGA, create the directory for this example.

```bash
$ mkdir -p ~/dart/2ips/
$ sudo shutdown -r now
```

Another DART limitation is that it does not generate the Linux device tree according to the new generated design. So the designer has to update it by hand. 


```bash
$ cd /boot
$ nano devicetree.dts
$ dtc -I dts -O dtb -f devicetree.dts -o devicetree.dtb
```

For this example with two partitions, the last lines of the **devicetree.dts** must be like this:

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

        slot_p1_s0@43c20000 {
                compatible = "generic-uio";
                reg = <0x43c20000 0x10000>;
                interrupt-parent = <0x4>;
                interrupts = <0x0 0x1e 0x4>;
        };

        pr_decoupler_p1_s0@43c30000 {
                compatible = "generic-uio";
                reg = <0x43c30000 0x10000>;
        };
	};
};
```

Finally, reboot the board. It's important to reboot the FPGA before loading a new hardware design for FRED or the device tree is changed.

```bash
$ sudo shutdown -r now
```

Back to the host computer, run the following commands to transfer the design to the FPGA:

```bash
$ cd ~/2ips/dart/fred
tar czf ../fred.tar.gz .
scp ../fred.tar.gz username@fpga_ip:~/dart/2ips/
```

Once again, let's return to the FPGA after reboot and load the new FPGA design with FRED.

```bash
$ mkdir -p ~/dart/2ips/
$ sudo su
$ /fredsys/launch_fred_server.sh .
   - Updating the FRED design ...
   - Launching the FRED device drivers ...
   - Loading the FPGA static part ...
   - Launching the FRED server ...
   - FRED server is up! Ready to run FRED applications ...
```

If the FRED server is not launched sucessfully, a red error message will appear instead of the messages showed above in green. Up to this part we successfully loaded a new FRED design generated by DART in the FPGA. The next step is to run a FRED test application to check whether the design is actually running.

#### Running the example in the FPGA


Let's prepare the FPGA for the software execution:

```bash
$ cd ~/dart/
$ mkdir -p 2ips/code/sub
$ mkdir -p 2ips/code/sum
```

In the host computer side, copy the example software to the FPGA:


```bash
$ cd dart_ips/ips/sub_vec
$ scp -r sw/ username@fpga_ip:~/dart/2ips/code/sub
$ cd dart_ips/ips/sum_vec
$ scp -r sw/ username@fpga_ip:~/dart/2ips/code/sum
```

Back to the FPGA, let's compile the example applications.
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
cc1: warning: command line option ‘-std=c++14’ is valid for C++/ObjC++ but not for C
[100%] Linking C executable ../synthetic
[100%] Built target synthetic
```

Now let's run the sum application with FRED. This example sums 0 + 1.
Feel free to change the inputs the expected values.

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

The message **Match!** indicates that the test was succesfull. Otherwise the message **Mismatch!** will appear.

Follow similar steps to test the `sub_vec` IP. In this case, the IP already have the correct FRED **hw_id**. So, no change is required. The expected output is:

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

#### Running an application using both IPs



<!---
### Example for pr_tool_without_part

The following example is for *pr_tool_without_part*, assuming five IPs and two partitions.
Partition 0 with IPs mul, nor, and xor, and partition 1 with the IPs sum and sub.

```
1,sum_vec,sum_vec_top
0,mul_vec,mul_vec_top
1,sub_vec,sub_vec_top
0,nor_vec,nor_vec_top
0,xor_vec,xor_vec_top
```

**RUN AND SHOW THE EXPECTED OUTPUT**

### Example for pr_tool_with_part

The next example is for *pr_tool_with_part*, assuming the same five IPs.
Note that the partitions are not defined by the user, but it will be defined by DART.
In this case, each IP has a slack of 900 ns and a deadline of 1000 ns. 
**IS THE UNIT CORRECT ?**

```
900,1000,sum_vec,sum_vec_top
900,1000,mul_vec,mul_vec_top
900,1000,sub_vec,sub_vec_top
900,1000,nor_vec,nor_vec_top
900,1000,xor_vec,xor_vec_top
```

**RUN AND SHOW THE EXPECTED OUTPUT**

### Example for FLORA

**PUT HERE EXAMPLES FOR RUNNING FLORA**

**RUN AND SHOW THE EXPECTED OUTPUT**
-->
