
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
#include "end_device.h"
#include "block_device.h"
// Forward declarations
class Array;
class EndDevice;

/* */
class Volume : public RaidDevice {
public:
    Volume();
    Volume(const String &path, unsigned int orginal);
    ~Volume();

    // ScopeObject

public:
    void getEndDevices(Container<EndDevice> &, bool all) const;

    // StorageObject

public:
    void attachEndDevice(EndDevice *pEndDevice);
    void addToSession(Session *pSession);

    // StorageDevice

public:
    SSI_Status writeStorageArea(void *pBuffer, unsigned int bufferSize);
    SSI_Status  readStorageArea(void *pBuffer, unsigned int bufferSize);

    // RaidDevice

public:
    SSI_Status remove();
    void create();

    // Volume
protected:
    unsigned int m_Ordinal;
    unsigned int m_RaidLevel;
    unsigned int m_MigrationProgress;
    bool m_WriteThrough;
    bool m_CachingEnabled;
    bool m_SystemVolume;
    unsigned int m_MismatchCount;
    unsigned int m_StripSize;
    unsigned long long m_ComponentSize;
    SSI_VolumeState m_State;
    BlockDevice blk;
    BlockDevice *m_pSourceDisk;

public:
    SSI_Status initialize();
    SSI_Status getInfo(SSI_VolumeInfo *pInfo);
    SSI_Status markAsNormal();
    SSI_Status rename(const String &newName);
    SSI_Status rebuild(EndDevice *pEndDevice);
    SSI_Status setCachePolicy(bool off);
    SSI_Status expand(unsigned long long newSize);
    SSI_Status cancelVerify();
    SSI_Status verify(bool repair);
    SSI_Status modify(SSI_StripSize chunkSize, SSI_RaidLevel raidLevel,
        unsigned long long newSize, const Container<EndDevice> &disks);

    void setComponentSize(unsigned long long size) {
        m_ComponentSize = size;
    }

    void setStripSize(SSI_StripSize stripSize);
    void setSourceDisk(EndDevice *pEndDevice);
    void setRaidLevel(SSI_RaidLevel raidLevel);

    SSI_VolumeState getState() const {
        return m_State;
    }
    unsigned int getStripSize() const {
        return m_StripSize;
    }
    unsigned long long getComponentSize() const {
        return m_ComponentSize;
    }

private:
    void __internal_initialize();
    void __wait_for_volume();
    SSI_Status __toRaid0(SSI_StripSize stripSize, unsigned long long newSize, const Container<EndDevice> &disks);
    SSI_Status __toRaid10(SSI_StripSize stripSize, unsigned long long newSize, const Container<EndDevice> &disks);
    SSI_Status __toRaid5(SSI_StripSize stripSize, unsigned long long newSize, const Container<EndDevice> &disks);
};

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
