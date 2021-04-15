// ==============================================================
// Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2019.2 (64-bit)
// Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef XACC_H
#define XACC_H

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
#include "xacc_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#else
typedef struct {
    u16 DeviceId;
    u32 Ctrl_bus_BaseAddress;
} XAcc_Config;
#endif

typedef struct {
    u32 Ctrl_bus_BaseAddress;
    u32 IsReady;
} XAcc;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XAcc_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XAcc_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XAcc_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XAcc_ReadReg(BaseAddress, RegOffset) \
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
int XAcc_Initialize(XAcc *InstancePtr, u16 DeviceId);
XAcc_Config* XAcc_LookupConfig(u16 DeviceId);
int XAcc_CfgInitialize(XAcc *InstancePtr, XAcc_Config *ConfigPtr);
#else
int XAcc_Initialize(XAcc *InstancePtr, const char* InstanceName);
int XAcc_Release(XAcc *InstancePtr);
#endif

void XAcc_Start(XAcc *InstancePtr);
u32 XAcc_IsDone(XAcc *InstancePtr);
u32 XAcc_IsIdle(XAcc *InstancePtr);
u32 XAcc_IsReady(XAcc *InstancePtr);
void XAcc_EnableAutoRestart(XAcc *InstancePtr);
void XAcc_DisableAutoRestart(XAcc *InstancePtr);

u32 XAcc_Get_id(XAcc *InstancePtr);
u32 XAcc_Get_id_vld(XAcc *InstancePtr);
void XAcc_Set_mem_in_V(XAcc *InstancePtr, u32 Data);
u32 XAcc_Get_mem_in_V(XAcc *InstancePtr);
void XAcc_Set_mem_out_V(XAcc *InstancePtr, u32 Data);
u32 XAcc_Get_mem_out_V(XAcc *InstancePtr);
u32 XAcc_Get_args_BaseAddress(XAcc *InstancePtr);
u32 XAcc_Get_args_HighAddress(XAcc *InstancePtr);
u32 XAcc_Get_args_TotalBytes(XAcc *InstancePtr);
u32 XAcc_Get_args_BitWidth(XAcc *InstancePtr);
u32 XAcc_Get_args_Depth(XAcc *InstancePtr);
u32 XAcc_Write_args_Words(XAcc *InstancePtr, int offset, int *data, int length);
u32 XAcc_Read_args_Words(XAcc *InstancePtr, int offset, int *data, int length);
u32 XAcc_Write_args_Bytes(XAcc *InstancePtr, int offset, char *data, int length);
u32 XAcc_Read_args_Bytes(XAcc *InstancePtr, int offset, char *data, int length);

void XAcc_InterruptGlobalEnable(XAcc *InstancePtr);
void XAcc_InterruptGlobalDisable(XAcc *InstancePtr);
void XAcc_InterruptEnable(XAcc *InstancePtr, u32 Mask);
void XAcc_InterruptDisable(XAcc *InstancePtr, u32 Mask);
void XAcc_InterruptClear(XAcc *InstancePtr, u32 Mask);
u32 XAcc_InterruptGetEnabled(XAcc *InstancePtr);
u32 XAcc_InterruptGetStatus(XAcc *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
