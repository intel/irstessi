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

#ifndef __VOLUME_H__INCLUDED__
#define __VOLUME_H__INCLUDED__

#include "raid_device.h"
#include "block_device.h"
#include <boost/enable_shared_from_this.hpp>

#ifdef SSI_HAS_PRAGMA_ONCE
#pragma once
#endif

/* */
class Volume : public RaidDevice, public boost::enable_shared_from_this<Volume> {
public:
    Volume();
    Volume(const String &path, unsigned int ordinal);

    virtual String getId() const;

    // ScopeObject

public:
    virtual void getEndDevices(Container<EndDevice> &, bool all) const;

    // StorageObject

public:
    virtual void attachEndDevice(const boost::shared_ptr<EndDevice>& pEndDevice);
    virtual void addToSession(const boost::shared_ptr<Session>& pSession);

    virtual void discover();

    // RaidDevice

public:
    virtual SSI_Status remove();
    virtual void create();

    // Volume
protected:
    unsigned int m_Ordinal;
    unsigned int m_RaidLevel;
    bool m_WriteThrough;
    bool m_CachingEnabled;
    bool m_SystemVolume;
    unsigned int m_MismatchCount;
    unsigned int m_StripSize;
    unsigned int m_MigrProgress;
    unsigned long long m_ComponentSize;
    SSI_VolumeState m_State;
    boost::shared_ptr<BlockDevice> blk;
    boost::shared_ptr<BlockDevice> m_pSourceDisk;
    SSI_RwhPolicy m_RwhPolicy;

public:
    SSI_Status initialize();
    SSI_Status getInfo(SSI_VolumeInfo *pInfo);
    SSI_Status markAsNormal();
    SSI_Status rename(const String &newName);
    SSI_Status rebuild(const boost::shared_ptr<EndDevice>& pEndDevice);
    SSI_Status setCachePolicy(bool off);
    SSI_Status expand(unsigned long long newSize);
    SSI_Status cancelVerify();
    SSI_Status verify(bool repair);
    SSI_Status modify(SSI_StripSize chunkSize, SSI_RaidLevel raidLevel, unsigned long long newSize, const Container<EndDevice> &disks);
    SSI_Status changeRwhPolicy(SSI_RwhPolicy policy);

    void setComponentSize(unsigned long long size, unsigned long long diskCount, SSI_RaidLevel level);
    void setStripSize(SSI_StripSize stripSize);
    void setSourceDisk(const boost::shared_ptr<EndDevice>& pEndDevice);
    void setRaidLevel(SSI_RaidLevel raidLevel);
    void setRwhPolicy(SSI_RwhPolicy policy);

    SSI_VolumeState getState() const {
        return m_State;
    }
    unsigned int getStripSize() const {
        return m_StripSize;
    }
    unsigned long long getComponentSize() const {
        return m_ComponentSize;
    }
    String getName() const {
        return m_Name;
    }
    bool hasSourceDisk() const {
        return m_pSourceDisk != NULL;
    }
    bool isSystemVolume() const {
        return m_SystemVolume;
    }
    unsigned long long getTotalSize() const {
        return blk->getTotalSize();
    }
    unsigned long long getLogicalSectorSize() const {
        return blk->getLogicalSectorSize();
    }
    SSI_RaidLevel getSsiRaidLevel() const;
    SSI_StripSize getSsiStripSize() const;

private:
    void __internal_initialize();
    void __wait_for_volume();
    SSI_Status __toRaid0(SSI_StripSize stripSize, unsigned long long newSize, const Container<EndDevice> &disks);
    SSI_Status __toRaid10(SSI_StripSize stripSize, unsigned long long newSize, const Container<EndDevice> &disks);
    SSI_Status __toRaid5(SSI_StripSize stripSize, unsigned long long newSize, const Container<EndDevice> &disks);

    unsigned int getRaidLevel(const String& raidLevel);
    String getMdadmAttribute(const String &attribute);
    unsigned int getPercentageStatus(const String &attribute);

    unsigned int getMigrationProgress();
    unsigned int getMigrationTargetLevel();
    unsigned int getVerificationProgress();

    SSI_RwhPolicy parseRwhPolicy(const String& policy) const;
    SSI_RwhPolicy parseNewRwhPolicy(const String& policy) const;
    String rwhPolicyToString(SSI_RwhPolicy policy) const;
    String newRwhPolicyToString(SSI_RwhPolicy policy) const;
};

#endif /* __VOLUME_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
