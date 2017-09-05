/*
 * Copyright 2011 - 2017 Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __ROUTING_DEVICE_H__INCLUDED__
#define __ROUTING_DEVICE_H__INCLUDED__

#include "storage_object.h"
#include <boost/enable_shared_from_this.hpp>

#ifdef SSI_HAS_PRAGMA_ONCE
#pragma once
#endif

/* */
class RoutingDevice : public StorageObject, public boost::enable_shared_from_this<RoutingDevice> {
 public:
    RoutingDevice(const String &path);

    // Object

 public:

    virtual bool operator ==(const Object &) const {
        return false;
    }
    virtual String getId() const;
    virtual String getPartId() const;

    // ScopeObject

 public:
    virtual void getPhys(Container<Phy> &) const;
    virtual void getRoutingDevices(Container<RoutingDevice> &, bool all) const;
    virtual void getPorts(Container<Port> &) const;
    virtual void getEndDevices(Container<EndDevice> &, bool all) const;
    virtual void getEnclosures(Container<Enclosure> &, bool all) const;

    virtual bool scopeTypeMatches(SSI_ScopeType scopeType) const {
        return scopeType == SSI_ScopeTypeRoutingDevice;
    }

    virtual void discover();

    // StorageObject

public:
    virtual void attachEndDevice(const boost::shared_ptr<EndDevice>& pEndDevice);
    virtual void attachVolume(const boost::shared_ptr<Volume>& pVolume);
    virtual void attachPhy(const boost::shared_ptr<Phy>& pPhy);
    virtual void attachRoutingDevice(const boost::shared_ptr<RoutingDevice>& pRoutingDevice);
    virtual void attachEnclosure(const boost::shared_ptr<Enclosure>& pEnclosure);
    virtual void attachPort(const boost::shared_ptr<Port>& pPort);
    virtual void attachArray(const boost::shared_ptr<Array>& pArray);

    virtual void addToSession(const boost::shared_ptr<Session>& pSession);

    // RoutingDevice

protected:
    Container<EndDevice> m_EndDevices;
    Container<EndDevice> m_EndDevices_Direct;
    Container<Phy> m_Phys;
    Container<Port> m_Ports;
    Container<RoutingDevice> m_RoutingDevices;
    Container<RoutingDevice> m_RoutingDevices_Direct;
    boost::weak_ptr<Enclosure> m_pEnclosure;
    boost::shared_ptr<Port> m_pSubtractivePort;
    String m_ProductId;
    String m_Vendor;
    String m_ProductRev;
    String m_ComponentVendorId;
    String m_ComponentId;
    String m_ComponentRev;

public:
    virtual boost::shared_ptr<Enclosure> getEnclosure() const {
        return m_pEnclosure.lock();
    }
    virtual void setEnclosure(const boost::shared_ptr<Enclosure>& pEnclosure) {
        m_pEnclosure = pEnclosure;
    }
    boost::shared_ptr<Port> getSubtractivePort() const {
        return m_pSubtractivePort;
    }
    void setSubtractivePort(boost::shared_ptr<Port>& pPort) {
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
    boost::shared_ptr<RaidInfo> getRaidInfo() const;
    boost::shared_ptr<Controller> getController() const {
        if (Parent parent = m_pParent.lock()) {
            return parent->getController();
        } else {
            return boost::shared_ptr<Controller>();
        }
    }
    SSI_Status getInfo(SSI_RoutingDeviceInfo *pInfo) const;

};

#endif /* __ROUTING_DEVICE_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
