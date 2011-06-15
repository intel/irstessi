
/*
Copyright (c) 2011, Intel Corporation
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

#include <features.h>
#include <asm/types.h>

#include <ssi.h>
#include <orom/orom.h>

#include "exception.h"
#include "list.h"
#include "container.h"
#include "string.h"
#include "object.h"
#include "raid_info.h"
#include "cache.h"
#include "controller.h"
#include "storage_device.h"
#include "raid_device.h"
#include "end_device.h"
#include "routing_device.h"
#include "phy.h"
#include "volume.h"
#include "port.h"
#include "enclosure.h"
#include "array.h"
#include "session.h"
#include "filesystem.h"
#include "pci_header.h"

/* */
Controller::Controller(const String &path)
    : StorageObject(path),
      m_PciVendorId(0),
      m_PciDeviceId(0),
      m_SubSystemId(0),
      m_HardwareRevisionId(0),
      m_SubClassCode(0),
      m_SubVendorId(0),
      m_pRaidInfo(0),
      m_twoTbVolumePrebootSupported(false),
      m_twoTbDiskPrebootSupported(false),
      m_ESATASpanning(false),
      m_NVSRAMSupported(false),
      m_HWXORSupported(false),
      m_PhyLocate(false),
      m_DiskUnlock(false),
      m_PatrolReadSupport(false)
{
    SysfsAttr attr;
    struct PCIHeader pciInfo;

    try {
        attr = m_Path + "/driver/module/version";
        attr >> m_DriverVersion;
    } catch (...) {
        /* TODO: log that version of the driver cannot be determined. */
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
    } catch (...) {
        /* TODO: log that PCI header cannot be read from sysfs. */
    }
    struct orom_info *pInfo = orom_get(m_PciDeviceId);
    if (pInfo != 0) {
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
Controller::~Controller()
{
}

/* */
SSI_Status Controller::getInfo(SSI_ControllerInfo *pInfo) const
{
    if (pInfo == 0) {
        return SSI_StatusInvalidParameter;
    }
    pInfo->controllerHandle = getId();
    getAddress(pInfo->controllerAddress);

    m_Name.get(pInfo->controllerName, sizeof(pInfo->controllerName));

    pInfo->controllerType = getControllerType();
    if (m_pRaidInfo != 0) {
        pInfo->raidInfoHandle = m_pRaidInfo->getId();
    } else {
        pInfo->raidInfoHandle = SSI_NULL_HANDLE;
    }

    m_DriverVersion.get(pInfo->driverVer,
        sizeof(pInfo->driverVer));

    m_PrebootMgrVersion.get(pInfo->prebootManagerVer,
        sizeof(pInfo->prebootManagerVer));

    pInfo->hardwareVer.vendorId = m_PciVendorId;
    pInfo->hardwareVer.deviceId = m_PciDeviceId;
    pInfo->hardwareVer.subSystemId = m_SubSystemId;
    pInfo->hardwareVer.hardwareRevisionId = m_HardwareRevisionId;
    pInfo->hardwareVer.subClassCode = m_SubClassCode;
    pInfo->hardwareVer.subVendorId = m_SubVendorId;

    pInfo->prebootManagerLoaded =
        m_PrebootMgrVersion ? SSI_TRUE : SSI_FALSE;
    pInfo->twoTbVolumePrebootSupport =
        m_twoTbVolumePrebootSupported ? SSI_TRUE : SSI_FALSE;
    pInfo->twoTbDiskPrebootSupport =
        m_twoTbDiskPrebootSupported ? SSI_TRUE : SSI_FALSE;
    pInfo->disableESataSpanning =
        m_ESATASpanning ? SSI_TRUE : SSI_FALSE;
    pInfo->xorSupported =
        m_HWXORSupported ? SSI_TRUE : SSI_FALSE;
    pInfo->nvsramSupported =
        m_NVSRAMSupported ? SSI_FALSE : SSI_FALSE;
    pInfo->phyLocateSupport =
        m_PhyLocate ? SSI_TRUE : SSI_FALSE;
    pInfo->diskUnlockSupport =
        m_DiskUnlock ? SSI_TRUE : SSI_FALSE;
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
    pInfo->readPatrolSupport =
        m_PatrolReadSupport ? SSI_TRUE : SSI_FALSE;

    return SSI_StatusOk;
}

SSI_Status Controller::makeSpare(EndDevice *pEndDevice)
{
    for (Iterator<Array *> i = m_Arrays; *i != 0; i++) {
        if ((*i)->addSpare(pEndDevice) == SSI_StatusOk)
            return SSI_StatusOk;
    }
    Array *pArray = 0;
    Container<EndDevice> container;
    container.add(pEndDevice);
    try {
       pArray = new Array();
    } catch (...) {
        return SSI_StatusInsufficientResources;
    }
    try {
        pArray->setEndDevices(container);
    } catch (...) {
        delete pArray;
        return SSI_StatusInvalidState;
    }
    try {
        pArray->create();
    } catch (...) {
        delete pArray;
        return SSI_StatusFailed;
    }
    delete pArray;
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
    for (Iterator<Enclosure *> i = m_Enclosures_Direct; *i != 0; ++i)
        if ((*i)->attachedTo(const_cast<Controller *>(this)))
            container.add(*i);
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
SSI_Status Controller::readPatrolSetState(bool enable)
{
    (void)enable;
    return SSI_StatusOk;
}

/* */
bool Controller::equal(const Object *pObject) const
{
    return Object::equal(pObject) &&
        dynamic_cast<const Controller *>(pObject)->m_Path == m_Path;
}

/* */
void Controller::attachEndDevice(EndDevice *pEndDevice)
{
    m_EndDevices_Direct.add(pEndDevice);
}

/* */
void Controller::attachRoutingDevice(RoutingDevice *pRoutingDevice)
{
    m_RoutingDevices_Direct.add(pRoutingDevice);
    ScopeObject *pScopeObject = dynamic_cast<ScopeObject *>(pRoutingDevice);
    Container<EndDevice> endDevices;
    pScopeObject->getEndDevices(endDevices, true);
    m_EndDevices.add(endDevices);
    Container<RoutingDevice> routingDevices;
    pScopeObject->getRoutingDevices(routingDevices, true);
    m_RoutingDevices.add(routingDevices);
}

/* */
void Controller::attachPort(Port *pPort)
{
    m_Ports.add(pPort);
}

/* */
void Controller::attachVolume(Volume *pVolume)
{
    Iterator<Volume *> i;
    for (i = m_Volumes; *i != 0; ++i) {
        if (*i == pVolume) {
            break;
        }
    }
    if (*i == 0) {
        m_Volumes.add(pVolume);
    }
}

/* */
void Controller::attachPhy(Phy *pPhy)
{
    m_Phys.add(pPhy);
}

/* */
void Controller::attachArray(Array *pArray)
{
    Iterator<Array *> i;
    for (i = m_Arrays; *i != 0; ++i) {
        if (*i == pArray) {
            break;
        }
    }
    if (*i == 0) {
        m_Arrays.add(pArray);
    }
}

/* */
void Controller::attachEnclosure(Enclosure *pEnclosure)
{
    for (Iterator<Enclosure *> i = m_Enclosures_Direct; *i != 0; i++)
        if (pEnclosure->equal(*i)) {
            RoutingDevice *pRoutingDevice = dynamic_cast<RoutingDevice *>(pEnclosure->getParent());
            (*i)->attachRoutingDevice(pRoutingDevice);
            pRoutingDevice->setEnclosure(*i);
            delete pEnclosure;
            return;
        }
    m_Enclosures_Direct.add(pEnclosure);
}

/* */
void Controller::acquireId(Session *pSession)
{
    pSession->addController(this);
    if (m_pRaidInfo != 0) {
        m_pRaidInfo->acquireId(pSession);
    }

    for (Iterator<EndDevice *> i = m_EndDevices_Direct; *i != 0; ++i) {
        (*i)->acquireId(pSession);
    }
    for (Iterator<RoutingDevice *> i = m_RoutingDevices_Direct; *i != 0; ++i) {
        (*i)->acquireId(pSession);
    }
    for (Iterator<Port *> i = m_Ports; *i != 0; ++i) {
        (*i)->acquireId(pSession);
    }
    for (Iterator<Phy *> i = m_Phys; *i != 0; ++i) {
        (*i)->acquireId(pSession);
    }
    for (Iterator<Array *> i = m_Arrays; *i != 0; ++i) {
        (*i)->acquireId(pSession);
    }
    for (Iterator<Enclosure *> i = m_Enclosures_Direct; *i != 0; ++i) {
        (*i)->acquireId(pSession);
    }
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
