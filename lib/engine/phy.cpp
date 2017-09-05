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

#include "phy.h"
#include "port.h"
#include "controller.h"
#include "end_device.h"
#include "routing_device.h"
#include "session.h"
#include "filesystem.h"

using boost::shared_ptr;

/* */
Phy::Phy(const String &path, unsigned int number, const Parent& pParent)
    : StorageObject(path, pParent), m_pRemotePhy(), m_pPort(), m_Number(number)
{
    setProperties();
}

/* */
String Phy::getId() const
{
    if (Parent parent = m_pParent.lock()) {
        return "ph:" + parent->getPartId() + "/" + String(m_Number);
    } else {
        return "ph:" + String(getHandle());
    }
}

/* */
SSI_Status Phy::getInfo(SSI_PhyInfo *pInfo) const
{
    if (pInfo == NULL) {
        return SSI_StatusInvalidParameter;
    }

    pInfo->phyHandle = getHandle();
    getId().get(pInfo->uniqueId, sizeof(pInfo->uniqueId));
    pInfo->phyNumber = m_Number;
    pInfo->protocol = m_Protocol;
    if (shared_ptr<Port> port = m_pPort.lock()) {
        pInfo->associatedPort = port->getHandle();
    } else {
        pInfo->associatedPort = SSI_NULL_HANDLE;
    }

    if (Parent parent = m_pParent.lock()) {
        parent->getAddress(pInfo->phyAddress);
        if (dynamic_cast<Controller*>(parent.get())) {
            pInfo->deviceType = SSI_DeviceTypeController;
        } else if (dynamic_cast<EndDevice *>(parent.get())) {
            pInfo->deviceType = SSI_DeviceTypeEndDevice;
        } else if (dynamic_cast<RoutingDevice *>(parent.get())) {
            pInfo->deviceType = SSI_DeviceTypeRoutingDevice;
        } else {
            pInfo->deviceType = SSI_DeviceTypeUnknown;
        }
        pInfo->deviceHandle = parent->getHandle();

        if (dynamic_cast<EndDevice *>(parent.get())) {
            fetchSpeeds(pInfo);
        } else {
            setSpeeds(pInfo);
        }
    } else {
        pInfo->deviceType = SSI_DeviceTypeUnknown;
        pInfo->deviceHandle = SSI_NULL_HANDLE;
        setSpeeds(pInfo);
    }

    pInfo->isExternal = SSI_FALSE;
    pInfo->hotPlugCap = SSI_FALSE;
    pInfo->countsValid = SSI_FALSE;

    return SSI_StatusOk;
}

/* */
SSI_Status Phy::locate(bool mode) const
{
    if (Parent parent = m_pParent.lock()) {
        return parent->locate(mode);
    } else {
        return SSI_StatusInvalidState;
    }
}

/* */
SSI_Status Phy::remove() const
{
    if (Parent parent = m_pParent.lock()) {
        return parent->removeDisk();
    } else {
        return SSI_StatusInvalidState;
    }
}

/* */
void Phy::fetchSpeeds(SSI_PhyInfo *pInfo) const
{
    if (shared_ptr<Phy> remote = m_pRemotePhy.lock()) {
        remote->setSpeeds(pInfo);
        return;
    }

    if (shared_ptr<Port> port = m_pPort.lock()) {
        if (shared_ptr<Port> pPort = port->getRemotePort()) {
            Container<Phy> container;
            pPort->getPhys(container);
            foreach (i, container) {
                (*i)->setSpeeds(pInfo);
            }
        }
    }
}

/* */
void Phy::setSpeeds(SSI_PhyInfo *pInfo) const
{
    pInfo->minHWLinkSpeed = m_minHWLinkSpeed;
    pInfo->maxHWLinkSpeed = m_maxHWLinkSpeed;
    pInfo->minLinkSpeed = m_minLinkSpeed;
    pInfo->maxLinkSpeed = m_maxLinkSpeed;
    pInfo->negotiatedLinkSpeed = m_negotiatedLinkSpeed;
}

