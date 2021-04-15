// ==============================================================
// Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2019.2 (64-bit)
// Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
// ==============================================================
/***************************** Include Files *********************************/
#include "xacc.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XAcc_CfgInitialize(XAcc *InstancePtr, XAcc_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Ctrl_bus_BaseAddress = ConfigPtr->Ctrl_bus_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XAcc_Start(XAcc *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAcc_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XACC_CTRL_BUS_ADDR_AP_CTRL) & 0x80;
    XAcc_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XACC_CTRL_BUS_ADDR_AP_CTRL, Data | 0x01);
}

u32 XAcc_IsDone(XAcc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAcc_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XACC_CTRL_BUS_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XAcc_IsIdle(XAcc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAcc_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XACC_CTRL_BUS_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XAcc_IsReady(XAcc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAcc_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XACC_CTRL_BUS_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XAcc_EnableAutoRestart(XAcc *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAcc_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XACC_CTRL_BUS_ADDR_AP_CTRL, 0x80);
}

void XAcc_DisableAutoRestart(XAcc *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAcc_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XACC_CTRL_BUS_ADDR_AP_CTRL, 0);
}

u32 XAcc_Get_id(XAcc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAcc_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XACC_CTRL_BUS_ADDR_ID_DATA);
    return Data;
}

u32 XAcc_Get_id_vld(XAcc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAcc_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XACC_CTRL_BUS_ADDR_ID_CTRL);
    return Data & 0x1;
}

void XAcc_Set_mem_in_V(XAcc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAcc_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XACC_CTRL_BUS_ADDR_MEM_IN_V_DATA, Data);
}

u32 XAcc_Get_mem_in_V(XAcc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAcc_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XACC_CTRL_BUS_ADDR_MEM_IN_V_DATA);
    return Data;
}

void XAcc_Set_mem_out_V(XAcc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAcc_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XACC_CTRL_BUS_ADDR_MEM_OUT_V_DATA, Data);
}

u32 XAcc_Get_mem_out_V(XAcc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAcc_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XACC_CTRL_BUS_ADDR_MEM_OUT_V_DATA);
    return Data;
}

u32 XAcc_Get_args_BaseAddress(XAcc *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Ctrl_bus_BaseAddress + XACC_CTRL_BUS_ADDR_ARGS_BASE);
}

u32 XAcc_Get_args_HighAddress(XAcc *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Ctrl_bus_BaseAddress + XACC_CTRL_BUS_ADDR_ARGS_HIGH);
}

u32 XAcc_Get_args_TotalBytes(XAcc *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (XACC_CTRL_BUS_ADDR_ARGS_HIGH - XACC_CTRL_BUS_ADDR_ARGS_BASE + 1);
}

u32 XAcc_Get_args_BitWidth(XAcc *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XACC_CTRL_BUS_WIDTH_ARGS;
}

u32 XAcc_Get_args_Depth(XAcc *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XACC_CTRL_BUS_DEPTH_ARGS;
}

u32 XAcc_Write_args_Words(XAcc *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XACC_CTRL_BUS_ADDR_ARGS_HIGH - XACC_CTRL_BUS_ADDR_ARGS_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(int *)(InstancePtr->Ctrl_bus_BaseAddress + XACC_CTRL_BUS_ADDR_ARGS_BASE + (offset + i)*4) = *(data + i);
    }
    return length;
}

u32 XAcc_Read_args_Words(XAcc *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XACC_CTRL_BUS_ADDR_ARGS_HIGH - XACC_CTRL_BUS_ADDR_ARGS_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(int *)(InstancePtr->Ctrl_bus_BaseAddress + XACC_CTRL_BUS_ADDR_ARGS_BASE + (offset + i)*4);
    }
    return length;
}

u32 XAcc_Write_args_Bytes(XAcc *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XACC_CTRL_BUS_ADDR_ARGS_HIGH - XACC_CTRL_BUS_ADDR_ARGS_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(char *)(InstancePtr->Ctrl_bus_BaseAddress + XACC_CTRL_BUS_ADDR_ARGS_BASE + offset + i) = *(data + i);
    }
    return length;
}

u32 XAcc_Read_args_Bytes(XAcc *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XACC_CTRL_BUS_ADDR_ARGS_HIGH - XACC_CTRL_BUS_ADDR_ARGS_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(char *)(InstancePtr->Ctrl_bus_BaseAddress + XACC_CTRL_BUS_ADDR_ARGS_BASE + offset + i);
    }
    return length;
}

void XAcc_InterruptGlobalEnable(XAcc *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAcc_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XACC_CTRL_BUS_ADDR_GIE, 1);
}

void XAcc_InterruptGlobalDisable(XAcc *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAcc_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XACC_CTRL_BUS_ADDR_GIE, 0);
}

void XAcc_InterruptEnable(XAcc *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XAcc_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XACC_CTRL_BUS_ADDR_IER);
    XAcc_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XACC_CTRL_BUS_ADDR_IER, Register | Mask);
}

void XAcc_InterruptDisable(XAcc *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XAcc_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XACC_CTRL_BUS_ADDR_IER);
    XAcc_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XACC_CTRL_BUS_ADDR_IER, Register & (~Mask));
}

void XAcc_InterruptClear(XAcc *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAcc_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XACC_CTRL_BUS_ADDR_ISR, Mask);
}

u32 XAcc_InterruptGetEnabled(XAcc *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XAcc_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XACC_CTRL_BUS_ADDR_IER);
}

u32 XAcc_InterruptGetStatus(XAcc *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XAcc_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XACC_CTRL_BUS_ADDR_ISR);
}

