/*
Copyright (c) 2011 - 2016, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __CONTROLLER_H__INCLUDED__
#define __CONTROLLER_H__INCLUDED__

#include "storage_object.h"
#include <boost/enable_shared_from_this.hpp>

#ifdef SSI_HAS_PRAGMA_ONCE
#pragma once
#endif

/* */
class Controller : public StorageObject, public boost::enable_shared_from_this<Controller> {
public:
    Controller(const String &path);

    // Object

public:
    virtual String getId() const;
    virtual String getPartId() const;

    bool operator ==(const Object &object) const;

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
    void attachEndDevice(const boost::shared_ptr<EndDevice>& pEndDevice);
    void attachRoutingDevice(const boost::shared_ptr<RoutingDevice>& pRoutingDevice);
    void attachPort(const boost::shared_ptr<Port>& pPort);
    void attachVolume(const boost::shared_ptr<Volume>& pVolume);
    void attachPhy(const boost::shared_ptr<Phy>& pPhy);
    void attachArray(const boost::shared_ptr<Array>& pArray);
    void attachEnclosure(const boost::shared_ptr<Enclosure>& pEnclosure);

    void addToSession(const boost::shared_ptr<Session>& pSession);

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
    unsigned char m_prgIface;
    unsigned char m_ClassId;

    boost::shared_ptr<RaidInfo> m_pRaidInfo;

    bool m_twoTbVolumePrebootSupported;
    bool m_twoTbDiskPrebootSupported;
    bool m_ESATASpanning;
    bool m_NVSRAMSupported;
    bool m_HWXORSupported;
    bool m_PhyLocate;
    bool m_DiskUnlock;
    bool m_PatrolReadSupport;
    bool m_ROHISupport;

    SSI_HardwareKeyType m_hardwareMode;
    bool m_supportsTpv;

    virtual SSI_ControllerType getControllerType() const {
        return SSI_ControllerTypeUnknown;
    }

public:
    virtual boost::shared_ptr<RaidInfo> findRaidInfo(Container <RaidInfo> &container);
    SSI_Status readPatrolSetState(bool enable);
    SSI_Status getInfo(SSI_ControllerInfo *pInfo) const;

    boost::shared_ptr<RaidInfo> getRaidInfo() const {
        return m_pRaidInfo;
    }

    boost::shared_ptr<Controller> getController() const {
        return boost::const_pointer_cast<Controller>(shared_from_this());
    }

    unsigned short getPciVendorId() const {
        return m_PciVendorId;
    }

    unsigned short getPciDeviceId() const {
        return m_PciDeviceId;
    }

    SSI_HardwareKeyType getHardwareMode() const {
        return m_hardwareMode;
    }

    SSI_Status makeSpare(const boost::shared_ptr<EndDevice>& pEndDevice);
    void getEnclosures(const boost::shared_ptr<RoutingDevice>& pRoutingDevice, Container<Enclosure> &container);
};

#endif /* __CONTROLLER_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
