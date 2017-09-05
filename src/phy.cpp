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
#include <engine/phy.h>

using boost::shared_ptr;

static void getItems(const shared_ptr<ScopeObject>& pScopeObject, SSI_ScopeType, Container<Phy> &container)
{
    pScopeObject->getPhys(container);
}

static shared_ptr<Phy> getItem(const shared_ptr<Session>& pSession, SSI_Handle handle)
{
    return pSession->getPhy(handle);
}

/* */
SSI_Status SsiGetPhyHandles(SSI_Handle session, SSI_ScopeType scopeType,
    SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount)
{
    return SsiGetHandles(session, scopeType, scopeHandle, handleList, handleCount, getItems);
}

/* */
SSI_Status SsiGetPhyInfo(SSI_Handle session, SSI_Handle phyHandle,
    SSI_PhyInfo *phyInfo)
{
    return SsiGetInfo(session, phyHandle, phyInfo, getItem);
}

/* */
SSI_Status SsiPhyLocate(SSI_Handle phyHandle, SSI_Bool mode)
{
    shared_ptr<Session> pSession;
    if (SSI_Status status = getTempSession(pSession)) {
        return status;
    }

    shared_ptr<Phy> pPhy = getItem(pSession, phyHandle);
    if (!pPhy) {
        return SSI_StatusInvalidHandle;
    }

    return pPhy->locate(mode == SSI_TRUE);
}

/* */
SSI_Status SsiPhyRemove(SSI_Handle phyHandle, SSI_Bool mode)
{
    if (mode == SSI_FALSE) {
        return SSI_StatusOk;
    }

    shared_ptr<Session> pSession;
    if (SSI_Status status = getTempSession(pSession)) {
        return status;
    }

    shared_ptr<Phy> pPhy = getItem(pSession, phyHandle);
    if (!pPhy) {
        return SSI_StatusInvalidHandle;
    }

    return pPhy->remove();
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
