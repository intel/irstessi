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

#ifndef __END_DEVICE_H__INCLUDED__
#define __END_DEVICE_H__INCLUDED__

// Forward declaration
class Array;
class Enclosure;
class Phy;
class RaidInfo;
class Port;

/**
 */
class EndDevice : public StorageDevice {
public:
    EndDevice(const String &path);
    virtual ~EndDevice();

    // Object

public:
    bool equal(const Object *pObject) const;

    ObjectType getType() const {
        return ObjectType_EndDevice;
    }

    // ScopeObject

public:
    void getPhys(Container<Phy> &container) const;

    bool scopeTypeMatches(SSI_ScopeType scopeType) const {
        return scopeType == SSI_ScopeTypeEndDevice;
    }

    // StorageObject

public:
    void getAddress(SSI_Address &address) const;

    void attachPhy(Phy *pPhy);
    void attachPort(Port *pPort);

    virtual void acquireId(Session *pSession);

    // EndDevice

protected:
    String m_SerialNum;
    String m_SgName;
    Phy *m_pPhy;
    Port *m_pPort;
    String m_Model;
    String m_Firmware;
    unsigned long long m_TotalSize;
    unsigned int m_BlockSize;
    unsigned int m_BlocksFree;
    unsigned long long m_SASAddress;
    SSI_ScsiAddress m_SCSIAddress;
    SSI_WriteCachePolicy m_WriteCachePolicy;

public:
    SSI_Status getInfo(SSI_EndDeviceInfo *info) const;
    RaidInfo * getRaidInfo() const;

    String getSerialNum() const {
        return m_SerialNum;
    }
    Phy * getPhy() const {
        return m_pPhy;
    }
    Port * getPort() const {
        return m_pPort;
    }
    unsigned int getSectors() const {
        return m_BlockSize;
    }
    virtual unsigned char getStoragePoolId() const {
        return 0xff;
    }
    virtual Array * getArray() const {
        return 0;
    }
    virtual Enclosure * getEnclosure() const {
        return 0;
    }
    virtual bool isSystemDisk() const {
        return false;
    }
    virtual unsigned int getSlotNumber() const {
        return -1U;
    }
    virtual SSI_DiskUsage getDiskUsage() const {
        return SSI_DiskUsageUnknown;
    }
    virtual SSI_DiskType getDiskType() const {
        return SSI_DiskTypeUnknown;
    }
    virtual SSI_DiskState getDiskState() const {
        return SSI_DiskStateUnknown;
    }
    virtual SSI_Status unlock(SSI_DiskUnlockInfo *) {
        return SSI_StatusNotSupported;
    }
    virtual SSI_Status assignPoolId(unsigned char) {
        return SSI_StatusNotSupported;
    }
    virtual SSI_Status markAsNormal() {
        return SSI_StatusNotSupported;
    }
    virtual SSI_Status clearMetadata() {
        return SSI_StatusNotSupported;
    }
    virtual SSI_EndDeviceType getDeviceType() const {
        return SSI_EndDeviceTypeNonStorage;
    }
    virtual SSI_Status passthroughCmd(void *pInfo, void *pData, unsigned int dataSize,
        SSI_DataDirection dir);
};

#endif /* __END_DEVICE_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
