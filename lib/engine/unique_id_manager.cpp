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

#if (HAVE_CONFIG_H == 1)
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <features.h>

#include <ssi.h>

#include "exception.h"
#include "list.h"
#include "container.h"
#include "string.h"
#include "object.h"
#include "cache.h"
#include "unique_id_manager.h"

/* */
UniqueIdManager::UniqueIdManager()
{
}

/* */
UniqueIdManager::~UniqueIdManager()
{
}

/* */
unsigned int UniqueIdManager::acquireId(Object *pObject)
{
    if (pObject == 0) {
        throw E_NULL_POINTER;
    }
    switch (pObject->getType()) {
    case ObjectType_Session:
        m_Sessions.add(pObject);
        break;
    case ObjectType_Event:
        m_Events.add(pObject);
        break;
    case ObjectType_EndDevice:
        m_EndDevices.add(pObject);
        break;
    case ObjectType_Array:
        m_Arrays.add(pObject);
        break;
    case ObjectType_Enclosure:
        m_Enclosures.add(pObject);
        break;
    case ObjectType_Phy:
        m_Phys.add(pObject);
        break;
    case ObjectType_Volume:
        m_Volumes.add(pObject);
        break;
    case ObjectType_Port:
        m_Ports.add(pObject);
        break;
    case ObjectType_RoutingDevice:
        m_RoutingDevices.add(pObject);
        break;
    case ObjectType_RaidInfo:
        m_RaidInfo.add(pObject);
        break;
    case ObjectType_Controller:
        m_Controllers.add(pObject);
        break;
    default:
        break;
    }
    return pObject->getId();
}

/* */
void UniqueIdManager::releaseId(Object *pObject)
{
    if (pObject == 0) {
        throw E_NULL_POINTER;
    }
    switch (pObject->getType()) {
    case ObjectType_Session:
        m_Sessions.remove(pObject);
        break;
    case ObjectType_Event:
        m_Events.remove(pObject);
        break;
    case ObjectType_EndDevice:
        m_EndDevices.remove(pObject);
        break;
    case ObjectType_Array:
        m_Arrays.remove(pObject);
        break;
    case ObjectType_Enclosure:
        m_Enclosures.remove(pObject);
        break;
    case ObjectType_Phy:
        m_Phys.remove(pObject);
        break;
    case ObjectType_Volume:
        m_Volumes.remove(pObject);
        break;
    case ObjectType_Port:
        m_Ports.remove(pObject);
        break;
    case ObjectType_RoutingDevice:
        m_RoutingDevices.remove(pObject);
        break;
    case ObjectType_RaidInfo:
        m_RaidInfo.remove(pObject);
        break;
    case ObjectType_Controller:
        m_Controllers.remove(pObject);
        break;
    default:
        break;
    }
}

/* */
bool Id::operator == (const Object *pObject) const
{
    if (pObject == 0) {
        return false;
    }
    Iterator<Object *> i = m_Objects;
    if (*i == 0) {
        return false;
    }
    return *(*i) == pObject;
}

bool Id::operator != (const Object *pObject) const
{
    if (pObject == 0) {
        return true;
    }
    Iterator<Object *> i = m_Objects;
    if (*i == 0) {
        return true;
    }
    return !(*(*i) == pObject);
}

/* */
unsigned int IdCache::__findId() const {
    unsigned int id = 0;
    Iterator<Id *> i;
    while (++id <= 0x0fffffff) {
        for (i = first(); *i != 0; ++i) {
            if (((*i)->getId() & 0x0fffffff) == id) {
                break;
            }
        }
        if (*i == 0) {
            break;
        }
    }
    return (id & 0x0fffffff);
}

/* */
IdCache::IdCache()
{
}

/* */
IdCache::~IdCache()
{
    for (Iterator<Id *> i = first(); *i != 0; ++i) {
        delete *i;
    }
}

/* */
void IdCache::add(Object *pObject)
{
    if (pObject == 0) {
        throw E_NULL_POINTER;
    }
    Iterator<Id *> i;
    for (i = first(); *i != 0 && *(*i) != pObject; ++i) {
    }
    Id *pId = *i;
    if (pId == 0) {
        unsigned int id = __findId();
        if (id == 0) {
            throw E_OUT_OF_RESOURCES;
        }
        pId = new Id(id |= pObject->getType() << 28);
        List<Id *>::add(pId);
    }
    pId->add(pObject);
    pObject->setId(pId->getId());
}

/* */
void IdCache::remove(Object *pObject) {
    if (pObject == 0) {
        throw E_NULL_POINTER;
    }
    Iterator<Id *> i;
    for (i = first(); *i != 0; ++i) {
        if ((*i)->getId() == pObject->getId()) {
            break;
        }
    }
    if (*i == 0) {
        throw E_NOT_FOUND;
    }
    (*i)->remove(pObject);
    if ((*i)->count() == 0) {
        delete List<Id *>::remove(i);
    }
}
