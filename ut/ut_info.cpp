/*
 * Copyright 2011 - 2017 Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ssi.h>
#include "ut.h"

enum InfoOption {
    osystem = 0,
    oraidinfo,
    oraidlevel,
    ocontroller,
    ovolume,
    ophy,
    oenddevice,
    oenclosure,
    oroutingdevice,
    olast
};

string option[] = {"system", "raidinfo", "raidlevel", "controller", "volume", "phy", "enddevice", "enclosure", "routingdevice"};

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
        cout << "E. Unable to open session (status=" << statusStr[status] << ")" << endl;
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
                    cout << "handle=0x" << hex << handles[i] << dec << endl;
                    status = SsiGetRaidInfo(session, handles[i], &raidInfo);
                    if (status == SSI_StatusOk) {
                        cout << "\t\tUnique Id: " << hex << raidInfo.uniqueId << dec << endl;
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

                        SSI_RaidLevelInfo info;
                        cout << "    RaidLevelInfo: " << endl;
                        SSI_RaidLevel a[] = {SSI_Raid0, SSI_Raid1, SSI_Raid10, SSI_Raid5, SSI_Raid6, SSI_RaidUnknown};
                        string b[] = {"SSI_Raid0", "SSI_Raid1", "SSI_Raid10", "SSI_Raid5", "SSI_Raid6", "SSI_RaidUnknown"};
                        for (j = 0; j < 6 ; j++) {
                            status = SsiGetRaidLevelInfo(session, handles[i], a[j], &info);
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
                    } else {
                        cout << "E. unable to get RaidInfo information (status=" << statusStr[status] << ")" << endl;
                    }
                }
            } else {
                cout << "E. Unable to get raid info handles (status=" << statusStr[status] << ")" << endl;
            }

            break;

        case ocontroller:
            SSI_ControllerInfo controllerInfo;
            count = MAX_COUNT;
            status = SsiGetControllerHandles(session, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handles, &count);
            if (status == SSI_StatusOk) {
                cout << "Controllers: " << count << endl;
                for (unsigned int i = 0; i < count; ++i) {
                    cout << "handle=0x" << hex << handles[i] << dec << endl;
                    status = SsiGetControllerInfo(session, handles[i], &controllerInfo);
                    if (status == SSI_StatusOk) {
                        cout << "\tUnique Id: " << hex << controllerInfo.uniqueId << dec << endl;
                        cout << "\tcontrollerName: " << (const char *)controllerInfo.controllerName << endl;
                        cout << "\tSCSI Address: " << (unsigned) controllerInfo.controllerAddress.scsiAddress.host
                             << ":"<< (unsigned) controllerInfo.controllerAddress.scsiAddress.bus
                             << ":"<< (unsigned) controllerInfo.controllerAddress.scsiAddress.target
                             << ":"<< (unsigned) controllerInfo.controllerAddress.scsiAddress.lun << endl;
                        if (controllerInfo.controllerAddress.sasAddressPresent == SSI_TRUE)
                            cout << "\tSAS Address: 0x" << hex << controllerInfo.controllerAddress.sasAddress << dec << endl;
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
                        cout << "\trohiEnabled: " << (bool)controllerInfo.rohiEnabled << endl;
                        cout << "\trohiSupport: " << (bool)controllerInfo.rohiSupport << endl;
                    } else {
                        cout << "E. Unable to get controller info (status=" << statusStr[status] << ")" << endl;
                    }
                }
            } else {
                cout << "E. Unable to get controller handles (status=" << statusStr[status] << ")" << endl;
            }
            break;

        case ovolume:
            SSI_VolumeInfo volumeInfo;
            count = MAX_COUNT;
            status = SsiGetVolumeHandles(session, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handles, &count);
            if (status == SSI_StatusOk) {
                cout << "volumes: " << count << endl;
                for (unsigned int i = 0; i < count; ++i) {
                    cout << "handle=0x" << hex << handles[i] << dec << endl;
                    status = SsiGetVolumeInfo(session, handles[i], &volumeInfo);
                    if (status == SSI_StatusOk) {
                        cout << "\tUnique Id: " << hex << volumeInfo.uniqueId << dec << endl;
                        cout << "\tarrayHandle = 0x" << hex << volumeInfo.arrayHandle << dec << endl;
                        cout << "\tarrayOrdinal: " << volumeInfo.arrayOrdinal << endl;
                        cout << "\tvolumeName: " << volumeInfo.volumeName << endl;
                        cout << "\tvolume state: " << volumestate(volumeInfo.state) << endl;
                        cout << "\tvolume raidLevel: " << raidlevel(volumeInfo.raidLevel) << endl;
                        cout << "\ttotalSize: " << volumeInfo.totalSize << endl;
                        cout << "\tstripSize: " << volumeInfo.stripSize << endl;
                        cout << "\tnumDisks: " << volumeInfo.numDisks << endl;
                        cout << "\tmigrProgress: " << volumeInfo.migrProgress << endl;
                        cout << "\tcachePolicy: " << volumeInfo.cachePolicy << endl;
                        cout << "\tsystemVolume: " << volumeInfo.systemVolume << endl;
                        cout << "\tmigrating: " << volumeInfo.migrating << endl;
                        cout << "\tinitialized: " << volumeInfo.initialized << endl;
                        cout << "\tverifyErrors: " << volumeInfo.verifyErrors << endl;
                        cout << "\tverifyBadBlocks: " << volumeInfo.verifyBadBlocks << endl;
                        cout << "\tlogicalSectorSize: " << volumeInfo.logicalSectorSize << endl;
                        cout << "\tphysicalSectorSize: " << volumeInfo.physicalSectorSize << endl;

                        /* now show end devices for this volume*/
                        SSI_Handle ehandles[MAX_COUNT];
                        SSI_Uint32 ecount = MAX_COUNT;
                        status = SsiGetEndDeviceHandles(session, SSI_ScopeTypeArray, volumeInfo.arrayHandle, ehandles, &ecount);
                        if (status == SSI_StatusOk) {
                            cout << "\tend devices: " << ecount << endl;
                            for (unsigned int j = 0; j < ecount; ++j)
                                cout << "\t\thandle=0x" << hex << ehandles[j] << dec << endl;
                        } else {
                            cout << "E: unable to get end device handles (status=" << statusStr[status] << ")" << endl;
                        }
                    } else {
                        cout << "E: unable to get volume info (status=" << statusStr[status] << ")" << endl;
                    }
                }
            } else {
                cout << "E: unable to get volume handles (status=" << statusStr[status] << ")" << endl;
            }
            break;

        case ophy:
            SSI_PhyInfo phyInfo;
            count = MAX_COUNT;
            status = SsiGetPhyHandles(session, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handles, &count);
            if (status == SSI_StatusOk) {
                cout << "Phys: " << count << endl;
                for (unsigned int i = 0; i < count; ++i) {
                    cout << "handle=0x" << hex << handles[i] << dec << endl;
                    status = SsiGetPhyInfo(session, handles[i], &phyInfo);
                    if (status == SSI_StatusOk) {
                        cout << "\tUnique Id: " << hex << phyInfo.uniqueId << dec << endl;
                        cout << "\tphyAddress: " << (unsigned) phyInfo.phyAddress.scsiAddress.host
                             << ":"<< (unsigned) phyInfo.phyAddress.scsiAddress.bus
                             << ":"<< (unsigned) phyInfo.phyAddress.scsiAddress.target
                             << ":"<< (unsigned) phyInfo.phyAddress.scsiAddress.lun << endl;
                        if (phyInfo.phyAddress.sasAddressPresent == SSI_TRUE)
                            cout << "\tSAS Address: 0x" << hex << phyInfo.phyAddress.sasAddress << dec << endl;
                        cout << "\tphy protocol: " << phyprotocol[phyInfo.protocol] << endl;
                        cout << "\tdeviceType: " << devtype[phyInfo.deviceType] << endl;
                        cout << "\tnegotiated link speed: " << physpeed[phyInfo.negotiatedLinkSpeed] << endl;
                        cout << "\tphyNumber: " << phyInfo.phyNumber << endl;
                        cout << "\tassociatedPort: 0x" << hex << phyInfo.associatedPort << dec << endl;
                        cout << "\tdeviceHandle: 0x" << hex << phyInfo.deviceHandle << dec << endl;
                        cout << "\tisExternal: " << phyInfo.isExternal << endl;
                        cout << "\thotPlugCap: " << phyInfo.hotPlugCap << endl;
                        cout << "\tminHWLinkSpeed: " << physpeed[phyInfo.minHWLinkSpeed] << endl;
                        cout << "\tmaxHWLinkSpeed: " << physpeed[phyInfo.maxHWLinkSpeed] << endl;
                        cout << "\tminLinkSpeed: " << physpeed[phyInfo.minLinkSpeed] << endl;
                        cout << "\tmaxLinkSpeed: " << physpeed[phyInfo.maxLinkSpeed] << endl;
                        cout << "\tcountsValid: " << phyInfo.countsValid << endl;
                    } else {
                        cout << "E: unable to get phy info (status=" << statusStr[status] << ")" << endl;
                    }
                }
            } else {
                cout << "E: unable to get phy handles (status=" << statusStr[status] << ")" << endl;
            }
            break;

        case oenddevice:
            SSI_EndDeviceInfo edInfo;
            count = MAX_COUNT;
            status = SsiGetEndDeviceHandles(session, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handles, &count);
            if (status == SSI_StatusOk) {
                cout << "End Devices: " << count << endl;
                for (unsigned int i = 0; i < count; ++i) {
                    cout << "handle=0x" << hex << handles[i] << dec << endl;
                    status = SsiGetEndDeviceInfo(session, handles[i], &edInfo);
                    if (status == SSI_StatusOk) {
                        /*cout << "\t: " << edInfo. << endl;*/
                        cout << "\tcontrollerHandle: 0x" << hex << edInfo.controllerHandle << dec << endl;
                        cout << "\tUnique Id: " << hex << edInfo.uniqueId << dec << endl;
                        cout << "\tserialNo: " << edInfo.serialNo << endl;
                        cout << "\tmodel: " << edInfo.model << endl;
                        cout << "\tfirmware: " << edInfo.firmware << endl;
                        cout << "\tscsiAddress: " << (unsigned) edInfo.endDeviceAddress.scsiAddress.host
                             << ":"<< (unsigned) edInfo.endDeviceAddress.scsiAddress.bus
                             << ":"<< (unsigned) edInfo.endDeviceAddress.scsiAddress.target
                             << ":"<< (unsigned) edInfo.endDeviceAddress.scsiAddress.lun << endl;
                        cout << "\tsasAddress: 0x" << hex << edInfo.endDeviceAddress.sasAddress << dec << endl;
                        cout << "\tdeviceType: " << edInfo.deviceType << endl;
                        cout << "\tdiskType: " << edInfo.diskType << endl;
                        cout << "\tRaid Info handle: 0x" << hex << edInfo.raidInfoHandle << dec << endl;
                        cout << "\tArray handle: 0x" << hex << edInfo.arrayHandle << dec << endl;
                        cout << "\tEnclosure handle: 0x" << hex << edInfo.enclosureHandle << dec << endl;
                        if (edInfo.enclosureHandle) {
                            cout << "\t\tslot Address (scsi): " << (unsigned) edInfo.slotAddress.scsiAddress.host
                                 << ":"<< (unsigned) edInfo.slotAddress.scsiAddress.bus
                                 << ":"<< (unsigned) edInfo.slotAddress.scsiAddress.target
                                 << ":"<< (unsigned) edInfo.slotAddress.scsiAddress.lun << endl;
                            cout << "\t\tslot Address (sas): 0x" << hex << edInfo.slotAddress.sasAddress << dec << endl;
                            cout << "\t\tslot Number: " << edInfo.slotNumber << endl;
                        }
                        cout << "\tstate: " << diskstate[edInfo.state] << endl;
                        cout << "\tusage: " << diskusage[edInfo.usage] << endl;
                        cout << "\ttotalSize: " << edInfo.totalSize << endl;
                        cout << "\tblockSize: " << edInfo.blockSize << endl;
                        cout << "\tblocksTotal: " << edInfo.blocksTotal << endl;
                        cout << "\tblocksFree: " << edInfo.blocksFree << endl;
                        cout << "\tsystemDisk: " << edInfo.systemDisk << endl;
                        cout << "\tstoragePool: " << edInfo.storagePool << endl;
                        cout << "\twriteCachePolicy: " << edInfo.writeCachePolicy << endl;
                        cout << "\tlocateLEDSupport: " << edInfo.locateLEDSupport << endl;
                        cout << "\tisPreBootVisible: " << edInfo.isPreBootVisible << endl;
                        cout << "\tledState: " << edInfo.ledState << endl;
                        cout << "\tsystemIoBusNumber: " << edInfo.systemIoBusNumber << endl;
                        cout << "\tPCISlotNumber: " << edInfo.PCISlotNumber << endl;
                    } else {
                        cout << "E: unable to get end device info (status=" << statusStr[status] << ")" << endl;
                    }
                }
            } else {
                cout << "E: unable to get end device handles (status=" << statusStr[status]<< ")" << endl;
            }
            break;

    case oenclosure:
            SSI_EnclosureInfo enclosureInfo;
            count = MAX_COUNT;
            status = SsiGetEnclosureHandles(session, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handles, &count);
            if (status == SSI_StatusOk) {
            cout << "enclosures: " << count << endl;
            for (unsigned int i = 0; i < count; ++i) {
                cout << "handle=0x" << hex << handles[i] << dec << endl;
                status = SsiGetEnclosureInfo(session, handles[i], &enclosureInfo);
                if (status == SSI_StatusOk) {
                    cout << "\tUnique Id: " << hex << enclosureInfo.uniqueId << dec << endl;
                    cout << "\tenclosureKey: 0x" << hex <<enclosureInfo.enclosureKey << dec << endl;
                    cout << "\tvendorInfo: " << enclosureInfo.vendorInfo << endl;
                    cout << "\tproductId: " << enclosureInfo.productId << endl;
                    cout << "\tproductRev: " << enclosureInfo.productRev << endl;
                    cout << "\tlogicalId: " << enclosureInfo.logicalId << endl;
                    cout << "\tprocessorCount: " << enclosureInfo.processorCount << endl;
                    cout << "\tsubenclosureCount: " << enclosureInfo.subenclosureCount << endl;
                    cout << "\telementCount: " << enclosureInfo.elementCount << endl;
                    cout << "\tnumberOfSlots: " << enclosureInfo.numberOfSlots << endl;
                } else {
                    cout << "E: unable to get enclosure info (status=" << statusStr[status] << ")" << endl;
                }
            }
            } else {
            cout << "E: unable to get enclosure handles (status=" << statusStr[status] << ")" << endl;
            }
            break;

    case oroutingdevice:
            SSI_RoutingDeviceInfo rdInfo;
            count = MAX_COUNT;
            status = SsiGetRoutingDeviceHandles(session, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handles, &count);
            if (status == SSI_StatusOk) {
            cout << "routing devices: " << count << endl;
            for (unsigned int i = 0; i < count; ++i) {
                cout << "handle=0x" << hex << handles[i] << dec << endl;
                status = SsiGetRoutingDeviceInfo(session, handles[i], &rdInfo);
                if (status == SSI_StatusOk) {
                    cout << "\tUnique Id: " << hex << rdInfo.uniqueId << dec << endl;
                    cout << "\tAddress (scsi): " << (unsigned) rdInfo.routingDeviceAddress.scsiAddress.host
                         << ":"<< (unsigned) rdInfo.routingDeviceAddress.scsiAddress.bus
                         << ":"<< (unsigned) rdInfo.routingDeviceAddress.scsiAddress.target
                         << ":"<< (unsigned) rdInfo.routingDeviceAddress.scsiAddress.lun << endl;
                    cout << "\tAddress (sas): 0x" << hex << rdInfo.routingDeviceAddress.sasAddress << dec << endl;
                    cout << "\troutingDeviceType: " << rdtype[rdInfo.routingDeviceType] << endl;
                    cout << "\tenclosureHandle: 0x" << hex << rdInfo.enclosureHandle << dec << endl;
                    cout << "\tnumberPhys: " << rdInfo.numberPhys << endl;
                    cout << "\tvendorId: " << rdInfo.vendorId << endl;
                    cout << "\tproductId: " << rdInfo.productId << endl;
                    cout << "\tproductRev: " << rdInfo.productRev << endl;
                    cout << "\tcomponentVendorId: " << rdInfo.componentVendorId << endl;
                    cout << "\tcomponentId: " << rdInfo.componentId << endl;
                    cout << "\tcomponentRev: " << rdInfo.componentRev << endl;
                    cout << "\texpanderType: " << expandertype[rdInfo.expanderType] << endl;
                    cout << "\texpanderChangeCount: " << rdInfo.expanderChangeCount << endl;
                    cout << "\texpanderRouteIndexes: " << rdInfo.expanderRouteIndexes << endl;
                    cout << "\tselfConfiguring: " << rdInfo.selfConfiguring << endl;
                } else {
                    cout << "E: unable to get routing device info (status=" << statusStr[status] << ")" << endl;
                }
            }
            } else {
            cout << "E: unable to get routing device handles (status=" << statusStr[status] << ")" << endl;
            }
            break;

    default:
        cout << "invalid option" << endl;
        }
    }
/*
SSI_API SSI_Status SsiGetPortInfo(SSI_Handle session, SSI_Handle portHandle, SSI_PortInfo *info); tt
SSI_API SSI_Status SsiGetArrayInfo(SSI_Handle session, SSI_Handle arrayHandle, SSI_ArrayInfo *info); tt

*/

    status = SsiFinalize();
    if (status != SSI_StatusOk) {
        return -2;
    }
    return 0;
}

// ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=96 noexpandtab:
