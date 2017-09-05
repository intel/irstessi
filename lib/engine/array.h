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

#ifndef __ARRAY_H__INCLUDED__
#define __ARRAY_H__INCLUDED__

#define MPB_SECTOR_CNT 2210
#define IMSM_RESERVED_SECTORS 4096

#include "raid_device.h"

#ifdef SSI_HAS_PRAGMA_ONCE
#pragma once
#endif

/* */
class Array : public RaidDevice, public boost::enable_shared_from_this<Array> {
public:
    Array();
    Array(const String &path);

    // Object

public:
    virtual String getId() const;
    virtual String getPartId() const;

    // ScopeObject

public:
    virtual void getEndDevices(Container<EndDevice> &, bool all) const;
    virtual void getVolumes(Container<Volume> &) const;

    virtual bool scopeTypeMatches(SSI_ScopeType scopeType) const {
        return scopeType == SSI_ScopeTypeArray;
    }

    // StorageObject

public:
    virtual void attachVolume(const boost::shared_ptr<Volume>& pVolume);
    virtual void attachEndDevice(const boost::shared_ptr<EndDevice>& pEndDevice);
    virtual void addToSession(const boost::shared_ptr<Session>& pSession);

    virtual void discover();

    // RaidDevice

public:
    virtual SSI_Status remove();
    virtual void create();

    // Array

protected:
    Container<Volume> m_Volumes;
    bool m_Busy;
    unsigned long long m_TotalSize;
    unsigned long long m_FreeSize;

public:
    SSI_Status addSpare(const boost::shared_ptr<EndDevice>& pEndDevice);
    SSI_Status addSpare(const Container<EndDevice> &endDevices);
    SSI_Status grow(const Container<EndDevice> &endDevices);
    SSI_Status setWriteCacheState(bool enable);
    virtual void setEndDevices(const Container<EndDevice> &endDevices);
    SSI_Status removeSpare(const boost::shared_ptr<EndDevice>& pEndDevice, bool force = false);
    SSI_Status removeSpare(const Container<EndDevice>& endDevices, bool force = false);
    SSI_Status removeVolume(const unsigned int ordinal);
    SSI_Status renameVolume(const unsigned int ordinal, String newName);
    SSI_Status assemble();
    SSI_Status getInfo(SSI_ArrayInfo *pInfo) const;
    static Container<EndDevice> getSpareableEndDevices(const Container<EndDevice>& endDevices);
    SSI_Status canAddEndDevices(const Container<EndDevice>& endDevices) const;
private:
    void __internal_determine_total_and_free_size();
    void __internal_determine_array_name();
    void __internal_attach_end_device(const boost::shared_ptr<Session>& pSession, const String &path);
    void __wait_for_container();
};

#endif /* __ARRAY_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
