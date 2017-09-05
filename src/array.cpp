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
#include <engine/array.h>
#include <engine/volume.h>

using boost::shared_ptr;

static void getItems(const shared_ptr<ScopeObject>& pScopeObject, SSI_ScopeType, Container<Array> &container)
{
    pScopeObject->getArrays(container);
}

static shared_ptr<Array> getItem(const shared_ptr<Session>& pSession, SSI_Handle handle)
{
    return pSession->getArray(handle);
}

/* */
SSI_Status SsiGetArrayHandles(SSI_Handle session, SSI_ScopeType scopeType, SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount)
{
    return SsiGetHandles(session, scopeType, scopeHandle, handleList, handleCount, getItems);
}

/* */
SSI_Status SsiGetArrayInfo(SSI_Handle session, SSI_Handle arrayHandle, SSI_ArrayInfo *arrayInfo)
{
    return SsiGetInfo(session, arrayHandle, arrayInfo, getItem);
}

/* */
SSI_Status SsiAddDisksToArray(SSI_Handle arrayHandle, SSI_Handle *diskHandles, SSI_Uint32 diskHandleCount, SSI_StripSize)
{
    shared_ptr<Session> pSession;
    if (SSI_Status status = getTempSession(pSession)) {
        return status;
    }

    shared_ptr<Array> pArray = getItem(pSession, arrayHandle);
    if (!pArray) {
        return SSI_StatusInvalidHandle;
    }

    Container<Volume> volumes;
    try {
        pArray->getVolumes(volumes);
    } catch (...) {
        return SSI_StatusInsufficientResources;
    }

    bool isRaid0 = true;
    foreach (iter, volumes) {
        Volume& volume = *(*iter);

        SSI_RaidLevel raid = volume.getSsiRaidLevel();
        if (raid == SSI_Raid1 || raid == SSI_Raid10) {
            return SSI_StatusInvalidRaidLevel;
        }

        isRaid0 = (raid == SSI_Raid0);
    }

    if (diskHandles == NULL) {
        return SSI_StatusInvalidParameter;
    }

    if (diskHandleCount == 0) {
        return SSI_StatusBufferTooSmall;
    }

    Container<EndDevice> enddevices;
    try {
        pArray->getEndDevices(enddevices, true);
    } catch (...) {
        return SSI_StatusInsufficientResources;
    }

    if (enddevices.empty()) {
        return SSI_StatusInternalError;
    }

    unsigned int blockSize = enddevices.front()->getLogicalSectorSize();
    Container<EndDevice> container;
    for (unsigned int i = 0; i < diskHandleCount; i++) {
        shared_ptr<EndDevice> pEndDevice = pSession->getEndDevice(diskHandles[i]);
        if (!pEndDevice) {
            return SSI_StatusInvalidHandle;
        }

        if (!isRaid0 && pEndDevice->isFultondalex8()) {
            return SSI_StatusNotSupported;
        }

        if (pEndDevice->getLogicalSectorSize() != blockSize) {
            return SSI_StatusNotSupported;
        }

        if (SSI_Status status = container.add(pEndDevice)) {
            return status;
        }
    }

    try {
        return pArray->grow(container);
    } catch (...) {
        return SSI_StatusInternalError;
    }
}

/* */
SSI_Status SsiArraySetWriteCacheState(SSI_Handle arrayHandle, SSI_Bool cacheEnable)
{
    shared_ptr<Session> pSession;
    if (SSI_Status status = getTempSession(pSession)) {
        return status;
    }

    shared_ptr<Array> pArray = getItem(pSession, arrayHandle);
    if (!pArray) {
        return SSI_StatusInvalidHandle;
    }

    return pArray->setWriteCacheState(cacheEnable == SSI_TRUE);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
