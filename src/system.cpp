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

#include <ssi.h>

#include <engine/exception.h>
#include <engine/list.h>
#include <engine/container.h>
#include <engine/string.h>
#include <engine/object.h>
#include <engine/cache.h>
#include <engine/storage_device.h>
#include <engine/session.h>
#include <engine/context_manager.h>

/* */
SSI_Status SsiGetSystemInfo(SSI_SystemInfo *systemInfo)
{
    if (pContextMgr == 0) {
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
    if (pContextMgr == 0) {
        return SSI_StatusNotInitialized;
    }
    return SSI_StatusNotSupported;
}

/* */
SSI_Status SsiSetVolCacheSize(SSI_VolCacheSize cacheSize)
{
    if (pContextMgr == 0) {
        return SSI_StatusNotInitialized;
    }
    (void)cacheSize;
    return SSI_StatusNotSupported;
}

/* */
SSI_Status SsiReadStorageArea(SSI_Handle deviceHandle,
    SSI_StorageArea storageArea, void *buffer, SSI_Uint32 bufferLen)
{
    if (pContextMgr == 0) {
        return SSI_StatusNotInitialized;
    }
    Session *pSession;
    try {
        pSession = pContextMgr->getSession(SSI_NULL_HANDLE);
    } catch (...) {
        return SSI_StatusFailed;
    }
    if (storageArea != SSI_StorageAreaCim) {
        return SSI_StatusInvalidParameter;
    }
    StorageDevice *pDevice = pSession->getDevice(deviceHandle);
    if (pDevice == 0) {
        return SSI_StatusInvalidHandle;
    }
    return pDevice->readStorageArea(buffer, bufferLen);
}

/* */
SSI_Status SsiWriteStorageArea(SSI_Handle deviceHandle,
    SSI_StorageArea storageArea, void *buffer, SSI_Uint32 bufferLen)
{
    if (pContextMgr == 0) {
        return SSI_StatusNotInitialized;
    }
    Session *pSession;
    try {
        pSession = pContextMgr->getSession(SSI_NULL_HANDLE);
    } catch (...) {
        return SSI_StatusFailed;
    }
    if (storageArea != SSI_StorageAreaCim) {
        return SSI_StatusInvalidParameter;
    }
    StorageDevice *pDevice = pSession->getDevice(deviceHandle);
    if (pDevice == 0) {
        return SSI_StatusInvalidHandle;
    }
    return pDevice->writeStorageArea(buffer, bufferLen);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
