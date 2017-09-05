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

#include "routing_device.h"
#include "end_device.h"
#include "phy.h"
#include "port.h"
#include "controller.h"
#include "routing_device.h"
#include "enclosure.h"
#include "session.h"

using boost::shared_ptr;
using boost::const_pointer_cast;
using boost::dynamic_pointer_cast;

/* */
RoutingDevice::RoutingDevice(const String &path)
    : StorageObject(path), m_pSubtractivePort()
{

}

/* */
String RoutingDevice::getId() const
{
    return "ro:" + getPartId();
}

/* */
String RoutingDevice::getPartId() const
{
    return m_ComponentId;
}

/* */
void RoutingDevice::discover()
{

}

/* */
void RoutingDevice::addToSession(const shared_ptr<Session>& pSession)
{
    pSession->addRoutingDevice(shared_from_this());
    foreach (i, m_EndDevices_Direct) {
        (*i)->addToSession(pSession);
    }

    foreach (i, m_Phys) {
        (*i)->addToSession(pSession);
    }

    foreach (i, m_Ports) {
        (*i)->addToSession(pSession);
    }

    foreach (i, m_RoutingDevices_Direct) {
        (*i)->addToSession(pSession);
    }

    pSession->addPort(m_pSubtractivePort);
}

/* */
void RoutingDevice::getPhys(Container<Phy> &container) const
{
    container = m_Phys;
}

/* */
void RoutingDevice::getEndDevices(Container<EndDevice> &container, bool all) const
{
    container = m_EndDevices_Direct;
    if (all) {
        container.add(m_EndDevices);
    }
}

/* */
void RoutingDevice::getPorts(Container<Port> &container) const
{
    container = m_Ports;
    container.add(m_pSubtractivePort);
}

/* */
void RoutingDevice::getRoutingDevices(Container<RoutingDevice> &container, bool all) const
{
    container = m_RoutingDevices_Direct;
    if (all) {
        container.add(m_RoutingDevices);
    }
}

/* */
void RoutingDevice::getEnclosures(Container<Enclosure> &container, bool) const
{
    if (shared_ptr<Controller> pController = getController()) {
        pController->getEnclosures(const_pointer_cast<RoutingDevice>(shared_from_this()), container);
    }
}

/* */
shared_ptr<RaidInfo> RoutingDevice::getRaidInfo() const
{
    return m_pSubtractivePort->getRaidInfo();
}

/* */
SSI_Status RoutingDevice::getInfo(SSI_RoutingDeviceInfo *pInfo) const
{
    if (pInfo == NULL) {
        return SSI_StatusInvalidParameter;
    }
    pInfo->routingDeviceHandle = getHandle();
    getId().get(pInfo->uniqueId, sizeof(pInfo->uniqueId));
    pInfo->routingDeviceType = getRoutingDeviceType();
    getAddress(pInfo->routingDeviceAddress);

    if (shared_ptr<Enclosure> pEnclosure = getEnclosure()) {
        pInfo->enclosureHandle = pEnclosure->getHandle();
    } else {
        pInfo->enclosureHandle = SSI_NULL_HANDLE;
    }
    pInfo->numberPhys = getNumberOfPhys();

    m_ProductId.get(pInfo->productId, sizeof(pInfo->productId));
    m_Vendor.get(pInfo->vendorId, sizeof(pInfo->vendorId));
    m_ProductRev.get(pInfo->productRev, sizeof(pInfo->productRev));
    m_ComponentVendorId.get(pInfo->componentVendorId, sizeof(pInfo->componentVendorId));
    m_ComponentId.get(pInfo->componentId, sizeof(pInfo->componentId));
    m_ComponentRev.get(pInfo->componentRev, sizeof(pInfo->componentRev));

    pInfo->expanderType = getExpanderType();
    pInfo->selfConfiguring = SSI_FALSE;
    pInfo->expanderChangeCount = 0;
    pInfo->expanderRouteIndexes = 0;
    return SSI_StatusOk;
}

/* */
void RoutingDevice::attachArray(const shared_ptr<Array>& pArray)
{
    m_pSubtractivePort->attachArray(pArray);
}

/* */
void RoutingDevice::attachVolume(const shared_ptr<Volume>& pVolume)
{
    m_pSubtractivePort->attachVolume(pVolume);
}

/* */
void RoutingDevice::attachPhy(const shared_ptr<Phy>& pPhy)
{
    m_Phys.add(pPhy);
}

/* */
void RoutingDevice::attachEndDevice(const shared_ptr<EndDevice>& pEndDevice)
{
    m_EndDevices_Direct.add(pEndDevice);
}

/* */
void RoutingDevice::attachPort(const shared_ptr<Port>& pPort)
{
    m_Ports.add(pPort);
}

/* */
void RoutingDevice::attachRoutingDevice(const shared_ptr<RoutingDevice>& pRoutingDevice)
{
    m_RoutingDevices_Direct.add(pRoutingDevice);
    if (shared_ptr<ScopeObject> pScopeObject = dynamic_pointer_cast<ScopeObject>(pRoutingDevice)) {
        Container<EndDevice> endDevices;
        pScopeObject->getEndDevices(endDevices, true);
        m_EndDevices.add(endDevices);
        Container<RoutingDevice> routingDevices;
        pScopeObject->getRoutingDevices(routingDevices, true);
        m_RoutingDevices.add(routingDevices);
    }
}

/* */
void RoutingDevice::attachEnclosure(const shared_ptr<Enclosure>& pEnclosure)
{
    if (Parent parent = m_pParent.lock()) {
        parent->attachEnclosure(pEnclosure);
    }
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
