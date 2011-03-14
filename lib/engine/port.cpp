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
#include "phy.h"
#include "storage_device.h"
#include "end_device.h"
#include "routing_device.h"
#include "port.h"
#include "session.h"

/* */
Port::Port(StorageObject *pParent, const String &path)
    : StorageObject(pParent, path), m_pRemotePort(0)
{
}

/* */
Port::~Port()
{
}

/* */
SSI_Status Port::getInfo(SSI_PortInfo *pInfo) const
{
    if (pInfo == 0) {
        return SSI_StatusInvalidParameter;
    }
    pInfo->portHandle = getId();
    m_pParent->getAddress(pInfo->portAddress);
    pInfo->numPhys = m_Phys.count();
    switch (m_pParent->getType()) {
    case ObjectType_Controller:
        pInfo->localDeviceType = SSI_DeviceTypeController;
        break;
    case ObjectType_EndDevice:
        pInfo->localDeviceType = SSI_DeviceTypeEndDevice;
        break;
    case ObjectType_RoutingDevice:
        pInfo->localDeviceType = SSI_DeviceTypeRoutingDevice;
        break;
    default:
        pInfo->localDeviceType = SSI_DeviceTypeUnknown;
    }
    pInfo->localDeviceHandle = m_pParent->getId();
    if (m_pRemotePort != 0) {
        pInfo->connectedToPort = m_pRemotePort->getId();
    } else {
        pInfo->connectedToPort = SSI_NULL_HANDLE;
    }
    return SSI_StatusOk;
}

/* */
void Port::getPhys(Container &container) const
{
    container = m_Phys;
}

/* */
bool Port::equal(const Object *pObject) const
{
    const Port *pPort = dynamic_cast<const Port *>(pObject);
    return Object::equal(pPort) &&
        m_pParent->equal(pPort->m_pParent) && m_Path == pPort->m_Path;
}

/* */
void Port::attachArray(Object *pArray)
{
    m_pParent->attachArray(pArray);
}

/* */
void Port::attachVolume(Object *pVolume)
{
    m_pParent->attachVolume(pVolume);
}

/* */
void Port::attachEndDevice(Object *pEndDevice)
{
    m_pParent->attachEndDevice(pEndDevice);
}

/* */
void Port::attachRoutingDevice(Object *pRoutingDevice)
{
    m_pParent->attachRoutingDevice(pRoutingDevice);
}

/* */
void Port::attachPhy(Object *pPhy)
{
    m_Phys.add(pPhy);
    dynamic_cast<Phy *>(pPhy)->attachPort(this);
}

/* */
void Port::attachPort(Object *pPort)
{
    if (pPort == this) {
        throw E_INVALID_OBJECT;
    }
    m_pRemotePort = dynamic_cast<Port *>(pPort);
    m_pRemotePort->attachPort(this);
}

/* */
void Port::acquireId(Session *pSession)
{
    pSession->addPort(this);
}

/* */
RaidInfo * Port::getRaidInfo() const
{
    return m_pParent->getRaidInfo();
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
