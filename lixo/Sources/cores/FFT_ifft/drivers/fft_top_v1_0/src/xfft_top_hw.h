// ==============================================================
// File generated on Sat Apr 25 01:01:23 EDT 2020
// Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2018.3 (64-bit)
// SW Build 2405991 on Thu Dec  6 23:36:41 MST 2018
// IP Build 2404404 on Fri Dec  7 01:43:56 MST 2018
// Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
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
// 0x10 : Data signal of direction
//        bit 0  - direction[0] (Read/Write)
//        others - reserved
// 0x14 : reserved
// 0x18 : Data signal of in_M_real_V
//        bit 31~0 - in_M_real_V[31:0] (Read/Write)
// 0x1c : reserved
// 0x20 : Data signal of in_M_imag_V
//        bit 31~0 - in_M_imag_V[31:0] (Read/Write)
// 0x24 : reserved
// 0x28 : Data signal of out_M_real_V
//        bit 31~0 - out_M_real_V[31:0] (Read/Write)
// 0x2c : reserved
// 0x30 : Data signal of out_M_imag_V
//        bit 31~0 - out_M_imag_V[31:0] (Read/Write)
// 0x34 : reserved
// 0x38 : Data signal of ovflo
//        bit 0  - ovflo[0] (Read)
//        others - reserved
// 0x3c : Control signal of ovflo
//        bit 0  - ovflo_ap_vld (Read/COR)
//        others - reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XFFT_TOP_CTRL_BUS_ADDR_AP_CTRL           0x00
#define XFFT_TOP_CTRL_BUS_ADDR_GIE               0x04
#define XFFT_TOP_CTRL_BUS_ADDR_IER               0x08
#define XFFT_TOP_CTRL_BUS_ADDR_ISR               0x0c
#define XFFT_TOP_CTRL_BUS_ADDR_DIRECTION_DATA    0x10
#define XFFT_TOP_CTRL_BUS_BITS_DIRECTION_DATA    1
#define XFFT_TOP_CTRL_BUS_ADDR_IN_M_REAL_V_DATA  0x18
#define XFFT_TOP_CTRL_BUS_BITS_IN_M_REAL_V_DATA  32
#define XFFT_TOP_CTRL_BUS_ADDR_IN_M_IMAG_V_DATA  0x20
#define XFFT_TOP_CTRL_BUS_BITS_IN_M_IMAG_V_DATA  32
#define XFFT_TOP_CTRL_BUS_ADDR_OUT_M_REAL_V_DATA 0x28
#define XFFT_TOP_CTRL_BUS_BITS_OUT_M_REAL_V_DATA 32
#define XFFT_TOP_CTRL_BUS_ADDR_OUT_M_IMAG_V_DATA 0x30
#define XFFT_TOP_CTRL_BUS_BITS_OUT_M_IMAG_V_DATA 32
#define XFFT_TOP_CTRL_BUS_ADDR_OVFLO_DATA        0x38
#define XFFT_TOP_CTRL_BUS_BITS_OVFLO_DATA        1
#define XFFT_TOP_CTRL_BUS_ADDR_OVFLO_CTRL        0x3c

