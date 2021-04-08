// ==============================================================
// File generated on Sat Apr 25 01:01:23 EDT 2020
// Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2018.3 (64-bit)
// SW Build 2405991 on Thu Dec  6 23:36:41 MST 2018
// IP Build 2404404 on Fri Dec  7 01:43:56 MST 2018
// Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xfft_top.h"

extern XFft_top_Config XFft_top_ConfigTable[];

XFft_top_Config *XFft_top_LookupConfig(u16 DeviceId) {
	XFft_top_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XFFT_TOP_NUM_INSTANCES; Index++) {
		if (XFft_top_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XFft_top_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XFft_top_Initialize(XFft_top *InstancePtr, u16 DeviceId) {
	XFft_top_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XFft_top_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XFft_top_CfgInitialize(InstancePtr, ConfigPtr);
}

#endif

