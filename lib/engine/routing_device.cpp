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
#include "routing_device.h"
#include "port.h"
#include "session.h"
#include "enclosure.h"

/* */
RoutingDevice::RoutingDevice(const String &path)
    : StorageObject(0, path), m_pSubtractivePort(0)
{
}

/* */
RoutingDevice::~RoutingDevice()
{
}

/* */
void RoutingDevice::acquireId(Session *pSession)
{
    pSession->addRoutingDevice(this);
    Iterator<Object *> i;
    for (i = m_EndDevices_Direct; *i != 0; ++i) {
        dynamic_cast<StorageObject *>(*i)->acquireId(pSession);
    }
    for (i = m_Phys; *i != 0; ++i) {
        dynamic_cast<StorageObject *>(*i)->acquireId(pSession);
    }
    for (i = m_Ports; *i != 0; ++i) {
        dynamic_cast<StorageObject *>(*i)->acquireId(pSession);
    }
    for (i = m_RoutingDevices_Direct; *i != 0; ++i) {
        dynamic_cast<StorageObject *>(*i)->acquireId(pSession);
    }
    pSession->addPort(m_pSubtractivePort);
}

/* */
void RoutingDevice::getPhys(Container &container) const
{
    container = m_Phys;
}

/* */
void RoutingDevice::getEndDevices(Container &container, bool all) const
{
    container = m_EndDevices_Direct;
    if (all) {
        container.add(m_EndDevices);
    }
}

/* */
void RoutingDevice::getPorts(Container &container) const
{
    container = m_Ports;
    container.add(m_pSubtractivePort);
}

/* */
void RoutingDevice::getRoutingDevices(Container &container, bool all) const
{
    container = m_RoutingDevices_Direct;
    if (all) {
        container.add(m_RoutingDevices);
    }
}

/* */
RaidInfo * RoutingDevice::getRaidInfo() const
{
    return m_pSubtractivePort->getRaidInfo();
}

/* */
SSI_Status RoutingDevice::getInfo(SSI_RoutingDeviceInfo *pInfo) const
{
    if (pInfo == 0) {
        return SSI_StatusInvalidParameter;
    }
    pInfo->routingDeviceHandle = getId();
    pInfo->routingDeviceType = getRoutingDeviceType();
    getAddress(pInfo->routingDeviceAddress);

    Enclosure *pEnclosure = getEnclosure();
    if (pEnclosure != 0) {
        pInfo->enclosureHandle = pEnclosure->getId();
    } else {
        pInfo->enclosureHandle = SSI_NULL_HANDLE;
    }
    pInfo->numberPhys = getNumberOfPhys();

    m_ProductId.get(pInfo->productId,
        sizeof(pInfo->productId));

    m_Vendor.get(pInfo->vendorId,
        sizeof(pInfo->vendorId));

    m_ProductRev.get(pInfo->productRev,
        sizeof(pInfo->productRev));

    m_ComponentVendorId.get(pInfo->componentVendorId,
        sizeof(pInfo->componentVendorId));

    m_ComponentId.get(pInfo->componentId,
        sizeof(pInfo->componentId));

    m_ComponentRev.get(pInfo->componentRev,
        sizeof(pInfo->componentRev));

    pInfo->expanderType = getExpanderType();
    pInfo->selfConfiguring = SSI_FALSE;
    pInfo->expanderChangeCount = 0;
    pInfo->expanderRouteIndexes = 0;
    return SSI_StatusOk;
}

/* */
void RoutingDevice::attachArray(Object *pObject)
{
    m_pSubtractivePort->attachArray(pObject);
}

/* */
void RoutingDevice::attachVolume(Object *pObject)
{
    m_pSubtractivePort->attachVolume(pObject);
}

/* */
void RoutingDevice::attachPhy(Object *pPhy)
{
    m_Phys.add(pPhy);
}

/* */
void RoutingDevice::attachEndDevice(Object *pEndDevice)
{
    m_EndDevices_Direct.add(pEndDevice);
}

/* */
void RoutingDevice::attachPort(Object *pPort)
{
    m_Ports.add(pPort);
}

/* */
void RoutingDevice::attachRoutingDevice(Object *pRoutingDevice)
{
    m_RoutingDevices_Direct.add(pRoutingDevice);
    ScopeObject *pScopeObject = dynamic_cast<ScopeObject *>(pRoutingDevice);
    Container container;
    pScopeObject->getEndDevices(container, true);
    m_EndDevices.add(container);
    pScopeObject->getRoutingDevices(container, true);
    m_RoutingDevices.add(container);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
