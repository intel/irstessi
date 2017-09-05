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

#ifndef __RAID_DEVICE_H__INCLUDED__
#define __RAID_DEVICE_H__INCLUDED__

#include "block_device.h"
#include "filesystem.h"

#ifdef SSI_HAS_PRAGMA_ONCE
#pragma once
#endif

/**
 */
class RaidDevice : public StorageDevice {
public:
    RaidDevice();
    RaidDevice(const String &path);

    // Object

public:
    virtual bool operator ==(const Object &object) const;

    // StorageObject

public:
    virtual void addToSession(const boost::shared_ptr<Session>& pSession);

    virtual void discover();

    // RaidDevice

protected:
    String m_Name;
    String m_Uuid;
    Container<BlockDevice, boost::weak_ptr<BlockDevice> > m_BlockDevices;
    std::list<boost::shared_ptr<String> > m_Components;

    void attachComponent(const Container<EndDevice> &endDevices, const String &devName);

public:
    String getUuid() const {
        return m_Uuid;
    }
    virtual boost::shared_ptr<RaidInfo> getRaidInfo() const;

    void update();
    virtual SSI_Status remove() = 0;
    virtual void create() = 0;

    virtual void setEndDevices(const Container<EndDevice> &endDevices);
    void setName(const String &prefix);
    void determineDeviceName(const String &prefix);
    File getMapFile();

private:
    void __internal_initialize();
    void __internal_update(String &map);
};

#endif /* __RAID_DEVICE_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
