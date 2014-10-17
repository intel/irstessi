
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
#include "container.h"
#include "string.h"
#include "filesystem.h"
#include "object.h"
#include "unique_id_manager.h"
#include "utils.h"

/* reload id:key file */
void UniqueIdManager::refresh()
{
    File keyFile = String(SSI_IDKEY_FILE);
    String keyList;
    try {
        keyFile >> keyList;
        keyList += "\n";
    } catch (...) {
        dlog("ssi.keys file missing")
        /* no file? that's ok */
    }
    /* process the list to update cache */
    while (keyList) {
        unsigned int id;
        String sid = keyList.left(":");
        String key = keyList.between(sid + ":", "\n");
        keyList = keyList.after(key + "\n");
        try {
            id = (unsigned int)(sid);
            if (id == 0 || key == "") { /* bad line  - just skip */
                dlog("bad line");
                continue;
            }
            add(id, key);
        } catch (...) {
            /* just skip the line */
            dlog(sid + " failed to convert to unsigned int");
        }
    }
}

/* */
bool UniqueIdManager::Id::operator == (const Object *pObject) const
{
    if (pObject == NULL)
        return false;

    if (m_Objects.empty()) {
        String key = pObject->getKey();
        if (key == m_Key)
            return true;
        return false;
    }

    return *m_Objects.front() == *pObject;
}

bool UniqueIdManager::Id::operator != (const Object *pObject) const
{
    return !this->operator ==(pObject);
}

/* save id:key to key file */
void UniqueIdManager::Id::store()
{
    if ((m_Id >> 28) == ObjectType_Session ||
        (m_Id >> 28) == ObjectType_Event)
        return;
    if(m_Key == "")
        return;

    AFile keyFile(String(SSI_IDKEY_FILE));
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
unsigned int UniqueIdManager::__findId() const {
    unsigned int id;
    for(id = 1; id <= 0x0fffffff; id++) {
        Id *pId = NULL;
        foreach (i, m_cache) {
            if (((*i)->getId() & 0x0fffffff) == id) {
                pId = *i;
                break;
            }
        }
        if (pId == NULL)
            break;
    }
    return (id & 0x0fffffff);
}

/* */
UniqueIdManager::~UniqueIdManager()
{
    foreach (i, m_cache)
        delete *i;
}

/* add object to cache and set Id */
void UniqueIdManager::add(Object *pObject)
{
    if (pObject == NULL) {
        throw E_NULL_POINTER;
    }

    Id *pId = NULL;
    foreach (i, m_cache) {
        if (**i == pObject) {
            pId = *i;
            break;
        }
    }

    if (pId == NULL) {
        unsigned int id = __findId();
        /* TODO when out of id's clean the id file:
         * remove all id:key pairs of the same type that have no objects in cache */
        if (id == 0) {
            throw E_OUT_OF_RESOURCES;
        }
        pId = new Id(id |= pObject->getType() << 28, pObject->getKey());
        m_cache.push_back(pId);
        pId->store();
    }

    pId->add(pObject);
    pObject->setId(pId->getId());
}

/* add id + key (from file) to cache */
void UniqueIdManager::add(unsigned int id, String key)
{
    if ((id >> 28) == ObjectType_Session ||
        (id >> 28) == ObjectType_Event)
        return;

    Id *pId = NULL;
    foreach (i, m_cache) {
        if ((*i)->getId() == id) {
            pId = *i;
            break;
        }
    }

    if (pId == NULL) {
        /* it is not in cache */
        dlog(String(id) + key + " adding to cache");
        Id *pId = new Id(id, key);
        m_cache.push_back(pId);
    } else {
        /* already in cache */
        if (pId->getKey() != key)
            dlog(String("id - key conflict between cache and file: ") + String(id) + " : " + key + " \nkey in cache:" + pId->getKey());
    }
}

/* remove object from cache */
void UniqueIdManager::remove(Object *pObject) {
    if (pObject == NULL) {
        throw E_NULL_POINTER;
    }

    Id *pId = NULL;
    foreach (i, m_cache) {
        if ((*i)->getId() == pObject->getId()) {
            pId = *i;
            break;
        }
    }

    if (pId == NULL)
        throw E_NOT_FOUND;

    pId->remove(pObject);

    /* session and event Id's can be reused so remove from cache
     * other object type Id's should remain even when no objects left
     * for consistency between sessions */
    unsigned int type = pId->getId() >> 28;
    if ((type == ObjectType_Session || type == ObjectType_Event) && pId->count() == 0) {
        delete pId;
        m_cache.remove(pId);
    }
}
