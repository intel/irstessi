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

#include <sys/stat.h>
#include <sys/types.h>
#include <cerrno>
#include <unistd.h>
#include <limits>
#include <log/log.h>

#include "array.h"
#include "controller.h"
#include "volume.h"
#include "mdadm_config.h"
#include "context_manager.h"

using std::numeric_limits;
using boost::shared_ptr;
using boost::dynamic_pointer_cast;

namespace {
    bool isBlockDeviceSata(const BlockDevice& device) {
        return device.getDiskType() == SSI_DiskTypeSATA;
    }

    bool isBlockDeviceVmd(const BlockDevice& device) {
        return device.getDiskType() == SSI_DiskTypeVMD;
    }
}

/* */
Array::Array()
    : RaidDevice(),
      m_Busy(false),
      m_TotalSize(0),
      m_FreeSize(0)
{

}

/* */
Array::Array(const String &path)
    : RaidDevice(path),
      m_Busy(false),
      m_TotalSize(0),
      m_FreeSize(0)
{

}

/* */
void Array::discover()
{
    RaidDevice::discover();
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

        shared_ptr<Volume> pVolume = shared_ptr<Volume>(new Volume(*(*i), metadata.reverse_after("/")));
        pVolume->discover();
        attachVolume(pVolume);
        pVolume->setParent(shared_from_this());
    }
}

/* */
String Array::getId() const {
    return "ar:" + getPartId();
}

/* */
String Array::getPartId() const {
    return m_Name;
}

