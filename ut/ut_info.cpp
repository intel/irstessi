
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

#define MAX_COUNT 200

using namespace std;

int main(int argc, char *argv[])
{
    SSI_Status status;
    SSI_Handle session;
    SSI_Handle handles[MAX_COUNT];
    SSI_Uint32 count;

    unsigned int j = 0;
    (void)argc;
    (void)argv;


    status = SsiInitialize();
    if (status != SSI_StatusOk) {
        return -1;
    }

    status = SsiSessionOpen(&session);
    if (status != SSI_StatusOk) {
        cout << "E. Unable to open session" << endl;
        return -1;
    }

    SSI_SystemInfo systemInfo;
    status = SsiGetSystemInfo(&systemInfo);
    if (status == SSI_StatusOk) {
        cout << "interfaceVersionMajor: " << (int)systemInfo.interfaceVersionMajor << endl;
        cout << "interfaceVersionMinor: " << (int)systemInfo.interfaceVersionMinor << endl;
        cout << "libraryVersion: " << (const char *)systemInfo.libraryVersion << endl;
        cout << "maxSessions: " << (int)systemInfo.maxSessions << endl;
        cout << "setVolCacheSizeSupport: " << (bool)systemInfo.setVolCacheSizeSupport << endl;
        cout << "passthroughCommandSupport: " << (bool)systemInfo.passthroughCommandSupport << endl;
    }

    SSI_RaidInfo raidInfo;
    count = MAX_COUNT;
    status = SsiGetRaidInfoHandles(session, handles, &count);
    if (status == SSI_StatusOk) {
        cout << "Raid Infos: " << count << endl;
        for (unsigned int i = 0; i < count; ++i) {
            cout << "\thandle=0x" << hex << handles[i] << endl;
            status = SsiGetRaidInfo(session, handles[i], &raidInfo);
            if (status == SSI_StatusOk) {
                cout << "\tmaxDisksPerArray: " << (int)raidInfo.maxDisksPerArray << endl;
                cout << "\tmaxRaidDisksSupported: " << (int)raidInfo.maxRaidDisksSupported << endl;
                cout << "\tmaxVolumesPerArray: " << (int)raidInfo.maxVolumesPerArray << endl;
                cout << "\tmaxVolumesPerHba: " << (int)raidInfo.maxVolumesPerHba << endl;
                cout << "\traidEnabled: " << (bool)raidInfo.raidEnabled << endl;
                cout << "\tsupportsGlobalSpare: " << (bool)raidInfo.supportsGlobalSpare << endl;
                cout << "\tsupportsDedicatedSpare: " << (bool)raidInfo.supportsDedicatedSpare << endl;
                cout << "\tsupportsCreateFromExisting: " << (bool)raidInfo.supportsCreateFromExisting << endl;
                cout << "\tsupportsEmptyArrays: " << (bool)raidInfo.supportsEmptyArrays << endl;
//                cout << "\t: " << ()raidInfo. << endl;
            } else {
                cout << "E. unable to get RaidInfo information" << endl;
            }
        }
    } else {
        cout << "E. Unable to get raid info handles" << endl;
    }

    SSI_RaidLevelInfo info;
    if (count > 0 && count != MAX_COUNT) {
        SSI_RaidLevel a[] = {SSI_Raid0, SSI_Raid1, SSI_Raid10, SSI_Raid5, SSI_Raid6, SSI_RaidUnknown};
        for (j = 0; j < 6 ; j++) {
            status = SsiGetRaidLevelInfo(session, handles[0], (SSI_RaidLevel) a[j], &info);
            if (status == SSI_StatusOk) {
                cout << "\tsupported: " << (bool)info.supported << endl;
                cout << "\tminDisks: " << (int)info.minDisks << endl;
                cout << "\tmaxDisks: " << (int)info.maxDisks << endl;
                cout << "\tdefaultStripSize: " << (int)info.defaultStripSize << endl;
                cout << "\tmigrSupport: " << (int)info.migrSupport << endl;
                cout << "\tmigrDiskAdd: " << (int)info.migrDiskAdd << endl;
                cout << "\tevenDiskCount: " << (bool)info.evenDiskCount << endl;
                cout << "\toddDiskCount: " << (bool)info.oddDiskCount << endl;
                cout << "\tstripSizesSupported: " << (int)info.stripSizesSupported << endl;
            }
        }
    }

    SSI_ControllerInfo controllerInfo;
    count = MAX_COUNT;
    status = SsiGetControllerHandles(session, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handles, &count);
    if (status == SSI_StatusOk) {
        cout << "Controllers: " << count << endl;
        for (unsigned int i = 0; i < count; ++i) {
            cout << "handle=0x" << hex << handles[i] << endl;
            status = SsiGetControllerInfo(session, handles[i], &controllerInfo);
            if (status == SSI_StatusOk) {
                cout << "\tcontrollerName: " << (const char *)controllerInfo.controllerName << endl;
                cout << "\tcontrollerType: " << (int)controllerInfo.controllerType << endl;
                cout << "\traidInfoHandle: " << hex << controllerInfo.raidInfoHandle << dec << endl;
                cout << "\tdriverVer: " << controllerInfo.driverVer << endl;
                cout << "\tprebootManagerVer: " << controllerInfo.prebootManagerVer << endl;
                cout << "\tprebootManagerLoaded: " << (bool)controllerInfo.prebootManagerLoaded << endl;
                cout << "\ttwoTbVolumePrebootSupport: " << (bool)controllerInfo.twoTbVolumePrebootSupport << endl;
                cout << "\tdisableESataSpanning: " << (bool)controllerInfo.disableESataSpanning << endl;
                cout << "\txorSupported: " << (bool)controllerInfo.xorSupported << endl;
                cout << "\tnvsramSupported: " << (bool)controllerInfo.nvsramSupported << endl;
                cout << "\tphyLocateSupport: " << (bool)controllerInfo.phyLocateSupport << endl;
                cout << "\tdiskUnlockSupport: " << (bool)controllerInfo.diskUnlockSupport << endl;
                cout << "\tassignStoragePoolSupport: " << (bool)controllerInfo.assignStoragePoolSupport << endl;
                cout << "\tmarkAsNormalSupport: " << (bool)controllerInfo.markAsNormalSupport << endl;
                cout << "\tvolumeDeleteSupport: " << (bool)controllerInfo.volumeDeleteSupport << endl;
                cout << "\tarrayCreateSupport: " << (bool)controllerInfo.arrayCreateSupport << endl;
                cout << "\tvolumeModifySupport: " << (bool)controllerInfo.volumeModifySupport << endl;
                cout << "\tvolumeRenameSupport: " << (bool)controllerInfo.volumeRenameSupport << endl;
                cout << "\taddDisksToArraySupport: " << (bool)controllerInfo.addDisksToArraySupport << endl;
                cout << "\tvolumeSetCachePolicySupport: " << (bool)controllerInfo.volumeSetCachePolicySupport << endl;
                cout << "\tvolumeCancelVerifySupport: " << (bool)controllerInfo.volumeCancelVerifySupport << endl;
                cout << "\tmarkAsSpareSupport: " << (bool)controllerInfo.markAsSpareSupport << endl;
                cout << "\treadPatrolSupport: " << (bool)controllerInfo.readPatrolSupport << endl;
                cout << "\treadPatrolEnabled: " << (bool)controllerInfo.readPatrolEnabled << endl;
                cout << "\txorEnabled: " << (bool)controllerInfo.xorEnabled << endl;
            } else {
                cout << "E. Unable to get controller info" << endl;
            }
        }
    } else {
        cout << "E. Unable to get controller handles" << endl;
    }

    SSI_VolumeInfo volumeInfo;
    count = MAX_COUNT;
    status = SsiGetVolumeHandles(session, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handles, &count);
    if (status == SSI_StatusOk) {
        cout << "volumes: " << count << endl;
        for (unsigned int i = 0; i < count; ++i) {
            cout << "\thandle=0x" << hex << handles[i] << endl;
            status = SsiGetVolumeInfo(session, handles[i], &volumeInfo);
            if (status == SSI_StatusOk) {
                cout << "\tarrayHandle=0x" << hex << volumeInfo.arrayHandle<< endl;
                cout << "\tarrayOrdinal" << volumeInfo.arrayOrdinal << endl;
                cout << "\tvolumeName" << volumeInfo.volumeName << endl;
                cout << "\tvolume state: " << volumeInfo.state << endl;
                cout << "\tvolume raidLevel: " << volumeInfo.raidLevel << endl;
            } else {
                cout << "E: unable to get volume info (status=" << status << ")" << endl;
            }
        }
    } else {
        cout << "E: unable to get volume handles (status=" << status << ")" << endl;
    }

    SSI_PhyInfo phyInfo;
    count = MAX_COUNT;
    status = SsiGetPhyHandles(session, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handles, &count);
    if (status == SSI_StatusOk) {
        cout << "Phys: " << count << endl;
        for (unsigned int i = 0; i < count; ++i) {
            cout << "\thandle=0x" << hex << handles[i] << endl;
            status = SsiGetPhyInfo(session, handles[i], &phyInfo);
            if (status == SSI_StatusOk) {
                cout << "\tphy protocol: " << phyInfo.protocol << endl;
            } else {
                cout << "E: unable to get phy info (status=" << status << ")" << endl;
            }
        }
    } else {
        cout << "E: unable to get phy handles (status=" << status << ")" << endl;
    }
/*
SSI_API SSI_Status SsiGetPortInfo(SSI_Handle session, SSI_Handle portHandle, SSI_PortInfo *info); tt
SSI_API SSI_Status SsiGetEnclosureInfo(SSI_Handle session, SSI_Handle enclosureHandle, SSI_EnclosureInfo *info);tt
SSI_API SSI_Status SsiGetEndDeviceInfo(SSI_Handle session, SSI_Handle endDeviceHandle, SSI_EndDeviceInfo *info); tt
SSI_API SSI_Status SsiGetRoutingDeviceInfo(SSI_Handle session, SSI_Handle routingDeviceHandle, SSI_RoutingDeviceInfo *info); tt
SSI_API SSI_Status SsiGetArrayInfo(SSI_Handle session, SSI_Handle arrayHandle, SSI_ArrayInfo *info); tt
SSI_API SSI_Status SsiGetVolumeInfo(SSI_Handle session, SSI_Handle volumeHandle, SSI_VolumeInfo *info); ok
SSI_API SSI_Status SsiGetRaidLevelInfo(SSI_Handle session, SSI_Handle raidInfoHandle, SSI_RaidLevel raidLevel, SSI_RaidLevelInfo *info); bugs
*/

    status = SsiFinalize();
    if (status != SSI_StatusOk) {
        return -2;
    }
    return 0;
}

// ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=96 noexpandtab:
