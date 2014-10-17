
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

#ifndef __CONTROLLER_H__INCLUDED__
#define __CONTROLLER_H__INCLUDED__

// Forward declaration
class RaidInfo;
class RoutingDevice;
class EndDevice;
class Phy;
class Port;
class Volume;
class Array;
class Enclosure;

/* */
class Controller : public StorageObject {
public:
    Controller(const String &path);
    virtual ~Controller();

    // Object

public:
    bool operator ==(const Object &object) const;

    ObjectType getType() const {
        return ObjectType_Controller;
    }
    String getKey() const {
        return m_Path;
    }

    // ScopeObject

public:
    void getEndDevices(Container<EndDevice> &, bool all) const;
    void getRoutingDevices(Container<RoutingDevice> &, bool all) const;
    void getPorts(Container<Port> &) const;
    void getVolumes(Container<Volume> &) const;
    void getArrays(Container<Array> &) const;
    void getPhys(Container<Phy> &) const;
    void getEnclosures(Container<Enclosure> &, bool all) const;

    bool scopeTypeMatches(SSI_ScopeType scopeType) const {
        return scopeType == SSI_ScopeTypeControllerDirect ||
               scopeType == SSI_ScopeTypeControllerAll;
    }

    // StorageObject

public:
    void attachEndDevice(EndDevice *pEndDevice);
    void attachRoutingDevice(RoutingDevice *pRoutingDevice);
    void attachPort(Port *pPort);
    void attachVolume(Volume *pVolume);
    void attachPhy(Phy *pPhy);
    void attachArray(Array *pArray);
    void attachEnclosure(Enclosure *pEnclosure);

    void acquireId(Session *pSession);

    // Controller

protected:
    Container<EndDevice> m_EndDevices_Direct;
    Container<EndDevice> m_EndDevices;
    Container<RoutingDevice> m_RoutingDevices_Direct;
    Container<RoutingDevice> m_RoutingDevices;
    Container<Port> m_Ports;
    Container<Volume> m_Volumes;
    Container<Phy> m_Phys;
    Container<Array> m_Arrays;
    Container<Enclosure> m_Enclosures_Direct;
    Container<Enclosure> m_Enclosures;

    String m_Name;
    String m_PrebootMgrVersion;
    String m_DriverVersion;

    unsigned short m_PciVendorId;
    unsigned short m_PciDeviceId;
    unsigned short m_SubSystemId;
    unsigned char m_HardwareRevisionId;
    unsigned char m_SubClassCode;
    unsigned short m_SubVendorId;

    RaidInfo *m_pRaidInfo;

    bool m_twoTbVolumePrebootSupported;
    bool m_twoTbDiskPrebootSupported;
    bool m_ESATASpanning;
    bool m_NVSRAMSupported;
    bool m_HWXORSupported;
    bool m_PhyLocate;
    bool m_DiskUnlock;
    bool m_PatrolReadSupport;

    virtual SSI_ControllerType getControllerType() const {
        return SSI_ControllerTypeUnknown;
    }

public:
    RaidInfo *findRaidInfo(Container<RaidInfo> &container);
    virtual RaidInfo *findRaidInfo();
    SSI_Status readPatrolSetState(bool enable);
    SSI_Status getInfo(SSI_ControllerInfo *pInfo) const;

    RaidInfo * getRaidInfo() const {
        return m_pRaidInfo;
    }
    Controller * getController() const {
        return const_cast<Controller *>(this);
    }
    SSI_Status makeSpare(EndDevice *pEndDevice);
    void getEnclosures(RoutingDevice *pRoutingDevice, Container<Enclosure> &container);
};

#endif /* __CONTROLLER_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
