
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
#include "enclosure.h"
#include "port.h"
#include "session.h"

/* */
Port::Port(const String &path)
    : StorageObject(path), m_pRemotePort(0)
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
void Port::getPhys(Container<Phy> &container) const
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
void Port::attachArray(Array *pArray)
{
    m_pParent->attachArray(pArray);
}

/* */
void Port::attachVolume(Volume *pVolume)
{
    m_pParent->attachVolume(pVolume);
}

/* */
void Port::attachEndDevice(EndDevice *pEndDevice)
{
    m_pParent->attachEndDevice(pEndDevice);
}

/* */
void Port::attachRoutingDevice(RoutingDevice *pRoutingDevice)
{
    m_pParent->attachRoutingDevice(pRoutingDevice);
}

/* */
void Port::attachEnclosure(Enclosure *pEnclosure)
{
    RoutingDevice *pRoutingDevice = dynamic_cast<RoutingDevice *>(m_pParent);
    pEnclosure->attachRoutingDevice(pRoutingDevice);
    pRoutingDevice->setEnclosure(pEnclosure);
    pRoutingDevice->attachEnclosure(pEnclosure);
}

/* */
void Port::attachPhy(Phy *pPhy)
{
    m_Phys.add(pPhy);
    pPhy->attachPort(this);
}

/* */
void Port::attachPort(Port *pPort)
{
    if (pPort == this) {
        throw E_INVALID_OBJECT;
    }
    m_pRemotePort = pPort;
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
