// ==============================================================
// Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2019.2 (64-bit)
// Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
// ==============================================================
// ctrl_bus
// 0x00 : Control signals
//        bit 0  - ap_start (Read/Write/COH)
//        bit 1  - ap_done (Read/COR)
//        bit 2  - ap_idle (Read)
//        bit 3  - ap_ready (Read)
//        bit 7  - auto_restart (Read/Write)
//        others - reserved
// 0x04 : Global Interrupt Enable Register
//        bit 0  - Global Interrupt Enable (Read/Write)
//        others - reserved
// 0x08 : IP Interrupt Enable Register (Read/Write)
//        bit 0  - Channel 0 (ap_done)
//        bit 1  - Channel 1 (ap_ready)
//        others - reserved
// 0x0c : IP Interrupt Status Register (Read/TOW)
//        bit 0  - Channel 0 (ap_done)
//        bit 1  - Channel 1 (ap_ready)
//        others - reserved
// 0x10 : Data signal of id
//        bit 31~0 - id[31:0] (Read)
// 0x14 : Control signal of id
//        bit 0  - id_ap_vld (Read/COR)
//        others - reserved
// 0x40 : Data signal of mem_in_V
//        bit 31~0 - mem_in_V[31:0] (Read/Write)
// 0x44 : reserved
// 0x48 : Data signal of mem_out_V
//        bit 31~0 - mem_out_V[31:0] (Read/Write)
// 0x4c : reserved
// 0x20 ~
// 0x3f : Memory 'args' (8 * 32b)
//        Word n : bit [31:0] - args[n]
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XACC_CTRL_BUS_ADDR_AP_CTRL        0x00
#define XACC_CTRL_BUS_ADDR_GIE            0x04
#define XACC_CTRL_BUS_ADDR_IER            0x08
#define XACC_CTRL_BUS_ADDR_ISR            0x0c
#define XACC_CTRL_BUS_ADDR_ID_DATA        0x10
#define XACC_CTRL_BUS_BITS_ID_DATA        32
#define XACC_CTRL_BUS_ADDR_ID_CTRL        0x14
#define XACC_CTRL_BUS_ADDR_MEM_IN_V_DATA  0x40
#define XACC_CTRL_BUS_BITS_MEM_IN_V_DATA  32
#define XACC_CTRL_BUS_ADDR_MEM_OUT_V_DATA 0x48
#define XACC_CTRL_BUS_BITS_MEM_OUT_V_DATA 32
#define XACC_CTRL_BUS_ADDR_ARGS_BASE      0x20
#define XACC_CTRL_BUS_ADDR_ARGS_HIGH      0x3f
#define XACC_CTRL_BUS_WIDTH_ARGS          32
#define XACC_CTRL_BUS_DEPTH_ARGS          8

