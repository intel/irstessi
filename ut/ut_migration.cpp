
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

enum TestResult {
    NotRun = -1,
    Passed = 0,
    Failed = 1
};

TestResult raid1_to_raid0(SSI_Handle *pDevice, unsigned int count);
using namespace std;

int main(int argc, char *argv[])
{
    SSI_Handle session;
    SSI_Status status;

    SSI_Handle controllerHandle[HANDLE_COUNT];
    SSI_Uint32 controllerCount;
    SSI_Handle handle[HANDLE_COUNT];
    SSI_Uint32 count;
    SSI_Handle ahciDevice[HANDLE_COUNT];

    unsigned int ahciCount = 0;
    unsigned int scuCount = 0;
    unsigned int passed = 0;
    unsigned int failed = 0;
    unsigned int notrun = 0;

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
        cout << "E. Unable to open session" << endl;
        return -1;
    }

    /* get Controllers */
    controllerCount = HANDLE_COUNT;
    status = SsiGetControllerHandles(session, SSI_ScopeTypeNone, SSI_NULL_HANDLE , controllerHandle, &controllerCount);
    if (status != SSI_StatusOk) {
        cout << "E. Unable to get controller handles" << endl;
        return -1;
    }

    for (unsigned int c = 0; c < controllerCount; c++) {
        cout << "Controller handle " << c << "\t"<< hex << controllerHandle[c] << dec << endl;
        SSI_ControllerInfo controllerInfo;
        status = SsiGetControllerInfo(session, controllerHandle[c], &controllerInfo);
        /* get devices */
        cout << "-->SsiGetEndDeviceHandles"<< endl;
        count = HANDLE_COUNT;
        status = SsiGetEndDeviceHandles(session, SSI_ScopeTypeControllerAll, controllerHandle[c], handle, &count);
        if (status == SSI_StatusOk) {
            SSI_EndDeviceInfo info;
            cout << "end device handles retrieved:"<< count << endl;
            for (unsigned int i = 0; i < count; i++) {
                cout << "handle " << i << "\t"<< hex << handle[i] << dec;
                status = SsiGetEndDeviceInfo(session, handle[i], &info);
                if (status == SSI_StatusOk) {
                    if (info.systemDisk == SSI_FALSE && info.deviceType == SSI_EndDeviceTypeDisk &&
                        info.state == SSI_DiskStateNormal && info.usage == SSI_DiskUsagePassThru) {
                        if (controllerInfo.controllerType == SSI_ControllerTypeAHCI) {
                            ahciDevice[ahciCount++] = handle[i];
                            cout << "\tAHCI";
                        } else {

                            cout << "\tISCI";
                        }
                        cout << " device suitable for raid" << endl;
                    } else {
                    cout << "\tdevice unusable" << endl;
                    }
                }
            }
        }
    }

    if (ahciCount < 3) {
        cout << "E: there are not enough available AHCI disks in the system to perform test" << endl;
        return -2;
    }
    if (scuCount < 4) {
        cout << "E: there are not enough available ISCI disks in the system to perform test" << endl;
        return -2;
    }

    int rv = raid1_to_raid0(ahciDevice, ahciCount);
    switch (rv) {
        case 0:
            passed++;
            break;
        case -1:
            notrun++;
            break;
        default:
            failed++;
    }

    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;

    status = SsiFinalize();
    if (status != SSI_StatusOk) {
        return -2;
    }
    return 0;
}

TestResult raid1_to_raid0(SSI_Handle *pDevice, unsigned int count)
{
    SSI_Status status;
    SSI_Handle volumeHandle0 = 0;
    /* create raid0 volume on AHCI */
    SSI_CreateFromDisksParams params;
    params.disks = pDevice;
    params.numDisks = 2;
    params.sourceDisk = SSI_NULL_HANDLE;
    strcpy(params.volumeName, "ahciR1");
    params.stripSize = SSI_StripSizeUnknown;
    params.raidLevel = SSI_Raid1;
    params.sizeInBytes = 1024*1024*128;

    cout << "-->SsiVolumeCreateFromDisks..." << endl;
    status = SsiVolumeCreateFromDisks(params, &volumeHandle0);
    if (status == SSI_StatusOk) {
        cout << "Created volume: volumeHandle=0x" << hex << volumeHandle0 << endl;
    } else {
        return NotRun;
    }

    /* wait for resync to finish */
    system("while grep -i resync /proc/mdstat; do sleep 1; done");

    /* modify volume */

    /* 1 -> 0 */
    SSI_RaidLevelModifyParams modifyParams;
    modifyParams.newStripSize = SSI_StripSizeUnknown;
    modifyParams.newRaidLevel = SSI_Raid0;
    modifyParams.newSizeInBytes = 0;
    modifyParams.diskHandles = pDevice + 2;
    modifyParams.diskHandleCount = 2;
    status = SsiRaidLevelModify(volumeHandle0, modifyParams);
    if (status != SSI_StatusOk) {
        cout << "E: Modify volume failed (status = " << status << ")"<< endl;
        return Failed;
    } else {
        cout << "Modify volume succeeded" << endl;
        return Passed;
    }

    /* delete both volumes
    cout << "-->SsiVolumeDelete..." << endl;
    status = SsiVolumeDelete(volumeHandle0);
    if (status != SSI_StatusOk) {
        cout << "E: unable to delete volume from the array." << endl;
    }
    status = SsiVolumeDelete(volumeHandle1);
    if (status != SSI_StatusOk) {
        cout << "E: unable to delete volume from the array." << endl;
    }
    */
}
// ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=96 noexpandtab:
