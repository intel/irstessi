
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
#include <climits>
#include <cstdio>

#include <ssi.h>
#include <log/log.h>

#include "exception.h"
#include "container.h"
#include "string.h"
#include "object.h"
#include "unique_id_manager.h"
#include "utils.h"
#include "session.h"
#include "event.h"
#include "filesystem.h"

/* when not in cache find new Id */
unsigned int UniqueIdManager::findId() const
{
    for(unsigned int id = 1; id <= UINT_MAX; id++) {
        if (m_cache.find(id) == m_cache.end() && id >= m_availableId) {
            return id;
        }
    }
    return 0;
}

/* add id + key (from file) to cache */
void UniqueIdManager::add(unsigned int id, String key)
{
    String cachedKey = m_cache[id];

    if (cachedKey == "") {
        /* it is not in cache */
        dlog(String(id) + key + " adding to cache");
        m_cache[id] = key;
    } else {
        /* already in cache */
        if (cachedKey != key)
            dlog(String("id - key conflict between cache and file: ") + String(id) + " : " + key + " \nkey in cache:" + cachedKey);
    }
}

/* add object to cache and set Id */
void UniqueIdManager::add(Object *pObject)
{
    if (pObject == NULL)
        throw E_NULL_POINTER;

    String key = pObject->getKey();
    unsigned int id = 0;
    foreach (i, m_cache) {
        if ((*i).second == key) {
            id = (*i).first;
            break;
        }
    }

    if (id == 0) {
        id = findId();
        /* TODO when out of id's clean the id file:
         * remove all id:key pairs of the same type that have no objects in cache */
        if (id == 0)
            throw E_OUT_OF_RESOURCES;

        m_cache[id] = key;

        if (!(dynamic_cast<Session *>(pObject) || dynamic_cast<Event *>(pObject) || key == "")) {
            File keyFile(String(SSI_IDKEY_FILE));
            File idIdxFile(String(SSI_IDKEY_IDX_FILE));
            try {
                m_availableId = id < UINT_MAX ? id +1 : UINT_MAX;
                idIdxFile.write(m_availableId);
            } catch (...) {
                dlog("failed to store id index");
            }
            try {
                char s[11];
                snprintf(s, sizeof(s), "0x%x", id);
                String idkey = String(s) + String(":") + key + String("\n");
                keyFile.write(idkey, true);
            } catch (...) {
                dlog("failed to store id:key");
            }
        }
    }

    pObject->setId(id);
}

/* remove object from cache */
void UniqueIdManager::remove(Object *pObject)
{
    if (pObject == NULL)
        throw E_NULL_POINTER;

    unsigned int id = pObject->getId();

    if (m_cache.find(id) == m_cache.end())
        throw E_NOT_FOUND;

    /* session and event Id's can be reused so remove from cache
     * other object type Id's should remain even when no objects left
     * for consistency between sessions */
    if (dynamic_cast<Session *>(pObject) || dynamic_cast<Event *>(pObject)) {
        m_cache.erase(id);
    }
}

/* remove object Id from file*/
void UniqueIdManager::removeId(Object *pObject)
{
    if (pObject == NULL)
        throw E_NULL_POINTER;

    std::map<unsigned int, String> idKeyMap = loadIdsFromFile();
    unsigned int id = pObject->getId();

    if (idKeyMap.find(id) == idKeyMap.end())
        // nothing to remove
        return;

    File keyFile(String(SSI_IDKEY_FILE));
    keyFile.write("");
    foreach (i, idKeyMap) {
        String idkey = String((*i).first) + String(":") + (*i).second + String("\n");
        if ((*i).first == pObject->getId() && (*i).second == pObject->getKey())
            continue;
        else {
            try {
                char s[11];
                snprintf(s, sizeof(s), "0x%x", (*i).first);
                idkey = String(s) + String(":") + (*i).second + String("\n");
                keyFile.write(idkey, true);
            } catch (...) {
                dlog("failed to store id:key");
            }
        }
    }
}

/* reload id:key file */
void UniqueIdManager::refresh()
{
    // if irstessi.keys or irstessi.idx file is missing restore them using cached data
    // note: IDs of session and events will be written to file too.
    File keyFile = String(SSI_IDKEY_FILE);
    File idIdxFile = String(SSI_IDKEY_IDX_FILE);
    if (!keyFile.exists() || !idIdxFile.exists()) {
        foreach (i, m_cache) {
            try {
                char s[11];
                snprintf(s, sizeof(s), "0x%x", (*i).first);
                String idkey = String(s) + String(":") + (*i).second + String("\n");
                keyFile.write(idkey, true);
            } catch (...) {
                dlog("failed to store id:key");
            }
        }

        idIdxFile.write(m_cache.size() == 0 ? 0 : m_availableId);
    }
    else {
        std::map<unsigned int, String> id_key_map = loadIdsFromFile();
        foreach (i, id_key_map) {
            add((*i).first, (*i).second);
        }

        m_availableId = loadIdIndexFromFile();
    }
}
