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
#include "cache.h"
#include "object.h"
#include "controller.h"
#include "session.h"

/* */
Controller::Controller(const String &path)
    : StorageObject(0, path)
{
    readRaidInfo();
}

/* */
Controller::~Controller()
{
}

/* */
SSI_Status Controller::getInfo(SSI_ControllerInfo *pInfo) const
{
    if (pInfo == 0) {
        return SSI_StatusInvalidParameter;
    }
    pInfo->controllerHandle = getId();
    return SSI_StatusOk;
}

/* */
void Controller::getPhys(Container &container) const
{
    container = m_Phys;
}

/* */
void Controller::getEnclosures(Container &container, bool all) const
{
    if (all) {
        container = m_Enclosures;
    } else {
        container = m_Enclosures_Direct;
    }
}

/* */
void Controller::getPorts(Container &container) const
{
    container = m_Ports;
}

/* */
void Controller::getEndDevices(Container &container, bool all) const
{
    if (all) {
        container = m_EndDevices;
    } else {
        container = m_EndDevices_Direct;
    }
}

/* */
void Controller::getRoutingDevices(Container &container, bool all) const
{
    if (all) {
        container = m_RoutingDevices;
    } else {
        container = m_RoutingDevices_Direct;
    }
}

/* */
void Controller::getArrays(Container &container) const
{
    container = m_Arrays;
}

/* */
void Controller::getVolumes(Container &container) const
{
    container = m_Volumes;
}

/* */
SSI_Status Controller::readPatrolSetState(bool enable)
{
    (void)enable;
    return SSI_StatusOk;
}

/* */
bool Controller::equal(const Object *pObject) const
{
    return Object::equal(pObject) &&
        dynamic_cast<const Controller *>(pObject)->m_Path == m_Path;
}

/* */
void Controller::attachEndDevice(Object *pEndDevice, bool direct)
{
    if (pEndDevice == 0) {
        throw E_NULL_POINTER;
    }
    if (direct) {
        m_EndDevices_Direct.add(pEndDevice);
    }
    m_EndDevices.add(pEndDevice);
}

/* */
void Controller::attachRoutingDevice(Object *pRoutingDevice, bool direct)
{
    if (pRoutingDevice == 0) {
        throw E_NULL_POINTER;
    }
    if (direct) {
        m_RoutingDevices_Direct.add(pRoutingDevice);
    }
    m_RoutingDevices.add(pRoutingDevice);
}

/* */
void Controller::attachPort(Object *pPort)
{
    if (pPort == 0) {
        throw E_NULL_POINTER;
    }
    m_Ports.add(pPort);
}

/* */
void Controller::attachVolume(Object *pVolume)
{
    if (pVolume == 0) {
        throw E_NULL_POINTER;
    }
    m_Volumes.add(pVolume);
}

/* */
void Controller::attachPhy(Object *pPhy)
{
    if (pPhy == 0) {
        throw E_NULL_POINTER;
    }
    m_Phys.add(pPhy);
}

/* */
void Controller::attachArray(Object *pArray)
{
    if (pArray == 0) {
        throw E_NULL_POINTER;
    }
    m_Arrays.add(pArray);
}

/* */
void Controller::attachEnclosure(Object *pEnclosure, bool direct)
{
    if (pEnclosure == 0) {
        throw E_NULL_POINTER;
    }
    if (direct) {
        m_Enclosures_Direct.add(pEnclosure);
    }
    m_Enclosures.add(pEnclosure);
}

/* */
void Controller::getAddress(SSI_Address *pAddress) const
{
    if (pAddress) {
        /* TODO: this is temporary method, it will be moved to specialized classes. */
    }
}

/* */
void Controller::acquireId(Session *pSession)
{
    pSession->addController(this);
    Iterator<Object *> i;
    for (i = m_EndDevices_Direct; *i != 0; ++i) {
        dynamic_cast<StorageObject *>(*i)->acquireId(pSession);
    }
    for (i = m_RoutingDevices_Direct; *i != 0; ++i) {
        dynamic_cast<StorageObject *>(*i)->acquireId(pSession);
    }
    for (i = m_Ports; *i != 0; ++i) {
        dynamic_cast<StorageObject *>(*i)->acquireId(pSession);
    }
    for (i = m_Phys; *i != 0; ++i) {
        dynamic_cast<StorageObject *>(*i)->acquireId(pSession);
    }
    for (i = m_Arrays; *i != 0; ++i) {
        dynamic_cast<StorageObject *>(*i)->acquireId(pSession);
    }
    for (i = m_Enclosures_Direct; *i != 0; ++i) {
        dynamic_cast<StorageObject *>(*i)->acquireId(pSession);
    }
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
