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

	 bool equal(const Object *pObject) const {
		return false;
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

    bool scopeTypeMatches(SSI_ScopeType scopeType) const {
        return scopeType == SSI_ScopeTypeRoutingDevice;
    }

    // StorageObject

public:
    void attachEndDevice(EndDevice *pEndDevice);
    void attachVolume(Volume *pVolume);
    void attachPhy(Phy *pPhy);
    void attachRoutingDevice(RoutingDevice *pRoutingDevice);
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
        return m_Phys;
    }
    RaidInfo * getRaidInfo() const;
    SSI_Status getInfo(SSI_RoutingDeviceInfo *pInfo) const;

};

#endif /* __ROUTING_DEVICE_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
