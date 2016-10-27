
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

#include <features.h>
#include <cstddef>

#include <ssi.h>

#include <engine/exception.h>
#include <engine/container.h>
#include <engine/string.h>
#include <engine/object.h>
#include <engine/storage_device.h>
#include <engine/raid_device.h>
#include <engine/array.h>
#include <engine/end_device.h>
#include <engine/session.h>
#include <engine/context_manager.h>

#include "templates.h"

namespace {
    void getItems(ScopeObject *pScopeObject, SSI_ScopeType scopeType, Container<EndDevice> &container)
    {
        pScopeObject->getEndDevices(container, scopeType == SSI_ScopeTypeControllerAll);
    }

    EndDevice * getItem(Session *pSession, SSI_Handle handle)
    {
        return pSession->getEndDevice(handle);
    }
}

/* */
SSI_Status SsiGetEndDeviceHandles(SSI_Handle session, SSI_ScopeType scopeType,
    SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount)
{
    return SsiGetHandles(session, scopeType, scopeHandle, handleList, handleCount, getItems);
}

/* */
SSI_Status SsiGetEndDeviceInfo(SSI_Handle session, SSI_Handle endDeviceHandle,
    SSI_EndDeviceInfo *endDeviceInfo)
{
    return SsiGetInfo(session, endDeviceHandle, endDeviceInfo, getItem);
}

/* */
SSI_Status SsiDiskClearMetadata(SSI_Handle diskHandle)
{
    EndDevice *pEndDevice = NULL;
    if (SSI_Status status = SsiGetItem(diskHandle, &pEndDevice, getItem)) {
        return status;
    }

    return pEndDevice->clearMetadata();
}

/* */
SSI_Status SsiDiskMarkAsSpare(SSI_Handle diskHandle, SSI_Handle arrayHandle)
{
    TemporarySession session;
    if (!session.isValid()) {
        return SSI_StatusNotInitialized;
    }

    EndDevice *pEndDevice = getItem(session.get(), diskHandle);
    if (pEndDevice == NULL) {
        return SSI_StatusInvalidHandle;
    }

    if (pEndDevice->isFultondalex8()) {
        return SSI_StatusNotSupported;
    }

    if (arrayHandle == SSI_NULL_HANDLE) {
        return pEndDevice->makeSpare();
    }

    Array *pArray = session->getArray(arrayHandle);
    if (pArray == NULL) {
        return SSI_StatusInvalidHandle;
    }

    if (pEndDevice->getArray() == pArray) {
        return SSI_StatusInvalidState;
    }

    return pArray->addSpare(pEndDevice);
}

/* */
SSI_Status SsiDiskUnmarkAsSpare(SSI_Handle diskHandle)
{
    EndDevice *pEndDevice = NULL;
    if (SSI_Status status = SsiGetItem(diskHandle, &pEndDevice, getItem)) {
        return status;
    }

    if (pEndDevice->isFultondalex8()) {
        return SSI_StatusInvalidState;
    }

    Array *pArray = pEndDevice->getArray();
    if (pArray == NULL) {
        return SSI_StatusInvalidState;
    }

    return pArray->removeSpare(pEndDevice);
}

/* */
SSI_Status SsiDiskMarkAsNormal(SSI_Handle diskHandle)
{
    EndDevice *pEndDevice = NULL;
    if (SSI_Status status = SsiGetItem(diskHandle, &pEndDevice, getItem)) {
        return status;
    }

    return pEndDevice->markAsNormal();
}

/* */
SSI_Status SsiDiskUnlock(SSI_Handle diskHandle, SSI_DiskUnlockInfo *unlockInfo)
{
    EndDevice *pEndDevice = NULL;
    if (SSI_Status status = SsiGetItem(diskHandle, &pEndDevice, getItem)) {
        return status;
    }

    return pEndDevice->unlock(unlockInfo);
}

/* */
SSI_Status SsiDiskAssignStoragePool(SSI_Handle diskHandle, SSI_Uint8 poolId)
{
    EndDevice *pEndDevice = NULL;
    if (SSI_Status status = SsiGetItem(diskHandle, &pEndDevice, getItem)) {
        return status;
    }

    return pEndDevice->assignPoolId(poolId);
}

/* */
SSI_Status SsiPassthroughCommand(SSI_Handle deviceHandle, void *cmdInfoUnit,
    void *dataBuffer, SSI_Uint32 dataBufferLen, SSI_DataDirection dataDirection)
{
    EndDevice *pEndDevice = NULL;
    if (SSI_Status status = SsiGetItem(deviceHandle, &pEndDevice, getItem)) {
        return status;
    }

    return pEndDevice->passthroughCmd(cmdInfoUnit, dataBuffer, dataBufferLen, dataDirection);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
