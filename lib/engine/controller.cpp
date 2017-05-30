/*
Copyright (c) 2011 - 2017, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <asm/types.h>
#include <cstddef>
#include <typeinfo>
#include <orom/orom.h>
#include <vector>

#include "controller.h"
#include "filesystem.h"
#include "pci_header.h"
#include "raid_info.h"
#include "array.h"
#include "enclosure.h"
#include "routing_device.h"
#include "session.h"
#include "port.h"
#include "phy.h"

using std::vector;

using boost::shared_ptr;
using boost::dynamic_pointer_cast;
using boost::const_pointer_cast;

namespace
{
    String getVersionFromMdadm()
    {
        String version = "";
        String output;
        if (shell_output("mdadm --misc --detail-platform", output) == 0) {
            vector<String> lines;
            splitStringToLines(output, lines);

            size_t size = lines.size();
            for (size_t i = 0; i < size; ++i) {
                try {
                    lines[i].find("Version");
                    version = lines[i].after(":");
                    version.trim();
                    break;
                } catch (...) {
                    /* not found */
                }
            }
        }
        return version;
    }
} // <<anonymous>>

/* */
Controller::Controller(const String &path)
    : StorageObject(path),
      m_PciVendorId(0),
      m_PciDeviceId(0),
      m_SubSystemId(0),
      m_HardwareRevisionId(0),
      m_SubClassCode(0),
      m_SubVendorId(0),
      m_prgIface(0),
      m_ClassId(0),
      m_pRaidInfo(),
      m_twoTbVolumePrebootSupported(false),
      m_twoTbDiskPrebootSupported(false),
      m_ESATASpanning(false),
      m_NVSRAMSupported(false),
      m_HWXORSupported(false),
      m_PhyLocate(false),
      m_PhyRemoveDisk(false),
      m_DiskUnlock(false),
      m_PatrolReadSupport(false),
      m_ROHISupport(false),
      m_hardwareMode(SSI_Unknown_SKU),
      m_supportsTpv(false),
      m_trialState(SSI_Trial_State_Unknown),
      m_remainingDays(0)
{
    File attr;
    struct PCIHeader pciInfo;

    const String UnknownDriver = "UNKNOWN";
    try {
        m_DriverVersion = getVersionFromMdadm();

        if (m_DriverVersion == "") {
            m_DriverVersion = UnknownDriver;
        }
    } catch (...) {
        m_DriverVersion = UnknownDriver;
    }

    try {
        attr = m_Path + "/config";
        attr.read(&pciInfo, sizeof(struct PCIHeader));
        m_PciVendorId = pciInfo.vendorId;
        m_PciDeviceId = pciInfo.deviceId;
        m_SubSystemId = pciInfo.subSystemId;
        m_HardwareRevisionId = pciInfo.revisionId;
        m_SubClassCode = pciInfo.subClassId;
        m_SubVendorId = pciInfo.subSystemVendorId;
        m_prgIface = pciInfo.prgIface;
        m_ClassId = pciInfo.classId;

    } catch (...) {
        /* TODO: log that PCI header cannot be read from sysfs. */
    }
    struct orom_info_ext *pInfo_ext = orom_get(m_PciDeviceId);
    struct orom_info *pInfo = &pInfo_ext->data;

    if (pInfo != NULL) {
        m_PrebootMgrVersion =
            String(pInfo->prod_ver.major) + String(".") +
            String(pInfo->prod_ver.minor) + String(".") +
            String(pInfo->prod_ver.hotfix) + String(".") +
            String(pInfo->prod_ver.build);
        m_twoTbVolumePrebootSupported = pInfo->a_2tb_vol;
        m_twoTbDiskPrebootSupported = pInfo->a_2tb_disk;
        m_ESATASpanning = pInfo->c_esata;
        m_NVSRAMSupported = pInfo->a_nvm;
        m_HWXORSupported = pInfo->f_hardware_xor;
        m_PhyLocate = pInfo->f_led_locate;
        m_DiskUnlock = pInfo->c_hdd_passwd;
        m_PatrolReadSupport = pInfo->f_read_patrol;
    }
}

/* */
String Controller::getId() const
{
    return "co:" + getPartId();
}

String Controller::getPartId() const
{
    return String(m_PciDeviceId);
}

/* */
shared_ptr<RaidInfo> Controller::findRaidInfo(Container<RaidInfo>&)
{
    return shared_ptr<RaidInfo>();
}

