
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
#include <string>
#include <ssi.h>

#define MAX_COUNT 200

using namespace std;
enum InfoOption {
    osystem = 0,
    oraidinfo,
    oraidlevel,
    ocontroller,
    ovolume,
    ophy,
    olast
};

string option[] = {"system", "raidinfo", "raidlevel", "controller", "volume", "phy"};
string devtype[] = {"SSI_DeviceTypeUnknown", "SSI_DeviceTypeController", "SSI_DeviceTypeEndDevice", "SSI_DeviceTypeRoutingDevice"};
string physpeed[] = {"SSI_PhySpeedUnknown" , "SSI_PhySpeed_1_5_GEN1", "SSI_PhySpeed_3_0_GEN2", "SSI_PhySpeed_6_0_GEN3"};
string phyprotocol[] = {"SSI_PhyProtocolUnknown", "SSI_PhyProtocolSATA", "SSI_PhyProtocolSMP", "SSI_PhyProtocolSTP", "SSI_PhyProtocolSSP"};

int main(int argc, char *argv[])
{
    SSI_Status status;
    SSI_Handle session;
    SSI_Handle handles[MAX_COUNT];
    SSI_Uint32 count;
    InfoOption optcount = olast;
    int opt;

    unsigned int j = 0;

    status = SsiInitialize();
    if (status != SSI_StatusOk) {
        return -1;
    }

    status = SsiSessionOpen(&session);
    if (status != SSI_StatusOk) {
        cout << "E. Unable to open session" << endl;
        return -1;
    }

    for (int k = 1; k < argc; k++) {
        for(opt = 0; opt < optcount; opt++) {
            if (argv[k] == option[opt])
                break;
        }
        switch (opt) {
        case osystem:
            cout << "System info:" << endl;
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
            break;

        case oraidinfo:
        case oraidlevel:
            SSI_RaidInfo raidInfo;
            count = MAX_COUNT;
            status = SsiGetRaidInfoHandles(session, handles, &count);
            if (status == SSI_StatusOk) {
                cout << "Raid Infos: " << count << endl;
                for (unsigned int i = 0; i < count; ++i) {
                    cout << "\thandle=0x" << hex << handles[i] << endl;
                    status = SsiGetRaidInfo(session, handles[i], &raidInfo);
                    if (status == SSI_StatusOk) {
                        cout << "\t\tmaxDisksPerArray: " << (int)raidInfo.maxDisksPerArray << endl;
                        cout << "\t\tmaxRaidDisksSupported: " << (int)raidInfo.maxRaidDisksSupported << endl;
                        cout << "\t\tmaxVolumesPerArray: " << (int)raidInfo.maxVolumesPerArray << endl;
                        cout << "\t\tmaxVolumesPerHba: " << (int)raidInfo.maxVolumesPerHba << endl;
                        cout << "\t\traidEnabled: " << (bool)raidInfo.raidEnabled << endl;
                        cout << "\t\tsupportsGlobalSpare: " << (bool)raidInfo.supportsGlobalSpare << endl;
                        cout << "\t\tsupportsDedicatedSpare: " << (bool)raidInfo.supportsDedicatedSpare << endl;
                        cout << "\t\tsupportsCreateFromExisting: " << (bool)raidInfo.supportsCreateFromExisting << endl;
                        cout << "\t\tsupportsEmptyArrays: " << (bool)raidInfo.supportsEmptyArrays << endl;
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
                cout << "    RaidLevelInfo: " << endl;
                SSI_RaidLevel a[] = {SSI_Raid0, SSI_Raid1, SSI_Raid10, SSI_Raid5, SSI_Raid6, SSI_RaidUnknown};
                string b[] = {"SSI_Raid0", "SSI_Raid1", "SSI_Raid10", "SSI_Raid5", "SSI_Raid6", "SSI_RaidUnknown"};
                for (j = 0; j < 6 ; j++) {
                    status = SsiGetRaidLevelInfo(session, handles[0], a[j], &info);
                    if (status == SSI_StatusOk) {
                        cout << "\tRaid Level: " << b[j] << endl;
                        cout << "\t\tsupported: " << (bool)info.supported << endl;
                        cout << "\t\tminDisks: " << (int)info.minDisks << endl;
                        cout << "\t\tmaxDisks: " << (int)info.maxDisks << endl;
                        cout << "\t\tdefaultStripSize: " << (int)info.defaultStripSize << endl;
                        cout << "\t\tmigrSupport: " << (int)info.migrSupport << endl;
                        cout << "\t\tmigrDiskAdd: " << (int)info.migrDiskAdd << endl;
                        cout << "\t\tevenDiskCount: " << (bool)info.evenDiskCount << endl;
                        cout << "\t\toddDiskCount: " << (bool)info.oddDiskCount << endl;
                        cout << "\t\tstripSizesSupported: " << (int)info.stripSizesSupported << endl;
                    }
                }
            }
            break;

        case ocontroller:
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
            break;

        case ovolume:
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
            break;

        case ophy:
            SSI_PhyInfo phyInfo;
            count = MAX_COUNT;
            status = SsiGetPhyHandles(session, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handles, &count);
            if (status == SSI_StatusOk) {
                cout << "Phys: " << count << endl;
                for (unsigned int i = 0; i < count; ++i) {
                    cout << "\thandle=0x" << hex << handles[i] << endl;
                    status = SsiGetPhyInfo(session, handles[i], &phyInfo);
                    if (status == SSI_StatusOk) {
                        cout << "\tphyAddress: " << (unsigned) phyInfo.phyAddress.scsiAddress.host << ":"<< (unsigned) phyInfo.phyAddress.scsiAddress.bus << ":"<< (unsigned) phyInfo.phyAddress.scsiAddress.target << ":"<< (unsigned) phyInfo.phyAddress.scsiAddress.lun << endl;
                        cout << "\tphy protocol: " << phyprotocol[phyInfo.protocol] << endl;
                        cout << "\tdeviceType: " << devtype[phyInfo.deviceType] << endl;
                        cout << "\tnegotiated link speed: " << physpeed[phyInfo.negotiatedLinkSpeed] << endl;
#if 0
                        SSI_Address phyAddress;
                        /** Indicates which phy on the parent device this represents */
                        SSI_Uint32 phyNumber;
                        /** Connection protocol */
                        SSI_PhyProtocol protocol;
                        /** Logical port object that identifies connection between phys.
                            Value is SSI_INVALID_HANDLE if phy is not connected. */
                        SSI_Handle associatedPort;

                        /** Type of device that phy is contained in */
                        SSI_DeviceType deviceType;
                        /** Handle to the device that phy is contained in */
                        SSI_Handle deviceHandle;

                        /** If true, phy is external */
                        SSI_Bool isExternal;
                        /** If true, hot plug even can be detected */
                        SSI_Bool hotPlugCap;

                        /** Minimum hardware speed phy can operate at */
                        SSI_PhySpeed minHWLinkSpeed;
                        /** Maximum hardware speed phy can operate at */
                        SSI_PhySpeed maxHWLinkSpeed;
                        /** Minimum programmed speed phy can operate at */
                        SSI_PhySpeed minLinkSpeed;
                        /** Maximum programmed speed phy can operate at */
                        SSI_PhySpeed maxLinkSpeed;
                        /** Negotiated link speed */
                        SSI_PhySpeed negotiatedLinkSpeed;

                        /** If true, following counts contain valid */
                        SSI_Bool   countsValid;
                        /** Number of invalid DWORDs that have been received since PWR reset */
                        SSI_Uint32 invalildDWORDsCount;
                        /** Number of frames received */
                        SSI_Uint32 receivedFramesCount;
                        /** Number of frames transmitted */
                        SSI_Uint32 transmittedFramesCount;
                        /** Number of DWORDs received */
                        SSI_Uint32 receivedDWORDsCount;
                        /** Number of DWORDs transmitted */
                        SSI_Uint32 transmittedDWORDsCount;
                        /** Number of times the py has restarted the link reset sequence due to lost DWORD synchronization */
                        SSI_Uint32 syncLostCount;
                        /** Number of DWORDs containing running disparity errors */
                        SSI_Uint32 runningDisparityErrorsCount;
                        /** Number of CRC errors */
                        SSI_Uint32 crcErrorsCount;
                        /** Number of short frames received */
                        SSI_Uint32 shortFramesReceivedCount;
                        /** Number of done frames received */
                        SSI_Uint32 doneFrameReceivedCount;
                        /** Number of times the phy reset problem has occurred */
                        SSI_Uint32 PHYResetCount;
#endif
                    } else {
                        cout << "E: unable to get phy info (status=" << status << ")" << endl;
                    }
                }
            } else {
                cout << "E: unable to get phy handles (status=" << status << ")" << endl;
            }
            break;

    default:
        cout << "invalid option" << endl;
        }
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
