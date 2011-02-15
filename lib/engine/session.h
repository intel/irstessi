/*
 * IMPORTANT - READ BEFORE COPYING, INSTALLING OR USING.
 * BY LOADING OR USING THIS SOFTWARE, YOU AGREE TO THE TERMS OF THIS INTEL
 * SOFTWARE LICENSE AGREEMENT.  IF YOU DO NOT WISH TO SO AGREE, DO NOT COPY,
 * INSTALL OR USE THIS SOFTWARE.
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2010 Intel Corporation. All Rights Reserved.
 *
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation or its
 * suppliers or licensors.
 *
 * Title to the Material remains with Intel Corporation or its suppliers and
 * licensors. The Material contains trade secrets and proprietary and
 * confidential information of Intel or its suppliers and licensors.
 * The Material is protected by worldwide copyright and trade secret laws and
 * treaty provisions. No part of the Material may be used, copied, reproduced,
 * modified, published, uploaded, posted, transmitted, distributed,
 * or disclosed in any way without Intel's prior express written permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure
 * or delivery of the Materials, either expressly, by implication, inducement,
 * estoppel or otherwise. Any license under such intellectual property rights
 * must be express and approved by Intel in writing.
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
    Container m_EndDevices;
    Container m_Arrays;
    Container m_Enclosures;
    Container m_RaidInfo;
    Container m_Phys;
    Container m_Volumes;
    Container m_Ports;
    Container m_RoutingDevices;
    Container m_Controllers;

public:
    Container getEndDevices() const {
        return m_EndDevices;
    }
    Container getArrays() const {
        return m_Arrays;
    }
    Container getEnclosures() const {
        return m_Enclosures;
    }
    Container getRaidInfo() const {
        return m_RaidInfo;
    }
    Container getPhys() const {
        return m_Phys;
    }
    Container getVolumes() const {
        return m_Volumes;
    }
    Container getPorts() const {
        return m_Ports;
    }
    Container getRoutingDevices() const {
        return m_RoutingDevices;
    }
    Container getControllers() const {
        return m_Controllers;
    }

private:
    NoneScopeObject *m_pNoneScopeObj;

    void __discoverControllers();
    void __discoverRaidDevices();
    void __releaseIds(Container &) const;

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

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
