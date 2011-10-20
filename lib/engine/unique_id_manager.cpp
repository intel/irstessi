
/*
Copyright (c) 2011, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/




#if (HAVE_CONFIG_H == 1)
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <features.h>
#include <cstdio>

#include <ssi.h>
#include <log/log.h>

#include "exception.h"
#include "list.h"
#include "container.h"
#include "string.h"
#include "filesystem.h"
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
        String key = pObject->getKey();
        if (key == m_Key)
            return true;
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
        String key = pObject->getKey();
        if (key != m_Key)
            return true;
        return false;
    }
    return !(*(*i) == pObject);
}

/* save id:key to key file */
void Id::store()
{
    if ((m_Id >> 28) == ObjectType_Session ||
        (m_Id >> 28) == ObjectType_Event)
        return;

    AFile keyFile = String(SSI_IDKEY_FILE);
    try {
        char s[11];
        sprintf(s, "0x%x", m_Id);
        String idkey = String(s) + String(":") + m_Key + String("\n");
        keyFile << idkey;
    } catch (...) {
        dlog("failed to store id:key");
    }
}

/* when not in cache find new Id */
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

/* add object to cache and set Id */
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
        pId = new Id(id |= pObject->getType() << 28, pObject->getKey());
        List<Id *>::add(pId);
        pId->store();
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
