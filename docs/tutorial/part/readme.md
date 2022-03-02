
### DART Design with Partipoining mode ON

| :exclamation:  Design under construction   |
|--------------------------------------------|

This tutorial creates a similar design compared [2rr](../2rr/readme.md), but here DART will perform the IP partition instead of a manual partition did before.

#### Generating the bitstreams

- Make sure that DART was compiled to your board and with **PARTITIONING_MODE** `ON`;
- Make sure that these environment variables are set **XILINX_VIVADO**, **XILINX_HLS**, **DART_HOME**;
- If the required IPs were not compiled previoulsly, then edit the script in this directory to `compile_ips=1`;
- If the required IPs were compiled previoulsly, then make sure that **DART_IP_PATH** is set;
- Run `run.sh` and wait until the end of the synthesis;
- The FRED generated configuration files, the devicetree, and bitstreams are in the `fred.tar.gz` file.

#### Running in the board

| :exclamation:  Design under construction   |
|--------------------------------------------|
