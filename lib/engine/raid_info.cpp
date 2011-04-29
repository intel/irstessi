
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

#include <ssi.h>

#include "exception.h"
#include "list.h"
#include "container.h"
#include "string.h"
#include "object.h"
#include "raid_info.h"
#include "session.h"

/* */
RaidInfo::RaidInfo(int disksPerArray, int totalRaidDisks, int volumesPerArray,
    int volumesPerHBA, unsigned short supportedChunkSize)
    : m_DisksPerArray(disksPerArray),
      m_RaidDisksSupported(totalRaidDisks),
      m_VolumesPerHBA(volumesPerHBA),
      m_VolumesPerArray(volumesPerArray),
      m_SupportedStripSizes(supportedChunkSize)
{
}

/* */
void RaidInfo::getControllers(Container<Controller> &container) const
{
    container = m_Controllers;
}

/* */
void RaidInfo::acquireId(Session *pSession)
{
    pSession->addRaidInfo(this);
}

/* */
SSI_Status RaidInfo::getInfo(SSI_RaidInfo *pInfo) const
{
    if (pInfo == 0) {
        return SSI_StatusInvalidParameter;
    }
    pInfo->raidHandle = getId();
    pInfo->maxDisksPerArray = m_DisksPerArray;
    pInfo->maxRaidDisksSupported = m_RaidDisksSupported;
    pInfo->maxVolumesPerHba = m_VolumesPerHBA;
    pInfo->maxVolumesPerArray = m_VolumesPerArray;
    pInfo->raidEnabled = SSI_TRUE;
    pInfo->supportsDedicatedSpare = SSI_TRUE;
    pInfo->supportsGlobalSpare = SSI_TRUE;
    pInfo->supportsCreateFromExisting = SSI_TRUE;
    pInfo->supportsEmptyArrays = SSI_TRUE;

    return SSI_StatusOk;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
