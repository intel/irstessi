/*
Copyright (c) 2011 - 2016, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __PORT_H__INCLUDED__
#define __PORT_H__INCLUDED__

#include "storage_object.h"
#include <boost/enable_shared_from_this.hpp>

#ifdef SSI_HAS_PRAGMA_ONCE
#pragma once
#endif

/* */
class Port : public StorageObject, public boost::enable_shared_from_this<Port> {
public:
    Port(const String &path);

    // Object

public:
    virtual bool operator ==(const Object &object) const;
    virtual String getId() const;

    // ScopeObject

public:
    virtual void getPhys(Container<Phy> &container) const;

    virtual bool scopeTypeMatches(SSI_ScopeType scopeType) const {
        return scopeType == SSI_ScopeTypePort;
    }

    // StorageObject

public:
    virtual void attachArray(const boost::shared_ptr<Array>& pArray);
    virtual void attachPort(const boost::shared_ptr<Port>& pPort);
    virtual void attachVolume(const boost::shared_ptr<Volume>& pVolume);
    virtual void attachRoutingDevice(const boost::shared_ptr<RoutingDevice>& pRoutingDevice);
    virtual void attachEnclosure(const boost::shared_ptr<Enclosure>& pEnclosure);
    virtual void attachEndDevice(const boost::shared_ptr<EndDevice>& pEndDevice);
    virtual void attachPhy(const boost::shared_ptr<Phy>& pPhy);

    virtual void addToSession(const boost::shared_ptr<Session>& pSession);

    // Port

public:
    SSI_Status getInfo(SSI_PortInfo *pInfo) const;
    virtual SSI_Status locate(bool mode) const;
    virtual boost::shared_ptr<RaidInfo> getRaidInfo() const;
    boost::shared_ptr<Port> getRemotePort() const {
        return m_pRemotePort.lock();
    }

protected:
    boost::weak_ptr<Port> m_pRemotePort;
    Container<Phy> m_Phys;
};

#endif /* __PORT_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
