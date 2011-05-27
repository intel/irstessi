
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
#include <asm/types.h>

#include <ssi.h>

#include "exception.h"
#include "list.h"
#include "container.h"
#include "string.h"
#include "filesystem.h"
#include "object.h"
#include "controller.h"
#include "isci.h"
#include "raid_info.h"
#include "isci_raid_info.h"
#include "utils.h"

/* */
ISCI_RaidInfo::ISCI_RaidInfo(ISCI *pISCI, int disksPerArray, int totalRaidDisks,
    int volsPerArray, int volsPerHBA, unsigned short supportedChunkSize)
    : RaidInfo(disksPerArray, totalRaidDisks, volsPerArray, volsPerHBA, supportedChunkSize)
{
    attachController(pISCI);
}

/* */
bool ISCI_RaidInfo::equal(const Object *pObject) const
{
    return Object::equal(pObject) &&
        dynamic_cast<const RaidInfo *>(pObject)->getControllerType() == SSI_ControllerTypeSCU;
}

/* */
SSI_Status ISCI_RaidInfo::getRaidLevelInfo(SSI_RaidLevel raidLevel, SSI_RaidLevelInfo *pInfo) const
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
