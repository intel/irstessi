
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

#include <sys/stat.h>
#include <sys/types.h>
#include <cerrno>
#include <unistd.h>

#include <ssi.h>

#include "exception.h"
#include "list.h"
#include "container.h"
#include "string.h"
#include "object.h"
#include "cache.h"
#include "storage_device.h"
#include "raid_device.h"
#include "array.h"
#include "volume.h"
#include "end_device.h"
#include "utils.h"
#include "filesystem.h"
#include "session.h"
#include "block_device.h"

#include "log/log.h"
/* */
Array::Array(const String &path)
    : RaidDevice(path),
      m_Busy(false)
{
    String metadata;
    Directory dir("/sys/devices/virtual/block");
    for (Iterator<Directory *> i = dir; *i != 0; ++i) {
        SysfsAttr attr = *(*i) + "md/metadata_version";
        try {
            attr >> metadata;
        } catch (...) {
            continue;
        }
        try {
            metadata.find("/" + m_DevName);
        } catch (...) {
            try {
                metadata.find("-" + m_DevName);
            } catch (...) {
                continue;
            }
        }
        Volume *pVolume = new Volume(*(*i), metadata.reverse_after("/"));
        attachVolume(pVolume);
        pVolume->setParent(this);
    }
}

/* */
Array::~Array()
{
}

/* */
SSI_Status Array::addSpare(const Container<EndDevice> &container)
{
    unsigned int count = 0;

    String endDevices;
    for (Iterator<EndDevice *> i = container; *i != 0; ++i) {
        BlockDevice *pBlockDevice = dynamic_cast<BlockDevice *>(*i);
        if (pBlockDevice == 0) {
            return SSI_StatusInvalidState;
        }
        if (pBlockDevice->getArray() == this &&
           pBlockDevice->getDiskUsage() == SSI_DiskUsageSpare) {
            /* TODO: log that the given end device is already a component of this array. */
            continue;
        }
        if (pBlockDevice->isSystemDisk()) {
            return SSI_StatusInvalidState;
        }
        if (pBlockDevice->getDiskUsage() != SSI_DiskUsagePassThru) {
            return SSI_StatusInvalidState;
        }
        if (pBlockDevice->getDiskState() != SSI_DiskStateNormal) {
            return SSI_StatusInvalidState;
        }
        endDevices += " /dev/" + pBlockDevice->getDevName();
        count++;
    }
    if (count == 0) {
        return SSI_StatusOk;
    }
    if (shell("mdadm /dev/" + m_DevName + " -a" + endDevices) == 0) {
        return SSI_StatusOk;
    }
    return SSI_StatusFailed;
}

/* */
SSI_Status Array::addSpare(EndDevice *pEndDevice)
{
    Container<EndDevice> container;
    if (pEndDevice == 0) {
            return SSI_StatusInvalidHandle;
    }
    container.add(pEndDevice);
    return this->addSpare(container);
}

/* */
SSI_Status Array::grow(const Container<EndDevice> &container)
{
    SSI_Status status;
    Container<EndDevice> tmp;
    if (m_Busy) {
        return SSI_StatusInvalidState;
    }
    status = this->addSpare(container);
    this->getEndDevices(tmp,false);
    if (status == SSI_StatusOk)
        if (shell("echo mdadm --grow /dev/" + m_DevName + " --raid-devices " +
                  String(tmp.count() + container.count())) != 0) {
            status = SSI_StatusFailed;
        }
    return status;
}

/* */
SSI_Status Array::getInfo(SSI_ArrayInfo *pInfo) const
{
    if (pInfo == 0) {
        return SSI_StatusInvalidParameter;
    }
    pInfo->arrayHandle = getId();
    m_Name.get(pInfo->name, sizeof(pInfo->name));
    if (m_Busy) {
        pInfo->state = SSI_ArrayStateBusy;
    } else {
        pInfo->state = SSI_ArrayStateNormal;
    }
    pInfo->totalSize = m_TotalSize;
    pInfo->freeSize = m_FreeSize;
    pInfo->writeCachePolicy = SSI_WriteCachePolicyOn;
    pInfo->numVolumes = m_Volumes;
    pInfo->numDisks = m_BlockDevices;
    return SSI_StatusOk;
}

/* */
SSI_Status Array::setWriteCacheState(bool enable)
{
    if (m_Busy) {
        return SSI_StatusInvalidState;
    }
    for (Iterator<BlockDevice *> i = m_BlockDevices; *i != 0; ++i) {
        (*i)->setWriteCache(enable);
    }
    return SSI_StatusOk;
}