/* */
SSI_Status Controller::getInfo(SSI_ControllerInfo *pInfo) const
{
    if (pInfo == NULL) {
        return SSI_StatusInvalidParameter;
    }
    pInfo->controllerHandle = getHandle();
    getId().get(pInfo->uniqueId, sizeof(pInfo->uniqueId));
    getAddress(pInfo->controllerAddress);

    m_Name.get(pInfo->controllerName, sizeof(pInfo->controllerName));

    pInfo->controllerType = getControllerType();
    if (m_pRaidInfo != NULL) {
        pInfo->raidInfoHandle = m_pRaidInfo->getHandle();
    } else {
        pInfo->raidInfoHandle = SSI_NULL_HANDLE;
    }

    m_DriverVersion.get(pInfo->driverVer, sizeof(pInfo->driverVer));

    m_PrebootMgrVersion.get(pInfo->prebootManagerVer, sizeof(pInfo->prebootManagerVer));

    pInfo->hardwareVer.vendorId = m_PciVendorId;
    pInfo->hardwareVer.deviceId = m_PciDeviceId;
    pInfo->hardwareVer.subSystemId = m_SubSystemId;
    pInfo->hardwareVer.hardwareRevisionId = m_HardwareRevisionId;
    pInfo->hardwareVer.subClassCode = m_SubClassCode;
    pInfo->hardwareVer.subVendorId = m_SubVendorId;

    pInfo->prebootManagerLoaded = m_PrebootMgrVersion ? SSI_TRUE : SSI_FALSE;
    pInfo->twoTbVolumePrebootSupport = m_twoTbVolumePrebootSupported ? SSI_TRUE : SSI_FALSE;
    pInfo->twoTbDiskPrebootSupport = m_twoTbDiskPrebootSupported ? SSI_TRUE : SSI_FALSE;
    pInfo->disableESataSpanning = m_ESATASpanning ? SSI_TRUE : SSI_FALSE;
    pInfo->xorSupported = m_HWXORSupported ? SSI_TRUE : SSI_FALSE;
    pInfo->nvsramSupported = m_NVSRAMSupported ? SSI_FALSE : SSI_FALSE;
    pInfo->phyLocateSupport = m_PhyLocate ? SSI_TRUE : SSI_FALSE;
    pInfo->phyRemoveDiskSupport = m_PhyRemoveDisk ? SSI_TRUE : SSI_FALSE;
    pInfo->diskUnlockSupport = m_DiskUnlock ? SSI_TRUE : SSI_FALSE;
    pInfo->assignStoragePoolSupport = SSI_FALSE;
    pInfo->markAsNormalSupport = SSI_FALSE;
    pInfo->volumeDeleteSupport = SSI_TRUE;
    pInfo->arrayCreateSupport = SSI_TRUE;
    pInfo->volumeModifySupport = SSI_TRUE;
    pInfo->volumeRenameSupport = SSI_TRUE;
    pInfo->addDisksToArraySupport = SSI_TRUE;
    pInfo->volumeCancelVerifySupport = SSI_TRUE;
    pInfo->markAsSpareSupport = SSI_TRUE;
    pInfo->readPatrolEnabled = SSI_TRUE;
    pInfo->readPatrolSupport = m_PatrolReadSupport ? SSI_TRUE : SSI_FALSE;
    pInfo->rohiEnabled = SSI_FALSE;
    pInfo->rohiSupport = m_ROHISupport ? SSI_TRUE : SSI_FALSE;

    pInfo->hardwareSkuMode = m_hardwareMode;
    pInfo->supportsTPV = m_supportsTpv ? SSI_TRUE : SSI_FALSE;

    pInfo->trialState = m_trialState;
    pInfo->remainingDays = m_remainingDays;

    return SSI_StatusOk;
}

SSI_Status Controller::makeSpare(const shared_ptr<EndDevice>& pEndDevice)
{
    foreach (i, m_Arrays) {
        if ((*i)->addSpare(pEndDevice) == SSI_StatusOk)
            return SSI_StatusOk;
    }
    shared_ptr<Array> pArray;
    Container<EndDevice> container;
    container.add(pEndDevice);
    try {
       pArray = shared_ptr<Array>(new Array());
    } catch (...) {
        return SSI_StatusInsufficientResources;
    }

    try {
        pArray->setEndDevices(container);
    } catch (...) {
        return SSI_StatusInvalidState;
    }

    try {
        pArray->create();
    } catch (...) {
        return SSI_StatusFailed;
    }

    return SSI_StatusOk;
}

/* */
void Controller::getPhys(Container<Phy> &container) const
{
    container = m_Phys;
}

