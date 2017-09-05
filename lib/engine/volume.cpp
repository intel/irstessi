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

#include <cstddef>
#include <unistd.h>
#include <string>
#include <cstdio>
#include <algorithm>
#include <limits>

#include "volume.h"
#include "array.h"
#include "raid_info.h"
#include "context_manager.h"

using std::vector;
using std::find_if;
using boost::shared_ptr;
using boost::dynamic_pointer_cast;

namespace {
    vector<String>::const_iterator findErrorMessage(const vector<String>& lines, const String& message);
    SSI_RaidLevel ui2raidlevel(unsigned int level);
    SSI_StripSize ui2stripsize(unsigned int chunk);
    unsigned int stripsize2ui(SSI_StripSize chunk);
    void createVolume(const String& command);
}

/* */
Volume::Volume() : RaidDevice(),
      m_Ordinal(-1U),
      m_RaidLevel(-1U),
      m_WriteThrough(false),
      m_CachingEnabled(false),
      m_SystemVolume(false),
      m_MismatchCount(0),
      m_StripSize(0),
      m_MigrProgress(0),
      m_ComponentSize(0),
      m_State(SSI_VolumeStateUnknown),
      blk(shared_ptr<BlockDevice>(new BlockDevice(""))),
      m_pSourceDisk(),
      m_RwhPolicy(SSI_RwhInvalid)
{

}

/* */
Volume::Volume(const String &path, unsigned int ordinal)
    : RaidDevice(path),
      m_Ordinal(ordinal),
      m_RaidLevel(-1U),
      m_WriteThrough(false),
      m_CachingEnabled(false),
      m_SystemVolume(false),
      m_MismatchCount(0),
      m_StripSize(0),
      m_MigrProgress(0),
      m_ComponentSize(0),
      m_State(SSI_VolumeStateUnknown),
      blk(shared_ptr<BlockDevice>(new BlockDevice(path))),
      m_pSourceDisk(),
      m_RwhPolicy(SSI_RwhInvalid)
{

}

