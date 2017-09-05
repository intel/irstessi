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

#include "templates.h"
#include <engine/raid_info.h>

using boost::shared_ptr;

/* */
SSI_Status SsiGetRaidInfoHandles(SSI_Handle session, SSI_Handle *handleList, SSI_Uint32 *handleCount)
{
    shared_ptr<Session> pSession;
    if (SSI_Status status = getSession(session, pSession)) {
        return status;
    }

    Container<RaidInfo> container;
    pSession->getRaidInfo(container);
    return container.getHandles(handleList, handleCount);
}

/* */
SSI_Status SsiGetRaidInfo(SSI_Handle session, SSI_Handle raidInfoHandle, SSI_RaidInfo *raidInfo)
{
    shared_ptr<Session> pSession;
    if (SSI_Status status = getSession(session, pSession)) {
        return status;
    }

    shared_ptr<RaidInfo> pRaidInfo = pSession->getRaidInfo(raidInfoHandle);
    if (!pRaidInfo) {
        return SSI_StatusInvalidHandle;
    }

    return pRaidInfo->getInfo(raidInfo);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
