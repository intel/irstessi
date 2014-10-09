
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
#include <stddef.h>
#include <unistd.h>

#include <ssi.h>

#include "exception.h"
#include "list.h"
#include "container.h"
#include "string.h"
#include "object.h"
#include "storage_device.h"
#include "raid_device.h"
#include "array.h"
#include "volume.h"
#include "end_device.h"
#include "session.h"
#include "filesystem.h"
#include "utils.h"
#include "block_device.h"
#include "raid_info.h"

SSI_RaidLevel ui2raidlevel(unsigned int level);
SSI_StripSize ui2stripsize(unsigned int chunk);
unsigned int stripsize2ui(SSI_StripSize chunk);

/* */
Volume::Volume() : RaidDevice(),
      m_Ordinal(-1U),
      m_TotalSize(0),
      m_RaidLevel(-1U),
      m_MigrationProgress(0),
      m_WriteThrough(false),
      m_CachingEnabled(false),
      m_SystemVolume(false),
      m_MismatchCount(0),
      m_StripSize(0),
      m_ComponentSize(0),
      m_State(SSI_VolumeStateUnknown),
      m_pSourceDisk(0)
{
}

/* */
Volume::Volume(const String &path, unsigned int ordinal)
    : RaidDevice(path),
      m_Ordinal(ordinal),
      m_TotalSize(0),
      m_RaidLevel(-1U),
      m_MigrationProgress(0),
      m_WriteThrough(false),
      m_CachingEnabled(false),
      m_SystemVolume(false),
      m_MismatchCount(0),
      m_StripSize(0),
      m_ComponentSize(0),
      m_State(SSI_VolumeStateUnknown),
      m_pSourceDisk(0)
{
    String temp;
    try {
        SysfsAttr attr = m_Path + "/md/level";
        attr >> temp;
        if (temp == "raid0") {
            m_RaidLevel = 0;
        } else
        if (temp == "raid1") {
            m_RaidLevel = 1;
        } else
        if (temp == "raid10") {
            m_RaidLevel = 10;
        } else
        if (temp == "raid5") {
            m_RaidLevel = 5;
        } else
        if (temp == "raid6") {
            m_RaidLevel = 6;
        } else {
            m_RaidLevel = -1U;
        }
    } catch (...) {
        // Intentionaly left blank
    }
    if (m_State == SSI_VolumeStateUnknown) {
        try {
            SysfsAttr attr = m_Path + "/md/array_state";
            attr >> temp;
            if (temp == "readonly") {
                m_State = SSI_VolumeStateLocked;
            }
        } catch (...) {
            // Intentionaly left blank
        }
    }
    if (m_State == SSI_VolumeStateUnknown) {
        try {
            SysfsAttr attr = m_Path + "/md/sync_action";
            attr >> temp;
            if (temp == "resync") {
                m_State = SSI_VolumeStateInitializing;
            } else
            if (temp == "recover") {
                m_State = SSI_VolumeStateRebuilding;
            } else
            if (temp == "idle") {
                m_State = SSI_VolumeStateNormal;
            } else
            if (temp == "check") {
                m_State = SSI_VolumeStateVerifying;
            } else
            if (temp == "repair") {
                m_State = SSI_VolumeStateVerifyingAndFix;
            } else
            if (temp == "reshape") {
                m_State = SSI_VolumeStateGeneralMigration;
            }
        } catch (...) {
            // Intentionally left blank
        }
    }
    if (m_State == SSI_VolumeStateUnknown || m_State == SSI_VolumeStateNormal) {
    try {
        int degraded = 0;
        SysfsAttr attr = m_Path + "/md/degraded";
        attr >> degraded;
        if (degraded > 0) {
            switch (m_RaidLevel) {
            case 1:
            case 5:
                if (degraded > 1) {
                    m_State = SSI_VolumeStateFailed;
                } else {
                    m_State = SSI_VolumeStateDegraded;
                }
                break;
            case 6:
            case 10:
                if (degraded > 2) {
                    m_State = SSI_VolumeStateFailed;
                } else {
                    m_State = SSI_VolumeStateDegraded;
                }
                break;
            default:
                break;
            }
        }
    } catch (...) {
        // Intentionally left blank
    }
    }

    try {
        SysfsAttr attr = m_Path + "/md/array_size";
        attr >> m_TotalSize;
    } catch (...) {
        // Intentionaly left blank
    }
    try {
        SysfsAttr attr = m_Path + "/md/chunk_size";
        attr >> m_StripSize;
    } catch (...) {
        // Intentionaly left blank
    }
    try {
        SysfsAttr attr = m_Path + "/md/mismatch_cnt";
        attr >> m_MismatchCount;
    } catch (...) {
        // Intentionaly left blank
    }
    try {
        SysfsAttr attr = m_Path + "/md/component_size";
        attr >> m_ComponentSize;
    } catch (...) {
        // Intentionaly left blank
    }
    String result;
    if (shell_cap("df /boot", result) == 0) {
        try {
            result.find(m_DevName);
            m_SystemVolume = true;
        } catch (...) {
            // Intentionaly left blank
        }
    }
}

