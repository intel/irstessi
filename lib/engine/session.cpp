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

#include <dirent.h>
#include <sys/types.h>
#include <cstdlib>

#include <ssi.h>

#include "exception.h"
#include "list.h"
#include "container.h"
#include "string.h"
#include "filesystem.h"
#include "object.h"
#include "cache.h"
#include "storage_device.h"
#include "raid_device.h"
#include "array.h"
#include "end_device.h"
#include "volume.h"
#include "controller.h"
#include "session.h"
#include "phy.h"
#include "enclosure.h"
#include "port.h"
#include "routing_device.h"
#include "raid_info.h"
#include "session_manager.h"
#include "ahci.h"
#include "isci.h"
#include "context_manager.h"

/* */
Session::Session() : m_pNoneScopeObj(0)
{
    Directory dir;

    dir = "/sys/bus/pci/drivers/ahci";
    for (Iterator<Directory *> i = dir; *i != 0; ++i) {
        CanonicalPath path = *(*i) + "driver";
        if (path == dir) {
            AHCI *pAHCI = new AHCI(*(*i));
            pAHCI->acquireId(this);
        }
    }
    dir = "/sys/bus/pci/drivers/isci";
    for (Iterator<Directory *> i = dir; *i != 0; ++i) {
        CanonicalPath path = *(*i) + "driver";
        if (path == dir) {
            ISCI *pISCI = new ISCI(*(*i));
            pISCI->acquireId(this);
        }
    }

    m_pNoneScopeObj = new NoneScopeObject(this);
}

/* */
Session::~Session()
{
    delete m_pNoneScopeObj;

    __releaseIds(m_EndDevices);
    __releaseIds(m_Arrays);
    __releaseIds(m_Enclosures);
    __releaseIds(m_RaidInfo);
    __releaseIds(m_Phys);
    __releaseIds(m_Volumes);
    __releaseIds(m_Ports);
    __releaseIds(m_RoutingDevices);
    __releaseIds(m_Controllers);
}

/* */
ScopeObject * Session::getObject(unsigned int id) const
{
    if (id == 0) {
        return m_pNoneScopeObj;
    }
    Object *pObject = 0;
    try {
        switch (getTypeOfId(id)) {
        case ObjectType_EndDevice:
            pObject = m_EndDevices.find(id);
            break;
        case ObjectType_Array:
            pObject = m_Arrays.find(id);
            break;
        case ObjectType_Enclosure:
            pObject = m_Enclosures.find(id);
            break;
        case ObjectType_Phy:
            pObject = m_Phys.find(id);
            break;
        case ObjectType_Volume:
            pObject = m_Volumes.find(id);
            break;
        case ObjectType_Port:
            pObject = m_Ports.find(id);
            break;
        case ObjectType_RoutingDevice:
            pObject = m_RoutingDevices.find(id);
            break;
        case ObjectType_Controller:
            pObject = m_Controllers.find(id);
            break;
        default:
            pObject = 0;
        }
    } catch (...) {
        pObject = 0;
    }
    return dynamic_cast<ScopeObject *>(pObject);
}

/* */
RaidInfo * Session::getRaidInfo(unsigned int id) const
{
    RaidInfo *pResult = 0;
    if (getTypeOfId(id) == ObjectType_RaidInfo) {
        try {
            pResult = dynamic_cast<RaidInfo *>(m_RaidInfo.find(id));
        } catch (...) {
            pResult = 0;
        }
    }
    return pResult;
}

/* */
Array * Session::getArray(unsigned int id) const
{
    Array *pResult = 0;
    if (getTypeOfId(id) == ObjectType_Array) {
        try {
            pResult = dynamic_cast<Array *>(m_Arrays.find(id));
        } catch (...) {
            pResult = 0;
        }
    }
    return pResult;
}

/* */
Controller * Session::getController(unsigned int id) const
{
    Controller *pResult = 0;
    if (getTypeOfId(id) == ObjectType_Controller) {
        try {
            pResult = dynamic_cast<Controller *>(m_Controllers.find(id));
        } catch (...) {
            pResult = 0;
        }
    }
    return pResult;
}

/* */
StorageDevice * Session::getDevice(unsigned int id) const
{
    StorageDevice *pResult = 0;
    try {
        switch (getTypeOfId(id)) {
        case ObjectType_Array:
            pResult = dynamic_cast<StorageDevice *>(m_Arrays.find(id));
            break;
        case ObjectType_EndDevice:
            pResult = dynamic_cast<StorageDevice *>(m_EndDevices.find(id));
            break;
        case ObjectType_Volume:
            pResult = dynamic_cast<StorageDevice *>(m_Volumes.find(id));
            break;
        default:
            pResult = 0;
        }
    } catch (...) {
        pResult = 0;
    }
    return pResult;
}

