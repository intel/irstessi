/*
Copyright (c) 2011 - 2016, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __BLOCK_DEVICE_H__INCLUDED__
#define __BLOCK_DEVICE_H__INCLUDED__

#include "end_device.h"

#ifdef SSI_HAS_PRAGMA_ONCE
#pragma once
#endif

/* */
class BlockDevice : public EndDevice {
public:
    BlockDevice(const String &path);

    // EndDevice

public:
    virtual void discover();

    virtual SSI_EndDeviceType getDeviceType() const {
        return SSI_EndDeviceTypeDisk;
    }

    virtual bool isSystemDisk() const {
        return m_IsSystem;
    }

    virtual SSI_DiskUsage getDiskUsage() const {
        return m_DiskUsage;
    }

    virtual SSI_DiskState getDiskState() const {
        return m_DiskState;
    }

    virtual unsigned char getStoragePoolId() const {
        return m_StoragePoolId;
    }

    virtual SSI_Status makeSpare();

    // BlockDevice

protected:
    Container<Volume> m_Volumes;
    boost::shared_ptr<Array> m_pArray;
    SSI_DiskUsage m_DiskUsage;
    SSI_DiskState m_DiskState;
    bool m_IsSystem;
    unsigned char m_StoragePoolId;

public:
    SSI_Status unlock(SSI_DiskUnlockInfo *pInfo);
    SSI_Status clearMetadata();
    SSI_Status assignPoolId(unsigned char poolId);
    SSI_Status markAsNormal();

    void attachArray(const boost::shared_ptr<Array>& pArray);
    void attachVolume(const boost::shared_ptr<Volume>& pVolume);

public:
    virtual boost::shared_ptr<Array> getArray() const {
        return m_pArray;
    }
    void setWriteCache(bool enable);

protected:
    void __internal_determine_disk_usage();
    void __internal_determine_disk_state();
    void __internal_determine_disk_is_system();
};

#endif /* __BLOCK_DEVICE_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
