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
#include "session.h"

/* */
RoutingDevice::RoutingDevice(const String &path)
    : StorageObject(0, path)
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
}

/* */
void RoutingDevice::getPhys(Container &container) const
{
    container = m_Phys;
}

/* */
void RoutingDevice::getEndDevices(Container &container, bool all) const
{
    if (all) {
        container = m_EndDevices;
    } else {
        container = m_EndDevices_Direct;
    }
}

/* */
void RoutingDevice::getAddress(SSI_Address *pAddress) const
{
    if (pAddress) {
        /* TODO: copy the address of routing device (SAS address attached to SMP target) */
    }
}

/* */
void RoutingDevice::getPorts(Container &container) const
{
    container = m_Ports;
}

/* */
SSI_Status RoutingDevice::getInfo(SSI_RoutingDeviceInfo *pInfo) const
{
    if (pInfo == 0) {
        return SSI_StatusInvalidParameter;
    }
    pInfo->routingDeviceHandle = getId();
    return SSI_StatusOk;
}

/* */
void RoutingDevice::attachPhy(Object *pPhy)
{
    m_Phys.add(pPhy);
}

/* */
void RoutingDevice::attachEndDevice(Object *pEndDevice, bool direct)
{
    if (direct) {
        m_EndDevices_Direct.add(pEndDevice);
    }
    m_EndDevices.add(pEndDevice);
}

/* */
void RoutingDevice::attachPort(Object *pPort)
{
    m_Ports.add(pPort);
}

/* */
void RoutingDevice::attachRoutingDevice(Object *pRoutingDevice, bool direct)
{
    if (direct) {
        m_RoutingDevices_Direct.add(pRoutingDevice);
    }
    m_RoutingDevices.add(pRoutingDevice);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