/* */
void Volume::discover()
{
    RaidDevice::discover();
    blk->discover();
    String temp;
    File attr;
    try {
        attr = m_Path + "/md/level";
        attr >> temp;
        m_RaidLevel = getRaidLevel(temp);
    } catch (...) {
        // Intentionaly left blank
    }

    if (m_State == SSI_VolumeStateUnknown) {
        try {
            attr = m_Path + "/md/array_state";
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
            attr = m_Path + "/md/sync_action";
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
            // when there's no sync_action, assume that the volume state is normal
            m_State = SSI_VolumeStateNormal;
        }
    }

    if (m_State == SSI_VolumeStateUnknown || m_State == SSI_VolumeStateNormal) {
        try {
            int degraded = 0;
            attr = m_Path + "/md/degraded";
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
    } else if (m_State == SSI_VolumeStateGeneralMigration) {
        m_RaidLevel = getMigrationTargetLevel();
        m_MigrProgress = getMigrationProgress();
    } else if (m_State == SSI_VolumeStateVerifying || m_State == SSI_VolumeStateVerifyingAndFix)
    {
        m_MigrProgress = getVerificationProgress();
    }

    try {
        SysfsAttr attr = m_Path + "/md/chunk_size";
        attr >> m_StripSize;
    } catch (...) {
        // Intentionaly left blank
    }
    try {
        attr = m_Path + "/md/mismatch_cnt";
        attr >> m_MismatchCount;
    } catch (...) {
        // Intentionaly left blank
    }
    try {
        attr = m_Path + "/md/component_size";
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
    try {
        attr = m_Path + "/md/rwh_policy";
        attr >> temp;
        m_RwhPolicy = parseRwhPolicy(temp);
    } catch (...) {
        /* For RHEL 7.4 and SLES 12 SP3 */
        try {
            attr = m_Path + "/md/consistency_policy";
            attr >> temp;
            m_RwhPolicy = parseNewRwhPolicy(temp);
        } catch (...) {
            // Intentionaly left blank
        }
    }
}

/* */
String Volume::getId() const
{
    return "vo:" + m_Name;
}

/* */
SSI_Status Volume::initialize()
{
    return SSI_StatusNotSupported;
}

/* */
SSI_RaidLevel Volume::getSsiRaidLevel() const
{
    return ui2raidlevel(m_RaidLevel);
}

/* */
SSI_StripSize Volume::getSsiStripSize() const
{
    return ui2stripsize(m_StripSize);
}

/* */
SSI_Status Volume::rebuild(const shared_ptr<EndDevice>& pEndDevice)
{
    const unsigned long long volumeSize = getTotalSize();
    unsigned long long requiredSize = std::numeric_limits<unsigned long long>::max();
    switch (m_RaidLevel) {
    case 1:
    {
        requiredSize = volumeSize;
        break;
    }
    case 5:
    {
        Container<EndDevice> container;
        getEndDevices(container, true);
        if (container.size() == 0) {
            return SSI_StatusInvalidParameter;
        }
        requiredSize = volumeSize / container.size();
        break;
    }
    case 10:
    {
        requiredSize = volumeSize / 2;
        break;
    }
    case 0:
    default:
        return SSI_StatusInvalidParameter;
    }

    if (pEndDevice->getTotalSize() < requiredSize) {
        String errorMessage = "For RAID " + String(m_RaidLevel) +
                              " minimum size is: " + String(requiredSize) +
                              " and an actual size of the disk is: " + String(pEndDevice->getTotalSize());
        setLastErrorMessage(errorMessage);
        return SSI_StatusInvalidParameter;
    }
    if (shared_ptr<Array> pArray = dynamic_pointer_cast<Array>(m_pParent.lock())) {
        return pArray->addSpare(pEndDevice);
    } else {
        return SSI_StatusFailed;
    }
}

/* */
SSI_Status Volume::expand(unsigned long long newSize)
{
    if (m_State != SSI_VolumeStateNormal) {
        return SSI_StatusInvalidState;
    }
    /* calculate size depending on raid level */
    switch (m_RaidLevel) {
    case 0:
        return SSI_StatusNotSupported;

    case 1:
        if (newSize && newSize < m_ComponentSize * m_BlockDevices.size() / 2) {
            return SSI_StatusInvalidSize;
        }
        break;

    case 5:
        if (m_BlockDevices.size() == 1) {
            return SSI_StatusNotSupported;
        }
        if (newSize && newSize < m_ComponentSize * (m_BlockDevices.size() - 1)) {
            return SSI_StatusInvalidSize;
        }

        newSize /= (m_BlockDevices.size() - 1);
        break;

    case 10:
        if (newSize && newSize < m_ComponentSize * m_BlockDevices.size() / 2) {
            return SSI_StatusInvalidSize;
        }

        newSize /= 2;
        break;

    default:
        return SSI_StatusNotSupported;
    }

    /* convert size for mdadm */
    String size = (newSize == 0)?"max":String(newSize);
    if (shellEx("mdadm --grow '/dev/" + m_DevName + "' --size=" + size) == 0) {
        return SSI_StatusOk;
    }
    return SSI_StatusFailed;
}

/* */
SSI_Status Volume::rename(const String &newName)
{
    if (shared_ptr<Array> pArray = dynamic_pointer_cast<Array>(m_pParent.lock())) {
        if (m_State != SSI_VolumeStateNormal) {
            return SSI_StatusInvalidState;
        }

        /* MDADM issue */
        /* mdadm cuts last character from 16 character-long name after successful rename */
        if (shellEx("mdadm -S '/dev/" + m_DevName + "'") == 0 &&
                pArray->renameVolume(m_Ordinal, newName) == SSI_StatusOk) {
            return pArray->assemble();
        }
        /* assemble volume again even when rename failed */
        pArray->assemble();
        return SSI_StatusFailed;
    } else {
        return SSI_StatusInvalidState;
    }
}

/* */
SSI_Status Volume::remove()
{
    if (shared_ptr<Array> pArray = dynamic_pointer_cast<Array>(m_pParent.lock())) {
        int status = shellEx("mdadm -S '/dev/" + m_DevName + "'");
        if (status == SSI_StatusOk) {
            status |= pArray->removeVolume(m_Ordinal);
        }

        if (status == SSI_StatusOk) {
            pContextMgr->remove(this);
            Container<Volume> volumes;
            pArray->getVolumes(volumes);
            if (volumes.size() > 1) {
                return SSI_StatusOk;
            } else {
                return pArray->remove();
            }
        }

        return SSI_StatusFailed;
    } else {
        return SSI_StatusInvalidState;
    }
}

/* */
SSI_Status Volume::markAsNormal()
{
    return SSI_StatusNotSupported;
}

/* */
SSI_Status Volume::verify(bool repair)
{
    if (m_RaidLevel == 0) {
        return SSI_StatusNotSupported;
    }
    File attr = m_Path + "/md/sync_action";
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
    File attr = m_Path + "/md/sync_action";
    try {
        attr << "idle";
    } catch (...) {
        return SSI_StatusFailed;
    }
    return SSI_StatusOk;
}

/* */
SSI_Status Volume::modify(SSI_StripSize stripSize, SSI_RaidLevel raidLevel, unsigned long long newSize, const Container<EndDevice> &disks)
{
    SSI_RaidLevel volumeLevel = ui2raidlevel(m_RaidLevel);
    shared_ptr<RaidInfo> pRaidInfo = getRaidInfo();

    /* get raidinfo for this volume*/
    SSI_RaidLevelInfo info;

    /* get raidlevel info for this volume */
    if (pRaidInfo == NULL) {
        return SSI_StatusFailed;
    }
    pRaidInfo->getRaidLevelInfo(volumeLevel, &info);

    /* check new chunk is valid for this level */
    if ((stripSize & info.stripSizesSupported) == 0 && stripSize != SSI_StripSizeUnknown) {
        return SSI_StatusInvalidStripSize;
    }

    /* check migration to new level is possible */
    if ((raidLevel & info.migrSupport) == 0) {
        return SSI_StatusInvalidRaidLevel;
    }

    /* size change is not supported */
    if (newSize && newSize != (m_ComponentSize << 10) * m_BlockDevices.size()) {
        return SSI_StatusInvalidSize;
    }

    /* FDx8 support */
    if (raidLevel != SSI_Raid0) {
        foreach (iter, m_BlockDevices) {
            if (shared_ptr<EndDevice> endDevice = (*iter).lock()) {
                if (endDevice->isFultondalex8()) {
                    return SSI_StatusInvalidRaidLevel;
                }
            }
        }

        foreach (iter, disks) {
            if ((*iter)->isFultondalex8()) {
                return SSI_StatusInvalidRaidLevel;
            }
        }
    }

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

SSI_Status Volume::changeRwhPolicy(SSI_RwhPolicy policy)
{
    if (policy != SSI_RwhDistributed && policy != SSI_RwhOff)
        return SSI_StatusNotSupported;

    if (m_RwhPolicy == policy)
        return SSI_StatusOk;

    /* For RHEL 7.3 and SLES 12 SP1 */
    if (shellEx("mdadm --rwh-policy=" + rwhPolicyToString(policy) + " '/dev/" + m_DevName + "'") == 0) {
        m_RwhPolicy = policy;
        return SSI_StatusOk;
    }

    /* For RHEL 7.4 and SLES 12 SP3 */
    if (shellEx("mdadm -G '/dev/" + m_DevName + "' --consistency-policy=" + newRwhPolicyToString(policy)) == 0) {
        m_RwhPolicy = policy;
        return SSI_StatusOk;
    }

    return SSI_StatusFailed;
}

/* */
SSI_Status Volume::getInfo(SSI_VolumeInfo *pInfo)
{
    if (pInfo == NULL) {
        return SSI_StatusInvalidParameter;
    }
    pInfo->volumeHandle = getHandle();
    getId().get(pInfo->uniqueId, sizeof(pInfo->uniqueId));
    if (Parent parent = m_pParent.lock()) {
        pInfo->arrayHandle = parent->getHandle();
    } else {
        pInfo->arrayHandle = SSI_NULL_HANDLE;
    }
    pInfo->arrayOrdinal = m_Ordinal;
    m_Name.get(pInfo->volumeName, sizeof(pInfo->volumeName));
    pInfo->raidLevel = ui2raidlevel(m_RaidLevel);
    pInfo->state = m_State;
    pInfo->totalSize = blk->getTotalSize();
    pInfo->stripSize = ui2stripsize(m_StripSize);
    pInfo->numDisks = m_BlockDevices.size();
    pInfo->migrating = (m_State == SSI_VolumeStateGeneralMigration);
    pInfo->migrProgress = m_MigrProgress;
    if (m_CachingEnabled == false) {
        pInfo->cachePolicy = SSI_VolumeCachePolicyOff;
    } else {
        pInfo->cachePolicy = m_WriteThrough ? SSI_VolumeCachePolicyWriteThrough : SSI_VolumeCachePolicyWriteBack;
    }
    pInfo->systemVolume = m_SystemVolume ? SSI_TRUE : SSI_FALSE;
    pInfo->initialized = m_State != SSI_VolumeStateInitializing ? SSI_TRUE : SSI_FALSE;
    pInfo->logicalSectorSize = blk->getLogicalSectorSize();
    pInfo->verifyErrors = m_MismatchCount;
    pInfo->verifyBadBlocks = 0;
    pInfo->physicalSectorSize = blk->getPhysicalSectorSize();
    pInfo->rwhPolicy = m_RwhPolicy;

    return SSI_StatusOk;
}

/* */
SSI_Status Volume::setCachePolicy(bool)
{
    return SSI_StatusNotSupported;
}

/* */
void Volume::getEndDevices(Container<EndDevice> &container, bool) const
{
    container.clear();
    foreach (i, m_BlockDevices) {
        if (shared_ptr<EndDevice> tmp = (*i).lock()) {
            container.add(tmp);
        }
    }
}

/* */
void Volume::addToSession(const shared_ptr<Session>& pSession)
{
    RaidDevice::addToSession(pSession);
    pSession->addVolume(shared_from_this());
    if (m_RaidLevel == 0) {
        foreach (i, m_BlockDevices) {
            if (shared_ptr<BlockDevice> tmp = (*i).lock()) {
                if (tmp->getDiskState() != SSI_DiskStateNormal) {
                    m_State = SSI_VolumeStateNonRedundantVolumeFailedDisk;
                    break;
                }
            }
        }
    }
}

/* */
void Volume::attachEndDevice(const shared_ptr<EndDevice>& pEndDevice)
{
    shared_ptr<BlockDevice> pBlockDevice = dynamic_pointer_cast<BlockDevice>(pEndDevice);
    if (!pBlockDevice) {
        throw E_INVALID_OBJECT;
    }

    pBlockDevice->attachVolume(shared_from_this());
    m_BlockDevices.add(pBlockDevice);
}

/* */
void Volume::setSourceDisk(const shared_ptr<EndDevice>& pEndDevice)
{
    m_pSourceDisk = dynamic_pointer_cast<BlockDevice>(pEndDevice);
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
        case SSI_Raid6: /* not supported */
        default:
            throw E_INVALID_RAID_LEVEL;
    }
}

unsigned int Volume::getRaidLevel(const String& raidLevel)
{
    if (raidLevel == "raid0") {
        return 0;
    } else if (raidLevel == "raid1") {
        return 1;
    } else if (raidLevel == "raid10") {
        return 10;
    } else if (raidLevel == "raid5") {
        return 5;
    } else if (raidLevel == "raid6") {
        return 6;
    } else {
        return -1U;
    }
}

void Volume::setRwhPolicy(SSI_RwhPolicy policy)
{
    if (policy == SSI_RwhOff || policy == SSI_RwhDistributed) {
        m_RwhPolicy = policy;
    } else if (policy == SSI_RwhJournalingDrive) {
        throw E_NOT_SUPPORTED;
    } else {
        throw E_INVALID_OBJECT;
    }
}

String Volume::getMdadmAttribute(const String &attribute)
{
    const String command = String("mdadm -D '/dev/md/") + m_Name + "' | grep '" + attribute + "'";
    FILE* pipe = popen(static_cast<const char*>(command), "r");
    if (!pipe) return 0;
    char buffer[128];
    std::string line = "";
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL) {
            line += buffer;
        }
    }
    pclose(pipe);
    if(line.find(static_cast<const char*>(attribute)) != std::string::npos) {
        const unsigned int trimStart = line.find(static_cast<const char*>(attribute))+attribute.length() + 3;
        const unsigned int trimEnd = line.find_last_not_of("\n\t ");
        const unsigned int trimRange = trimEnd - trimStart + 1;
        line = line.substr(trimStart, trimRange);
        return String(line.c_str());
    }
    return "";
}