/* */
Volume::~Volume()
{
}

/* */
SSI_Status Volume::initialize()
{
    return SSI_StatusNotSupported;
}

/* */
SSI_Status Volume::rebuild(EndDevice *pEndDevice)
{
    Array *pArray = dynamic_cast<Array *>(m_pParent);
    if (pArray == 0) {
        return SSI_StatusFailed;
    } else {
        return pArray->addSpare(pEndDevice);
    }
}

/* */
SSI_Status Volume::expand(unsigned long long newSize)
{
    if (m_State != SSI_VolumeStateNormal)
        return SSI_StatusInvalidState;
    if (newSize && newSize < m_ComponentSize * m_BlockDevices)
        return SSI_StatusInvalidSize;
    /* convert size for mdadm */
    String size = (newSize == 0)?"max":String(newSize/m_BlockDevices);
    if (shell("mdadm --grow /dev/" + m_DevName + " --size=" + size) == 0)
        return SSI_StatusOk;
    return SSI_StatusFailed;
}

/* */
SSI_Status Volume::rename(const String &newName)
{
    Array *pArray = dynamic_cast<Array *>(m_pParent);
    if (pArray == 0) {
        return SSI_StatusFailed;
    }

    if (m_State != SSI_VolumeStateNormal)
        return SSI_StatusInvalidState;

    if (shell("mdadm -S /dev/" + m_DevName) == 0 &&
            pArray->renameVolume(m_Ordinal, newName) == SSI_StatusOk) {
        return pArray->assemble();
    }
    /* assemble volume again even when rename failed */
    pArray->assemble();
    return SSI_StatusFailed;
}

/* */
SSI_Status Volume::remove()
{
    Array *pArray = dynamic_cast<Array *>(m_pParent);
    if (pArray == 0) {
        return SSI_StatusFailed;
    }
    if (shell("mdadm -S /dev/" + m_DevName) == 0 &&
        pArray->removeVolume(m_Ordinal) == SSI_StatusOk) {

        Container<Volume> volumes;
        pArray->getVolumes(volumes);
        if (volumes.count() > 1)
            return SSI_StatusOk;
        else
            return pArray->remove();
    }
    return SSI_StatusFailed;
}

/* */
SSI_Status Volume::markAsNormal()
{
    return SSI_StatusOk;
}

/* */
SSI_Status Volume::verify(bool repair)
{
    SysfsAttr attr = m_Path + "/md/sync_action";
    try {
        if (repair) {
            attr << "repair";
        } else {
            attr << "check";
        }
    } catch (...) {
        return SSI_StatusFailed;
    }
    return SSI_StatusOk;
}

/* */
SSI_Status Volume::cancelVerify()
{
    SysfsAttr attr = m_Path + "/md/sync_action";
    try {
        attr << "idle";
    } catch (...) {
        return SSI_StatusFailed;
    }
    return SSI_StatusOk;
}

/* */
SSI_Status Volume::modify(SSI_StripSize stripSize, SSI_RaidLevel raidLevel,
    unsigned long long newSize, const Container<EndDevice> &disks)
{
    SSI_RaidLevel volumeLevel = ui2raidlevel(m_RaidLevel);
    RaidInfo *pRaidInfo = getRaidInfo();
    /* get raidinfo for this volume*/
    SSI_RaidLevelInfo info;
    /* get raidlevel info for this volume */
    if (pRaidInfo == 0)
        return SSI_StatusFailed;
    pRaidInfo->getRaidLevelInfo(volumeLevel, &info);
    /* check new chunk is valid for this level */
    if ((stripSize & info.stripSizesSupported) == 0 && stripSize != SSI_StripSizeUnknown)
        return SSI_StatusInvalidStripSize;
    /* check migration to new level is possible */
    if ((raidLevel & info.migrSupport) == 0)
        return SSI_StatusInvalidRaidLevel;
    /* size change is not supported */
    if (newSize && newSize != (m_ComponentSize << 10) * m_BlockDevices)
        return SSI_StatusInvalidSize;
    /* migrate */
    switch (raidLevel) {
        case SSI_Raid0:
            return __toRaid0(stripSize, newSize, disks);
        case SSI_Raid10:
            return __toRaid10(stripSize, newSize, disks);
        case SSI_Raid5:
            return __toRaid5(stripSize, newSize, disks);
        default:
            return SSI_StatusNotSupported;
    }
}

