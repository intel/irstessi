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
#include <engine/storage_device.h>
#include <engine/cache.h>
#include <engine/raid_device.h>
#include <engine/array.h>
#include <engine/end_device.h>
#include <engine/session.h>
#include <engine/context_manager.h>

/* */
SSI_Status SsiGetEndDeviceHandles(SSI_Handle session, SSI_ScopeType scopeType,
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
    Container<EndDevice> container;
    try {
        pScopeObject->getEndDevices(container, scopeType == SSI_ScopeTypeControllerAll);
    } catch (...) {
        return SSI_StatusInvalidScope;
    }
    return container.getHandles(handleList, handleCount);
}

/* */
SSI_Status SsiGetEndDeviceInfo(SSI_Handle session, SSI_Handle endDeviceHandle,
    SSI_EndDeviceInfo *endDeviceInfo)
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
    EndDevice *pEndDevice = pSession->getEndDevice(endDeviceHandle);
    if (pEndDevice == 0) {
        return SSI_StatusInvalidHandle;
    }
    return pEndDevice->getInfo(endDeviceInfo);
}

/* */
SSI_Status SsiDiskClearMetadata(SSI_Handle diskHandle)
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
        return SSI_StatusFailed;
    }
    EndDevice *pEndDevice = pSession->getEndDevice(diskHandle);
    if (pEndDevice == 0) {
        return SSI_StatusInvalidHandle;
    }
    return pEndDevice->clearMetadata();
}

/* */
SSI_Status SsiDiskMarkAsSpare(SSI_Handle diskHandle, SSI_Handle arrayHandle)
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
        return SSI_StatusFailed;
    }
    Array *pArray = pSession->getArray(arrayHandle);
    if (pArray == 0) {
        return SSI_StatusInvalidHandle;
    }
    EndDevice *pEndDevice = pSession->getEndDevice(diskHandle);
    if (pEndDevice == 0) {
        return SSI_StatusInvalidHandle;
    }
    return pArray->addSpare(pEndDevice);
}

/* */
SSI_Status SsiDiskUnmarkAsSpare(SSI_Handle diskHandle)
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
        return SSI_StatusFailed;
    }
    EndDevice *pEndDevice = pSession->getEndDevice(diskHandle);
    if (pEndDevice == 0) {
        return SSI_StatusInvalidHandle;
    }
    Array *pArray = pEndDevice->getArray();
    if (pArray == 0) {
        return SSI_StatusInvalidState;
    }
    if (pArray->removeSpare(pEndDevice) != SSI_StatusOk) {
        return SSI_StatusFailed;
    }
    return pEndDevice->clearMetadata();
}

/* */
SSI_Status SsiDiskMarkAsNormal(SSI_Handle diskHandle)
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
        return SSI_StatusFailed;
    }
    EndDevice *pEndDevice = pSession->getEndDevice(diskHandle);
    if (pEndDevice == 0) {
        return SSI_StatusInvalidHandle;
    }
    return pEndDevice->markAsNormal();
}

/* */
SSI_Status SsiDiskUnlock(SSI_Handle diskHandle, SSI_DiskUnlockInfo *unlockInfo)
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
        return SSI_StatusFailed;
    }
    EndDevice *pEndDevice = pSession->getEndDevice(diskHandle);
    if (pEndDevice == 0) {
        return SSI_StatusInvalidHandle;
    }
    return pEndDevice->unlock(unlockInfo);
}

/* */
SSI_Status SsiDiskAssignStoragePool(SSI_Handle diskHandle, SSI_Uint8 poolId)
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
        return SSI_StatusFailed;
    }
    EndDevice *pEndDevice = pSession->getEndDevice(diskHandle);
    if (pEndDevice == 0) {
        return SSI_StatusInvalidHandle;
    }
    return pEndDevice->assignPoolId(poolId);
}

/* */
SSI_Status SsiPassthroughCommand(SSI_Handle deviceHandle, void *cmdInfoUnit,
    void *dataBuffer, SSI_Uint32 dataBufferLen, SSI_DataDirection dataDirection)
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
        return SSI_StatusFailed;
    }
    EndDevice *pEndDevice = pSession->getEndDevice(deviceHandle);
    if (pEndDevice == 0) {
        return SSI_StatusInvalidHandle;
    }
    return pEndDevice->passthroughCmd(cmdInfoUnit, dataBuffer, dataBufferLen,
        dataDirection);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
