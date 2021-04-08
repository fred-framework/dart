# ==============================================================
# File generated on Sat Apr 25 02:37:19 EDT 2020
# Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2018.3 (64-bit)
# SW Build 2405991 on Thu Dec  6 23:36:41 MST 2018
# IP Build 2404404 on Fri Dec  7 01:43:56 MST 2018
# Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
# ==============================================================
proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "XTop_fdct" \
        "NUM_INSTANCES" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BUS_BASEADDR" \
        "C_S_AXI_CTRL_BUS_HIGHADDR"

    xdefine_config_file $drv_handle "xtop_fdct_g.c" "XTop_fdct" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BUS_BASEADDR"

    xdefine_canonical_xpars $drv_handle "xparameters.h" "XTop_fdct" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BUS_BASEADDR" \
        "C_S_AXI_CTRL_BUS_HIGHADDR"
}

