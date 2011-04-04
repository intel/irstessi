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

// Forward declarations
class Array;
class EndDevice;

/* */
class Volume : public RaidDevice {
public:
    Volume();
    Volume(const String &path, unsigned int orginal);
    ~Volume();

    // Object

public:
    ObjectType getType() const {
        return ObjectType_Volume;
    }

    // ScopeObject

public:
    void getEndDevices(Container<EndDevice> &, bool all) const;

    // StorageObject

public:
    void attachEndDevice(EndDevice *pEndDevice);
    void acquireId(Session *pSession);

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
    unsigned long long m_TotalSize;
    unsigned int m_RaidLevel;
    unsigned int m_MigrationProgress;
    bool m_WriteThrough;
    bool m_CachingEnabled;
    bool m_SystemVolume;
    unsigned int m_MismatchCount;
    unsigned int m_StripSize;
    unsigned long long m_ComponentSize;
    SSI_VolumeState m_State;
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
    unsigned long long getComponentSize() const {
        return m_ComponentSize;
    }

private:
    void __internal_initialize();
};

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