/* */
Phy * Session::getPhy(unsigned int id) const
{
    Phy *pResult = 0;
    if (getTypeOfId(id) == ObjectType_Phy) {
        try {
            pResult = dynamic_cast<Phy *>(m_Phys.find(id));
        } catch (...) {
            pResult = 0;
        }
    }
    return pResult;
}

/* */
Port * Session::getPort(unsigned int id) const
{
    Port *pResult = 0;
    if (getTypeOfId(id) == ObjectType_Port) {
        try {
            pResult = dynamic_cast<Port *>(m_Ports.find(id));
        } catch (...) {
            pResult = 0;
        }
    }
    return pResult;
}

/* */
RoutingDevice * Session::getRoutingDevice(unsigned int id) const
{
    RoutingDevice *pResult = 0;
    if (getTypeOfId(id) == ObjectType_RoutingDevice) {
        try {
            pResult = dynamic_cast<RoutingDevice *>(m_RoutingDevices.find(id));
        } catch (...) {
            pResult = 0;
        }
    }
    return pResult;
}

/* */
Volume * Session::getVolume(unsigned int id) const
{
    Volume *pResult = 0;
    if (getTypeOfId(id) == ObjectType_Volume) {
        try {
            pResult = dynamic_cast<Volume *>(m_Volumes.find(id));
        } catch (...) {
            pResult = 0;
        }
    }
    return pResult;
}

/* */
Enclosure * Session::getEnclosure(unsigned int id) const
{
    Enclosure *pResult = 0;
    if (getTypeOfId(id) == ObjectType_Enclosure) {
        try {
            pResult = dynamic_cast<Enclosure *>(m_Enclosures.find(id));
        } catch(...) {
            pResult = 0;
        }
    }
    return pResult;
}

/* */
EndDevice * Session::getEndDevice(unsigned int id) const
{
    EndDevice *pResult = 0;
    if (getTypeOfId(id) == ObjectType_EndDevice) {
        try {
            pResult = dynamic_cast<EndDevice *>(m_EndDevices.find(id));
        } catch (...) {
            pResult = 0;
        }
    }
    return pResult;
}

/* */
void Session::__releaseIds(Container &container) const
{
    for (Iterator<Object *> i = container; *i != 0; ++i) {
        pContextMgr->releaseId(*i);
    }
}

/* */
void Session::addEndDevice(EndDevice *pEndDevice)
{
    if (pEndDevice == 0) {
        throw E_NULL_POINTER;
    }
    pContextMgr->acquireId(pEndDevice);
    m_EndDevices.add(pEndDevice);
}

/* */
void Session::addArray(Array *pArray)
{
    if (pArray == 0) {
        throw E_NULL_POINTER;
    }
    pContextMgr->acquireId(pArray);
    m_Arrays.add(pArray);
}

/* */
void Session::addRoutingDevice(RoutingDevice *pRoutingDevice)
{
    if (pRoutingDevice == 0) {
        throw E_NULL_POINTER;
    }
    pContextMgr->acquireId(pRoutingDevice);
    m_RoutingDevices.add(pRoutingDevice);
}

/* */
void Session::addEnclosure(Enclosure *pEnclosure)
{
    if (pEnclosure == 0) {
        throw E_NULL_POINTER;
    }
    pContextMgr->acquireId(pEnclosure);
    m_Enclosures.add(pEnclosure);
}

/* */
void Session::addPhy(Phy *pPhy)
{
    if (pPhy == 0) {
        throw E_NULL_POINTER;
    }
    pContextMgr->acquireId(pPhy);
    m_Phys.add(pPhy);
}

/* */
void Session::addVolume(Volume *pVolume)
{
    if (pVolume == 0) {
        throw E_NULL_POINTER;
    }
    pContextMgr->acquireId(pVolume);
    m_Volumes.add(pVolume);
}

/* */
void Session::addPort(Port *pPort)
{
    if (pPort == 0) {
        throw E_NULL_POINTER;
    }
    pContextMgr->acquireId(pPort);
    m_Ports.add(pPort);
}

/* */
void Session::addController(Controller *pController)
{
    if (pController == 0) {
        throw E_NULL_POINTER;
    }
    pContextMgr->acquireId(pController);
    m_Controllers.add(pController);
}

/* */
void Session::addRaidInfo(RaidInfo *pRaidInfo)
{
    if (pRaidInfo == 0) {
        throw E_NULL_POINTER;
    }
    pContextMgr->acquireId(pRaidInfo);
    m_RaidInfo.add(pRaidInfo);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
