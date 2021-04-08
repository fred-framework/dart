// ==============================================================
// File generated on Sat Apr 25 01:01:23 EDT 2020
// Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2018.3 (64-bit)
// SW Build 2405991 on Thu Dec  6 23:36:41 MST 2018
// IP Build 2404404 on Fri Dec  7 01:43:56 MST 2018
// Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
// ==============================================================
/***************************** Include Files *********************************/
#include "xfft_top.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XFft_top_CfgInitialize(XFft_top *InstancePtr, XFft_top_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Ctrl_bus_BaseAddress = ConfigPtr->Ctrl_bus_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XFft_top_Start(XFft_top *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFft_top_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_AP_CTRL) & 0x80;
    XFft_top_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_AP_CTRL, Data | 0x01);
}

u32 XFft_top_IsDone(XFft_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFft_top_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XFft_top_IsIdle(XFft_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFft_top_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XFft_top_IsReady(XFft_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFft_top_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XFft_top_EnableAutoRestart(XFft_top *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFft_top_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_AP_CTRL, 0x80);
}

void XFft_top_DisableAutoRestart(XFft_top *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFft_top_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_AP_CTRL, 0);
}

void XFft_top_Set_direction(XFft_top *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFft_top_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_DIRECTION_DATA, Data);
}

u32 XFft_top_Get_direction(XFft_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFft_top_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_DIRECTION_DATA);
    return Data;
}

void XFft_top_Set_in_M_real_V(XFft_top *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFft_top_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_IN_M_REAL_V_DATA, Data);
}

u32 XFft_top_Get_in_M_real_V(XFft_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFft_top_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_IN_M_REAL_V_DATA);
    return Data;
}

void XFft_top_Set_in_M_imag_V(XFft_top *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFft_top_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_IN_M_IMAG_V_DATA, Data);
}

u32 XFft_top_Get_in_M_imag_V(XFft_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFft_top_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_IN_M_IMAG_V_DATA);
    return Data;
}

void XFft_top_Set_out_M_real_V(XFft_top *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFft_top_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_OUT_M_REAL_V_DATA, Data);
}

u32 XFft_top_Get_out_M_real_V(XFft_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFft_top_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_OUT_M_REAL_V_DATA);
    return Data;
}

void XFft_top_Set_out_M_imag_V(XFft_top *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFft_top_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_OUT_M_IMAG_V_DATA, Data);
}

u32 XFft_top_Get_out_M_imag_V(XFft_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFft_top_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_OUT_M_IMAG_V_DATA);
    return Data;
}

u32 XFft_top_Get_ovflo(XFft_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFft_top_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_OVFLO_DATA);
    return Data;
}

u32 XFft_top_Get_ovflo_vld(XFft_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFft_top_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_OVFLO_CTRL);
    return Data & 0x1;
}

void XFft_top_InterruptGlobalEnable(XFft_top *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFft_top_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_GIE, 1);
}

void XFft_top_InterruptGlobalDisable(XFft_top *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFft_top_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_GIE, 0);
}

void XFft_top_InterruptEnable(XFft_top *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XFft_top_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_IER);
    XFft_top_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_IER, Register | Mask);
}

void XFft_top_InterruptDisable(XFft_top *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XFft_top_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_IER);
    XFft_top_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_IER, Register & (~Mask));
}

void XFft_top_InterruptClear(XFft_top *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFft_top_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_ISR, Mask);
}

u32 XFft_top_InterruptGetEnabled(XFft_top *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XFft_top_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_IER);
}

u32 XFft_top_InterruptGetStatus(XFft_top *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XFft_top_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XFFT_TOP_CTRL_BUS_ADDR_ISR);
}

