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
#include "port.h"
#include "session.h"

/* */
Phy::Phy(StorageObject *pParent, const String &path, unsigned int number)
    : StorageObject(pParent, path), m_pRemotePhy(0), m_pPort(0), m_Number(number)
{
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
    /* pInfo->phyAddress */
    pInfo->phyNumber = m_Number;
    /* pInfo->phyProtocol */
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
    /* pInfo->minHWLinkSpeed */
    /* pInfo->maxHWLinkSpeed */
    /* pInfo->minLinkSpeed */
    /* pInfo->maxLinkSpeed */
    /* pInfo->negotiatedLinkSpeed */
    pInfo->countsValid = SSI_FALSE;
    return SSI_StatusOk;
}

/* */
SSI_Status Phy::locate(bool mode) const
{
    return SSI_StatusNotSupported;
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
void Phy::attachPhy(Object *pPhy)
{
    if (pPhy != this ) {
        m_pRemotePhy = dynamic_cast<Phy *>(pPhy);
        if (pPhy == 0) {
            throw E_NULL_POINTER;
        }
        m_pRemotePhy->attachPhy(this);
    }
}

/* */
void Phy::attachPort(Object *pPort)
{
    if (pPort == 0) {
        throw E_NULL_POINTER;
    }
    m_pPort = dynamic_cast<Port *>(pPort);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