/* */
SSI_Status Volume::getInfo(SSI_VolumeInfo *pInfo)
{
    if (pInfo == 0) {
        return SSI_StatusInvalidParameter;
    }
    pInfo->volumeHandle = getId();
    pInfo->arrayHandle = m_pParent->getId();
    pInfo->arrayOrdinal = m_Ordinal;
    m_Name.get(pInfo->volumeName, sizeof(pInfo->volumeName));
    pInfo->raidLevel = ui2raidlevel(m_RaidLevel);
    pInfo->state = m_State;
    pInfo->totalSize = (m_ComponentSize << 10) * m_BlockDevices;
    pInfo->stripSize = ui2stripsize(m_StripSize);
    pInfo->numDisks = m_BlockDevices;
    pInfo->migrating = (m_State == SSI_VolumeStateGeneralMigration);
    pInfo->migrProgress = m_MigrationProgress;
    if (m_CachingEnabled == false) {
        pInfo->cachePolicy = SSI_VolumeCachePolicyOff;
    } else {
        pInfo->cachePolicy = m_WriteThrough ? SSI_VolumeCachePolicyWriteThrough : SSI_VolumeCachePolicyWriteBack;
    }
    pInfo->systemVolume = m_SystemVolume ? SSI_TRUE : SSI_FALSE;
    pInfo->initialized = m_State != SSI_VolumeStateInitializing ? SSI_TRUE : SSI_FALSE;
    pInfo->logicalSectorSize = 0;
    pInfo->verifyErrors = m_MismatchCount;
    pInfo->verifyBadBlocks = 0;
    pInfo->physicalSectorSize = 0;

    return SSI_StatusOk;
}

/* */
SSI_Status Volume::setCachePolicy(bool cacheOff)
{
    (void)cacheOff;
    return SSI_StatusOk;
}

/* */
SSI_Status Volume::writeStorageArea(void *pBuffer, unsigned int bufferSize)
{
    (void)bufferSize;
    (void)pBuffer;
    return SSI_StatusOk;
}

/* */
SSI_Status Volume::readStorageArea(void *pBuffer, unsigned int bufferSize)
{
    (void)bufferSize;
    (void)pBuffer;
    return SSI_StatusOk;
}

/* */
void Volume::getEndDevices(Container<EndDevice> &container, bool __attribute__((unused)) all) const
{
    container.clear();
    for (Iterator<BlockDevice *> i = m_BlockDevices; *i != 0; ++i) {
        container.add(*i);
    }
}

/* */
void Volume::acquireId(Session *pSession)
{
    RaidDevice::acquireId(pSession);
    pSession->addVolume(this);
    if (m_RaidLevel == 0) {
        m_State = SSI_VolumeStateNormal;
        for (Iterator<BlockDevice *> i = m_BlockDevices; *i != 0; ++i) {
            if ((*i)->getDiskState() != SSI_DiskStateNormal) {
                m_State = SSI_VolumeStateNonRedundantVolumeFailedDisk;
            }
        }
    }
}

/* */
void Volume::attachEndDevice(EndDevice *pEndDevice)
{
    BlockDevice *pBlockDevice = dynamic_cast<BlockDevice *>(pEndDevice);
    if (pBlockDevice == 0) {
        throw E_INVALID_OBJECT;
    }
    pBlockDevice->attachVolume(this);
    m_BlockDevices.add(pBlockDevice);
}

/* */
void Volume::setSourceDisk(EndDevice *pEndDevice)
{
    m_pSourceDisk = dynamic_cast<BlockDevice *>(pEndDevice);
}

/* */
void Volume::setRaidLevel(SSI_RaidLevel raidLevel)
{
    switch (raidLevel) {
    case SSI_Raid0:
        m_RaidLevel = 0;
        break;
    case SSI_Raid1:
        m_RaidLevel = 1;
        break;
    case SSI_Raid10:
        m_RaidLevel = 10;
        break;
    case SSI_Raid5:
        m_RaidLevel = 5;
        break;
    case SSI_Raid6:
        m_RaidLevel = 6;
        break;
    default:
        throw E_INVALID_RAID_LEVEL;
    }
}

