
/*
Copyright (c) 2011, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/




#if __GNUC_PREREQ(3, 4)
#pragma once
#endif /* __GNUC_PREREQ */

#ifndef __UNIQUE_ID_MANAGER_H__INCLUDED__
#define __UNIQUE_ID_MANAGER_H__INCLUDED__

#define SSI_IDKEY_FILE "/var/run/ssi.keys"

/* */
class Id {
public:
    Id(unsigned int id, String key)
        : m_Id(id), m_Key(key) {
    }
    ~Id() {
    }
    unsigned int getId() const {
        return m_Id;
    }
    void add(Object * pObject) {
        if (pObject == 0) {
            throw E_NULL_POINTER;
        }
        m_Objects.push_back(pObject);
    }
    void remove(Object *pObject) {
        if (pObject == 0) {
            throw E_NULL_POINTER;
        }
        m_Objects.remove(pObject);
    }
    unsigned int count() const {
        return m_Objects.size();
    }
    String getKey() {
        return m_Key;
    }
    void setKey(String key) {
        m_Key = key;
    }

public:
    bool operator == (const Object *pObject) const;
    bool operator != (const Object *pObject) const;
    void store();

private:
    unsigned int m_Id;
    std::list<Object *> m_Objects;
    String m_Key;

    friend class UniqueIdManager;
};

/* */
class IdCache {
public:
    ~IdCache();

    void remove(Object *pObject);
    void add(Object *pObject);
    void add(unsigned int id, String key);

private:
    unsigned int __findId() const;
    std::list<Id *> _list;
};

/* */
class UniqueIdManager {
public:
    UniqueIdManager();
    ~UniqueIdManager();

private:
    IdCache m_Sessions;
    IdCache m_Events;
    IdCache m_EndDevices;
    IdCache m_Arrays;
    IdCache m_Enclosures;
    IdCache m_Phys;
    IdCache m_Volumes;
    IdCache m_Ports;
    IdCache m_RoutingDevices;
    IdCache m_RaidInfo;
    IdCache m_Controllers;

public:
    unsigned int acquireId(Object *);
    void add(unsigned int id, String key);
    void releaseId(Object *);
    void refresh();
};

#endif /* __UNIQUE_ID_MANAGER_H__INCLUDED__ */