/* */
SSI_Status Array::addSpare(const Container<EndDevice> &container)
{
    unsigned int count = 0;

    String endDevices;
    foreach (i, container) {
        if (shared_ptr<BlockDevice> pBlockDevice = dynamic_pointer_cast<BlockDevice>(*i)) {
            if (pBlockDevice->getArray().get() == this && pBlockDevice->getDiskUsage() == SSI_DiskUsageSpare) {
                /* TODO: log that the given end device is already a component of this array. */
                continue;
            }

            shared_ptr<Controller> pController = pBlockDevice->getController();
            if (!pController) {
                return SSI_StatusInvalidState;
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
        } else {
            return SSI_StatusInvalidState;
        }
    }

    if (count == 0) {
        return SSI_StatusOk;
    }

    unsigned int lines = 3;
    unsigned int offset = 0;
    if (shellEx("mdadm '/dev/" + m_DevName + "' -a" + endDevices, lines, offset) == 0) {
        return SSI_StatusOk;
    }

    return SSI_StatusFailed;
}

/* */
SSI_Status Array::addSpare(const shared_ptr<EndDevice>& pEndDevice)
{
    Container<EndDevice> container;
    if (!pEndDevice) {
        return SSI_StatusInvalidHandle;
    }

    container.add(pEndDevice);
    return addSpare(container);
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
    status = addSpare(container);
    getEndDevices(tmp, false);
    if (status == SSI_StatusOk) {
        usleep(3000000);
        if (shellEx("mdadm --grow '/dev/" + m_DevName + "' --raid-devices " + String(tmp.size() + container.size())) != 0) {
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

    pInfo->arrayHandle = getHandle();
    getId().get(pInfo->uniqueId, sizeof(pInfo->uniqueId));
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
        if (shared_ptr<BlockDevice> tmp = (*i).lock()) {
            tmp->setWriteCache(enable);
        }
    }
    return SSI_StatusNotSupported;
}

/* */
void Array::setEndDevices(const Container<EndDevice> &container)
{
    m_BlockDevices.clear();
    foreach (i, container) {
        if (shared_ptr<BlockDevice> pBlockDevice = dynamic_pointer_cast<BlockDevice>(*i)) {
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
        } else {
            throw E_INVALID_OBJECT;
        }
    }
}

/* */
SSI_Status Array::removeSpare(const shared_ptr<EndDevice>& pEndDevice, bool force)
{
    if (!force)
    {
        if (pEndDevice->getArray().get() != this) {
            return SSI_StatusInvalidState;
        }

        if (shared_ptr<BlockDevice> pBlockDevice = dynamic_pointer_cast<BlockDevice>(pEndDevice)) {
            if (pBlockDevice->getDiskUsage() != SSI_DiskUsageSpare) {
                return SSI_StatusInvalidState;
            }

            SSI_DiskState state = pBlockDevice->getDiskState();
            if (state != SSI_DiskStateNormal && state != SSI_DiskStateFailed && state != SSI_DiskStateSmartEventTriggered) {
                return SSI_StatusInvalidState;
            }
        } else {
            return SSI_StatusInvalidState;
        }
    }

    int result = shellEx("mdadm '/dev/" + m_DevName + "' -r '/dev/" + pEndDevice->getDevName() + "'");
    if (result == 0) {
        usleep(3000000);
        result = shellEx("mdadm --zero-superblock '/dev/" + pEndDevice->getDevName() + "'");
    }

    if (result == 0) {
        return SSI_StatusOk;
    }

    return SSI_StatusFailed;
}

SSI_Status Array::removeSpare(const Container<EndDevice>& endDevices, bool force)
{
    SSI_Status status = SSI_StatusOk;
    foreach (i, endDevices)
    {
        SSI_Status localStatus;
        if ((localStatus = removeSpare(*i, force)) != SSI_StatusOk) {
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
    if (shellEx("mdadm --kill-subarray=" + String(ordinal) + " '/dev/" + m_DevName + "'") == 0) {
        return SSI_StatusOk;
    }
    return SSI_StatusFailed;
}

/* */
SSI_Status Array::renameVolume(const unsigned int ordinal, String newName)
{
    const unsigned int npos = numeric_limits<unsigned int>::max();
    usleep(3000000);

    unsigned int pos = npos;
    /*
       When mdadm yields error on create with "/", on rename it's temporary replaced with "-" so all is "ok".
       When temporary name is set to requested name (newName), volume is deleted and endDevices cannot be removed
       from array through SSI API

       Also, spaces are also banned.
    */
    try {
        pos = newName.find("/");
    } catch (...) {
        /* newName does not have "/" character */
    }


    try {
        String trimmed = newName;
        trimmed.trim();
        pos = trimmed.find(" ");
    } catch (...) {
        /* newName does not have " " character */
    }

    SSI_Status status = SSI_StatusOk;
    if (pos != npos) {
        setLastErrorMessage(newName + " is an invalid name for an md device");
        status =  SSI_StatusInvalidString;
    }

    if (status == SSI_StatusOk) {
        if (shellEx("mdadm --misc --update-subarray=" + String(ordinal) + " --update=name -N '" + newName + "' '/dev/md/" + m_Name + "'", 1, 1) != 0) {
            status = SSI_StatusFailed;
        }
    }

    String output;
    if (shell_output("mdadm -Ebs", output) != 0) {
        return SSI_StatusFailed;
    }

    File attr = String(MDADM_CONFIG_PATH);
    attr.write(output, true);

    return status;
}

/* */
SSI_Status Array::assemble()
{
    usleep(3000000);
    if (shellEx("mdadm -I '/dev/md/" + m_Name + "'") == 0) {
        return SSI_StatusOk;
    }
    return SSI_StatusFailed;
}

/* */
void Array::getEndDevices(Container<EndDevice> &container, bool all) const
{
    container.clear();
    foreach (i, m_BlockDevices) {
        if (shared_ptr<EndDevice> tmp = (*i).lock()) {
            if (all || tmp->getDiskUsage() == SSI_DiskUsageArrayMember) {
                container.add(tmp);
            }
        }
    }
}

/* */
void Array::getVolumes(Container<Volume> &container) const
{
    container = m_Volumes;
}

/* */
void Array::addToSession(const shared_ptr<Session>& pSession)
{
    RaidDevice::addToSession(pSession);
    pSession->addArray(shared_from_this());
    foreach (i, m_Volumes) {
        (*i)->addToSession(pSession);
    }

    foreach (i, m_Volumes) {
        if ((*i)->getState() != SSI_VolumeStateNormal) {
            m_Busy = true;
            break;
        }
    }
    __internal_determine_total_and_free_size();

    Container<EndDevice> endDevices;
    getEndDevices(endDevices, true);
    foreach (endDevice, endDevices) {
        (*endDevice)->determineBlocksFree(shared_from_this());
    }
}

/* */
SSI_Status Array::remove()
{
    int n = 0;
    do {
        if (shellEx("mdadm -S '/dev/" + m_DevName  + "'") == 0) {
            pContextMgr->remove(this);
            String devices;
            foreach (i, m_BlockDevices) {
                if (shared_ptr<BlockDevice> tmp = (*i).lock()) {
                    devices += " '/dev/" + tmp->getDevName() + "'";
                }
            }
            usleep(3000000);
            if (shellEx("mdadm --zero-superblock" + devices) == 0) {
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
        shared_ptr<BlockDevice> pBlockDevice = dynamic_pointer_cast<BlockDevice>(*i);
        if(pBlockDevice->getDiskUsage() != SSI_DiskUsageSpare) {
            result.add(*i);
        }
    }
    return result;
}

SSI_Status Array::canAddEndDevices(const Container<EndDevice>& endDevices) const
{
    bool isSata = false;

    /* Checking which devices we have (assuming if one is SATA - all are SATA etc.) */
    /* For now, we only support SATA and VMD */
    if (!m_BlockDevices.empty()) {
        shared_ptr<BlockDevice> device = m_BlockDevices.front().lock();

        isSata = isBlockDeviceSata(*device);
    }

    /* Checking if new devices can be add */
    foreach (i, endDevices) {
        shared_ptr<BlockDevice> device = dynamic_pointer_cast<BlockDevice>(*i);

        if (device) {
            if ((isBlockDeviceSata(*device) && !isSata) || (isBlockDeviceVmd(*device) && isSata)) {
                return SSI_StatusInvalidParameter;
            }
        }
    }

    return SSI_StatusOk;
}

void Array::__wait_for_container()
{
    int i = 0;
    bool link, found = false;
    /* first find out what dev name was assigned */
    do {
        update();
        if (m_Uuid == "") {
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
        found = (shell_command("ls '/dev/md/" + m_Name + "'") == 0);
        if (!found && link && shell_command("ln -s '/dev/" + m_DevName + "' '/dev/md/" + m_Name + "'") == 0) {
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
        if (shared_ptr<BlockDevice> tmp = (*i).lock()) {
            devices += " /dev/" + tmp->getDevName();
        }
    }

    if (m_BlockDevices.size() == 0) {
        throw E_BUFFER_TOO_SMALL;
    }

    unsigned int lines = 3;
    unsigned int offset = 0;
    if (shellEx("mdadm -CR '" + m_Name + "' -f -amd -eimsm -n" + String(m_BlockDevices.size()) + devices, lines, offset) != 0) {
        throw E_ARRAY_CREATE_FAILED;
    }
    __wait_for_container();
}

/* */
void Array::attachEndDevice(const shared_ptr<EndDevice>& pObject)
{
    shared_ptr<BlockDevice> pBlockDevice = dynamic_pointer_cast<BlockDevice>(pObject);
    if (!pBlockDevice) {
        throw E_INVALID_OBJECT;
    }
    pBlockDevice->attachArray(shared_from_this());
    m_BlockDevices.add(pBlockDevice);
}

/* */
void Array::attachVolume(const shared_ptr<Volume>& pVolume)
{
    m_Volumes.add(pVolume);
}

/* */
void Array::__internal_determine_total_and_free_size()
{
    u_int64_t raidSectorSize = DEFAULT_SECTOR_SIZE;
    unsigned long long int totalSize = -1ULL;
    foreach (i, m_BlockDevices) {
        if (shared_ptr<BlockDevice> tmp = (*i).lock()) {
            totalSize = ssi_min(totalSize, tmp->getTotalSize());
        }
    }
    m_TotalSize = (totalSize * m_BlockDevices.size());
    unsigned long long occupiedSize = 0;
    unsigned int stripSize = 0;
    int volumeCount = 0;
    if (!m_Volumes.empty()) {
        raidSectorSize = m_Volumes.front()->getLogicalSectorSize();
    }
    foreach (i, m_Volumes) {
        occupiedSize += (unsigned long long) (*i)->getComponentSize() << 10;
        occupiedSize += IMSM_RESERVED_SECTORS * raidSectorSize;
        stripSize = (*i)->getStripSize();
        volumeCount++;

    }
    if (occupiedSize > 0) {
        occupiedSize += MPB_SECTOR_CNT * raidSectorSize;
        occupiedSize = occupiedSize * m_BlockDevices.size();
    }
    if (occupiedSize > m_TotalSize) {
        m_FreeSize = 0;
    } else {
        m_FreeSize = m_TotalSize - occupiedSize;
    }

    if ((m_FreeSize < stripSize) || (volumeCount > 1)) {
        m_FreeSize = 0;
    }
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
