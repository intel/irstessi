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

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <features.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include <ssi.h>

#include "exception.h"
#include "list.h"
#include "container.h"
#include "string.h"
#include "object.h"
#include "storage_device.h"
#include "raid_device.h"
#include "end_device.h"
#include "port.h"
#include "block_device.h"
#include "array.h"
#include "utils.h"
#include "filesystem.h"

/* */
BlockDevice::BlockDevice(const String &path)
    : EndDevice(path),
    m_pArray(0),
    m_DiskUsage(SSI_DiskUsageUnknown),
    m_DiskState(SSI_DiskStateUnknown),
    m_IsSystem(false),
    m_StoragePoolId(0)
{
    __internal_determine_disk_state();
    __internal_determine_disk_usage();
    __internal_determine_disk_is_system();
}

/* */
SSI_Status BlockDevice::unlock(SSI_DiskUnlockInfo *pInfo)
{
    (void)pInfo;
    return SSI_StatusOk;
}

/* */
SSI_Status BlockDevice::clearMetadata()
{
    if (m_DiskState != SSI_DiskStateNormal) {
        return SSI_StatusInvalidState;
    }
    if (m_IsSystem) {
        return SSI_StatusInvalidState;
    }
    if (m_DiskUsage != SSI_DiskUsageOfflineArray && m_DiskUsage != SSI_DiskUsagePassThruReadOnlyMount) {
        return SSI_StatusInvalidState;
    }
    if (shell("mdadm --zero-superblock /dev/" + m_DevName) == 0) {
        return SSI_StatusOk;
    }
    return SSI_StatusFailed;
}

/* */
SSI_Status BlockDevice::assignPoolId(unsigned char poolId)
{
    (void)poolId;
    return SSI_StatusNotSupported;
}

/* */
SSI_Status BlockDevice::markAsNormal()
{
    return SSI_StatusOk;
}

/* */
SSI_Status BlockDevice::readStorageArea(void *pBuffer, unsigned int bufferSize)
{
    (void)bufferSize;
    (void)pBuffer;
    return SSI_StatusOk;
}

/* */
SSI_Status BlockDevice::writeStorageArea(void *pBuffer, unsigned int bufferSize)
{
    (void)bufferSize;
    (void)pBuffer;
    return SSI_StatusOk;
}

/* */
void BlockDevice::attachArray(Object *pArray)
{
    if (pArray == 0) {
        throw E_NULL_POINTER;
    }
    m_pArray = dynamic_cast<Array *>(pArray);
    m_pPort->attachArray(pArray);

    __internal_determine_disk_usage();
}

/* */
void BlockDevice::attachVolume(Object *pVolume)
{
    if (pVolume == 0) {
        throw E_NULL_POINTER;
    }
    m_Volumes.add(pVolume);
    m_pPort->attachVolume(pVolume);
}

/* */
void BlockDevice::setWriteCache(bool enable)
{
    (void)enable;
}

/* */
void BlockDevice::__internal_determine_disk_usage()
{
    bool isSpare = shell("mdadm -Es /dev/" + m_DevName + " | grep spares") == 0;
    if (m_pArray) {
        m_DiskUsage = isSpare ? SSI_DiskUsageSpare : SSI_DiskUsageArrayMember;
    } else {
        m_DiskUsage = isSpare ? SSI_DiskUsagePassThruReadOnlyMount : SSI_DiskUsagePassThru;
    }
}

/* */
void BlockDevice::__internal_determine_disk_state()
{
    m_DiskState = SSI_DiskStateNormal;
}

/* */
void BlockDevice::__internal_determine_disk_is_system()
{
    String result;
    if (shell_cap("df", result) == 0) {
        try {
            result.find("/dev/" + m_DevName);
            m_IsSystem = true;
        } catch (...) {
            m_IsSystem = false;
        }
    }
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
