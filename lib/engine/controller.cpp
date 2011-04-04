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

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <features.h>

#include <ssi.h>

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

/* */
void Controller::getPhys(Container<Phy> &container) const
{
    container = m_Phys;
}

/* */
void Controller::getEnclosures(Container<Enclosure> &container, bool all) const
{
    container = m_Enclosures_Direct;
    if (all) {
        container.add(m_Enclosures);
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
