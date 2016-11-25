/*
Copyright (c) 2011 - 2016, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __SESSION_H__INCLUDED__
#define __SESSION_H__INCLUDED__

#include "scope_object.h"
#include <boost/enable_shared_from_this.hpp>

#ifdef SSI_HAS_PRAGMA_ONCE
#pragma once
#endif

// Forward declarations
class StorageDevice;
class EndDevice;
class Volume;
class Array;
class RoutingDevice;
class Port;
class Enclosure;
class Phy;
class Controller;
class RaidInfo;
class StorageObject;

/* */
class Session : public ScopeObject, public boost::enable_shared_from_this<Session> {
public:
    Session();
    virtual ~Session();

protected:
    Container<EndDevice> m_EndDevices;
    Container<Array> m_Arrays;
    Container<Enclosure> m_Enclosures;
    Container<RaidInfo> m_RaidInfo;
    Container<Phy> m_Phys;
    Container<Volume> m_Volumes;
    Container<Port> m_Ports;
    Container<RoutingDevice> m_RoutingDevices;
    Container<Controller> m_Controllers;
    Container<StorageObject> m_Objects;

public:
    void initialize();

    void getEndDevices(Container<EndDevice> &container, bool) const {
        container = m_EndDevices;
    }
    void getArrays(Container<Array> &container) const {
        container = m_Arrays;
    }
    void getEnclosures(Container<Enclosure> &container, bool) const {
        container = m_Enclosures;
    }
    void getRaidInfo(Container<RaidInfo> &container) const {
        container = m_RaidInfo;
    }
    void getPhys(Container<Phy> &container) const {
        container = m_Phys;
    }
    void getVolumes(Container<Volume> &container) const {
        container = m_Volumes;
    }
    void getPorts(Container<Port> &container) const {
        container = m_Ports;
    }
    void getRoutingDevices(Container<RoutingDevice> &container, bool) const {
        container = m_RoutingDevices;
    }
    void getControllers(Container<Controller> &container) const {
        container = m_Controllers;
    }
    bool scopeTypeMatches(SSI_ScopeType scopeType) const {
        return scopeType == SSI_ScopeTypeNone;
    }

private:
    void __internal_attach_imsm_array(const String &path);
    void __internal_attach_imsm_device(const String &path);

public:
    boost::shared_ptr<ScopeObject> getObject(SSI_Handle handle);
    boost::shared_ptr<StorageDevice> getDevice(SSI_Handle handle) const;
    boost::shared_ptr<EndDevice> getEndDevice(SSI_Handle handle) const;
    boost::shared_ptr<Array> getArray(SSI_Handle handle) const;
    boost::shared_ptr<RoutingDevice> getRoutingDevice(SSI_Handle handle) const;
    boost::shared_ptr<Enclosure> getEnclosure(SSI_Handle handle) const;
    boost::shared_ptr<Phy> getPhy(SSI_Handle handle) const;
    boost::shared_ptr<Volume> getVolume(SSI_Handle handle) const;
    boost::shared_ptr<Port> getPort(SSI_Handle handle) const;
    boost::shared_ptr<Controller> getController(SSI_Handle handle) const;
    boost::shared_ptr<RaidInfo> getRaidInfo(SSI_Handle handle) const;

public:
    bool operator ==(const Object &object) const;

    void addEndDevice(const boost::shared_ptr<EndDevice>& pEndDevice);
    void addArray(const boost::shared_ptr<Array>& pArray);
    void addRoutingDevice(const boost::shared_ptr<RoutingDevice>& pRoutingDevice);
    void addEnclosure(const boost::shared_ptr<Enclosure>& pEnclosure);
    void addPhy(const boost::shared_ptr<Phy>& pPhy);
    void addVolume(const boost::shared_ptr<Volume>& pVolume);
    void addPort(const boost::shared_ptr<Port>& pPort);
    void addController(const boost::shared_ptr<Controller>& pController);
    void addRaidInfo(const boost::shared_ptr<RaidInfo>& pRaidInfo);
};

#endif /* __SESSION_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
