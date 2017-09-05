/*
 * Copyright 2011 - 2017 Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __END_DEVICE_H__INCLUDED__
#define __END_DEVICE_H__INCLUDED__

#define DEFAULT_SECTOR_SIZE 512
#define KILOBYTE 1024

#include "storage_device.h"
#include <boost/enable_shared_from_this.hpp>

#ifdef SSI_HAS_PRAGMA_ONCE
#pragma once
#endif

/* */
class EndDevice : public StorageDevice, public boost::enable_shared_from_this<EndDevice> {
public:
    EndDevice(const String &path);

    // Object
private:
    EndDevice(const EndDevice &endDevice);
    void operator =(const EndDevice&) {}

public:
    bool operator ==(const Object &object) const;
    virtual String getId() const;
    virtual String getPartId() const;

    // ScopeObject

public:
    virtual void getPhys(Container<Phy> &container) const;
    virtual void getPorts(Container<Port> &container) const;

    virtual bool scopeTypeMatches(SSI_ScopeType scopeType) const {
        return scopeType == SSI_ScopeTypeEndDevice;
    }

    // StorageObject

public:
    virtual void getAddress(SSI_Address &address) const;

    virtual void attachPhy(const boost::shared_ptr<Phy>& pPhy);
    virtual void attachPort(const boost::shared_ptr<Port>& pPort);

    virtual void addToSession(const boost::shared_ptr<Session>& pSession);

    // EndDevice

public:
    virtual void discover();

protected:
    String m_VendorId;
    String m_SerialNum;
    String m_SgName;
    boost::shared_ptr<Phy> m_pPhy;
    boost::shared_ptr<Port> m_pPort;
    boost::shared_ptr<Enclosure> m_pEnclosure;
    String m_Model;
    String m_Firmware;
    unsigned long long m_TotalSize;
    unsigned long long m_BlocksTotal;
    unsigned long long m_BlocksFree;
    unsigned int m_LogicalSectorSize;
    unsigned int m_PhysicalSectorSize;
    unsigned long long m_SASAddress;
    SSI_ScsiAddress m_SCSIAddress;
    SSI_BdfAddress m_BDFAddress;
    SSI_WriteCachePolicy m_WriteCachePolicy;
    unsigned int m_ledState;
    unsigned long long m_systemIoBusNumer;
    unsigned long long m_PCISlotNumber;
    unsigned int m_FDx8Disk;
    unsigned int m_vmdDomain;
    bool m_isIntelNvme;
    bool m_locateLedSupport;
    int getAtaDiskInfo(const String &devPath, String &model, String &serial, String &firmware);
    virtual SSI_Status removeDevice() {
        return SSI_StatusNotSupported;
    }

public:
    SSI_Status getInfo(SSI_EndDeviceInfo *info) const;
    boost::shared_ptr<RaidInfo> getRaidInfo() const;
    boost::shared_ptr<Controller> getController() const {
        if (Parent parent = m_pParent.lock()) {
            return parent->getController();
        } else {
            return boost::shared_ptr<Controller>();
        }
    }
    virtual SSI_Status locate(bool mode) const;
    virtual SSI_Status removeDisk();

    unsigned long long getTotalSize() const {
        return m_TotalSize;
    }

    unsigned int getLogicalSectorSize() const {
        return m_LogicalSectorSize;
    }

    unsigned int getPhysicalSectorSize() const {
        return m_PhysicalSectorSize;
    }

    String getVendorId() const {
        return m_VendorId;
    }

    String getSerialNum() const {
        return m_SerialNum;
    }

    virtual boost::shared_ptr<Phy> getPhy() const {
        return m_pPhy;
    }

    virtual boost::shared_ptr<Port> getPort() const {
        return m_pPort;
    }

    bool isIntelNvme() const {
        return m_isIntelNvme;
    }

    unsigned int getSectors() const {
        return m_BlocksTotal;
    }

    bool isFultondalex8() const {
        return m_FDx8Disk != 0;
    }

    virtual bool canRemoveDisk() const {
        return false;
    }

    unsigned int getVmdDomain() const {
        return m_vmdDomain;
    }

    virtual unsigned char getStoragePoolId() const {
        return 0xff;
    }
    virtual boost::shared_ptr<Array> getArray() const {
        return boost::shared_ptr<Array>();
    }
    virtual boost::shared_ptr<Enclosure> getEnclosure() const {
        return m_pEnclosure;
    }
    virtual void setEnclosure(const boost::shared_ptr<Enclosure>& pEnclosure) {
        m_pEnclosure = pEnclosure;
    }
    virtual bool isSystemDisk() const {
        return false;
    }
    virtual unsigned long long getSlotNumber() const;
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
    virtual SSI_Status passthroughCmd(void *pInfo, void *pData, unsigned int dataSize, SSI_DataDirection dir);
    virtual SSI_Status makeSpare() {
        return SSI_StatusInvalidState;
    }

    virtual SSI_Status resetSmartEvent() {
        return SSI_StatusNotSupported;
    }

    void determineBlocksFree(const boost::shared_ptr<Array>& pArray);
};

#endif /* __END_DEVICE_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
