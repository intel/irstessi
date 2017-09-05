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

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "port.h"
#include "controller.h"
#include "end_device.h"
#include "routing_device.h"
#include "enclosure.h"
#include "phy.h"
#include "session.h"

using boost::shared_ptr;
using boost::dynamic_pointer_cast;

/* */
Port::Port(const String &path)
    : StorageObject(path), m_pRemotePort()
{

}

String Port::getId() const
{
    if (Parent parent = m_pParent.lock()) {
        return "po:" + parent->getPartId() + "/" + String(getHandle()); //TODO: Some kind of stable id
    } else {
        return "po:" + String(getHandle());
    }
}

/* */
SSI_Status Port::getInfo(SSI_PortInfo *pInfo) const
{
    if (pInfo == NULL) {
        return SSI_StatusInvalidParameter;
    }
    pInfo->portHandle = getHandle();
    getId().get(pInfo->uniqueId, sizeof(pInfo->uniqueId));
    pInfo->numPhys = m_Phys.size();

    if (Parent parent = m_pParent.lock()) {
        parent->getAddress(pInfo->portAddress);
        if (dynamic_cast<Controller *>(parent.get())) {
            pInfo->localDeviceType = SSI_DeviceTypeController;
        } else if (dynamic_cast<EndDevice *>(parent.get())) {
            pInfo->localDeviceType = SSI_DeviceTypeEndDevice;
        } else if (dynamic_cast<RoutingDevice *>(parent.get())) {
            pInfo->localDeviceType = SSI_DeviceTypeRoutingDevice;
        } else {
            pInfo->localDeviceType = SSI_DeviceTypeUnknown;
        }

        pInfo->localDeviceHandle = parent->getHandle();
    } else {
        pInfo->localDeviceType = SSI_DeviceTypeUnknown;
        pInfo->localDeviceHandle = SSI_NULL_HANDLE;
    }

    if (shared_ptr<Port> remote = m_pRemotePort.lock()) {
        pInfo->connectedToPort = remote->getHandle();
    } else {
        pInfo->connectedToPort = SSI_NULL_HANDLE;
    }

    return SSI_StatusOk;
}

SSI_Status Port::locate(bool mode) const
{
    if (Parent parent = m_pParent.lock()) {
        return parent->locate(mode);
    } else {
        return SSI_StatusInvalidState;
    }
}

/* */
void Port::getPhys(Container<Phy> &container) const
{
    container = m_Phys;
}

/* */
bool Port::operator ==(const Object &object) const
{
    const Port *pPort = dynamic_cast<const Port *>(&object);
    if (pPort != NULL) {
        Parent parent = m_pParent.lock();
        Parent portParent = pPort->m_pParent.lock();
        return parent && portParent && *parent == *portParent && m_Path == pPort->m_Path;
    } else {
        return false;
    }
}

/* */
void Port::attachArray(const shared_ptr<Array>& pArray)
{
    if (Parent parent = m_pParent.lock()) {
        parent->attachArray(pArray);
    }
}

/* */
void Port::attachVolume(const shared_ptr<Volume>& pVolume)
{
    if (Parent parent = m_pParent.lock()) {
        parent->attachVolume(pVolume);
    }
}

/* */
void Port::attachEndDevice(const shared_ptr<EndDevice>& pEndDevice)
{
    if (Parent parent = m_pParent.lock()) {
        parent->attachEndDevice(pEndDevice);
    }
}

/* */
void Port::attachRoutingDevice(const shared_ptr<RoutingDevice>& pRoutingDevice)
{
    if (Parent parent = m_pParent.lock()) {
        parent->attachRoutingDevice(pRoutingDevice);
    }
}

/* */
void Port::attachEnclosure(const shared_ptr<Enclosure>& pEnclosure)
{
    if (Parent parent = m_pParent.lock()) {
        if (shared_ptr<RoutingDevice> pRoutingDevice = dynamic_pointer_cast<RoutingDevice>(parent)) {
            pEnclosure->attachRoutingDevice(pRoutingDevice);
            pRoutingDevice->setEnclosure(pEnclosure);
            pRoutingDevice->attachEnclosure(pEnclosure);
        }
    }
}

/* */
void Port::attachPhy(const shared_ptr<Phy>& pPhy)
{
    m_Phys.add(pPhy);
    pPhy->attachPort(shared_from_this());
}

/* */
void Port::attachPort(const shared_ptr<Port>& pPort)
{
    if (pPort.get() == this) {
        throw E_INVALID_OBJECT;
    }

    m_pRemotePort = pPort;
    pPort->attachPort(shared_from_this());
}

/* */
void Port::addToSession(const shared_ptr<Session>& pSession)
{
    pSession->addPort(shared_from_this());
}

/* */
shared_ptr<RaidInfo> Port::getRaidInfo() const
{
    if (Parent parent = m_pParent.lock()) {
        return parent->getRaidInfo();
    }

    return shared_ptr<RaidInfo>();
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
