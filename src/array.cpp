
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
#include <cstddef>

#include <ssi.h>

#include <engine/exception.h>
#include <engine/container.h>
#include <engine/string.h>
#include <engine/object.h>
#include <engine/storage_device.h>
#include <engine/end_device.h>
#include <engine/raid_device.h>
#include <engine/array.h>
#include <engine/volume.h>
#include <engine/session.h>
#include <engine/context_manager.h>

#include "templates.h"

static void getItems(ScopeObject *pScopeObject, SSI_ScopeType scopeType, Container<Array> &container)
{
    pScopeObject->getArrays(container);
}

/* */
SSI_Status SsiGetArrayHandles(SSI_Handle session, SSI_ScopeType scopeType,
    SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount)
{
    return SsiGetHandles(session, scopeType, scopeHandle, handleList, handleCount, getItems);
}

/* */
SSI_Status SsiGetArrayInfo(SSI_Handle session, SSI_Handle arrayHandle,
    SSI_ArrayInfo *arrayInfo)
{
    if (pContextMgr == NULL) {
        return SSI_StatusNotInitialized;
    }
    Session *pSession;
    try {
        pSession = pContextMgr->getSession(session);
    } catch (...) {
        return SSI_StatusFailed;
    }
    if (pSession == NULL) {
        return SSI_StatusInvalidSession;
    }
    Array *pArray = pSession->getArray(arrayHandle);
    if (pArray == NULL) {
        return SSI_StatusInvalidHandle;
    }
    return pArray->getInfo(arrayInfo);
}

/* */
SSI_Status SsiAddDisksToArray(SSI_Handle arrayHandle, SSI_Handle *diskHandles,
    SSI_Uint32 diskHandleCount)
{
    if (pContextMgr == NULL) {
        return SSI_StatusNotInitialized;
    }
    Session *pSession;
    try {
        pSession = pContextMgr->getSession(SSI_NULL_HANDLE);
    } catch (...) {
        return SSI_StatusFailed;
    }
    if (pSession == NULL) {
        return SSI_StatusInvalidSession;
    }
    Array *pArray = pSession->getArray(arrayHandle);
    if (pArray == NULL) {
        return SSI_StatusInvalidHandle;
    }
    if (diskHandles == NULL) {
        return SSI_StatusInvalidParameter;
    }
    if (diskHandleCount == 0) {
        return SSI_StatusBufferTooSmall;
    }
    try {
        Container<EndDevice> container;
        for (unsigned int i = 0; i < diskHandleCount; i++) {
            EndDevice *pEndDevice = pSession->getEndDevice(diskHandles[i]);
            if (pEndDevice == NULL) {
                return SSI_StatusInvalidHandle;
            }
            container.add(pEndDevice);
        }
        return pArray->grow(container);
    } catch (...) {
        return SSI_StatusBufferTooSmall;
    }
}

/* */
SSI_Status SsiArraySetWriteCacheState(SSI_Handle arrayHandle,
    SSI_Bool cacheEnable)
{
    if (pContextMgr == NULL) {
        return SSI_StatusNotInitialized;
    }
    Session *pSession;
    try {
        pSession = pContextMgr->getSession(SSI_NULL_HANDLE);
    } catch (...) {
        return SSI_StatusFailed;
    }
    if (pSession == NULL) {
        return SSI_StatusInvalidSession;
    }
    Array *pArray = pSession->getArray(arrayHandle);
    if (pArray == NULL) {
        return SSI_StatusInvalidHandle;
    }
    return pArray->setWriteCacheState(cacheEnable == SSI_TRUE);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
