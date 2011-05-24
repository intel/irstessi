
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
#include "port.h"
#include "session.h"

/* */
Phy::Phy(const String &path, unsigned int number, StorageObject *pParent)
    : StorageObject(path, pParent), m_pRemotePhy(0), m_pPort(0), m_Number(number)
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
    if (pInfo == 0) {
        return SSI_StatusInvalidParameter;
    }
    pInfo->phyHandle = getId();
    try {
        m_pParent->getAddress(pInfo->phyAddress);
    }
    catch (...) {
    }
    pInfo->phyNumber = m_Number;
    pInfo->protocol = m_Protocol;
    if (m_pPort != 0) {
        pInfo->associatedPort = m_pPort->getId();
    } else {
        pInfo->associatedPort = SSI_NULL_HANDLE;
    }
    switch (m_pParent->getType()) {
    case ObjectType_Controller:
        pInfo->deviceType = SSI_DeviceTypeController;
        break;
    case ObjectType_EndDevice:
        pInfo->deviceType = SSI_DeviceTypeEndDevice;
        break;
    case ObjectType_RoutingDevice:
        pInfo->deviceType = SSI_DeviceTypeRoutingDevice;
        break;
    default:
        pInfo->deviceType = SSI_DeviceTypeUnknown;
    }
    pInfo->deviceHandle = m_pParent->getId();
    pInfo->isExternal = SSI_FALSE;
    pInfo->hotPlugCap = SSI_FALSE;
    pInfo->minHWLinkSpeed = m_minHWLinkSpeed;
    pInfo->maxHWLinkSpeed = m_maxHWLinkSpeed;
    pInfo->minLinkSpeed = m_minLinkSpeed;
    pInfo->maxLinkSpeed = m_maxLinkSpeed;
    pInfo->negotiatedLinkSpeed = m_negotiatedLinkSpeed;
    pInfo->countsValid = SSI_FALSE;
    return SSI_StatusOk;
}

/* */
SSI_Status Phy::locate(bool mode) const
{
    return SSI_StatusNotSupported;
}

/* */
void Phy::setProperties()
{
    m_Protocol = SSI_PhyProtocolUnknown;
    m_minHWLinkSpeed = SSI_PhySpeedUnknown;
    m_maxHWLinkSpeed = SSI_PhySpeedUnknown;
    m_minLinkSpeed = SSI_PhySpeedUnknown;
    m_maxLinkSpeed = SSI_PhySpeedUnknown;
    m_negotiatedLinkSpeed = SSI_PhySpeedUnknown;
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
void Phy::attachPhy(Phy *pPhy)
{
    if (pPhy != this ) {
        m_pRemotePhy = pPhy;
        if (pPhy == 0) {
            throw E_NULL_POINTER;
        }
        m_pRemotePhy->attachPhy(this);
    }
}

/* */
void Phy::attachPort(Port *pPort)
{
    if (pPort == 0) {
        throw E_NULL_POINTER;
    }
    m_pPort = pPort;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
