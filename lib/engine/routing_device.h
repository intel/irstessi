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
    ObjectType getType() const {
        return ObjectType_RoutingDevice;
    }

    // ScopeObject

 public:
    virtual void getPhys(Container &) const;
    void getRoutingDevices(Container &, bool all) const;
    void getPorts(Container &) const;
    void getEndDevices(Container &, bool all) const;

    bool scopeTypeMatches(SSI_ScopeType scopeType) const {
        return scopeType == SSI_ScopeTypeRoutingDevice;
    }

    // StorageObject

public:
    void attachEndDevice(Object *pEndDevice);
    void attachVolume(Object *pVolume);
    void attachPhy(Object *pPhy);
    void attachRoutingDevice(Object *pRoutingDevice);
    void attachPort(Object *pPort);
    void attachArray(Object *pArray);

    virtual void acquireId(Session *pSession);

    // RoutingDevice

protected:
    Container m_EndDevices;
    Container m_EndDevices_Direct;
    Container m_Phys;
    Container m_Ports;
    Container m_RoutingDevices;
    Container m_RoutingDevices_Direct;
    Port *m_pSubtractivePort;
    String m_ProductId;
    String m_Vendor;
    String m_ProductRev;
    String m_ComponentVendorId;
    String m_ComponentId;
    String m_ComponentRev;

public:
    virtual Enclosure * getEnclosure() const {
        return 0;
    }
    Object * getPort() const {
        return reinterpret_cast<Object *>(m_pSubtractivePort);
    }
    virtual SSI_ExpanderType getExpanderType() const {
        return SSI_ExpanderTypeUnknown;
    }
    virtual SSI_RoutingDeviceType getRoutingDeviceType() const {
        return SSI_RoutingDeviceTypeUnknown;
    }
    virtual unsigned int getNumberOfPhys() const {
        return m_Phys;
    }
    RaidInfo * getRaidInfo() const;
    SSI_Status getInfo(SSI_RoutingDeviceInfo *pInfo) const;

};

#endif /* __ROUTING_DEVICE_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
