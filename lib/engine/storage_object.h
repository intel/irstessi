/*
Copyright (c) 2011 - 2016, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __STORAGE_OBJECT_H__INCLUDED__
#define __STORAGE_OBJECT_H__INCLUDED__

#include "scope_object.h"

#ifdef SSI_HAS_PRAGMA_ONCE
#pragma once
#endif

/* */
class StorageObject : public ScopeObject {
public:
    StorageObject(StorageObject *pParent = NULL)
        : m_pParent(pParent) {
    }
    StorageObject(const String &path, StorageObject *pParent = NULL)
        : m_pParent(pParent), m_Path(path) {
    }

protected:
    StorageObject *m_pParent;
    String m_Path;

public:
    String getPath() const {
        return m_Path;
    }
    void setPath(const String &path) {
        m_Path = path;
    }

public:
    virtual void attachEndDevice(EndDevice *) {
        throw E_INVALID_OPERATION;
    }
    virtual void attachRoutingDevice(RoutingDevice *) {
        throw E_INVALID_OPERATION;
    }
    virtual void attachPort(Port *) {
        throw E_INVALID_OPERATION;
    }
    virtual void attachVolume(Volume *) {
        throw E_INVALID_OPERATION;
    }
    virtual void attachArray(Array *) {
        throw E_INVALID_OPERATION;
    }
    virtual void attachPhy(Phy *) {
        throw E_INVALID_OPERATION;
    }
    virtual void attachEnclosure(Enclosure *) {
        throw E_INVALID_OPERATION;
    }
    StorageObject * getParent() const {
        return m_pParent;
    }
    void setParent(StorageObject *pParent) {
        if (pParent != this) {
            m_pParent = pParent;
        }
    }
    virtual Controller * getController() const {
        return NULL;
    }
    virtual RaidInfo * getRaidInfo() const {
        return NULL;
    }
    virtual void getAddress(SSI_Address &) const {
        throw E_INVALID_OPERATION;
    }
    virtual void setAddress(SSI_Address &) {
        throw E_INVALID_OPERATION;
    }

    virtual SSI_Status locate(bool mode) const {
        return SSI_StatusNotSupported;
    }

public:
    virtual Port * getPort() const {
        throw E_INVALID_OPERATION;
    }
    virtual Port * getPortByPath(const String &s) const {
        throw E_INVALID_OPERATION;
    }
    virtual void discover() {
        throw E_INVALID_OPERATION;
    }
    virtual void addToSession(Session *pSession) = 0;
};

#endif /* __STORAGE_OBJECT_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
