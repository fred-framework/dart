
### One Reconfigurable Region

This tutorial describes an example using the following partition:
- part 0: sum_vec, sub_vec, mul_vec, xor_vec, nor_vec;

Thus, there is a single reconfigurable region shared by 5 IPs. Compared to the configuration with [two reconfigurable regions](../2rr/readme.md) presented next, the chances of not meeting the deadline are higher since there is more contention for using this single reconfigurable region. This means, the five Hw tasks are competing for the same resource.

#### Generating the bitstreams

- Make sure that DART was compiled to your board and with **PARTITIONING_MODE** `OFF`;
- Make sure that these environment variables are set **XILINX_VIVADO**, **XILINX_HLS**, **DART_HOME**;
- If the required IPs were not compiled previoulsly, then edit the script in this directory to `compile_ips=1`;
- If the required IPs were compiled previoulsly, then make sure that **DART_IP_PATH** is set;
- Run `run.sh` and wait until the end of the synthesis;
- The FRED generated configuration files, the devicetree, and bitstreams are in the `fred.tar.gz` file.

#### Running in the board

In the board, run:

```
$ update_hw <user> <ip> <path>
$ load_hw
$ dmesg | tail -n 30
[    5.714832] of-fpga-region fpga-full: FPGA Region probed
[    6.056044] FAT-fs (mmcblk0p1): Volume was not properly unmounted. Some data may be corrupt. Please run fsck.
[    6.099623] Fred_BuffCtl: buffctl device initialized
[    6.133529] EXT4-fs (mmcblk0p2): re-mounted. Opts: (null)
[    7.259363] pps pps0: new PPS source ptp0
[    7.259380] macb ff0e0000.ethernet: gem-ptp-timer ptp clock registered.
[    8.299430] macb ff0e0000.ethernet eth0: link up (1000/Full)
[    8.299461] IPv6: ADDRCONF(NETDEV_CHANGE): eth0: link becomes ready
[   17.736294] random: crng init done
[   17.736301] random: 7 urandom warning(s) missed due to ratelimiting
[   63.173146] fpga_manager fpga0: writing static.bin to Xilinx ZynqMP FPGA Manager Fmod
[   63.399779] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /fpga-full/firmware-name
[   63.399801] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /fpga-full/resets
[   63.400236] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /amba/slot_p0_s0@A0000000/clock-names
[   63.400248] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /amba/slot_p0_s0@A0000000/clocks
[   63.400255] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /amba/slot_p0_s0@A0000000/compatible
[   63.400261] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /amba/slot_p0_s0@A0000000/reg
[   63.400267] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /amba/slot_p0_s0@A0000000/xlnx,s-axi-ctrl-bus-addr-width
[   63.400273] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /amba/slot_p0_s0@A0000000/xlnx,s-axi-ctrl-bus-data-width
[   63.400279] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /amba/slot_p0_s0@A0000000/interrupt-parent
[   63.400285] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /amba/slot_p0_s0@A0000000/interrupts
[   63.400325] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /amba/pr_decoupler_p0_s0@A0010000/clock-names
[   63.400334] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /amba/pr_decoupler_p0_s0@A0010000/clocks
[   63.400340] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /amba/pr_decoupler_p0_s0@A0010000/compatible
[   63.400346] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /amba/pr_decoupler_p0_s0@A0010000/reg
[   63.400376] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /__symbols__/overlay0
[   63.400390] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /__symbols__/overlay2
[   63.400403] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /__symbols__/afi0
[   63.400416] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /__symbols__/clocking0
[   63.400433] OF: overlay: WARNING: memory leak will occur if overlay removed, property: /__symbols__/overlay3
```

