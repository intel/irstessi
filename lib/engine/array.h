
/*
Copyright (c) 2011 - 2016, Intel Corporation
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

#ifndef __ARRAY_H__INCLUDED__
#define __ARRAY_H__INCLUDED__

#define MPB_SECTOR_CNT 2210
#define IMSM_RESERVED_SECTORS 4096
#define RAID_SECTOR_SIZE 512


// forward declaration
class Volume;
class BlockDevice;
class EndDevice;

#include "raid_device.h"

/* */
class Array : public RaidDevice {
public:
    Array()
        : RaidDevice() {
    }
    Array(const String &path);
    ~Array();

    // Object

public:
    String getKey() const;

    // ScopeObject

public:
    void getEndDevices(Container<EndDevice> &, bool all) const;
    void getVolumes(Container<Volume> &) const;

    bool scopeTypeMatches(SSI_ScopeType scopeType) const {
        return scopeType == SSI_ScopeTypeArray;
    }

    // StorageObject

public:
    void attachVolume(Volume *pVolume);
    void attachEndDevice(EndDevice *pEndDevice);
    void addToSession(Session *pSession);

    // StorageDevice

public:
    SSI_Status  readStorageArea(void *buffer, unsigned int size);
    SSI_Status writeStorageArea(void *buffer, unsigned int size);

    // RaidDevice

public:
    SSI_Status remove();
    void create();

    // Array

protected:
    Container<Volume> m_Volumes;
    bool m_Busy;
    unsigned long long m_TotalSize;
    unsigned long long m_FreeSize;

public:
    SSI_Status addSpare(EndDevice *pEndDevice);
    SSI_Status addSpare(const Container<EndDevice> &endDevices);
    SSI_Status grow(const Container<EndDevice> &endDevices);
    SSI_Status setWriteCacheState(bool enable);
    void setEndDevices(const Container<EndDevice> &endDevices);
    SSI_Status removeSpare(const EndDevice *pEndDevice, bool force = false);
    SSI_Status removeSpare(const Container<EndDevice>& endDevices, bool force = false);
    SSI_Status removeVolume(const unsigned int ordinal);
    SSI_Status renameVolume(const unsigned int ordinal, String newName);
    SSI_Status assemble();
    SSI_Status getInfo(SSI_ArrayInfo *pInfo) const;

    static Container<EndDevice> getSpareableEndDevices(const Container<EndDevice>& endDevices);

private:
    void __internal_determine_total_and_free_size();
    void __internal_determine_array_name();
    void __internal_attach_end_device(Session *pSession, const String &path);
    void __wait_for_container();
};

#endif /* __ARRAY_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