unsigned int Volume::getPercentageStatus(const String &attribute)
{
    unsigned int res = 0;
    String progress = getMdadmAttribute(attribute);
    try {
        unsigned int percentPos = progress.find("%");
        progress = progress.mid(0, percentPos);
        res = atoi(static_cast<const char*>(progress));
        if (res < 0 || res > 100) {
            return 0;
        }
        res = static_cast<unsigned int>(static_cast<unsigned long long>(res) * 0xFFFFFFFF / 100);
    } catch(...) {
        res = 0;
    }
    return res;
}

unsigned int Volume::getMigrationProgress()
{
    return getPercentageStatus("Reshape Status");
}

unsigned int Volume::getMigrationTargetLevel()
{
    String newLevel = getMdadmAttribute("New Level");
    unsigned int newRaidLevel = getRaidLevel(newLevel);
    if (newRaidLevel != -1U) {
        return newRaidLevel;
    }
    return m_RaidLevel;
}

unsigned int Volume::getVerificationProgress()
{
    return getPercentageStatus("Check Status");
}

SSI_RwhPolicy Volume::parseRwhPolicy(const String& policy) const
{
    if (policy == rwhPolicyToString(SSI_RwhOff)) {
        return SSI_RwhOff;
    } else if (policy == rwhPolicyToString(SSI_RwhDistributed)) {
        return SSI_RwhDistributed;
    } else if (policy == rwhPolicyToString(SSI_RwhJournalingDrive)) {
        return SSI_RwhJournalingDrive;
    } else {
        return SSI_RwhInvalid;
    }
}