/* */
void Volume::setStripSize(SSI_StripSize stripSize)
{
    m_StripSize = stripsize2ui(stripSize);
}

void Volume::__wait_for_volume()
{
    int j = 0;
    Array *pArray = dynamic_cast<Array *>(m_pParent);
    if (!pArray)
        return;
    while (getKey() == "" && j < 10) {
        pArray->update();
        update();
        usleep(3000000);
        j++;
    }
}

/* */
void Volume::create()
{
    if (m_Name == "") {
        determineDeviceName("Volume_");
    }
    if (m_Name.length() > 16) {
        throw E_INVALID_NAME;
    }
    String devices;
    for (Iterator<BlockDevice *> i = m_BlockDevices; *i != 0; ++i) {
        devices += " /dev/" + (*i)->getDevName();
    }
    String componentSize;
    if (m_ComponentSize == 0) {
        componentSize = "max";
    } else {
        componentSize = m_ComponentSize / 1024;
    }
    String chunk = (m_RaidLevel != 1) ? " --chunk=" + String(m_StripSize / 1024) : "";
    if (shell("mdadm -CR " + m_Name + " -amd -l" + String(m_RaidLevel) + " --size=" + componentSize +
            chunk + " -n" + String(m_BlockDevices) + devices) != 0) {
        throw E_VOLUME_CREATE_FAILED;
    }
    __wait_for_volume();
}

SSI_RaidLevel ui2raidlevel(unsigned int level)
{
    switch (level) {
        case 0:
            return SSI_Raid0;
        case 1:
            return SSI_Raid1;
        case 5:
            return SSI_Raid5;
        case 6:
            return SSI_Raid6;
        case 10:
            return SSI_Raid10;
        default:
            return SSI_RaidInvalid;
    }
}

SSI_StripSize ui2stripsize(unsigned int chunk)
{
    switch (chunk) {
        case (2 * 1024):
            return SSI_StripSize2kB;
        case (4 * 1024):
            return SSI_StripSize4kB;
        case (8 * 1024):
            return SSI_StripSize8kB;
        case (16 * 1024):
            return SSI_StripSize16kB;
        case (32 * 1024):
            return SSI_StripSize32kB;
        case (64 * 1024):
            return SSI_StripSize64kB;
        case (128 * 1024):
            return SSI_StripSize128kB;
        case (256 * 1024):
            return SSI_StripSize256kB;
        case (512 * 1024):
            return SSI_StripSize512kB;
        case (1024 * 1024):
            return SSI_StripSize1MB;
        case (2 * 1024 * 1024):
            return SSI_StripSize2MB;
        case (4 * 1024 * 1024):
            return SSI_StripSize4MB;
        case (8 * 1024 * 1024):
            return SSI_StripSize8MB;
        case (16 * 1024 * 1024):
            return SSI_StripSize16MB;
        case (32 * 1024 * 1024):
            return SSI_StripSize32MB;
        case (64 * 1024 * 1024):
            return SSI_StripSize64MB;
        default:
            return SSI_StripSizeUnknown;
    }
}

/* */
unsigned int stripsize2ui(SSI_StripSize stripSize)
{
    switch (stripSize) {
        case SSI_StripSize2kB:
            return 2 * 1024;
        case SSI_StripSize4kB:
            return 4 * 1024;
        case SSI_StripSize8kB:
            return 8 * 1024;
        case SSI_StripSize16kB:
            return 16 * 1024;
        case SSI_StripSize32kB:
            return 32 * 1024;
        case SSI_StripSize64kB:
            return 64 * 1024;
        case SSI_StripSize128kB:
            return 128 * 1024;
        case SSI_StripSize256kB:
            return 256 * 1024;
        case SSI_StripSize512kB:
            return 512 * 1024;
        case SSI_StripSize1MB:
            return 1 * 1024 * 1024;
        case SSI_StripSize2MB:
            return 2 * 1024 * 1024;
        case SSI_StripSize4MB:
            return 4 * 1024 * 1024;
        case SSI_StripSize8MB:
            return 8 * 1024 * 1024;
        case SSI_StripSize16MB:
            return 16 * 1024 * 1024;
        case SSI_StripSize32MB:
            return 32 * 1024 * 1024;
        case SSI_StripSize64MB:
            return 64 * 1024 * 1024;
        default:
            throw E_INVALID_STRIP_SIZE;
    }
}

