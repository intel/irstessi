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

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include "block_device.h"
#include "controller.h"
#include "port.h"

using boost::shared_ptr;

/* */
BlockDevice::BlockDevice(const String &path)
    : EndDevice(path),
    m_pArray(),
    m_DiskUsage(SSI_DiskUsageUnknown),
    m_DiskState(SSI_DiskStateUnknown),
    m_IsSystem(false),
    m_StoragePoolId(0)
{

}

/* */
void BlockDevice::discover()
{
    EndDevice::discover();
    __internal_determine_disk_state();
    __internal_determine_disk_usage();
    __internal_determine_disk_is_system();

    m_BlocksFree = m_BlocksTotal;
}

/* */
SSI_Status BlockDevice::unlock(SSI_DiskUnlockInfo *)
{
    return SSI_StatusOk;
}

/* */
SSI_Status BlockDevice::makeSpare()
{
    shared_ptr<Controller> pController = getController();
    if (!pController) {
        return SSI_StatusInvalidState;
    }

    return pController->makeSpare(shared_from_this());
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

    if (shellEx("mdadm --zero-superblock '/dev/" + m_DevName + "'") == 0) {
        return SSI_StatusOk;
    }

    return SSI_StatusFailed;
}

/* */
SSI_Status BlockDevice::assignPoolId(unsigned char)
{
    return SSI_StatusNotSupported;
}

/* */
SSI_Status BlockDevice::markAsNormal()
{
    return SSI_StatusNotSupported;
}

/* */
void BlockDevice::attachArray(const shared_ptr<Array>& pArray)
{
    if (!pArray) {
        throw E_NULL_POINTER;
    }

    m_pArray = pArray;
    m_pPort->attachArray(pArray);

    __internal_determine_disk_usage();
}

/* */
void BlockDevice::attachVolume(const shared_ptr<Volume>& pVolume)
{
    if (!pVolume) {
        throw E_NULL_POINTER;
    }

    m_Volumes.add(pVolume);
    m_pPort->attachVolume(pVolume);
}

/* */
void BlockDevice::setWriteCache(bool)
{
}

/* */
void BlockDevice::__internal_determine_disk_usage()
{
    String result;
    const String command = "mdadm -Es /dev/" + m_DevName;
    if (shell_cap(command, result) != 0) {
        m_DiskUsage = SSI_DiskUsagePassThru;
        return;
    }

    try {
        result.find("spare");
        m_DiskUsage = m_pArray ? SSI_DiskUsageSpare : SSI_DiskUsagePassThruReadOnlyMount;
        return;
    } catch (...) {
        /* it's not spared */
    }

    try {
        result.find("ARRAY");
        m_DiskUsage = m_pArray ? SSI_DiskUsageArrayMember : SSI_DiskUsageOfflineArray;
        return;
    } catch (...) {
        /* it's not in an array */
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