SSI_RwhPolicy Volume::parseNewRwhPolicy(const String& policy) const
{
    if (policy == newRwhPolicyToString(SSI_RwhOff)) {
        return SSI_RwhOff;
    } else if (policy == newRwhPolicyToString(SSI_RwhDistributed)) {
        return SSI_RwhDistributed;
    } else if (policy == newRwhPolicyToString(SSI_RwhJournalingDrive)) {
        return SSI_RwhJournalingDrive;
    } else {
        return SSI_RwhInvalid;
    }
}

String Volume::rwhPolicyToString(SSI_RwhPolicy policy) const
{
    if (policy == SSI_RwhOff) {
        return "off";
    } else if (policy == SSI_RwhDistributed) {
        return "ppl";
    } else if (policy == SSI_RwhJournalingDrive) {
        return "ppl-journal";
    } else {
        throw E_NOT_SUPPORTED;
    }
}

String Volume::newRwhPolicyToString(SSI_RwhPolicy policy) const
{
    if (policy == SSI_RwhOff) {
        return "resync";
    } else if (policy == SSI_RwhDistributed) {
        return "ppl";
    }

    return rwhPolicyToString(policy);
}

/* Convert total Volume size to component size and set it */
void Volume::setComponentSize(unsigned long long volumeSize, unsigned long long diskCount, SSI_RaidLevel level)
{
    shared_ptr<RaidInfo> pRaidInfo = getRaidInfo();
    /* get raidinfo for this volume*/
    SSI_RaidLevelInfo info;
    /* get raidlevel info for this volume */
    if (!pRaidInfo) {
        throw E_INVALID_HANDLE;
    }

    if (pRaidInfo->getRaidLevelInfo(level, &info) != SSI_StatusOk) {
        throw E_INVALID_RAID_LEVEL;
    }

    if (diskCount == 0 || diskCount < info.minDisks) {
        throw E_BUFFER_TOO_SMALL;
    }

    /* For RAID_0, you can create volume from 1 disk but it requires --force which is not available from user side */
    if (level == SSI_Raid0 && diskCount == 1) {
        throw E_BUFFER_TOO_SMALL;
    }

    if (diskCount > info.maxDisks) {
        throw E_BUFFER_TOO_LARGE;
    }

    unsigned long long divider = 1;
    switch (level) {
        case SSI_Raid0:
            divider = diskCount;
            break;
        case SSI_Raid1:
            divider = 1;
            break;
        case SSI_Raid10:
            divider = 2;
            break;
        case SSI_Raid5:
            divider = diskCount - 1;
            break;
        case SSI_Raid6: /* not supported */
        default:
            throw E_INVALID_RAID_LEVEL;
    }
    m_ComponentSize = volumeSize / divider;
}

