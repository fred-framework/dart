// ==============================================================
// Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2019.2 (64-bit)
// Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xacc.h"

extern XAcc_Config XAcc_ConfigTable[];

XAcc_Config *XAcc_LookupConfig(u16 DeviceId) {
	XAcc_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XACC_NUM_INSTANCES; Index++) {
		if (XAcc_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XAcc_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XAcc_Initialize(XAcc *InstancePtr, u16 DeviceId) {
	XAcc_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XAcc_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XAcc_CfgInitialize(InstancePtr, ConfigPtr);
}

#endif

