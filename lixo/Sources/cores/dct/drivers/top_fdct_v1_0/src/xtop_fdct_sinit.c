// ==============================================================
// File generated on Sat Apr 25 02:37:19 EDT 2020
// Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2018.3 (64-bit)
// SW Build 2405991 on Thu Dec  6 23:36:41 MST 2018
// IP Build 2404404 on Fri Dec  7 01:43:56 MST 2018
// Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xtop_fdct.h"

extern XTop_fdct_Config XTop_fdct_ConfigTable[];

XTop_fdct_Config *XTop_fdct_LookupConfig(u16 DeviceId) {
	XTop_fdct_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XTOP_FDCT_NUM_INSTANCES; Index++) {
		if (XTop_fdct_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XTop_fdct_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XTop_fdct_Initialize(XTop_fdct *InstancePtr, u16 DeviceId) {
	XTop_fdct_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XTop_fdct_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XTop_fdct_CfgInitialize(InstancePtr, ConfigPtr);
}

#endif