/* */
void Volume::setStripSize(SSI_StripSize stripSize)
{
    m_StripSize = stripsize2ui(stripSize);
}

void Volume::__wait_for_volume()
{
    int j = 0;
    if (shared_ptr<Array> pArray = dynamic_pointer_cast<Array>(m_pParent.lock())) {
        while (m_Uuid == "" && j < 10) {
            pArray->update();
            update();
            usleep(3000000);
            j++;
        }
    }
}

/* */
void Volume::create()
{
    if (m_Name == "") {
        determineDeviceName("Volume_");
    }

    if (m_RaidLevel != 0) {
        foreach (iter, m_BlockDevices) {
            if (shared_ptr<EndDevice> endDevice = (*iter).lock()) {
                if (endDevice->isFultondalex8()) {
                    throw E_INVALID_RAID_LEVEL;
                }
            }
        }
    }

    if (m_pSourceDisk == NULL) {
        String devices;
        foreach (i, m_BlockDevices) {
            if (shared_ptr<BlockDevice> tmp = (*i).lock()) {
                devices += " '/dev/" + tmp->getDevName() + "'";
            }
        }

        String componentSize;
        if (m_ComponentSize == 0) {
            componentSize = "max";
        } else {
            componentSize = m_ComponentSize / 1024;
        }

        String chunk = (m_RaidLevel != 1) ? " --chunk=" + String(m_StripSize / 1024) : "";
        String rwhPolicy = m_RwhPolicy == SSI_RwhDistributed ?
                " --rwh-policy=" + rwhPolicyToString(m_RwhPolicy) : "";

        String command = "mdadm -CR '" + m_Name + "' -amd -l" + String(m_RaidLevel) + " --size=" + componentSize +
                chunk + rwhPolicy + " -n" + String(m_BlockDevices.size()) + devices;

        try {
            createVolume(command);
        } catch (Exception e) {
            if (e == E_VOLUME_CREATE_FAILED) {
                /* For RHEL 7.4 & SLES 12 SP3 */
                rwhPolicy = m_RwhPolicy == SSI_RwhDistributed ?
                                " --consistency-policy=" + newRwhPolicyToString(m_RwhPolicy) : "";
                command = "mdadm -CR '" + m_Name + "' -amd -l" + String(m_RaidLevel) + " --size=" + componentSize +
                                chunk + rwhPolicy + " -n" + String(m_BlockDevices.size()) + devices;
                createVolume(command);
            } else {
                throw e;
            }
        }
    } else {
        throw E_NOT_SUPPORTED;
    }

    __wait_for_volume();
}

