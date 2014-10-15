
/*
Copyright (c) 2011, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
#include "controller.h"

/* */
BlockDevice::BlockDevice(const String &path)
    : EndDevice(path),
    m_pArray(NULL),
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
SSI_Status BlockDevice::makeSpare()
{
    Controller *pController = getController();
    if (pController == NULL)
           return SSI_StatusFailed;
    return pController->makeSpare(this);
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
void BlockDevice::attachArray(Array *pArray)
{
    if (pArray == NULL) {
        throw E_NULL_POINTER;
    }
    m_pArray = pArray;
    m_pPort->attachArray(pArray);

    __internal_determine_disk_usage();
}

/* */
void BlockDevice::attachVolume(Volume *pVolume)
{
    if (pVolume == NULL) {
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
    String result;
    if (shell_cap("mdadm -Es /dev/" + m_DevName, result) != 0) {
        m_DiskUsage = SSI_DiskUsagePassThru;
        return;
    }
    try {
	result.find("spare");
	m_DiskUsage = m_pArray ? SSI_DiskUsageSpare : SSI_DiskUsagePassThruReadOnlyMount;
	return;
    } catch (...) {
    }
    try {
	result.find("ARRAY");
	m_DiskUsage = m_pArray ? SSI_DiskUsageArrayMember : SSI_DiskUsageOfflineArray;
	return;
    } catch (...) {
    }
    m_DiskUsage = SSI_DiskUsagePassThru;
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
    if (shell_cap("df /boot", result) == 0) {
        try {
            result.find("/dev/" + m_DevName);
            m_IsSystem = true;
        } catch (...) {
            m_IsSystem = false;
        }
    }
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
