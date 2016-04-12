
/*
Copyright (c) 2011 - 2016, Intel Corporation
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
#include "container.h"
#include "string.h"
#include "object.h"
#include "storage_device.h"
#include "raid_device.h"
#include "array.h"
#include "volume.h"
#include "end_device.h"
#include "utils.h"
#include "filesystem.h"
#include "session.h"
#include "block_device.h"
#include "utils.h"

#include "log/log.h"
/* */
Array::Array(const String &path)
    : RaidDevice(path),
      m_Busy(false)
{
    String metadata;
    Directory dir("/sys/devices/virtual/block");
    std::list<Directory *> dirs = dir.dirs();
    foreach (i, dirs) {
        File attr = *(*i) + "md/metadata_version";
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
String Array::getKey() const {
    /* TODO this must change as all containers without volumes have uuid 0 */
    return m_Uuid;
}

/* */
SSI_Status Array::addSpare(const Container<EndDevice> &container)
{
    unsigned int count = 0;

    String endDevices;
    foreach (i, container) {
        BlockDevice *pBlockDevice = dynamic_cast<BlockDevice *>(*i);
        if (pBlockDevice == NULL) {
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
        endDevices += " '/dev/" + pBlockDevice->getDevName() + "'";
        count++;
    }
    if (count == 0) {
        return SSI_StatusOk;
    }
    if (shell("mdadm '/dev/" + m_DevName + "' -a" + endDevices) == 0) {
        return SSI_StatusOk;
    }
    return SSI_StatusFailed;
}

/* */
SSI_Status Array::addSpare(EndDevice *pEndDevice)
{
    Container<EndDevice> container;
    if (pEndDevice == NULL) {
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
    Container<EndDevice> addedToSpare = getSpareableEndDevices(container);
    status = this->addSpare(container);
    this->getEndDevices(tmp,false);
    if (status == SSI_StatusOk) {
        usleep(3000000);
        if (shell("mdadm --grow '/dev/" + m_DevName + "' --raid-devices " +
                  String(tmp.size() + container.size())) != 0) {
            removeSpare(addedToSpare, true);
            status = SSI_StatusFailed;
        }
    }
    return status;
}

/* */
SSI_Status Array::getInfo(SSI_ArrayInfo *pInfo) const
{
    if (pInfo == NULL) {
        return SSI_StatusInvalidParameter;
    }
    pInfo->arrayHandle = pInfo->uniqueId = getId();
    m_Name.get(pInfo->name, sizeof(pInfo->name));
    if (m_Busy) {
        pInfo->state = SSI_ArrayStateBusy;
    } else {
        pInfo->state = SSI_ArrayStateNormal;
    }
    pInfo->totalSize = m_TotalSize;
    pInfo->freeSize = m_FreeSize;
    pInfo->writeCachePolicy = SSI_WriteCachePolicyOn;
    pInfo->numVolumes = m_Volumes.size();
    pInfo->numDisks = m_BlockDevices.size();
    return SSI_StatusOk;
}

/* */
SSI_Status Array::setWriteCacheState(bool enable)
{
    if (m_Busy) {
        return SSI_StatusInvalidState;
    }
    foreach (i, m_BlockDevices) {
        (*i)->setWriteCache(enable);
    }
    return SSI_StatusNotSupported;
}

/* */
void Array::setEndDevices(const Container<EndDevice> &container)
{
    m_BlockDevices.clear();
    foreach (i, container) {
        BlockDevice *pBlockDevice = dynamic_cast<BlockDevice *>(*i);
        if (pBlockDevice == NULL) {
            throw E_INVALID_OBJECT;
        }
        if (pBlockDevice->isSystemDisk()) {
            throw E_SYSTEM_DEVICE;
        }
        if (pBlockDevice->getDiskUsage() != SSI_DiskUsagePassThru) {
            throw E_INVALID_USAGE;
        }
        if (pBlockDevice->getDiskState() != SSI_DiskStateNormal) {
            throw E_NOT_AVAILABLE;
        }
        m_BlockDevices.add(pBlockDevice);
    }
}

/* */
SSI_Status Array::removeSpare(const EndDevice *pEndDevice, bool force)
{
    if(!force)
    {
        if (pEndDevice->getArray() != this) {
            return SSI_StatusInvalidState;
        }
        const BlockDevice *pBlockDevice = dynamic_cast<const BlockDevice *>(pEndDevice);
        if (pBlockDevice == NULL) {
            return SSI_StatusInvalidState;
        }
        if (pBlockDevice->getDiskUsage() != SSI_DiskUsageSpare) {
            return SSI_StatusInvalidState;
        }
        SSI_DiskState state = pBlockDevice->getDiskState();
        if (state != SSI_DiskStateNormal && state != SSI_DiskStateFailed && state != SSI_DiskStateSmartEventTriggered) {
            return SSI_StatusInvalidState;
        }
    }
    int result = shell("mdadm '/dev/" + m_DevName + "' -r '/dev/" + pEndDevice->getDevName() + "'");
    if (result == 0) {
        usleep(3000000);
        result = shell("mdadm --zero-superblock '/dev/" + pEndDevice->getDevName() + "'");
    }
    if (result == 0) {
        return SSI_StatusOk;
    }
    return SSI_StatusFailed;
}

SSI_Status Array::removeSpare(const Container<EndDevice>& endDevices, bool force)
{
    SSI_Status status = SSI_StatusOk;
    foreach(i, endDevices)
    {
        SSI_Status localStatus;
        if((localStatus = removeSpare(*i, force)) != SSI_StatusOk) {
            status = localStatus;
        }
    }
    return status;
}

/* */
SSI_Status Array::removeVolume(const unsigned int ordinal)
{
    if (1 == m_Volumes.size()) {
        return SSI_StatusOk;
    }
    usleep(3000000);
    if (shell("mdadm --kill-subarray=" + String(ordinal) + " '/dev/" + m_DevName + "'") == 0) {
        return SSI_StatusOk;
    }
    return SSI_StatusFailed;
}

/* */
SSI_Status Array::renameVolume(const unsigned int ordinal, String newName)
{
    usleep(3000000);
    SSI_Status status = SSI_StatusOk;

    /*
       When mdadm yields error on create with "/", on rename it's temporary replaced with "-" so all is "ok".
       When temporary name is set to requested name (newName), volume is deleted and endDevices cannot be removed
       from array through SSI API
    */
    try {
        newName.find("/");

        setLastErrorMessage(newName + " is an invalid name for an md device.");
        status = SSI_StatusInvalidString;
    } catch (...) {
        // newName does not have "/" character
    }

    if (status == SSI_StatusOk) {
        if (shellEx("mdadm --misc --update-subarray=" + String(ordinal) + " --update=name -N '" + newName + "' '/dev/md/" + m_Name + "'", 1, 1) != 0) {
            status = SSI_StatusFailed;
        }

        if (shell("mdadm -Ebs >> /etc/mdadm.conf") != 0) {
            return SSI_StatusFailed;
        }
    }

    return status;
}

/* */
SSI_Status Array::assemble()
{
    usleep(3000000);
    if (shell("mdadm -I '/dev/md/" + m_Name + "'") == 0) {
        return SSI_StatusOk;
    }
    return SSI_StatusFailed;
}

/* */
void Array::getEndDevices(Container<EndDevice> &container, bool all) const
{
    container.clear();
    foreach (i, m_BlockDevices) {
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
void Array::addToSession(Session *pSession)
{
    RaidDevice::addToSession(pSession);
    pSession->addArray(this);
    foreach (i, m_Volumes) {
        (*i)->addToSession(pSession);
    }
    foreach (i, m_Volumes) {
        if ((*i)->getState() != SSI_VolumeStateNormal) {
            m_Busy = true; break;
        }
    }
    __internal_determine_total_and_free_size();

    Container<EndDevice> endDevices;
    getEndDevices(endDevices, true);
    foreach (endDevice, endDevices) {
        (*endDevice)->determineBlocksFree(this);
    }
}

/* */
SSI_Status Array::remove()
{
    int n = 0;
    do {
        if (shell("mdadm -S '/dev/" + m_DevName  + "'") == 0) {
            String devices;
            foreach (i, m_BlockDevices) {
                devices += " '/dev/" + (*i)->getDevName() + "'";
            }
            usleep(3000000);
            if (shell("mdadm --zero-superblock" + devices) == 0) {
                return SSI_StatusOk;
            }
        }
        usleep(3000000);
    } while (n++ < 3);
    return SSI_StatusFailed;
}

Container<EndDevice> Array::getSpareableEndDevices(const Container<EndDevice>& endDevices)
{
    Container<EndDevice> result;
    foreach(i, endDevices) {
        BlockDevice *pBlockDevice = dynamic_cast<BlockDevice *>(*i);
        if(pBlockDevice->getDiskUsage() != SSI_DiskUsageSpare) {
            result.add(*i);
        }
    }
    return result;
}

void Array::__wait_for_container()
{
    int i = 0;
    bool link, found = false;
    /* first find out what dev name was assigned */
    do {
        update();
        if (getKey() == "") {
            usleep(1000000);
            i++;
        } else {
            break;
        }
    } while (i < 10);
    /* wait for link - udev should create one but sometimes it does not
       create link if we know dev name */
    link = (m_DevName != "");
    i = 0;
    do {
        usleep(1000000);
        found = (shell("ls '/dev/md/" + m_Name + "'") == 0);
        if (!found && link && shell("ln -s '/dev/" + m_DevName + "' '/dev/md/" + m_Name + "'") == 0) {
            break;
        }
        i++;
    } while (!found && i < 10);
}

void Array::create()
{
    determineDeviceName("Imsm_");

    String devices;
    foreach (i, m_BlockDevices) {
        devices += " /dev/" + (*i)->getDevName();
    }

    if (m_BlockDevices.size() == 0) {
        throw E_BUFFER_TOO_SMALL;
    }

    if (shell("mdadm -CR '" + m_Name + "' -f -amd -eimsm -n" + String(m_BlockDevices.size()) + devices) != 0) {
        throw E_ARRAY_CREATE_FAILED;
    }
    __wait_for_container();
}

/* */
void Array::attachEndDevice(EndDevice *pObject)
{
    BlockDevice *pBlockDevice = dynamic_cast<BlockDevice *>(pObject);
    if (pBlockDevice == NULL) {
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
    unsigned long long int totalSize = -1ULL;
    for (Iterator<BlockDevice *> i = m_BlockDevices; *i != 0; ++i) {
        totalSize = min(totalSize, (*i)->getTotalSize());
    }
    m_TotalSize = (totalSize * m_BlockDevices);
    unsigned long long occupiedSize = 0;
    unsigned int stripSize = 0;
    int volumeCount = 0;
    for (Iterator<Volume *> i = m_Volumes; *i != 0; ++i) {
        occupiedSize += (unsigned long long) (*i)->getComponentSize() << 10;
        occupiedSize += IMSM_RESERVED_SECTORS * RAID_SECTOR_SIZE;
        stripSize = (*i)->getStripSize();
        volumeCount++;

    }
    if (occupiedSize > 0) {
        occupiedSize += MPB_SECTOR_CNT * RAID_SECTOR_SIZE;
        occupiedSize = occupiedSize * m_BlockDevices;
    }
    m_FreeSize = m_TotalSize - occupiedSize;

    if ((m_FreeSize < stripSize) || (volumeCount > 1)) {
        m_FreeSize = 0;
    }
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