namespace {
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

    vector<String>::const_iterator findErrorMessage(const vector<String>& lines, const String& message)
    {
        vector<String>::const_iterator iter, end = lines.end();
        for (iter = lines.begin(); iter != end; ++iter)
        {
            const String& str = *iter;

            try {
                str.find(message);
                break;
            } catch (...) {
                continue;
            }
        }

        return iter;
    }

    void createVolume(const String &command) {
        /*
         * Two cases:
         *     a) mdadm returns only message about insufficient size (contains "not enough space")
         *     b) mdadm returns message about insufficient size + line about why device doesn't fit + "create aborted"
         *
         * Workaround: We get all error output lines from mdadm and we find the line we need.
         *
         * Used this workaround because we don't have better tools to get mdadm errors.
         */
        String output;
        if (shell_output(command, output, true) != 0) {
            vector<String> lines;
            mdadmErrorLines(output, lines);

            if (!lines.empty()) {

                vector<String>::const_iterator found = findErrorMessage(lines, "not enough space");

                if (found != lines.end()) {
                    setLastErrorMessage(*found);
                } else {
                    /* We've got different error message (invalid name etc.) */
                    setLastErrorMessage(lines.back());
                }
            } else {
                setLastErrorMessage("");
            }

            throw E_VOLUME_CREATE_FAILED;
        }
    }
}

