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
class EndDevice;
class Array;

/* */
class Volume : public RaidDevice {
public:
    Volume(StorageObject *pParent, const String &path, Session *pSession);
    ~Volume();

protected:
    Container m_EndDevices;

public:
    void attachEndDevice(Object *, bool direct = false);

public:
    SSI_Status writeStorageArea(void *pBuffer, unsigned int bufferSize);
    SSI_Status  readStorageArea(void *pBuffer, unsigned int bufferSize);

    ObjectType getType() const {
        return ObjectType_Volume;
    }

public:
    SSI_Status initialize();
    SSI_Status getInfo(SSI_VolumeInfo *pInfo);
    SSI_Status markAsNormal();
    SSI_Status rename(const String &newName);
    SSI_Status remove();
    SSI_Status rebuild(EndDevice *pEndDevice);
    SSI_Status setCachePolicy(bool off);
    SSI_Status expand(unsigned long long newSize);
    SSI_Status cancelVerify();
    SSI_Status verify(bool repair);
    SSI_Status modify(SSI_StripSize chunkSize, SSI_RaidLevel raidLevel,
        unsigned long long newSize, const Container &disks);
};

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
