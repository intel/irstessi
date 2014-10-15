
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

#ifndef __ROUTING_DEVICE_H__INCLUDED__
#define __ROUTING_DEVICE_H__INCLUDED__

// Forward declaration
class Port;
class Enclosure;

/* */
class RoutingDevice : public StorageObject {
 public:
    RoutingDevice(const String &path);
    virtual ~RoutingDevice();

    // Object

 public:

    virtual bool equal(const Object *pObject) const {
        return false;
    }
    virtual String getKey() const {
        return m_Path;
    }
    ObjectType getType() const {
        return ObjectType_RoutingDevice;
    }

    // ScopeObject

 public:
    virtual void getPhys(Container<Phy> &) const;
    void getRoutingDevices(Container<RoutingDevice> &, bool all) const;
    void getPorts(Container<Port> &) const;
    void getEndDevices(Container<EndDevice> &, bool all) const;
    void getEnclosures(Container<Enclosure> &, bool all) const;

    bool scopeTypeMatches(SSI_ScopeType scopeType) const {
        return scopeType == SSI_ScopeTypeRoutingDevice;
    }

    // StorageObject

public:
    void attachEndDevice(EndDevice *pEndDevice);
    void attachVolume(Volume *pVolume);
    void attachPhy(Phy *pPhy);
    void attachRoutingDevice(RoutingDevice *pRoutingDevice);
    void attachEnclosure(Enclosure *pEnclosure);
    void attachPort(Port *pPort);
    void attachArray(Array *pArray);

    virtual void acquireId(Session *pSession);

    // RoutingDevice

protected:
    Container<EndDevice> m_EndDevices;
    Container<EndDevice> m_EndDevices_Direct;
    Container<Phy> m_Phys;
    Container<Port> m_Ports;
    Container<RoutingDevice> m_RoutingDevices;
    Container<RoutingDevice> m_RoutingDevices_Direct;
    Enclosure *m_pEnclosure;
    Port *m_pSubtractivePort;
    String m_ProductId;
    String m_Vendor;
    String m_ProductRev;
    String m_ComponentVendorId;
    String m_ComponentId;
    String m_ComponentRev;

public:
    virtual Enclosure * getEnclosure() const {
        return m_pEnclosure;
    }
    virtual void setEnclosure(Enclosure *pEnclosure) {
        m_pEnclosure = pEnclosure;
    }
    Port * getSubtractivePort() const {
        return m_pSubtractivePort;
    }
    void setSubtractivePort(Port *pPort) {
        if (pPort != m_pSubtractivePort) {
            m_pSubtractivePort = pPort;
        }
    }
    virtual SSI_ExpanderType getExpanderType() const {
        return SSI_ExpanderTypeUnknown;
    }
    virtual SSI_RoutingDeviceType getRoutingDeviceType() const {
        return SSI_RoutingDeviceTypeUnknown;
    }
    virtual unsigned int getNumberOfPhys() const {
        return m_Phys.size();
    }
    RaidInfo * getRaidInfo() const;
    Controller * getController() const {
        return m_pParent ? m_pParent->getController() : NULL;
    }
    SSI_Status getInfo(SSI_RoutingDeviceInfo *pInfo) const;

};

#endif /* __ROUTING_DEVICE_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