/* */
SSI_Status Volume::__toRaid0(SSI_StripSize stripSize, unsigned long long, const Container<EndDevice> &disks)
{
    if (shared_ptr<Array> pArray = dynamic_pointer_cast<Array>(m_pParent.lock())) {
        bool chunkChange = stripSize && stripSize != ui2stripsize(m_StripSize);
        String ch = "";
        try {
            ch = chunkChange ? " -c " + String(stripsize2ui(stripSize) / 1024) : "";
        } catch (...) {
            return SSI_StatusInvalidStripSize;
        }

        switch (m_RaidLevel) {
            case 0: {
                if (disks.size() == 0 && !chunkChange) {
                    return SSI_StatusOk;
                } else if (disks.size() > 0 && chunkChange) {
                    return SSI_StatusNotSupported;
                } else if (disks.size() > 0) {
                    SSI_Status status = pArray->canAddEndDevices(disks);
                    if (status == SSI_StatusOk) {
                        return pArray->grow(disks);
                    } else if (status == SSI_StatusInvalidParameter) {
                        setLastErrorMessage("Cannot have both SATA and NVMe disks in one volume");
                    }
                    return status;
                } else if (shellEx("mdadm '/dev/" + m_DevName + "' --grow -l0" + ch) == 0) {
                    return SSI_StatusOk;
                }
                break;
            }

            case 1: {
                if (chunkChange) {
                    setLastErrorMessage("Strip size cannot be changed during migration from Raid 1 to Raid 0");
                    return SSI_StatusInvalidStripSize;
                }

                if (stripSize != SSI_StripSize64kB) {
                    setLastErrorMessage("Strip size cannot be passed in Raid 1 to Raid 0 migration");
                    return SSI_StatusInvalidStripSize;
                }

                SSI_Status status = pArray->canAddEndDevices(disks);
                if (status == SSI_StatusOk && shellEx("mdadm '/dev/" + m_DevName + "' --grow -l0") == 0) {
                    if (disks.size() > 0) {
                        return pArray->grow(disks);
                    }
                    return SSI_StatusOk;
                }

                if (status == SSI_StatusInvalidParameter) {
                    setLastErrorMessage("Cannot have both SATA and NVMe disks in one volume");
                }

                return status;
            }

            case 10: {
                if (disks.size() > 0 && chunkChange) {
                    return SSI_StatusNotSupported;
                }

                SSI_Status status = pArray->canAddEndDevices(disks);
                if (status == SSI_StatusOk && shellEx("mdadm '/dev/" + m_DevName + "' --grow -l0") == 0) {
                    if (disks.size() == 0 && !chunkChange) {
                        return SSI_StatusOk;
                    }

                    usleep(3000000);
                    if (disks.size() > 0) {
                        return pArray->grow(disks);
                    } else if (shellEx("mdadm '/dev/" + m_DevName + "' --grow -l0" + ch) == 0) {
                        return SSI_StatusOk;
                    }
                }

                if (status == SSI_StatusInvalidParameter) {
                    setLastErrorMessage("Cannot have both SATA and NVMe disks in one volume");
                }

                return status;
            }

            case 5: {
                if (disks.size() > 0) {
                    return SSI_StatusNotSupported;
                } else if (chunkChange) {
                    return SSI_StatusInvalidStripSize;
                } else if (shellEx("mdadm '/dev/" + m_DevName + "' --grow -l0") == 0) {
                    return SSI_StatusOk;
                }
                break;
            }

            default:
                return SSI_StatusNotSupported;
        }
    }

    return SSI_StatusFailed;
}

