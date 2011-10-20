
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

#ifndef __END_DEVICE_H__INCLUDED__
#define __END_DEVICE_H__INCLUDED__

// Forward declaration
class Array;
class Enclosure;
class Phy;
class RaidInfo;
class Port;

/* */
class EndDevice : public StorageDevice {
public:
    EndDevice(const String &path);
    virtual ~EndDevice();

    // Object

public:
    bool equal(const Object *pObject) const;
    String getKey() const;
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
    Enclosure *m_pEnclosure;
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
    Controller * getController() const {
        return m_pParent?m_pParent->getController():0;
    }

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
        return m_pEnclosure;
    }
    virtual void setEnclosure(Enclosure *pEnclosure) {
        m_pEnclosure = pEnclosure;
    }
    virtual bool isSystemDisk() const {
        return false;
    }
    virtual unsigned int getSlotNumber() const;
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
    virtual SSI_Status makeSpare() {
        return SSI_StatusInvalidState;
    }
};

#endif /* __END_DEVICE_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
