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

#ifndef __BLOCK_DEVICE_H__INCLUDED__
#define __BLOCK_DEVICE_H__INCLUDED__

// Forward declaration
class Array;

/* */
class BlockDevice : public EndDevice {
public:
    BlockDevice(const String &path)
        : EndDevice(path), m_pArray(0) {
    }

protected:
    Array *m_pArray;

    void attachArray(Object *pArray);

public:
    SSI_Status unlock(SSI_DiskUnlockInfo *pInfo);
    SSI_Status clearMetadata();
    SSI_Status assignPoolId(unsigned char poolId);
    SSI_Status markAsNormal();
    SSI_Status writeStorageArea(void *pBuffer, unsigned int bufferSize);
    SSI_Status  readStorageArea(void *pBuffer, unsigned int bufferSize);

public:
    Array * getArray() const {
        return m_pArray;
    }
};

#endif /* __BLOCK_DEVICE_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
