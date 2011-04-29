
/*
Copyright (c) 2011, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