/* */
SSI_Status Volume::__toRaid0(SSI_StripSize stripSize, unsigned long long newSize, const Container<EndDevice> &disks)
{
    Array *pArray = dynamic_cast<Array *>(m_pParent);
    if (pArray == 0)
        return SSI_StatusFailed;

    bool chunkChange = stripSize && stripSize != ui2stripsize(m_StripSize);
    String ch = "";
    try {
        ch = chunkChange ? " -c " + String(stripsize2ui(stripSize)/1024) : "";
    } catch (...) {
        return SSI_StatusInvalidStripSize;
    }
    switch (m_RaidLevel) {
        case 0:
            if (disks == 0 && !chunkChange)
                return SSI_StatusOk;
            if (disks > 0 && chunkChange)
                return SSI_StatusNotSupported;
            if (disks > 0)
                return pArray->grow(disks);
            if (shell("mdadm /dev/" + m_DevName + " --grow -l0" + ch) == 0)
                return SSI_StatusOk;
        case 1:
            if (chunkChange)
                return SSI_StatusInvalidStripSize;
            if (shell("mdadm /dev/" + m_DevName + " --grow -l0") == 0) {
                if (disks > 0)
                    return pArray->grow(disks);
                return SSI_StatusOk;
            }
        case 10:
            if (disks > 0 && chunkChange)
                return SSI_StatusNotSupported;
            if (shell("mdadm /dev/" + m_DevName + " --grow -l0") == 0) {
                if (disks == 0 && !chunkChange)
                    return SSI_StatusOk;
                usleep(3000000);
                if (disks > 0)
                    return pArray->grow(disks);
                if (shell("mdadm /dev/" + m_DevName + " --grow -l0" + ch) == 0)
                    return SSI_StatusOk;
            }
        case 5:
            if (disks > 0)
                return SSI_StatusNotSupported;
            if (chunkChange)
                return SSI_StatusInvalidStripSize;
            if (shell("mdadm /dev/" + m_DevName + " --grow -l0") == 0)
                return SSI_StatusOk;
        default:
            return SSI_StatusNotSupported;
    }
    return SSI_StatusFailed;
}

/* */
SSI_Status Volume::__toRaid10(SSI_StripSize stripSize, unsigned long long newSize, const Container<EndDevice> &disks)
{
    SSI_Status status;
    Array *pArray = dynamic_cast<Array *>(m_pParent);
    if (m_RaidLevel != 0 || m_BlockDevices != 2  || disks < 2)
        return SSI_StatusNotSupported;
    if (stripSize && stripSize != ui2stripsize(m_StripSize))
        return SSI_StatusInvalidStripSize;
    if (pArray == 0)
        return SSI_StatusFailed;
    status = pArray->addSpare(disks);
    if (status != SSI_StatusOk)
        return status;
    if (shell("mdadm /dev/" + m_DevName + " --grow  -l10") == 0)
        return SSI_StatusOk;
    return SSI_StatusFailed;
}

/* */
SSI_Status Volume::__toRaid5(SSI_StripSize stripSize, unsigned long long newSize, const Container<EndDevice> &disks)
{
    SSI_Status status = SSI_StatusOk;
    Array *pArray = dynamic_cast<Array *>(m_pParent);
    if (pArray == 0)
        return SSI_StatusFailed;

    bool chunkChange = stripSize && stripSize != ui2stripsize(m_StripSize);
    String ch = "";
    try {
        ch = chunkChange ? " -c " + String(stripsize2ui(stripSize)/1024) : "";
    } catch (...) {
        return SSI_StatusInvalidStripSize;
    }
    switch (m_RaidLevel) {
        case 0:
            if (disks < 1)
                /* check if there are any spares */
                return SSI_StatusNotSupported;
            status = pArray->addSpare(disks);
            if (status != SSI_StatusOk)
                return status;
            if (shell("mdadm /dev/" + m_DevName + " --grow -l5 --layout=left-asymmetric" + ch) == 0)
                return SSI_StatusOk;
        case 10:
            if (disks > 0)
                return SSI_StatusNotSupported;
            if (shell("mdadm /dev/" + m_DevName + " --grow -l0") == 0) {
                if (shell("mdadm /dev/" + m_DevName + " --grow -l5 --layout=left-asymmetric" + ch) == 0)
                    return SSI_StatusOk;
            }
        case 5:
            if (disks > 0 && chunkChange)
                return SSI_StatusNotSupported;
            if (disks > 0)
                return pArray->grow(disks);
            if (shell("mdadm /dev/" + m_DevName + " --grow -l5" + ch) == 0)
                return SSI_StatusOk;
        default:
            return SSI_StatusNotSupported;
    }
    return SSI_StatusFailed;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