/* */
void Phy::setProperties()
{
    Path tmp = m_Path.reverse_left("/");
    Directory dir;
    std::list<Directory *> dirs;

    m_Protocol = SSI_PhyProtocolUnknown;
    m_minHWLinkSpeed = SSI_PhySpeedUnknown;
    m_maxHWLinkSpeed = SSI_PhySpeedUnknown;
    m_minLinkSpeed = SSI_PhySpeedUnknown;
    m_maxLinkSpeed = SSI_PhySpeedUnknown;
    m_negotiatedLinkSpeed = SSI_PhySpeedUnknown;

    if (Parent parent = m_pParent.lock()) {
        if (dynamic_cast<EndDevice *>(parent.get())) {
            tmp = tmp.reverse_left("/");
            dir = tmp + "sas_device";
            dirs = dir.dirs();
            foreach (i, dirs) {
                try {
                    File attr;
                    String protocol;
                    attr = *(*i) + "target_port_protocols";
                    attr >> protocol;
                    m_Protocol = __internal_parse_protocol(protocol);
                } catch (...) {
                    /* TODO: report read failure of attribtue. */
                }
            }
        } else if (dynamic_cast<Controller *>(parent.get()) || dynamic_cast<RoutingDevice *>(parent.get())) {
            dir = m_Path + "/sas_phy";
            dirs = dir.dirs();
            foreach (i, dirs) {
                File attr;
                String linkrate;
                try {
                    String protocol;
                    attr = *(*i) + "target_port_protocols";
                    attr >> protocol;
                    m_Protocol = __internal_parse_protocol(protocol);
                } catch (...) {
                }
                try {
                    attr = *(*i) + "maximum_linkrate";
                    attr >> linkrate;
                    m_maxLinkSpeed = __internal_parse_linkrate(linkrate);
                } catch (...) {
                }
                try {
                    attr = *(*i) + "maximum_linkrate_hw";
                    attr >> linkrate;
                    m_maxHWLinkSpeed = __internal_parse_linkrate(linkrate);
                } catch (...) {
                }
                try {
                    attr = *(*i) + "minimum_linkrate";
                    attr >> linkrate;
                    m_minLinkSpeed = __internal_parse_linkrate(linkrate);
                } catch (...) {
                }
                try {
                    attr = *(*i) + "minimum_linkrate_hw";
                    attr >> linkrate;
                    m_minHWLinkSpeed = __internal_parse_linkrate(linkrate);
                } catch (...) {
                }
                try {
                    attr = *(*i) + "negotiated_linkrate";
                    attr >> linkrate;
                    m_negotiatedLinkSpeed = __internal_parse_linkrate(linkrate);
                } catch (...) {
                }
                try {
                    unsigned long long sasAddress;
                    attr = *(*i) + "sas_address";
                    attr >> sasAddress;
                    if (sasAddress) {
                        SSI_Address address;
                        address.sasAddressPresent = SSI_TRUE;
                        address.sasAddress = sasAddress;
                        parent->setAddress(address);
                    }
                } catch (...) {
                }
            }
        }
    }
}

/* */
SSI_PhyProtocol Phy::__internal_parse_protocol(const String &protocol)
{
    try {
        protocol.find("sata");
        return SSI_PhyProtocolSATA;
    } catch (...) {
    }
    try {
        protocol.find("ssp");
        return SSI_PhyProtocolSSP;
    } catch (...) {
    }
    try {
        protocol.find("smp");
        return SSI_PhyProtocolSMP;
    } catch (...) {
    }
    try {
        protocol.find("stp");
        return SSI_PhyProtocolSTP;
    } catch (...) {
    }
    return SSI_PhyProtocolUnknown;
}

/* */
SSI_PhySpeed Phy::__internal_parse_linkrate(const String &linkrate)
{
    try {
        linkrate.find("1.5");
        return SSI_PhySpeed_1_5_GEN1;
    } catch (...) {
    }
    try {
        linkrate.find("3.0");
        return SSI_PhySpeed_3_0_GEN2;
    } catch (...) {
    }
    try {
        linkrate.find("6.0");
        return SSI_PhySpeed_6_0_GEN3;
    } catch (...) {
    }
    return SSI_PhySpeedUnknown;
}

/* */
void Phy::addToSession(const shared_ptr<Session>& pSession)
{
    pSession->addPhy(shared_from_this());
}

/* */
bool Phy::operator ==(const Object &object) const
{
    const Phy *pPhy = dynamic_cast<const Phy *>(&object);
    if (pPhy != NULL) {
        Parent parent = m_pParent.lock();
        Parent phyParent = pPhy->m_pParent.lock();
        return parent && phyParent && *parent == *phyParent && m_Number == pPhy->m_Number;
    } else {
        return false;
    }
}

/* */
void Phy::attachPhy(const shared_ptr<Phy>& pPhy)
{
    if (pPhy.get() != this) {
        m_pRemotePhy = pPhy;
        if (!pPhy) {
            throw E_NULL_POINTER;
        }
        if (shared_ptr<Phy> remote = m_pRemotePhy.lock()) {
            remote->attachPhy(shared_from_this());
        }
    }
}

/* */
void Phy::attachPort(const shared_ptr<Port>& pPort)
{
    if (!pPort) {
        throw E_NULL_POINTER;
    }
    m_pPort = pPort;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
