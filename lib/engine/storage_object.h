/*
Copyright (c) 2011 - 2017, Intel Corporation
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
#include <boost/weak_ptr.hpp>

#ifdef SSI_HAS_PRAGMA_ONCE
#pragma once
#endif

/* */
class StorageObject : public ScopeObject {
public:
    typedef boost::shared_ptr<StorageObject> Parent;

    StorageObject(const Parent& pParent = Parent())
        : m_pParent(pParent) {
    }

    StorageObject(const String &path, const Parent& pParent = Parent())
        : m_pParent(pParent), m_Path(path) {
    }

protected:
    boost::weak_ptr<StorageObject> m_pParent;
    String m_Path;

public:
    String getPath() const {
        return m_Path;
    }
    void setPath(const String &path) {
        m_Path = path;
    }

public:
    virtual void attachEndDevice(const boost::shared_ptr<EndDevice>&) {
        throw E_INVALID_OPERATION;
    }
    virtual void attachRoutingDevice(const boost::shared_ptr<RoutingDevice>&) {
        throw E_INVALID_OPERATION;
    }
    virtual void attachPort(const boost::shared_ptr<Port>&) {
        throw E_INVALID_OPERATION;
    }
    virtual void attachVolume(const boost::shared_ptr<Volume>&) {
        throw E_INVALID_OPERATION;
    }
    virtual void attachArray(const boost::shared_ptr<Array>&) {
        throw E_INVALID_OPERATION;
    }
    virtual void attachPhy(const boost::shared_ptr<Phy>&) {
        throw E_INVALID_OPERATION;
    }
    virtual void attachEnclosure(const boost::shared_ptr<Enclosure>&) {
        throw E_INVALID_OPERATION;
    }
    boost::weak_ptr<StorageObject> getParent() const {
        return m_pParent;
    }
    void setParent(const Parent& pParent) {
        if (pParent.get() != this) {
            m_pParent = pParent;
        }
    }
    virtual boost::shared_ptr<Controller> getController() const {
        return boost::shared_ptr<Controller>();
    }
    virtual boost::shared_ptr<RaidInfo> getRaidInfo() const {
        return boost::shared_ptr<RaidInfo>();
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

    virtual SSI_Status removeDisk() {
        return SSI_StatusNotSupported;
    }

public:
    virtual boost::shared_ptr<Port> getPort() const {
        throw E_INVALID_OPERATION;
    }
    virtual boost::shared_ptr<Port> getPortByPath(const String &s) const {
        throw E_INVALID_OPERATION;
    }
    virtual void discover() {
        throw E_INVALID_OPERATION;
    }
    virtual void addToSession(const boost::shared_ptr<Session>& pSession) = 0;
};

#endif /* __STORAGE_OBJECT_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