/* */
SSI_Status Volume::__toRaid10(SSI_StripSize stripSize, unsigned long long, const Container<EndDevice> &disks)
{
    SSI_Status status;
    if (shared_ptr<Array> pArray = dynamic_pointer_cast<Array>(m_pParent.lock())) {
        if (m_RaidLevel != 0 || m_BlockDevices.size() != 2) {
            return SSI_StatusNotSupported;
        } else if (disks.size() != 2) {
            setLastErrorMessage("Cannot migrate to RAID10. Migration to RAID10 is supported only with 2 disks");

            return SSI_StatusInvalidParameter;
        } else if (stripSize && stripSize != ui2stripsize(m_StripSize)) {
            return SSI_StatusInvalidStripSize;
        }

        status = pArray->canAddEndDevices(disks);

        if (status != SSI_StatusOk) {
            if (status == SSI_StatusInvalidParameter) {
                setLastErrorMessage("Cannot have both SATA and NVMe disks in one volume");
            }

            return status;
        }

        Container<EndDevice> addedToSpare = Array::getSpareableEndDevices(disks);

        status = pArray->addSpare(disks);
        if (status != SSI_StatusOk) {
            return status;
        }

        if (shellEx("mdadm '/dev/" + m_DevName + "' --grow  -l10") == 0) {
            return SSI_StatusOk;
        }

        pArray->removeSpare(addedToSpare, true);
    }

    return SSI_StatusFailed;
}

/* */
SSI_Status Volume::__toRaid5(SSI_StripSize stripSize, unsigned long long, const Container<EndDevice> &disks)
{
    Container<EndDevice> addedToSpare;
    SSI_Status status = SSI_StatusOk;
    if (shared_ptr<Array> pArray = dynamic_pointer_cast<Array>(m_pParent.lock())) {
        bool chunkChange = stripSize && stripSize != ui2stripsize(m_StripSize);
        String ch = "";
        try {
            ch = chunkChange ? " -c " + String(stripsize2ui(stripSize) / 1024) : "";
        } catch (...) {
            return SSI_StatusInvalidStripSize;
        }

        switch (m_RaidLevel) {
            case 0: {
                if (disks.size() != 1) {
                    setLastErrorMessage("Cannot migrate to RAID5. Migration to RAID5 is supported only with 1 disk");

                    return SSI_StatusInvalidParameter;
                }

                status = pArray->canAddEndDevices(disks);
                if (status != SSI_StatusOk) {
                    if (status == SSI_StatusInvalidParameter) {
                        setLastErrorMessage("Cannot have both SATA and NVMe disks in one volume");
                    }

                    return status;
                }

                addedToSpare = Array::getSpareableEndDevices(disks);
                status = pArray->addSpare(disks);
                if (status != SSI_StatusOk) {
                    return status;
                } else if (shellEx("mdadm '/dev/" + m_DevName + "' --grow -l5 --layout=left-asymmetric" + ch, 1, 0) == 0) {
                    return SSI_StatusOk;
                }

                pArray->removeSpare(addedToSpare, true);
                break;
            }

            case 10: {
                if (disks.size() > 0) {
                    setLastErrorMessage("Cannot migrate from RAID10 to RAID5 with additional disks");

                    return SSI_StatusInvalidParameter;
                }

                if (shellEx("mdadm '/dev/" + m_DevName + "' --grow -l0") == 0) {
                    if (shellEx("mdadm '/dev/" + m_DevName + "' --grow -l5 --layout=left-asymmetric" + ch) == 0) {
                        return SSI_StatusOk;
                    }
                }
                break;
            }

            case 5: {
                if (disks.size() > 0 && chunkChange) {
                    return SSI_StatusNotSupported;
                } else if (disks.size() > 0) {
                    status = pArray->canAddEndDevices(disks);
                    if (status != SSI_StatusOk) {
                        if (status == SSI_StatusInvalidParameter) {
                            setLastErrorMessage("Cannot have both SATA and NVMe disks in one volume");
                        }

                        return status;
                    }

                    /* MDADM issue
                       Not all scenarios are correctly handled by mdadm. For now, it yields undefined behavior */
                    return pArray->grow(disks);
                } else if (shellEx("mdadm '/dev/" + m_DevName + "' --grow -l5" + ch) == 0) {
                    return SSI_StatusOk;
                }
                break;
            }

            default:
                return SSI_StatusNotSupported;
        }
    }

    return SSI_StatusFailed;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
