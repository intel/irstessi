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
#include <engine/end_device.h>
#include <engine/raid_device.h>
#include <engine/array.h>
#include <engine/volume.h>
#include <engine/session.h>
#include <engine/context_manager.h>

/* */
SSI_Status SsiGetArrayHandles(SSI_Handle session, SSI_ScopeType scopeType,
    SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount)
{
    if (pContextMgr == 0) {
        return SSI_StatusNotInitialized;
    }
    Session *pSession;
    try {
        pSession = pContextMgr->getSession(session);
    } catch (...) {
        return SSI_StatusFailed;
    }
    if (pSession == 0) {
        return SSI_StatusInvalidSession;
    }
    ScopeObject *pScopeObject = pSession->getObject(scopeHandle);
    if (pScopeObject == 0) {
        return SSI_StatusInvalidScope;
    }
    if (*pScopeObject != scopeType) {
        return SSI_StatusInvalidScope;
    }
    Container container;
    try {
        pScopeObject->getArrays(container);
    } catch (...) {
        return SSI_StatusInvalidScope;
    }
    return container.getHandles(handleList, handleCount);
}

/* */
SSI_Status SsiGetArrayInfo(SSI_Handle session, SSI_Handle arrayHandle,
    SSI_ArrayInfo *arrayInfo)
{
    if (pContextMgr == 0) {
        return SSI_StatusNotInitialized;
    }
    Session *pSession;
    try {
        pSession = pContextMgr->getSession(session);
    } catch (...) {
        return SSI_StatusFailed;
    }
    if (pSession == 0) {
        return SSI_StatusInvalidSession;
    }
    Array *pArray = pSession->getArray(arrayHandle);
    if (pArray == 0) {
        return SSI_StatusInvalidHandle;
    }
    return pArray->getInfo(arrayInfo);
}

/* */
SSI_Status SsiAddDisksToArray(SSI_Handle arrayHandle, SSI_Handle *diskHandles,
    SSI_Uint32 diskHandleCount)
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
    if (pSession == 0) {
        return SSI_StatusInvalidSession;
    }
    Array *pArray = pSession->getArray(arrayHandle);
    if (pArray == 0) {
        return SSI_StatusInvalidHandle;
    }
    if (diskHandles == 0) {
        return SSI_StatusInvalidParameter;
    }
    if (diskHandleCount == 0) {
        return SSI_StatusBufferTooSmall;
    }
    try {
        Container container;
        for (unsigned int i = 0; i < diskHandleCount; i++) {
            EndDevice *pEndDevice = pSession->getEndDevice(diskHandles[i]);
            if (pEndDevice == 0) {
                return SSI_StatusInvalidHandle;
            }
            container.add(pEndDevice);
        }
        return pArray->addSpare(container);
    } catch (...) {
        return SSI_StatusBufferTooSmall;
    }
}

/* */
SSI_Status SsiArraySetWriteCacheState(SSI_Handle arrayHandle,
    SSI_Bool cacheEnable)
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
    if (pSession == 0) {
        return SSI_StatusInvalidSession;
    }
    Array *pArray = pSession->getArray(arrayHandle);
    if (pArray == 0) {
        return SSI_StatusInvalidHandle;
    }
    return pArray->setWriteCacheState(cacheEnable == SSI_TRUE);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
