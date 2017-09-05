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
#include <engine/volume.h>
#include <engine/array.h>

using boost::shared_ptr;

namespace {
    void getItems(const shared_ptr<ScopeObject>& pScopeObject, SSI_ScopeType, Container<Volume> &container)
    {
        pScopeObject->getVolumes(container);
    }

    shared_ptr<Volume> getItem(const shared_ptr<Session>& pSession, SSI_Handle handle)
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
    shared_ptr<Session> pSession;
    if (SSI_Status status = getTempSession(pSession)) {
        return status;
    }

    shared_ptr<Volume> pVolume = getItem(pSession, volumeHandle);
    if (!pVolume) {
        return SSI_StatusInvalidHandle;
    }

    return pVolume->markAsNormal();
}

/* */
SSI_Status SsiVolumeRebuild(SSI_Handle volumeHandle, SSI_Handle diskHandle)
{
    shared_ptr<Session> pSession;
    if (SSI_Status status = getTempSession(pSession)) {
        return status;
    }

    shared_ptr<Volume> pVolume = getItem(pSession, volumeHandle);
    if (!pVolume) {
        return SSI_StatusInvalidHandle;
    }

    shared_ptr<EndDevice> pEndDevice = pSession->getEndDevice(diskHandle);
    if (!pEndDevice) {
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
    shared_ptr<Session> pSession;
    if (SSI_Status status = getTempSession(pSession)) {
        return status;
    }

    shared_ptr<Volume> pVolume = pSession->getVolume(volumeHandle);
    if (!pVolume) {
        return SSI_StatusInvalidHandle;
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

    shared_ptr<Session> pSession;
    if (SSI_Status status = getTempSession(pSession)) {
        return status;
    }

    shared_ptr<Volume> pVolume;
    shared_ptr<Array> pArray;
    Container<EndDevice> container;
    shared_ptr<EndDevice> pEndDevice;

    /* create container */
    try {
        for (unsigned int i = 0; i < params.numDisks; ++i) {
            pEndDevice = pSession->getEndDevice(params.disks[i]);
            if (!pEndDevice) {
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

        pEndDevice = pSession->getEndDevice(params.sourceDisk);

        if (pEndDevice) {
            return SSI_StatusNotSupported;
        }

        try {
            pArray = shared_ptr<Array>(new Array());
        } catch (...) {
            return SSI_StatusInsufficientResources;
        }

        pArray->setEndDevices(container);
        pArray->create();
    } catch (Exception ex) {
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
            pVolume = shared_ptr<Volume>(new Volume());
        } catch (...) {
            pArray->remove();
            return SSI_StatusInsufficientResources;
        }

        pVolume->discover();
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
        pSession->addVolume(pVolume);
        pSession->addArray(pArray);
        *volumeHandle = pVolume->getHandle();
    } catch (Exception ex) {
        pArray->remove();
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
    shared_ptr<Session> pSession;
    if (SSI_Status status = getTempSession(pSession)) {
        return status;
    }

    shared_ptr<Array> pArray = pSession->getArray(params.arrayHandle);
    if (!pArray) {
        return SSI_StatusInvalidHandle;
    }

    Container<EndDevice> container;
    pArray->getEndDevices(container, false);
    if (container.size() == 0) {
        pArray->getEndDevices(container, true);
    }

    try {
        shared_ptr<Volume> pVolume;
        try {
            pVolume = shared_ptr<Volume>(new Volume());
        } catch (...) {
            return SSI_StatusInsufficientResources;
        }

        pVolume->discover();
        pVolume->setParent(pArray);
        pVolume->setEndDevices(container);
        pVolume->setComponentSize(params.sizeInBytes, container.size(), params.raidLevel);
        pVolume->setName(params.volumeName);
        pVolume->setStripSize(params.stripSize);
        pVolume->setRaidLevel(params.raidLevel);
        pVolume->setRwhPolicy(params.rwhPolicy);
        pVolume->create();
        pVolume->update();
        pSession->addVolume(pVolume);

        return SSI_StatusOk;
    } catch (Exception ex) {
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
SSI_Status SsiVolumeRename(SSI_Handle volumeHandle, const SSI_Char volumeName[SSI_VOLUME_NAME_LENGTH])
{
    shared_ptr<Session> pSession;
    if (SSI_Status status = getTempSession(pSession)) {
        return status;
    }

    shared_ptr<Volume> pVolume = getItem(pSession, volumeHandle);
    if (!pVolume) {
        return SSI_StatusInvalidHandle;
    }

    String name = volumeName;

    Container<Volume> volumes;
    pSession->getVolumes(volumes);
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
    shared_ptr<Session> pSession;
    if (SSI_Status status = getTempSession(pSession)) {
        return status;
    }

    shared_ptr<Volume> pVolume = getItem(pSession, volumeHandle);
    if (!pVolume) {
        return SSI_StatusInvalidHandle;
    }

    return pVolume->expand(newSizeMB * 1024ULL);
}

/* */
SSI_Status SsiVolumeSetCachePolicy(SSI_Handle volumeHandle, SSI_VolumeCachePolicy policy)
{
    shared_ptr<Session> pSession;
    if (SSI_Status status = getTempSession(pSession)) {
        return status;
    }

    shared_ptr<Volume> pVolume = getItem(pSession, volumeHandle);
    if (!pVolume) {
        return SSI_StatusInvalidHandle;
    }

    return pVolume->setCachePolicy(policy == SSI_VolumeCachePolicyOff);
}

/* */
SSI_Status SsiVolumeSetRwhPolicy(SSI_Handle volumeHandle, SSI_Handle, SSI_RwhPolicy policy)
{
    shared_ptr<Session> pSession;
    if (SSI_Status status = getTempSession(pSession)) {
        return status;
    }

    shared_ptr<Volume> pVolume = getItem(pSession, volumeHandle);
    if (!pVolume) {
        return SSI_StatusInvalidHandle;
    }

    return pVolume->changeRwhPolicy(policy);
}

/* */
SSI_Status SsiVolumeInitialize(SSI_Handle volumeHandle)
{
    shared_ptr<Session> pSession;
    if (SSI_Status status = getTempSession(pSession)) {
        return status;
    }

    shared_ptr<Volume> pVolume = getItem(pSession, volumeHandle);
    if (!pVolume) {
        return SSI_StatusInvalidHandle;
    }

    return pVolume->initialize();
}

/* */
SSI_Status SsiVolumeVerify(SSI_Handle volumeHandle, SSI_Bool repair)
{
    shared_ptr<Session> pSession;
    if (SSI_Status status = getTempSession(pSession)) {
        return status;
    }

    shared_ptr<Volume> pVolume = getItem(pSession, volumeHandle);
    if (!pVolume) {
        return SSI_StatusInvalidHandle;
    }

    if (pVolume->getState() != SSI_VolumeStateNormal) {
        return SSI_StatusInvalidState;
    }

    return pVolume->verify(repair == SSI_TRUE);
}

/* */
SSI_Status SsiVolumeCancelVerify(SSI_Handle volumeHandle)
{
    shared_ptr<Session> pSession;
    if (SSI_Status status = getTempSession(pSession)) {
        return status;
    }

    shared_ptr<Volume> pVolume = getItem(pSession, volumeHandle);
    if (!pVolume) {
        return SSI_StatusInvalidHandle;
    }

    if (pVolume->getState() != SSI_VolumeStateVerifying && pVolume->getState() != SSI_VolumeStateVerifyingAndFix) {
        return SSI_StatusInvalidState;
    }

    return pVolume->cancelVerify();
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
