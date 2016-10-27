
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
#include <engine/utils.h>

#include "templates.h"

namespace {
    void getItems(ScopeObject *pScopeObject, SSI_ScopeType, Container<Volume> &container)
    {
        pScopeObject->getVolumes(container);
    }

    Volume * getItem(Session *pSession, SSI_Handle handle)
    {
        return pSession->getVolume(handle);
    }
}

/* */
SSI_Status SsiGetVolumeHandles(SSI_Handle session, SSI_ScopeType scopeType, SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount)
{
    return SsiGetHandles(session, scopeType, scopeHandle, handleList, handleCount, getItems);
}

/* */
SSI_Status SsiGetVolumeInfo(SSI_Handle session, SSI_Handle volumeHandle, SSI_VolumeInfo *volumeInfo)
{
    return SsiGetInfo(session, volumeHandle, volumeInfo, getItem);
}

/* */
SSI_Status SsiVolumeMarkAsNormal(SSI_Handle volumeHandle)
{
    Volume *pVolume = NULL;
    if (SSI_Status status = SsiGetItem(volumeHandle, &pVolume, getItem)) {
        return status;
    }

    return pVolume->markAsNormal();
}

/* */
SSI_Status SsiVolumeRebuild(SSI_Handle volumeHandle, SSI_Handle diskHandle)
{
    TemporarySession session;
    if (!session.isValid()) {
        return SSI_StatusNotInitialized;
    }

    Volume *pVolume = getItem(session.get(), volumeHandle);
    if (pVolume == NULL) {
        return SSI_StatusInvalidHandle;
    }

    EndDevice *pEndDevice = session->getEndDevice(diskHandle);
    if (pEndDevice == NULL) {
        return SSI_StatusInvalidHandle;
    }

    if (pEndDevice->isFultondalex8()) {
        return SSI_StatusNotSupported;
    }

    if (pVolume->getState() != SSI_VolumeStateDegraded) {
        return SSI_StatusInvalidState;
    }

    if (pVolume->getLogicalSectorSize() != pEndDevice->getLogicalSectorSize()) {
        return SSI_StatusNotSupported;
    }

    return pVolume->rebuild(pEndDevice);
}

/* */
SSI_Status SsiVolumeDelete(SSI_Handle volumeHandle)
{
    Volume *pVolume = NULL;
    if (SSI_Status status = SsiGetItem(volumeHandle, &pVolume, getItem)) {
        return status;
    }

    return pVolume->remove();
}

/* */
SSI_Status SsiVolumeCreateFromDisks(SSI_CreateFromDisksParams params, SSI_Handle *volumeHandle)
{
    if (volumeHandle == NULL) {
        return SSI_StatusInvalidParameter;
    }

    if (params.numDisks == 0) {
        setLastErrorMessage("Not enough disks to create volume");
        return SSI_StatusInvalidParameter;
    }

    TemporarySession session;
    if (!session.isValid()) {
        return SSI_StatusNotInitialized;
    }

    Volume *pVolume = NULL;
    Array *pArray = NULL;
    Container<EndDevice> container;
    EndDevice *pEndDevice;

    /* create container */
    try {
        for (unsigned int i = 0; i < params.numDisks; ++i) {
            pEndDevice = session->getEndDevice(params.disks[i]);
            if (pEndDevice == NULL) {
                return SSI_StatusInvalidHandle;
            }
            container.add(pEndDevice);
        }

        // check block sizes
        unsigned int blockSize = container.front()->getLogicalSectorSize();
        foreach (iter, container) {
            EndDevice& disk = *(*iter);

            if (disk.getLogicalSectorSize() != blockSize) {
                return SSI_StatusNotSupported;
            }
        }

        pEndDevice = session->getEndDevice(params.sourceDisk);

        if (pEndDevice != NULL) {
            throw E_NOT_SUPPORTED;
        }

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
            case E_NOT_SUPPORTED:
                return SSI_StatusNotSupported;
            case E_INVALID_STRIP_SIZE:
                return SSI_StatusInvalidStripSize;
            case E_INVALID_NAME:
                return SSI_StatusInvalidString;
            case E_INVALID_SIZE:
                return SSI_StatusInvalidSize;
            case E_INVALID_RAID_LEVEL:
                return SSI_StatusInvalidRaidLevel;
            case E_INVALID_USAGE:
                return SSI_StatusInvalidState;
            case E_BUFFER_TOO_SMALL:
                return SSI_StatusBufferTooSmall;
            case E_SYSTEM_DEVICE:
                setLastErrorMessage("Volume with system device is not allowed");
                return SSI_StatusInvalidParameter;
            default:
                return SSI_StatusFailed;
        }
    }

    clearLastErrorMessage();

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
        pVolume->setComponentSize(params.sizeInBytes, container.size(), params.raidLevel);
        pVolume->setName(params.volumeName);
        pVolume->setRaidLevel(params.raidLevel);
        if (params.raidLevel != SSI_Raid1) {
            pVolume->setStripSize(params.stripSize);
        }
        pVolume->setRwhPolicy(params.rwhPolicy);
        pVolume->create();
        session->addVolume(pVolume);
        session->addArray(pArray);
        *volumeHandle = pVolume->getId();
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
            case E_BUFFER_TOO_SMALL:
                return SSI_StatusBufferTooSmall;
            case E_BUFFER_TOO_LARGE:
                return SSI_StatusBufferTooLarge;
            case E_INVALID_OBJECT:
                return SSI_StatusInvalidParameter;
            case E_NOT_SUPPORTED:
                return SSI_StatusNotSupported;
            default:
                return SSI_StatusFailed;
        }
    }

    return SSI_StatusOk;
}

