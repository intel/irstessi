
/*
Copyright (c) 2011, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/




#if __GNUC_PREREQ(3,4)
#pragma once
#endif /* __GNUC_PREREQ */

#ifndef __PORT_H__INCLUDED__
#define __PORT_H__INCLUDED__

// Forward declarations
class EndDevice;
class RoutingDevice;
class RaidInfo;

/* */
class Port : public StorageObject {
public:
    Port(const String &path);
    virtual ~Port();

    // Object

public:
    bool operator ==(const Object &object) const;
    String getKey() const;

    // ScopeObject

public:
    void getPhys(Container<Phy> &container) const;

    bool scopeTypeMatches(SSI_ScopeType scopeType) const {
        return scopeType == SSI_ScopeTypePort;
    }

    // StorageObject

public:
    virtual void attachArray(Array *pArray);
    virtual void attachPort(Port *pPort);
    virtual void attachVolume(Volume *pVolume);
    virtual void attachRoutingDevice(RoutingDevice *pRoutingDevice);
    virtual void attachEnclosure(Enclosure *pEnclosure);
    virtual void attachEndDevice(EndDevice *pEndDevice);
    void attachPhy(Phy *pPhy);

    void addToSession(Session *pSession);

    // Port

public:
    SSI_Status getInfo(SSI_PortInfo *pInfo) const;
    virtual SSI_Status locate(bool mode) const;
    virtual RaidInfo * getRaidInfo() const;
    Port *getRemotePort() const {
        return m_pRemotePort;
    }

protected:
    Port *m_pRemotePort;
    Container<Phy> m_Phys;
};

#endif /* __PORT_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
