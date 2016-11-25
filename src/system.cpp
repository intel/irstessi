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
#include <engine/storage_device.h>

using boost::shared_ptr;

/* */
SSI_Status SsiGetSystemInfo(SSI_SystemInfo *systemInfo)
{
    if (pContextMgr == NULL) {
        return SSI_StatusNotInitialized;
    }

    try {
        return pContextMgr->getSystemInfo(systemInfo);
    } catch (...) {
        return SSI_StatusFailed;
    }
}

/* */
SSI_Status SsiRescan(void)
{
    if (pContextMgr == NULL) {
        return SSI_StatusNotInitialized;
    }
    return SSI_StatusNotSupported;
}

/* */
SSI_Status SsiSetVolCacheSize(SSI_VolCacheSize)
{
    if (pContextMgr == NULL) {
        return SSI_StatusNotInitialized;
    }

    return SSI_StatusNotSupported;
}

/* */
SSI_Status SsiReadStorageArea(SSI_Handle deviceHandle, SSI_StorageArea storageArea, void *buffer, SSI_Uint32 bufferLen)
{
    shared_ptr<Session> pSession;
    if (SSI_Status status = getTempSession(pSession)) {
        return status;
    }

    if (storageArea != SSI_StorageAreaCim) {
        return SSI_StatusInvalidParameter;
    }

    shared_ptr<StorageDevice> pDevice = pSession->getDevice(deviceHandle);
    if (!pDevice) {
        return SSI_StatusInvalidHandle;
    }

    return pDevice->readStorageArea(buffer, bufferLen);
}

/* */
SSI_Status SsiWriteStorageArea(SSI_Handle deviceHandle, SSI_StorageArea storageArea, void *buffer, SSI_Uint32 bufferLen)
{
    shared_ptr<Session> pSession;
    if (SSI_Status status = getTempSession(pSession)) {
        return status;
    }

    if (storageArea != SSI_StorageAreaCim) {
        return SSI_StatusInvalidParameter;
    }

    shared_ptr<StorageDevice> pDevice = pSession->getDevice(deviceHandle);
    if (!pDevice) {
        return SSI_StatusInvalidHandle;
    }

    return pDevice->writeStorageArea(buffer, bufferLen);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