/* */
SSI_Status SsiVolumeCreate(SSI_CreateFromArrayParams params)
{
    TemporarySession session;
    if (!session.isValid()) {
        return SSI_StatusNotInitialized;
    }

    Array *pArray = session->getArray(params.arrayHandle);
    if (pArray == NULL) {
        return SSI_StatusInvalidHandle;
    }

    Container<EndDevice> container;
    pArray->getEndDevices(container, false);
    if (0 == container.size()) {
        pArray->getEndDevices(container, true);
    }

    Volume *pVolume = NULL;
    try {
        try {
            pVolume = new Volume();
        } catch (...) {
            return SSI_StatusInsufficientResources;
        }
        pVolume->setParent(pArray);
        pVolume->setEndDevices(container);
        pVolume->setComponentSize(params.sizeInBytes, container.size(), params.raidLevel);
        pVolume->setName(params.volumeName);
        pVolume->setStripSize(params.stripSize);
        pVolume->setRaidLevel(params.raidLevel);
        pVolume->setRwhPolicy(params.rwhPolicy);
        pVolume->create();
        pVolume->update();
        session->addVolume(pVolume);

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
            case E_BUFFER_TOO_SMALL:
                return SSI_StatusBufferTooSmall;
            case E_BUFFER_TOO_LARGE:
                return SSI_StatusBufferTooLarge;
            case E_INVALID_OBJECT:
                return SSI_StatusInvalidParameter;
            case E_NOT_SUPPORTED:
                return SSI_StatusNotSupported;
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
    Volume *pVolume = NULL;
    if (SSI_Status status = SsiGetItem(volumeHandle, &pVolume, getItem)) {
        return status;
    }

    TemporarySession session;
    if (!session.isValid()) {
        return SSI_StatusNotInitialized;
    }

    String name = volumeName;

    Container<Volume> volumes;
    session->getVolumes(volumes);
    foreach (iter, volumes) {
        Volume& volume = *(*iter);

        if (volume.getName() == name) {
            setLastErrorMessage("Volume name already in use");
            return SSI_StatusInvalidString;
        }
    }

    return pVolume->rename(name);
}

/* */
SSI_Status SsiExpandVolume(SSI_Handle volumeHandle, SSI_Uint64 newSizeMB)
{
    Volume *pVolume = NULL;
    if (SSI_Status status = SsiGetItem(volumeHandle, &pVolume, getItem)) {
        return status;
    }

    return pVolume->expand(newSizeMB * 1024ULL);
}

/* */
SSI_Status SsiVolumeSetCachePolicy(SSI_Handle volumeHandle, SSI_VolumeCachePolicy policy)
{
    Volume *pVolume = NULL;
    if (SSI_Status status = SsiGetItem(volumeHandle, &pVolume, getItem)) {
        return status;
    }

    return pVolume->setCachePolicy(policy == SSI_VolumeCachePolicyOff);
}

/* */
SSI_Status SsiVolumeSetRwhPolicy(SSI_Handle volumeHandle, SSI_Handle journalingDriveHandle, SSI_RwhPolicy policy)
{
    Volume *pVolume = NULL;
    if (SSI_Status status = SsiGetItem(volumeHandle, &pVolume, getItem)) {
        return status;
    }

    return pVolume->changeRwhPolicy(policy);
}

/* */
SSI_Status SsiVolumeInitialize(SSI_Handle volumeHandle)
{
    Volume *pVolume = NULL;
    if (SSI_Status status = SsiGetItem(volumeHandle, &pVolume, getItem)) {
        return status;
    }

    return pVolume->initialize();
}

/* */
SSI_Status SsiVolumeVerify(SSI_Handle volumeHandle, SSI_Bool repair)
{
    Volume *pVolume = NULL;
    if (SSI_Status status = SsiGetItem(volumeHandle, &pVolume, getItem)) {
        return status;
    }

    if (pVolume->getState() != SSI_VolumeStateNormal) {
        return SSI_StatusInvalidState;
    }

    return pVolume->verify(repair == SSI_TRUE);
}

/* */
SSI_Status SsiVolumeCancelVerify(SSI_Handle volumeHandle)
{
    Volume *pVolume = NULL;
    if (SSI_Status status = SsiGetItem(volumeHandle, &pVolume, getItem)) {
        return status;
    }

    if (pVolume->getState() != SSI_VolumeStateVerifying &&
            pVolume->getState() != SSI_VolumeStateVerifyingAndFix) {
        return SSI_StatusInvalidState;
    }

    return pVolume->cancelVerify();
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
