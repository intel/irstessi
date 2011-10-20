
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

#ifndef __SESSION_H__INCLUDED__
#define __SESSION_H__INCLUDED__

// Forward declarations
class SessionManager;

// Forward declarations
class Object;
class ScopeObject;
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

/* */
class Session : public Object {
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

public:
    Container<EndDevice> getEndDevices() const {
        return m_EndDevices;
    }
    Container<Array> getArrays() const {
        return m_Arrays;
    }
    Container<Enclosure> getEnclosures() const {
        return m_Enclosures;
    }
    Container<RaidInfo> getRaidInfo() const {
        return m_RaidInfo;
    }
    Container<Phy> getPhys() const {
        return m_Phys;
    }
    Container<Volume> getVolumes() const {
        return m_Volumes;
    }
    Container<Port> getPorts() const {
        return m_Ports;
    }
    Container<RoutingDevice> getRoutingDevices() const {
        return m_RoutingDevices;
    }
    Container<Controller> getControllers() const {
        return m_Controllers;
    }

private:
    NoneScopeObject *m_pNoneScopeObj;

    void __internal_attach_imsm_array(const String &path);
    void __internal_attach_imsm_device(const String &path);

public:
    ScopeObject * getObject(unsigned int id) const;
    StorageDevice * getDevice(unsigned int id) const;
    EndDevice * getEndDevice(unsigned int id) const;
    Array * getArray(unsigned int id) const;
    RoutingDevice * getRoutingDevice(unsigned int id) const;
    Enclosure * getEnclosure(unsigned int id) const;
    Phy * getPhy(unsigned int id) const;
    Volume * getVolume(unsigned int id) const;
    Port * getPort(unsigned int id) const;
    Controller * getController(unsigned int id) const;
    RaidInfo * getRaidInfo(unsigned int id) const;

public:
    bool equal(const Object *pObject) const;
    String getKey() const;
    ObjectType getType() const {
        return ObjectType_Session;
    }

    void addEndDevice(EndDevice *pEndDevice);
    void addArray(Array *pArray);
    void addRoutingDevice(RoutingDevice *pRoutingDevice);
    void addEnclosure(Enclosure *pEnclosure);
    void addPhy(Phy *pPhy);
    void addVolume(Volume *pVolume);
    void addPort(Port *pPort);
    void addController(Controller *pController);
    void addRaidInfo(RaidInfo *pRaidInfo);
};

#endif /* __SESSION_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
