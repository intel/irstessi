
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
#include <stddef.h>
#include <unistd.h>

#include <ssi.h>

#include <engine/exception.h>
#include <engine/container.h>
#include <engine/string.h>
#include <engine/object.h>
#include <engine/storage_device.h>
#include <engine/end_device.h>
#include <engine/raid_device.h>
#include <engine/volume.h>
#include <engine/session.h>
#include <engine/context_manager.h>
#include <engine/array.h>
#include <engine/volume.h>

#include "templates.h"

static void getItems(ScopeObject *pScopeObject, SSI_ScopeType scopeType, Container<Volume> &container)
{
    pScopeObject->getVolumes(container);
}

/* */
SSI_Status SsiGetVolumeHandles(SSI_Handle session, SSI_ScopeType scopeType,
    SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount)
{
    return SsiGetHandles(session, scopeType, scopeHandle, handleList, handleCount, getItems);
}

/* */
SSI_Status SsiGetVolumeInfo(SSI_Handle session, SSI_Handle volumeHandle,
    SSI_VolumeInfo *volumeInfo)
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
    Volume *pVolume = pSession->getVolume(volumeHandle);
    if (pVolume == 0) {
        return SSI_StatusInvalidHandle;
    }
    return pVolume->getInfo(volumeInfo);
}

/* */
SSI_Status SsiVolumeMarkAsNormal(SSI_Handle volumeHandle)
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
    Volume *pVolume = pSession->getVolume(volumeHandle);
    if (pVolume == 0) {
        return SSI_StatusInvalidHandle;
    }
    return pVolume->markAsNormal();
}

/* */
SSI_Status SsiVolumeRebuild(SSI_Handle volumeHandle, SSI_Handle diskHandle)
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
    Volume *pVolume = pSession->getVolume(volumeHandle);
    if (pVolume == 0) {
        return SSI_StatusInvalidHandle;
    }
    EndDevice *pEndDevice = pSession->getEndDevice(diskHandle);
    if (pEndDevice == 0) {
        return SSI_StatusInvalidHandle;
    }
    if (pVolume->getState() != SSI_VolumeStateDegraded)
        return SSI_StatusInvalidState;

    return pVolume->rebuild(pEndDevice);
}

/* */
SSI_Status SsiVolumeDelete(SSI_Handle volumeHandle)
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
    Volume *pVolume = pSession->getVolume(volumeHandle);
    if (pVolume == 0) {
        return SSI_StatusInvalidHandle;
    }
    return pVolume->remove();
}

