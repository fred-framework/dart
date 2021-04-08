// ==============================================================
// File generated on Sat Apr 25 02:37:19 EDT 2020
// Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2018.3 (64-bit)
// SW Build 2405991 on Thu Dec  6 23:36:41 MST 2018
// IP Build 2404404 on Fri Dec  7 01:43:56 MST 2018
// Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
// ==============================================================
/***************************** Include Files *********************************/
#include "xtop_fdct.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XTop_fdct_CfgInitialize(XTop_fdct *InstancePtr, XTop_fdct_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Ctrl_bus_BaseAddress = ConfigPtr->Ctrl_bus_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XTop_fdct_Set_in_block(XTop_fdct *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XTop_fdct_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XTOP_FDCT_CTRL_BUS_ADDR_IN_BLOCK_DATA, Data);
}

u32 XTop_fdct_Get_in_block(XTop_fdct *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XTop_fdct_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XTOP_FDCT_CTRL_BUS_ADDR_IN_BLOCK_DATA);
    return Data;
}

void XTop_fdct_Set_out_block(XTop_fdct *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XTop_fdct_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XTOP_FDCT_CTRL_BUS_ADDR_OUT_BLOCK_DATA, Data);
}

u32 XTop_fdct_Get_out_block(XTop_fdct *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XTop_fdct_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XTOP_FDCT_CTRL_BUS_ADDR_OUT_BLOCK_DATA);
    return Data;
}