The command [`update_hw`](https://github.com/fred-framework/meta-fred/blob/main/scripts/update_hw) fetches the new DART design and compile the new Linux devicetree segment, `load_hw` loads the new bitstream using `fpga-manager` device driver. Note that `dmesg` shows the devicetree messages. Always check these messages because they are a common source of mismatches in the hardware/software interface. Still in `dmesg` messages, note that the `fpga_manager` device driver is the modified one. See the message *Xilinx ZynqMP FPGA Manager Fmod*.

Next we execute the FRED server, where it loads the bitstream into bufffers, according to the `hw_tasks.csv` configuration.

```
$ fred-server &
fred_sys: building hw-tasks
fred_sys: pars: parsing file: /opt/fredsys/hw_tasks.csv
buff: buffer mapped at addresses: 0xffffa65f1000, length:1942168 
fred_sys: loaded slot 0 bitstream for hw-task sum_vec, size: 1942168
fred_sys: creating data buffer 0 of size 32768 for HW-task sum_vec
fred_sys: creating data buffer 1 of size 32768 for HW-task sum_vec
fred_sys: creating data buffer 2 of size 32768 for HW-task sum_vec
buff: buffer mapped at addresses: 0xffffa65f1000, length:1942168 
fred_sys: loaded slot 0 bitstream for hw-task sub_vec, size: 1942168
fred_sys: creating data buffer 0 of size 32768 for HW-task sub_vec
fred_sys: creating data buffer 1 of size 32768 for HW-task sub_vec
fred_sys: creating data buffer 2 of size 32769 for HW-task sub_vec
buff: buffer mapped at addresses: 0xffffa65f1000, length:1942168 
fred_sys: loaded slot 0 bitstream for hw-task mul_vec, size: 1942168
fred_sys: creating data buffer 0 of size 32768 for HW-task mul_vec
fred_sys: creating data buffer 1 of size 32768 for HW-task mul_vec
fred_sys: creating data buffer 2 of size 32769 for HW-task mul_vec
buff: buffer mapped at addresses: 0xffffa65f1000, length:1942168 
fred_sys: loaded slot 0 bitstream for hw-task xor_vec, size: 1942168
fred_sys: creating data buffer 0 of size 32768 for HW-task xor_vec
fred_sys: creating data buffer 1 of size 32768 for HW-task xor_vec
fred_sys: creating data buffer 2 of size 32770 for HW-task xor_vec
buff: buffer mapped at addresses: 0xffffa65f1000, length:1942168 
fred_sys: loaded slot 0 bitstream for hw-task nor_vec, size: 1942168
fred_sys: creating data buffer 0 of size 32768 for HW-task nor_vec
fred_sys: creating data buffer 1 of size 32768 for HW-task nor_vec
fred_sys: creating data buffer 2 of size 32770 for HW-task nor_vec
----------------------------------------- Layout -----------------------------------------
Partitions:
        partition 0 : p0 containing 1 slots
Hw-tasks:
        hw-task: sum_vec, id: 100, partition: p0, timeout: 1 ms, using 3 buffers
        hw-task: sub_vec, id: 101, partition: p0, timeout: 1 ms, using 3 buffers
        hw-task: mul_vec, id: 102, partition: p0, timeout: 1 ms, using 3 buffers
        hw-task: xor_vec, id: 103, partition: p0, timeout: 1 ms, using 3 buffers
        hw-task: nor_vec, id: 104, partition: p0, timeout: 1 ms, using 3 buffers
------------------------------------------------------------------------------------------
fred_sys: epoll reactor: adding event handler: signals handler on fd: 14
fred_sys: epoll reactor: adding event handler: devcfg on fd: 3
fred_sys: epoll reactor: adding event handler: slot device 0 on fd: 10
fred_sys: epoll reactor: adding event handler: slot timer on fd: 12
fred_sys: epoll reactor: adding event handler: sw-task listener on fd: 13
```

Note the `id` of each task. This id is used in application to refer to the hw tasks. Make sure that the application uses the same id used by FRED server.


#### Analysing the results

In another terminal, run:

```
$ app
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

The `app` will print messages showing that the some timing constraints were not met. Next we test a similar configuration, but with [two reconfigurable regions](../2rr/readme.md). 

