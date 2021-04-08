// ==============================================================
// File generated on Sat Apr 25 01:01:23 EDT 2020
// Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2018.3 (64-bit)
// SW Build 2405991 on Thu Dec  6 23:36:41 MST 2018
// IP Build 2404404 on Fri Dec  7 01:43:56 MST 2018
// Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef XFFT_TOP_H
#define XFFT_TOP_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifndef __linux__
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#else
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "xfft_top_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#else
typedef struct {
    u16 DeviceId;
    u32 Ctrl_bus_BaseAddress;
} XFft_top_Config;
#endif

typedef struct {
    u32 Ctrl_bus_BaseAddress;
    u32 IsReady;
} XFft_top;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XFft_top_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XFft_top_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XFft_top_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XFft_top_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

#define Xil_AssertVoid(expr)    assert(expr)
#define Xil_AssertNonvoid(expr) assert(expr)

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
int XFft_top_Initialize(XFft_top *InstancePtr, u16 DeviceId);
XFft_top_Config* XFft_top_LookupConfig(u16 DeviceId);
int XFft_top_CfgInitialize(XFft_top *InstancePtr, XFft_top_Config *ConfigPtr);
#else
int XFft_top_Initialize(XFft_top *InstancePtr, const char* InstanceName);
int XFft_top_Release(XFft_top *InstancePtr);
#endif

void XFft_top_Start(XFft_top *InstancePtr);
u32 XFft_top_IsDone(XFft_top *InstancePtr);
u32 XFft_top_IsIdle(XFft_top *InstancePtr);
u32 XFft_top_IsReady(XFft_top *InstancePtr);
void XFft_top_EnableAutoRestart(XFft_top *InstancePtr);
void XFft_top_DisableAutoRestart(XFft_top *InstancePtr);

void XFft_top_Set_direction(XFft_top *InstancePtr, u32 Data);
u32 XFft_top_Get_direction(XFft_top *InstancePtr);
void XFft_top_Set_in_M_real_V(XFft_top *InstancePtr, u32 Data);
u32 XFft_top_Get_in_M_real_V(XFft_top *InstancePtr);
void XFft_top_Set_in_M_imag_V(XFft_top *InstancePtr, u32 Data);
u32 XFft_top_Get_in_M_imag_V(XFft_top *InstancePtr);
void XFft_top_Set_out_M_real_V(XFft_top *InstancePtr, u32 Data);
u32 XFft_top_Get_out_M_real_V(XFft_top *InstancePtr);
void XFft_top_Set_out_M_imag_V(XFft_top *InstancePtr, u32 Data);
u32 XFft_top_Get_out_M_imag_V(XFft_top *InstancePtr);
u32 XFft_top_Get_ovflo(XFft_top *InstancePtr);
u32 XFft_top_Get_ovflo_vld(XFft_top *InstancePtr);

void XFft_top_InterruptGlobalEnable(XFft_top *InstancePtr);
void XFft_top_InterruptGlobalDisable(XFft_top *InstancePtr);
void XFft_top_InterruptEnable(XFft_top *InstancePtr, u32 Mask);
void XFft_top_InterruptDisable(XFft_top *InstancePtr, u32 Mask);
void XFft_top_InterruptClear(XFft_top *InstancePtr, u32 Mask);
u32 XFft_top_InterruptGetEnabled(XFft_top *InstancePtr);
u32 XFft_top_InterruptGetStatus(XFft_top *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
