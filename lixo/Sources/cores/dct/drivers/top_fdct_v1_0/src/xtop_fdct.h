// ==============================================================
// File generated on Sat Apr 25 02:37:19 EDT 2020
// Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2018.3 (64-bit)
// SW Build 2405991 on Thu Dec  6 23:36:41 MST 2018
// IP Build 2404404 on Fri Dec  7 01:43:56 MST 2018
// Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef XTOP_FDCT_H
#define XTOP_FDCT_H

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
#include "xtop_fdct_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#else
typedef struct {
    u16 DeviceId;
    u32 Ctrl_bus_BaseAddress;
} XTop_fdct_Config;
#endif

typedef struct {
    u32 Ctrl_bus_BaseAddress;
    u32 IsReady;
} XTop_fdct;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XTop_fdct_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XTop_fdct_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XTop_fdct_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XTop_fdct_ReadReg(BaseAddress, RegOffset) \
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
int XTop_fdct_Initialize(XTop_fdct *InstancePtr, u16 DeviceId);
XTop_fdct_Config* XTop_fdct_LookupConfig(u16 DeviceId);
int XTop_fdct_CfgInitialize(XTop_fdct *InstancePtr, XTop_fdct_Config *ConfigPtr);
#else
int XTop_fdct_Initialize(XTop_fdct *InstancePtr, const char* InstanceName);
int XTop_fdct_Release(XTop_fdct *InstancePtr);
#endif


void XTop_fdct_Set_in_block(XTop_fdct *InstancePtr, u32 Data);
u32 XTop_fdct_Get_in_block(XTop_fdct *InstancePtr);
void XTop_fdct_Set_out_block(XTop_fdct *InstancePtr, u32 Data);
u32 XTop_fdct_Get_out_block(XTop_fdct *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
