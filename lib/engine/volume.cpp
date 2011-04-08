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

#include <stdio.h>
#include <string.h>

#define BUFSIZE 4000
#define LOGFILENAME "/tmp/my.log"

int lprint(const char *sfilename, int nrline, const char *sinfo) {
  FILE *flog = fopen(LOGFILENAME, "a");
  fprintf(flog, "%s : %d %s\n", sfilename, nrline, sinfo);
  fclose(flog);
  return 0;
}

int lprinti(const char *sfilename, int nrline, int n) {
  static char s[BUFSIZE]; 
  sprintf(s, "%d", n);
  lprint( sfilename, nrline, s);

  return 0;
}

#define SHOWINT(x) lprinti(__FILE__, __LINE__,static_cast<int>(x))
#define SHOWCHAR(x) lprint(__FILE__, __LINE__,(const char*)(x))


/* */
Volume::Volume() : RaidDevice(),
      m_Ordinal(-1U),
      m_TotalSize(0),
      m_RaidLevel(-1U),
      m_MigrationProgress(0),
      m_WriteThrough(false),
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
    : RaidDevice(),
      m_Ordinal(ordinal),
      m_TotalSize(0),
      m_RaidLevel(-1U),
      m_MigrationProgress(0),
      m_WriteThrough(false),
      m_SystemVolume(false),
      m_MismatchCount(0),
      m_StripSize(0),
      m_ComponentSize(0),
      m_State(SSI_VolumeStateUnknown),
      m_pSourceDisk(0)
{
    String temp;
	setPath(path);
	SHOWCHAR(path);
	SHOWCHAR(m_Path);
    try {
        SysfsAttr attr = m_Path + "/md/level";
        attr >> temp;
		SHOWCHAR(temp);
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

    try {
		
        int degraded = 0;
SHOWINT(m_Path.length());		
SHOWCHAR(m_Path);
        SysfsAttr attr = m_Path + "/md/degraded";

        attr >> degraded;
		SHOWINT(degraded);
		SHOWINT(m_RaidLevel);
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
	SHOWINT(m_State);
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
    return SSI_StatusOk;
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
    (void)newSize;
    return SSI_StatusOk;
}

/* */
SSI_Status Volume::rename(const String &newName)
{
    Array *pArray = dynamic_cast<Array *>(m_pParent);
    if (pArray == 0) {
        return SSI_StatusFailed;
    }
    if (shell("mdadm --misc -N " + m_DevName + " --update=name " + newName + " /dev/" + pArray->getDevName()) == 0) {
        return SSI_StatusOk;
    }
    return SSI_StatusFailed;
}

/* */
SSI_Status Volume::remove()
{
    Array *pArray = dynamic_cast<Array *>(m_pParent);
    if (pArray == 0) {
        return SSI_StatusFailed;
    }
    if (shell("mdadm -S /dev/" + m_DevName) == 0) {
        if (shell("mdadm --kill-subarray=" + String(m_Ordinal) + " /dev/" + pArray->getDevName()) == 0) {
            return pArray->remove();
        }
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
SSI_Status Volume::modify(SSI_StripSize chunk, SSI_RaidLevel raidLevel,
    unsigned long long newSize, const Container<EndDevice> &disks)
{
    (void)chunk;
    (void)raidLevel;
    (void)disks;
    (void)newSize;
    return SSI_StatusOk;
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
    switch (m_RaidLevel) {
    case 0:
        pInfo->raidLevel = SSI_Raid0;
        break;
    case 1:
        pInfo->raidLevel = SSI_Raid1;
        break;
    case 5:
        pInfo->raidLevel = SSI_Raid5;
        break;
    case 6:
        pInfo->raidLevel = SSI_Raid6;
        break;
    case 10:
        pInfo->raidLevel = SSI_Raid10;
        break;
    default:
        pInfo->raidLevel = SSI_RaidInvalid;
        break;
    }
    pInfo->state = m_State;
    pInfo->totalSize = m_TotalSize;
    switch (m_StripSize) {
    case (2 * 1024):
        pInfo->stripSize = SSI_StripSize2kB;
        break;
    case (4 * 1024):
        pInfo->stripSize = SSI_StripSize4kB;
        break;
    case (8 * 1024):
        pInfo->stripSize = SSI_StripSize8kB;
        break;
    case (16 * 1024):
        pInfo->stripSize = SSI_StripSize16kB;
        break;
    case (32 * 1024):
        pInfo->stripSize = SSI_StripSize32kB;
        break;
    case (64 * 1024):
        pInfo->stripSize = SSI_StripSize64kB;
        break;
    case (128 * 1024):
        pInfo->stripSize = SSI_StripSize128kB;
        break;
    case (256 * 1024):
        pInfo->stripSize = SSI_StripSize256kB;
        break;
    case (512 * 1024):
        pInfo->stripSize = SSI_StripSize512kB;
        break;
    case (1024 * 1024):
        pInfo->stripSize = SSI_StripSize1MB;
        break;
    case (2 * 1024 * 1024):
        pInfo->stripSize = SSI_StripSize2MB;
        break;
    case (4 * 1024 * 1024):
        pInfo->stripSize = SSI_StripSize4MB;
        break;
    case (8 * 1024 * 1024):
        pInfo->stripSize = SSI_StripSize8MB;
        break;
    case (16 * 1024 * 1024):
        pInfo->stripSize = SSI_StripSize16MB;
        break;
    case (32 * 1024 * 1024):
        pInfo->stripSize = SSI_StripSize32MB;
        break;
    case (64 * 1024 * 1024):
        pInfo->stripSize = SSI_StripSize64MB;
        break;
    default:
        pInfo->stripSize = SSI_StripSizeUnknown;
    }
    pInfo->numDisks = m_BlockDevices;
    pInfo->migrProgress = m_MigrationProgress;
    if (m_CachingEnabled == false) {
        pInfo->cachePolicy = SSI_VolumeCachePolicyOff;
    } else {
        if (m_WriteThrough) {
            pInfo->cachePolicy = SSI_VolumeCachePolicyWriteThrough;
        } else {
            pInfo->cachePolicy = SSI_VolumeCachePolicyWriteBack;
        }
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
    switch (stripSize) {
    case SSI_StripSize2kB:
        m_StripSize = 2 * 1024;
        break;
    case SSI_StripSize4kB:
        m_StripSize = 4 * 1024;
        break;
    case SSI_StripSize8kB:
        m_StripSize = 8 * 1024;
        break;
    case SSI_StripSize16kB:
        m_StripSize = 16 * 1024;
        break;
    case SSI_StripSize32kB:
        m_StripSize = 32 * 1024;
        break;
    case SSI_StripSize64kB:
        m_StripSize = 64 * 1024;
        break;
    case SSI_StripSize128kB:
        m_StripSize = 128 * 1024;
        break;
    case SSI_StripSize256kB:
        m_StripSize = 256 * 1024;
        break;
    case SSI_StripSize512kB:
        m_StripSize = 512 * 1024;
        break;
    case SSI_StripSize1MB:
        m_StripSize = 1 * 1024 * 1024;
        break;
    case SSI_StripSize2MB:
        m_StripSize = 2 * 1024 * 1024;
        break;
    case SSI_StripSize4MB:
        m_StripSize = 4 * 1024 * 1024;
        break;
    case SSI_StripSize8MB:
        m_StripSize = 8 * 1024 * 1024;
        break;
    case SSI_StripSize16MB:
        m_StripSize = 16 * 1024 * 1024;
        break;
    case SSI_StripSize32MB:
        m_StripSize = 32 * 1024 * 1024;
        break;
    case SSI_StripSize64MB:
        m_StripSize = 64 * 1024 * 1024;
        break;
    default:
        throw E_INVALID_STRIP_SIZE;
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
        componentSize = m_ComponentSize;
    }
    if (shell("mdadm -CR " + m_Name + " -amd -l" + String(m_RaidLevel) + " --size=" + componentSize +
            " --chunk=" + String(m_StripSize / 1024) + " -n" + String(m_BlockDevices) + devices) != 0) {
        throw E_VOLUME_CREATE_FAILED;
    }
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
