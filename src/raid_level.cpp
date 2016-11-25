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

#include "templates.h"
#include <engine/raid_info.h>
#include <engine/volume.h>

using boost::shared_ptr;

/* */
SSI_Status SsiGetRaidLevelInfo(SSI_Handle session, SSI_Handle raidInfoHandle, SSI_RaidLevel raidLevel, SSI_RaidLevelInfo *raidLevelInfo)
{
    shared_ptr<Session> pSession;
    if (SSI_Status status = getSession(session, pSession)) {
        return status;
    }

    shared_ptr<RaidInfo> pRaidInfo = pSession->getRaidInfo(raidInfoHandle);
    if (!pRaidInfo) {
        return SSI_StatusInvalidHandle;
    }

    return pRaidInfo->getRaidLevelInfo(raidLevel, raidLevelInfo);
}

/* */
SSI_Status SsiRaidLevelModify(SSI_Handle volumeHandle, SSI_RaidLevelModifyParams params)
{
    shared_ptr<Session> pSession;
    if (SSI_Status status = getTempSession(pSession)) {
        return status;
    }

    shared_ptr<Volume> pVolume = pSession->getVolume(volumeHandle);
    if (!pVolume) {
        return SSI_StatusInvalidHandle;
    }

    if (params.diskHandles == NULL && params.diskHandleCount != 0) {
        return SSI_StatusInvalidParameter;
    }

    Container<EndDevice> container;

    for (unsigned int i = 0; i < params.diskHandleCount; i++) {
        shared_ptr<EndDevice> pEndDevice = pSession->getEndDevice(params.diskHandles[i]);
        if (!pEndDevice) {
            return SSI_StatusInvalidHandle;
        }

        if (pVolume->getLogicalSectorSize() != pEndDevice->getLogicalSectorSize()) {
            return SSI_StatusNotSupported;
        }

        if (SSI_Status status = container.add(pEndDevice)) {
            return status;
        }
    }

    return pVolume->modify(params.newStripSize, params.newRaidLevel, params.newSizeInBytes, container);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
