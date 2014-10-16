
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
#include "container.h"
#include "string.h"
#include "object.h"
#include "phy.h"
#include "port.h"
#include "session.h"
#include "filesystem.h"
#include "utils.h"
#include "controller.h"
#include "end_device.h"
#include "routing_device.h"

#include "log/log.h"

/* */
Phy::Phy(const String &path, unsigned int number, StorageObject *pParent)
    : StorageObject(path, pParent), m_pRemotePhy(NULL), m_pPort(NULL), m_Number(number)
{
    setProperties();
}

/* */
Phy::~Phy()
{
}

/* */
SSI_Status Phy::getInfo(SSI_PhyInfo *pInfo) const
{
    if (pInfo == NULL) {
        return SSI_StatusInvalidParameter;
    }
    pInfo->phyHandle = getId();
    m_pParent->getAddress(pInfo->phyAddress);
    pInfo->phyNumber = m_Number;
    pInfo->protocol = m_Protocol;
    if (m_pPort != NULL) {
        pInfo->associatedPort = m_pPort->getId();
    } else {
        pInfo->associatedPort = SSI_NULL_HANDLE;
    }

    if (dynamic_cast<Controller *>(m_pParent))
        pInfo->deviceType = SSI_DeviceTypeController;
    else if (dynamic_cast<EndDevice *>(m_pParent))
        pInfo->deviceType = SSI_DeviceTypeEndDevice;
    else if (dynamic_cast<RoutingDevice *>(m_pParent))
        pInfo->deviceType = SSI_DeviceTypeRoutingDevice;
    else
        pInfo->deviceType = SSI_DeviceTypeUnknown;

    pInfo->deviceHandle = m_pParent->getId();
    pInfo->isExternal = SSI_FALSE;
    pInfo->hotPlugCap = SSI_FALSE;

    if (dynamic_cast<EndDevice *>(m_pParent))
        fetchSpeeds(pInfo);
    else
        setSpeeds(pInfo);

    pInfo->countsValid = SSI_FALSE;
    return SSI_StatusOk;
}

/* */
SSI_Status Phy::locate(bool mode) const
{
    return m_pParent->locate(mode);
}

/* */
void Phy::fetchSpeeds(SSI_PhyInfo *pInfo) const
{
    if (m_pRemotePhy != NULL) {
        m_pRemotePhy->setSpeeds(pInfo);
        return;
    }
    if (m_pPort != NULL) {
        Port *pPort = m_pPort->getRemotePort();
        if (pPort != NULL) {
            Container<Phy> container;
            pPort->getPhys(container);
            foreach (i, container)
                (*i)->setSpeeds(pInfo);
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

    if (dynamic_cast<EndDevice *>(m_pParent)) {
        tmp = tmp.reverse_left("/");
        dir = tmp + "sas_device";
        dirs = dir.dirs();
        foreach (i, dirs) {
            try {
                SysfsAttr attr;
                String protocol;
                attr = *(*i) + "target_port_protocols";
                attr >> protocol;
                m_Protocol = __internal_parse_protocol(protocol);
            } catch (...) {
                /* TODO: report read failure of attribtue. */
            }
        }
    } else if (dynamic_cast<Controller *>(m_pParent) || dynamic_cast<RoutingDevice *>(m_pParent)) {
        dir = m_Path + "/sas_phy";
        dirs = dir.dirs();
        foreach (i, dirs) {
            SysfsAttr attr;
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
                    m_pParent->setAddress(address);
                }
            } catch (...) {
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
void Phy::acquireId(Session *pSession)
{
    pSession->addPhy(this);
}

/* */
bool Phy::equal(const Object *pObject) const
{
    const Phy *pPhy = dynamic_cast<const Phy *>(pObject);
    return Object::equal(pObject) &&
        m_pParent->equal(pPhy->m_pParent) && m_Number == pPhy->m_Number;
}

/* */
String Phy::getKey() const
{
    return m_pParent->getKey() + String(m_Number);
}

/* */
void Phy::attachPhy(Phy *pPhy)
{
    if (pPhy != this ) {
        m_pRemotePhy = pPhy;
        if (pPhy == NULL) {
            throw E_NULL_POINTER;
        }
        m_pRemotePhy->attachPhy(this);
    }
}

/* */
void Phy::attachPort(Port *pPort)
{
    if (pPort == NULL) {
        throw E_NULL_POINTER;
    }
    m_pPort = pPort;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
