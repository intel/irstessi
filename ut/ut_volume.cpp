/*
 * IMPORTANT - READ BEFORE COPYING, INSTALLING OR USING.
 * BY LOADING OR USING THIS SOFTWARE, YOU AGREE TO THE TERMS OF THIS INTEL
 * SOFTWARE LICENSE AGREEMENT.  IF YOU DO NOT WISH TO SO AGREE, DO NOT COPY,
 * INSTALL OR USE THIS SOFTWARE.
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2010 Intel Corporation. All Rights Reserved.
 *
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation or its
 * suppliers or licensors.
 *
 * Title to the Material remains with Intel Corporation or its suppliers and
 * licensors. The Material contains trade secrets and proprietary and
 * confidential information of Intel or its suppliers and licensors.
 * The Material is protected by worldwide copyright and trade secret laws and
 * treaty provisions. No part of the Material may be used, copied, reproduced,
 * modified, published, uploaded, posted, transmitted, distributed,
 * or disclosed in any way without Intel's prior express written permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure
 * or delivery of the Materials, either expressly, by implication, inducement,
 * estoppel or otherwise. Any license under such intellectual property rights
 * must be express and approved by Intel in writing.
 */

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>

#include <ssi.h>
#define HANDLE_COUNT 100

using namespace std;

int main(int argc, char *argv[])
{
	SSI_Status status;
	SSI_Uint32 count;
	SSI_Handle volumeHandle;

	(void)argc;
	(void)argv;

	if (system("cd scripts; ./clean.sh") != 0) {
		cout << "E: unable to stop existing md devices and clean metadata." << endl;
		return -1;
	}

	status = SsiInitialize();
	if (status != SSI_StatusOk) {
		return -1;
	}

	SSI_Handle r1_disks[2];
	SSI_Handle handles[HANDLE_COUNT];
	SSI_Handle endDevices[HANDLE_COUNT];
	unsigned int j = 0;

	count = HANDLE_COUNT;
	status = SsiGetEndDeviceHandles(SSI_NULL_HANDLE, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handles, &count);
	if (status == SSI_StatusOk) {
		SSI_EndDeviceInfo info;
		cout << count << endl;
		for (unsigned int i = 0; i < count; i++) {
			cout << "handle " << i << "\t"<< hex << handles[i] << dec;
			status = SsiGetEndDeviceInfo(SSI_NULL_HANDLE, handles[i], &info);
			if (status == SSI_StatusOk) {
				if (info.systemDisk == SSI_FALSE && info.deviceType == SSI_EndDeviceTypeDisk &&
					info.state == SSI_DiskStateNormal && info.usage == SSI_DiskUsagePassThru) {
					endDevices[j++] = handles[i];
					cout << "\tdevice suitable for raid"<< endl;
				} else {
					cout << "\tdevice unusable" << endl;
				}
			}
		}
	}

	if (j == 0) {
		cout << "E: there are no available devices in the system to create a RAID." << endl;
		return -2;
	}

	r1_disks[0] = endDevices[0];
	r1_disks[1] = endDevices[1];

	SSI_CreateFromDisksParams params;
	params.disks = r1_disks;
	params.numDisks = 2;
	params.sourceDisk = SSI_NULL_HANDLE;
	strcpy(params.volumeName, "ut_Volume_r1_01");
	params.stripSize = SSI_StripSize64kB;
	params.raidLevel = SSI_Raid1;
	params.sizeInBytes = 1024000;

	status = SsiVolumeCreateFromDisks(params, &volumeHandle);
	if (status == SSI_StatusOk) {
		cout << "volumeHandle=0x" << hex << volumeHandle << endl;
	}
	count = 10;
	status = SsiGetArrayHandles(SSI_NULL_HANDLE, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handles, &count);
	if (status == SSI_StatusOk) {
		cout << "arrays: " << count << endl;
		for (unsigned int i = 0; i < count; ++i) {
			cout << "\thandle=0x" << hex << handles[i] << endl;
		}
	} else {
		cout << "E: unable to get array handles (status=" << status << ")" << endl;
	}

/*	SSI_CreateFromArrayParams array_params;
	array_params.arrayHandle = handles[0];
	strcpy(array_params.volumeName, "ut_Volume_r0_01");
	array_params.stripSize = SSI_StripSize64kB;
	array_params.raidLevel = SSI_Raid0;
	array_params.sizeInBytes = 0;

	status = SsiVolumeCreate(array_params);
	if (status != SSI_StatusOk) {
		cout << "E: unable to create second volume in the same array." << endl;
	}*/

	count = 10;
	status = SsiGetVolumeHandles(SSI_NULL_HANDLE, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handles, &count);
	if (status == SSI_StatusOk) {
		cout << "volumes: " << count << endl;
		for (unsigned int i = 0; i < count; ++i) {
			cout << "\thandle=0x" << hex << handles[i] << endl;
		}
	} else {
		cout << "E: unable to get volume handles (status=" << status << ")" << endl;
	}

	status = SsiVolumeDelete(volumeHandle);
	if (status != SSI_StatusOk) {
		cout << "E: unable to delete volume from the array." << endl;
	}
	status = SsiFinalize();
	if (status != SSI_StatusOk) {
		return -2;
	}
	return 0; 
}

// ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=96 noexpandtab:
