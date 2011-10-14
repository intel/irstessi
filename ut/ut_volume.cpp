
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
    SSI_Handle volumeHandle = 0;
    SSI_Handle arrayHandle = 0;
	SSI_Handle session;

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
    status = SsiSessionOpen(&session);
    if (status != SSI_StatusOk) {
        return -1;
    }
	SSI_Handle handles[HANDLE_COUNT];
    SSI_Handle endDevices[HANDLE_COUNT];
    unsigned int j = 0;

    /* get devices */
    cout << "-->SsiGetEndDeviceHandles"<< endl;
    count = HANDLE_COUNT;
    status = SsiGetEndDeviceHandles(session, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handles, &count);
    if (status == SSI_StatusOk) {
        SSI_EndDeviceInfo info;
        cout << count << endl;
        for (unsigned int i = 0; i < count; i++) {
            cout << "handle " << i << "\t"<< hex << handles[i] << dec;
            status = SsiGetEndDeviceInfo(session, handles[i], &info);
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

    if (j < 10) {
        cout << "E: there are no available devices in the system to create a RAID." << endl;
        return -2;
    }

    cout << "Usable disks:" << endl;
    for (unsigned int i = 0; i < j; i++) {
        cout << "Disk " << i << "\t"<< hex << endDevices[i] << dec << endl;
    }

    /* create raid5 volume*/
    SSI_CreateFromDisksParams params;
    params.disks = endDevices;
    params.numDisks = 8;
    params.sourceDisk = SSI_NULL_HANDLE;
    strcpy(params.volumeName, "ut_Volume");
    params.stripSize = SSI_StripSize64kB;
    params.raidLevel = SSI_Raid5;
    params.sizeInBytes = 1024UL*1024*1024*7;

    cout << "-->SsiVolumeCreateFromDisks..." << endl;
    status = SsiVolumeCreateFromDisks(params, &volumeHandle);
    if (status == SSI_StatusOk) {
        cout << "Created volume: volumeHandle=0x" << hex << volumeHandle << endl;
    }

    /* find array handle */
    cout << "-->SsiGetArrayHandles..." << endl;
    count = 10;
    status = SsiGetArrayHandles(SSI_NULL_HANDLE, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handles, &count);
    if (status == SSI_StatusOk) {
        cout << "arrays: " << count << endl;
        for (unsigned int i = 0; i < count; ++i) {
            cout << "\thandle=0x" << hex << handles[i] << endl;
        }
        if (count > 0) {
            arrayHandle = handles[0];
        }
    } else {
        cout << "E: unable to get array handles (status=" << status << ")" << endl;
    }
    /* find array handle */
    cout << "-->SsiGetVolumeHandles..." << endl;
    count = 10;
    status = SsiGetVolumeHandles(SSI_NULL_HANDLE, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handles, &count);
    if (status == SSI_StatusOk) {
        cout << "volumes: " << count << endl;
        for (unsigned int i = 0; i < count; ++i) {
            cout << "\thandle=0x" << hex << handles[i] << endl;
		} 
	}else {
        cout << "E: unable to get volume handles (status=" << status << ")" << endl;
    }

    /* create raid0 volume on the same array */
    cout << "-->SsiVolumeCreate..." << endl;
    SSI_CreateFromArrayParams array_params;
    array_params.arrayHandle = arrayHandle;
    strcpy(array_params.volumeName, "ut_Volume_r0_01");
    array_params.stripSize = SSI_StripSize64kB;
    array_params.raidLevel = SSI_Raid0;
    array_params.sizeInBytes = 0;

    status = SsiVolumeCreate(array_params);
    if (status != SSI_StatusOk) {
        cout << "E: unable to create second volume in the same array." << endl;
    }

    /* get handles of both volumes */
    cout << "-->SsiGetVolumeHandles..." << endl;
    count = 10;
    status = SsiGetVolumeHandles(SSI_NULL_HANDLE, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handles, &count);
    if (status == SSI_StatusOk) {
        cout << "volumes: " << count << endl;
        for (unsigned int i = 0; i < count; ++i) {
            cout << "\thandle=0x" << hex << handles[i] << dec <<endl;
        }
    } else {
        cout << "E: unable to get volume handles (status=" << status << ")" << endl;
    }

 
    status = SsiFinalize();
    if (status != SSI_StatusOk) {
        return -2;
    }
    return 0;
}

// ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=96 noexpandtab:
