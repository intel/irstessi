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

#ifndef __RAID_DEVICE_H__INCLUDED__
#define __RAID_DEVICE_H__INCLUDED__

// Forward declaration
class Session;
class BlockDevice;

/**
 */
class RaidDevice : public StorageDevice {
public:
    RaidDevice();
    RaidDevice(const String &path);
    virtual ~RaidDevice();

    // Object

public:
    bool equal(const Object *pObject) const;

    // StorageObject

public:
    virtual void acquireId(Session *pSession);

    // RaidDevice

protected:
    String m_Name;
    String m_Uuid;
    Container<BlockDevice> m_BlockDevices;
    List<String *> m_Components;

    void attachComponent(const Container<EndDevice> &endDevices, const String &devName);

public:
    String getUuid() const {
        return m_Uuid;
    }

    void update();
    virtual SSI_Status remove() = 0;
    virtual void create() = 0;

    void setEndDevices(const Container<EndDevice> &endDevices);
    void setName(const String &prefix);
    void determineDeviceName(const String &prefix);

private:
    void __internal_initialize();
};

#endif /* __RAID_DEVICE_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