/* */
SSI_Status Array::removeSpare(const EndDevice *pEndDevice)
{
    if (pEndDevice->getArray() != this) {
        return SSI_StatusInvalidState;
    }
    const BlockDevice *pBlockDevice = dynamic_cast<const BlockDevice *>(pEndDevice);
    if (pBlockDevice == 0) {
        return SSI_StatusInvalidState;
    }
    if (pBlockDevice->getDiskUsage() != SSI_DiskUsageSpare) {
        return SSI_StatusInvalidState;
    }
    SSI_DiskState state = pBlockDevice->getDiskState();
    if (state != SSI_DiskStateNormal && state != SSI_DiskStateFailed && state != SSI_DiskStateSmartEventTriggered) {
        return SSI_StatusInvalidState;
    }
    int result = shell("mdadm /dev/" + m_DevName + " -r /dev/" + pEndDevice->getDevName());
    if (result == 0) {
        result = shell("mdadm --zero-superblock /dev/" + pEndDevice->getDevName());
    }
    if (result == 0) {
        return SSI_StatusOk;
    }
    return SSI_StatusFailed;
}

/**/
SSI_Status Array::removeVolume(const unsigned int ordinal)
{
    if (1 == m_Volumes) {
        return SSI_StatusOk;
    }
    if (shell("mdadm --kill-subarray=" + String(ordinal) + " /dev/" + m_DevName) == 0) {
        return SSI_StatusOk;
    }
    return SSI_StatusFailed;
}

/* */
void Array::getEndDevices(Container<EndDevice> &container, bool all) const
{
    container.clear();
    for (Iterator<BlockDevice *> i = m_BlockDevices; *i != 0; ++i) {
        if (all || (*i)->getDiskUsage() == SSI_DiskUsageArrayMember)
        container.add(*i);
    }
}

/* */
void Array::getVolumes(Container<Volume> &container) const
{
    container = m_Volumes;
}

/* */
SSI_Status Array::readStorageArea(void *pBuffer, unsigned int bufferSize)
{
    (void)bufferSize;
    (void)pBuffer;
    return SSI_StatusOk;
}

/* */
SSI_Status Array::writeStorageArea(void *pBuffer, unsigned int bufferSize)
{
    (void)bufferSize;
    (void)pBuffer;
    return SSI_StatusOk;
}

/* */
void Array::acquireId(Session *pSession)
{
    RaidDevice::acquireId(pSession);
    pSession->addArray(this);
    for (Iterator<Volume *> i = m_Volumes; *i != 0; ++i) {
        (*i)->acquireId(pSession);
    }
    for (Iterator<Volume *> i = m_Volumes; *i != 0; ++i) {
        if ((*i)->getState() != SSI_VolumeStateNormal) {
            m_Busy = true; break;
        }
    }
    __internal_determine_total_and_free_size();
}

/* */
SSI_Status Array::remove()
{
    if (m_Volumes > 1) {
        return SSI_StatusOk;
    }
    if (shell("mdadm -S /dev/" + m_DevName) == 0) {
        String devices;
        for (Iterator<BlockDevice *> i = m_BlockDevices; *i != 0; ++i) {
            devices += " /dev/" + (*i)->getDevName();
        }
        if (shell("mdadm --zero-superblock" + devices) == 0) {
            return SSI_StatusOk;
        }
    }
    return SSI_StatusFailed;
}

void Array::create()
{
    determineDeviceName("Imsm_");

    String devices;
    for (Iterator<BlockDevice *> i = m_BlockDevices; *i != 0; ++i) {
        devices += " /dev/" + (*i)->getDevName();
    }
    if (shell("mdadm -CR " + m_Name + " -f -amd -eimsm -n" + String(m_BlockDevices) + devices) != 0) {
        throw E_ARRAY_CREATE_FAILED;
    }
}

/* */
void Array::attachEndDevice(EndDevice *pObject)
{
    BlockDevice *pBlockDevice = dynamic_cast<BlockDevice *>(pObject);
    if (pBlockDevice == 0) {
        throw E_INVALID_OBJECT;
    }
    pBlockDevice->attachArray(this);
    m_BlockDevices.add(pBlockDevice);
}

/* */
void Array::attachVolume(Volume *pVolume)
{
    m_Volumes.add(pVolume);
}

/* */
void Array::__internal_determine_total_and_free_size()
{
    unsigned int totalSectors = -1U;
    for (Iterator<BlockDevice *> i = m_BlockDevices; *i != 0; ++i) {
        totalSectors = min(totalSectors, (*i)->getSectors());
    }
    m_TotalSize = ((totalSectors * m_BlockDevices) * 512);
    unsigned long long occupiedSectors = 0;
    for (Iterator<Volume *> i = m_Volumes; *i != 0; ++i) {
        occupiedSectors += (*i)->getComponentSize();
    }
    if (occupiedSectors > 0) {
        occupiedSectors = ((occupiedSectors * 1000) / 512) * m_BlockDevices;
    }
    m_FreeSize = (m_TotalSize - (occupiedSectors * 512));
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
