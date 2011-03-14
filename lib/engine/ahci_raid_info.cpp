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
#include <asm/types.h>

#include <ssi.h>

#include "exception.h"
#include "list.h"
#include "container.h"
#include "string.h"
#include "filesystem.h"
#include "object.h"
#include "controller.h"
#include "ahci.h"
#include "raid_info.h"
#include "ahci_raid_info.h"
#include "utils.h"

/* */
AHCI_RaidInfo::AHCI_RaidInfo(AHCI *pAHCI, int disksPerArray, int totalRaidDisks,
    int volsPerArray, int volsPerHBA, unsigned short supportedChunkSize)
    : RaidInfo(disksPerArray, totalRaidDisks, volsPerArray, volsPerHBA, supportedChunkSize)
{
    attachController(pAHCI);
}

/* */
bool AHCI_RaidInfo::equal(const Object *pObject) const
{
    return Object::equal(pObject) &&
        dynamic_cast<const RaidInfo *>(pObject)->getControllerType() == SSI_ControllerTypeAHCI;
}

/* */
SSI_Status AHCI_RaidInfo::getRaidLevelInfo(SSI_RaidLevel raidLevel, SSI_RaidLevelInfo *pInfo) const
{
    if (pInfo == 0) {
        return SSI_StatusInvalidParameter;
    }

    pInfo->defaultStripSize = SSI_StripSize64kB;
    pInfo->stripSizesSupported = static_cast<SSI_StripSize>(m_SupportedStripSizes);

    switch (raidLevel) {
    case SSI_Raid0:
        pInfo->supported = SSI_TRUE;
        pInfo->minDisks = min(1, m_RaidDisksSupported);
        pInfo->maxDisks = m_RaidDisksSupported;
        pInfo->migrSupport = static_cast<SSI_RaidLevel>(SSI_Raid1 | SSI_Raid10 | SSI_Raid5);
        pInfo->migrDiskAdd = static_cast<SSI_RaidLevel>(SSI_Raid10 | SSI_Raid5);
        pInfo->evenDiskCount = SSI_FALSE;
        pInfo->oddDiskCount = SSI_FALSE;
        break;
    case SSI_Raid1:
        pInfo->supported = SSI_TRUE;
        pInfo->minDisks = min(2, m_RaidDisksSupported);
        pInfo->maxDisks = min(2, m_RaidDisksSupported);
        pInfo->migrSupport = static_cast<SSI_RaidLevel>(SSI_Raid0 | SSI_Raid5);
        pInfo->migrDiskAdd = static_cast<SSI_RaidLevel>(SSI_Raid5);
        pInfo->evenDiskCount = SSI_TRUE;
        pInfo->oddDiskCount = SSI_FALSE;
        break;
    case SSI_Raid10:
        pInfo->supported = SSI_TRUE;
        pInfo->minDisks = min(4, m_RaidDisksSupported);
        pInfo->maxDisks = min(4, m_RaidDisksSupported);
        pInfo->migrSupport = static_cast<SSI_RaidLevel>(SSI_Raid0 | SSI_Raid5 | SSI_Raid1);
        pInfo->migrDiskAdd = SSI_RaidInvalid;
        pInfo->evenDiskCount = SSI_TRUE;
        pInfo->oddDiskCount = SSI_FALSE;
        break;
    case SSI_Raid5:
        pInfo->supported = SSI_TRUE;
        pInfo->minDisks = min(3, m_RaidDisksSupported);
        pInfo->maxDisks = m_RaidDisksSupported;
        pInfo->migrSupport = static_cast<SSI_RaidLevel>(SSI_Raid0 | SSI_Raid10);
        pInfo->migrDiskAdd = SSI_Raid10;
        pInfo->evenDiskCount = SSI_TRUE;
        pInfo->oddDiskCount = SSI_FALSE;
        break;
    case SSI_Raid6:
        pInfo->supported = SSI_FALSE;
        pInfo->minDisks = 0;
        pInfo->maxDisks = 0;
        pInfo->migrSupport = SSI_RaidInvalid;
        pInfo->migrDiskAdd = SSI_RaidInvalid;
        pInfo->evenDiskCount = SSI_FALSE;
        pInfo->oddDiskCount = SSI_FALSE;
        break;
    default:
        return SSI_StatusInvalidRaidLevel;
    }
    return SSI_StatusOk;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
