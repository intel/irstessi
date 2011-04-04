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

#ifndef __ARRAY_H__INCLUDED__
#define __ARRAY_H__INCLUDED__

// forward declaration
class Volume;
class BlockDevice;
class EndDevice;

/* */
class Array : public RaidDevice {
public:
    Array()
        : RaidDevice(0) {
    }
    Array(const String &path);
    ~Array();

    // Object

public:
    ObjectType getType() const {
        return ObjectType_Array;
    }

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
    void acquireId(Session *pSession);

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
    SSI_Status addSpare(const EndDevice *pEndDevice);
    SSI_Status addSpare(const Container<EndDevice> &endDevices);
    SSI_Status setWriteCacheState(bool enable);
    SSI_Status removeSpare(const EndDevice *pEndDevice);

    SSI_Status getInfo(SSI_ArrayInfo *pInfo) const;

private:
    void __internal_determine_total_and_free_size();
    void __internal_determine_array_name();
    void __internal_attach_end_device(Session *pSession, const String &path);
};

#endif /* __ARRAY_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