/* */
SSI_Status SsiVolumeCreateFromDisks(SSI_CreateFromDisksParams params, SSI_Handle *volumeHandle)
{
    if (pContextMgr == 0) {
        return SSI_StatusNotInitialized;
    }
    if (volumeHandle == 0) {
        return SSI_StatusInvalidParameter;
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
    Volume *pVolume = 0;
    Array *pArray = 0;
    Container<EndDevice> container;
    EndDevice *pEndDevice;
    /* create container */
    try {
        for (unsigned int i = 0; i < params.numDisks; ++i) {
            pEndDevice = pSession->getEndDevice(params.disks[i]);
            if (pEndDevice == 0) {
                return SSI_StatusInvalidHandle;
            }
            container.add(pEndDevice);
        }
        pEndDevice = pSession->getEndDevice(params.sourceDisk);
        try {
            pArray = new Array();
        } catch (...) {
            return SSI_StatusInsufficientResources;
        }
        pArray->setEndDevices(container);
        pArray->create();
    } catch (Exception ex) {
        delete pArray;
        switch (ex) {
            case E_INVALID_STRIP_SIZE:
                return SSI_StatusInvalidStripSize;
            case E_INVALID_NAME:
                return SSI_StatusInvalidString;
            case E_INVALID_RAID_LEVEL:
                return SSI_StatusInvalidRaidLevel;
            case E_INVALID_USAGE:
                return SSI_StatusInvalidState;
            default:
                return SSI_StatusFailed;
        }
    }
    /* create volume */
    try {
        try {
            pVolume = new Volume();
        } catch (...) {
            pArray->remove();
            delete pArray;
            return SSI_StatusInsufficientResources;
        }
        pVolume->setParent(pArray);
        pVolume->setSourceDisk(pEndDevice);
        pVolume->setEndDevices(container);
        pVolume->setComponentSize(params.sizeInBytes / container.size());
        pVolume->setName(params.volumeName);
        pVolume->setRaidLevel(params.raidLevel);
        if (params.raidLevel != SSI_Raid1)
            pVolume->setStripSize(params.stripSize);
        pVolume->create();
        pSession->addVolume(pVolume);
        pSession->addArray(pArray);
        *volumeHandle = pVolume->getId();
        return SSI_StatusOk;
    } catch (Exception ex) {
        delete pVolume;
        pArray->remove();
        delete pArray;
        switch (ex) {
        case E_INVALID_STRIP_SIZE:
            return SSI_StatusInvalidStripSize;
        case E_INVALID_NAME:
            return SSI_StatusInvalidString;
        case E_INVALID_RAID_LEVEL:
            return SSI_StatusInvalidRaidLevel;
        default:
            return SSI_StatusFailed;
        }
    }
}

/* */
SSI_Status SsiVolumeCreate(SSI_CreateFromArrayParams params)
{
    Volume *pVolume = 0;

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
    Array *pArray = pSession->getArray(params.arrayHandle);
    if (pArray == 0) {
        return SSI_StatusInvalidHandle;
    }
    Container<EndDevice> container;
    pArray->getEndDevices(container, false);
    if (0 == container.size())
        pArray->getEndDevices(container, true);
    try {
        try {
            pVolume = new Volume();
        } catch (...) {
            return SSI_StatusInsufficientResources;
        }
        pVolume->setParent(pArray);
        pVolume->setEndDevices(container);
        pVolume->setComponentSize(params.sizeInBytes / container.size());
        pVolume->setName(params.volumeName);
        pVolume->setStripSize(params.stripSize);
        pVolume->setRaidLevel(params.raidLevel);
        pVolume->create();
        pVolume->update();
        pSession->addVolume(pVolume);
        return SSI_StatusOk;
    } catch (Exception ex) {
        delete pVolume;
        switch (ex) {
        case E_INVALID_STRIP_SIZE:
            return SSI_StatusInvalidStripSize;
        case E_INVALID_NAME:
            return SSI_StatusInvalidString;
        case E_INVALID_RAID_LEVEL:
            return SSI_StatusInvalidRaidLevel;
        default:
            return SSI_StatusFailed;
        }
    } catch (...) {
        return SSI_StatusInsufficientResources;
    }
}

/* */
SSI_Status SsiVolumeRename(SSI_Handle volumeHandle,
    const SSI_Char volumeName[SSI_VOLUME_NAME_LENGTH])
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
    Volume *pVolume = pSession->getVolume(volumeHandle);
    if (pVolume == 0) {
        return SSI_StatusInvalidHandle;
    }
    return pVolume->rename(volumeName);
}

/* */
SSI_Status SsiExpandVolume(SSI_Handle volumeHandle, SSI_Uint64 newSizeMB)
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
    Volume *pVolume = pSession->getVolume(volumeHandle);
    if (pVolume == 0) {
        return SSI_StatusInvalidHandle;
    }
    return pVolume->expand(newSizeMB*1024ULL);
}

/* */
SSI_Status SsiVolumeSetCachePolicy(SSI_Handle volumeHandle,
    SSI_VolumeCachePolicy policy)
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
    Volume *pVolume = pSession->getVolume(volumeHandle);
    if (pVolume == 0) {
        return SSI_StatusInvalidHandle;
    }
    return pVolume->setCachePolicy(policy == SSI_VolumeCachePolicyOff);
}

/* */
SSI_Status SsiVolumeInitialize(SSI_Handle volumeHandle)
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
    Volume *pVolume = pSession->getVolume(volumeHandle);
    if (pVolume == 0) {
        return SSI_StatusInvalidHandle;
    }
    return pVolume->initialize();
}

/* */
SSI_Status SsiVolumeVerify(SSI_Handle volumeHandle, SSI_Bool repair)
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
    Volume *pVolume = pSession->getVolume(volumeHandle);
    if (pVolume == 0) {
        return SSI_StatusInvalidHandle;
    }
    if (pVolume->getState() != SSI_VolumeStateNormal)
        return SSI_StatusInvalidState;

    return pVolume->verify(repair == SSI_TRUE);
}

/* */
SSI_Status SsiVolumeCancelVerify(SSI_Handle volumeHandle)
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
    Volume *pVolume = pSession->getVolume(volumeHandle);
    if (pVolume == 0) {
        return SSI_StatusInvalidHandle;
    }
    if (pVolume->getState() != SSI_VolumeStateVerifying &&
        pVolume->getState() != SSI_VolumeStateVerifyingAndFix)
        return SSI_StatusInvalidState;

    return pVolume->cancelVerify();
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
