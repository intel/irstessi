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

#ifndef __CONTROLLER_H__INCLUDED__
#define __CONTROLLER_H__INCLUDED__

// Forward declaration
class RaidInfo;

/* */
class Controller : public StorageObject {
public:
    Controller(const String &path);
    virtual ~Controller();

    // Object

public:
    bool equal(const Object *pObject) const;

    ObjectType getType() const {
        return ObjectType_Controller;
    }

    // ScopeObject

public:
    void getEndDevices(Container &, bool all) const;
    void getRoutingDevices(Container &, bool all) const;
    void getPorts(Container &) const;
    void getVolumes(Container &) const;
    void getArrays(Container &) const;
    void getPhys(Container &) const;
    void getEnclosures(Container &, bool all) const;

    bool scopeTypeMatches(SSI_ScopeType scopeType) const {
        return scopeType == SSI_ScopeTypeControllerDirect ||
               scopeType == SSI_ScopeTypeControllerAll;
    }

    // StorageObject

public:
    void attachEndDevice(Object *pEndDevice);
    void attachRoutingDevice(Object *pRoutingDevice);
    void attachPort(Object *pPort);
    void attachVolume(Object *pVolume);
    void attachPhy(Object *pPhy);
    void attachArray(Object *pArray);
    void attachEnclosure(Object *pEnclosure);

    void acquireId(Session *pSession);

    // Controller

protected:
    Container m_EndDevices;
    Container m_EndDevices_Direct;
    Container m_RoutingDevices;
    Container m_RoutingDevices_Direct;
    Container m_Ports;
    Container m_Volumes;
    Container m_Phys;
    Container m_Arrays;
    Container m_Enclosures_Direct;
    Container m_Enclosures;

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
    SSI_Status readPatrolSetState(bool enable);
    SSI_Status getInfo(SSI_ControllerInfo *pInfo) const;

    RaidInfo * getRaidInfo() const {
        return m_pRaidInfo;
    }
};

#endif /* __CONTROLLER_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