/* */
void Controller::getEnclosures(Container<Enclosure> &container, bool all) const
{
    if (all) {
        container = m_Enclosures_Direct;
        return;
    }
    foreach (i, m_Enclosures_Direct)
        if ((*i)->attachedTo(const_pointer_cast<Controller>(shared_from_this()))) {
            container.add(*i);
        }
}

/* */
void Controller::getPorts(Container<Port> &container) const
{
    container = m_Ports;
}

/* */
void Controller::getEndDevices(Container<EndDevice> &container, bool all) const
{
    container = m_EndDevices_Direct;
    if (all) {
        container.add(m_EndDevices);
    }
}

/* */
void Controller::getRoutingDevices(Container<RoutingDevice> &container, bool all) const
{
    container = m_RoutingDevices_Direct;
    if (all) {
        container.add(m_RoutingDevices);
    }
}

/* */
void Controller::getArrays(Container<Array> &container) const
{
    container = m_Arrays;
}

/* */
void Controller::getVolumes(Container<Volume> &container) const
{
    container = m_Volumes;
}

/* */
SSI_Status Controller::readPatrolSetState(bool)
{
    return SSI_StatusNotSupported;
}

/* */
bool Controller::operator ==(const Object &object) const
{
    return typeid(*this) == typeid(object) &&
        static_cast<const Controller *>(&object)->m_Path == m_Path;
}

/* */
void Controller::attachEndDevice(const shared_ptr<EndDevice>& pEndDevice)
{
    m_EndDevices_Direct.add(pEndDevice);
}

/* */
void Controller::attachRoutingDevice(const shared_ptr<RoutingDevice>& pRoutingDevice)
{
    m_RoutingDevices_Direct.add(pRoutingDevice);
    shared_ptr<ScopeObject> pScopeObject = dynamic_pointer_cast<ScopeObject>(pRoutingDevice);
    Container<EndDevice> endDevices;
    pScopeObject->getEndDevices(endDevices, true);
    m_EndDevices.add(endDevices);
    Container<RoutingDevice> routingDevices;
    pScopeObject->getRoutingDevices(routingDevices, true);
    m_RoutingDevices.add(routingDevices);
}

/* */
void Controller::attachPort(const shared_ptr<Port>& pPort)
{
    m_Ports.add(pPort);
}

/* */
void Controller::attachVolume(const shared_ptr<Volume>& pVolume)
{
    foreach (i, m_Volumes) {
        if (*i == pVolume) {
            return;
        }
    }

    m_Volumes.add(pVolume);
}

/* */
void Controller::attachPhy(const shared_ptr<Phy>& pPhy)
{
    m_Phys.add(pPhy);
}

/* */
void Controller::attachArray(const shared_ptr<Array>& pArray)
{
    foreach (i, m_Arrays) {
        if (*i == pArray) {
            return;
        }
    }

    m_Arrays.add(pArray);
}

/* */
void Controller::attachEnclosure(const shared_ptr<Enclosure>& pEnclosure)
{
    foreach (i, m_Enclosures_Direct) {
        if (pEnclosure == *i) {
            if (shared_ptr<StorageObject> parent = pEnclosure->getParent().lock()) {
                shared_ptr<RoutingDevice> pRoutingDevice = dynamic_pointer_cast<RoutingDevice>(parent);
                (*i)->attachRoutingDevice(pRoutingDevice);
                pRoutingDevice->setEnclosure(*i);
                return;
            }
        }
    }

    m_Enclosures_Direct.add(pEnclosure);
}

/* */
void Controller::getEnclosures(const shared_ptr<RoutingDevice>& pRoutingDevice, Container<Enclosure> &container)
{
    foreach (i, m_Enclosures_Direct) {
        if (pRoutingDevice->getEnclosure() != *i && (*i)->attachedTo(pRoutingDevice)) {
            container.add(*i);
        }
    }
}

/* */
void Controller::addToSession(const shared_ptr<Session>& pSession)
{
    pSession->addController(shared_from_this());

    foreach (i, m_EndDevices_Direct) {
        (*i)->addToSession(pSession);
    }
    foreach (i, m_RoutingDevices_Direct) {
        (*i)->addToSession(pSession);
    }
    foreach (i, m_Ports) {
        (*i)->addToSession(pSession);
    }
    foreach (i, m_Phys) {
        (*i)->addToSession(pSession);
    }
    foreach (i, m_Arrays) {
        (*i)->addToSession(pSession);
    }
    foreach (i, m_Enclosures_Direct) {
        (*i)->addToSession(pSession);
    }
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
