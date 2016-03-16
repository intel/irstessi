
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

#include "templates.h"

namespace {
    void getItems(ScopeObject *pScopeObject, SSI_ScopeType scopeType, Container<Volume> &container)
    {
        pScopeObject->getVolumes(container);
    }

    Volume * getItem(Session *pSession, SSI_Handle handle)
    {
        return pSession->getVolume(handle);
    }

    bool isVolumeNameUnique(const String& name, const Session* session)
    {
        Container<Volume> volumes;
        session->getVolumes(volumes);

        foreach (iter, volumes) {
            Volume& volume = *(*iter);

            if (volume.getName() == name) {
                return false;
            } else {
                /* temporary mdadm workaround for rename */
                String copy = name + "_0";
                if (volume.getName() == copy) {
                    return false;
                }
            }
        }

        return true;
    }

    void verifyVolumeName(const String& name, const Session* session)
    {
        if (name.isEmpty() || name.length() > (SSI_VOLUME_NAME_LENGTH - 1) || name[0] == ' ') {
            throw E_INVALID_NAME;
        }

        for (unsigned int index = 0; index < name.size(); index++) {
            char character = name[index];
            if (character < 32 || character > 126 || character == '\\') {
                throw E_INVALID_NAME;
            }
        }

        if (!isVolumeNameUnique(name, session)) {
            throw E_INVALID_NAME;
        }
    }

    /** Function makes SSI wait until specific volume state changes to normal
     *
     * If volume state changes to any of invalid states, exception is thrown
     */
    void waitUntilVolumeIsNormal(Volume **pVolume)
    {
        const unsigned int ONE_SECOND = 1000000;
        const unsigned int WAIT_TIME = 5 * ONE_SECOND;

        SSI_Handle volumeId = (*pVolume)->getId();
        bool isNotNormal = true;
        while (isNotNormal) {
            TemporarySession session;
            if (!session.isValid()) {
                throw E_NOT_AVAILABLE;
            }

            *pVolume = session->getVolume(volumeId);
            switch ((*pVolume)->getState()) {
            case SSI_VolumeStateDegraded: /* Degraded state is still a valid volume state, especially during creation of RAID 1 with migration
                                             (degraded volume must be created in the process) */
            case SSI_VolumeStateNormal:
                isNotNormal = false;
                break;

            case SSI_VolumeStateUnknown:
            case SSI_VolumeStateFailed:
            case SSI_VolumeStateLocked:
            case SSI_VolumeStateNonRedundantVolumeFailedDisk:
                throw E_VOLUME_CREATE_FAILED;

            default:
                /* keep waiting */;
            }

            usleep(WAIT_TIME);
        }
    }
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
    const unsigned long long UNUSED_PARAMETER = 0;

    if (volumeHandle == NULL) {
        return SSI_StatusInvalidParameter;
    }

    TemporarySession session;
    if (!session.isValid()) {
        return SSI_StatusNotInitialized;
    }

    Volume *pVolume = NULL;
    Array *pArray = NULL;
    Container<EndDevice> container;
    Container<EndDevice> source;
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

        pEndDevice = session->getEndDevice(params.sourceDisk);

        try {
            pArray = new Array();
        } catch (...) {
            return SSI_StatusInsufficientResources;
        }

        if (pEndDevice != NULL) {
            if (container.find(pEndDevice->getId()) == NULL) {
                throw E_VOLUME_CREATE_FAILED;
            }

            if (params.raidLevel == SSI_Raid1) {
                if (params.sizeInBytes != 0) {
                    throw E_INVALID_SIZE;
                }

                foreach (iter, container) {
                    if ((*iter)->getId() != pEndDevice->getId()) {
                        if ((*iter)->getTotalSize() > pEndDevice->getTotalSize()) {
                            throw E_BUFFER_TOO_SMALL;
                        }

                        source.add(*iter);
                    }
                }
            } else {
                source.add(pEndDevice);
            }

            pArray->setEndDevices(source);
        } else {
            pArray->setEndDevices(container);
        }
        pArray->create();
    } catch (Exception ex) {
        delete pArray;
        switch (ex) {
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
        pVolume->setComponentSize(params.sizeInBytes, container.size(), params.raidLevel);

        /* At this point, we are sure that we've got enough disks inside container */

        verifyVolumeName(params.volumeName, session.get());
        pVolume->setName(params.volumeName);
        pVolume->setRaidLevel(params.raidLevel);
        if (params.raidLevel != SSI_Raid1) {
            pVolume->setStripSize(params.stripSize);
        }
        pVolume->create();
        session->addVolume(pVolume);
        session->addArray(pArray);
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
        default:
            return SSI_StatusFailed;
        }
    }

    try {
        if (pVolume->hasSourceDisk()) {
            waitUntilVolumeIsNormal(&pVolume);

            if (container.find(pEndDevice->getId()) != NULL) {
                container.remove(pEndDevice->getId());
            }

            switch (params.raidLevel) {
            case SSI_Raid0:
                if (pVolume->modify(pVolume->getSsiStripSize(), SSI_Raid0, UNUSED_PARAMETER, container) != SSI_StatusOk) {
                    throw E_VOLUME_CREATE_FAILED;
                }
                break;

            case SSI_Raid5:
                /* While migration to RAID_5 is not working correctly, this solution will not work! */
                if (pVolume->modify(pVolume->getSsiStripSize(), SSI_Raid5, UNUSED_PARAMETER, container) != SSI_StatusOk) {
                    throw E_VOLUME_CREATE_FAILED;
                }
                break;

            case SSI_Raid10:
                {
                    /* Resizing to 2-disk RAID_0 to resize it further */
                    Container<EndDevice> secondDisk;
                    EndDevice* second = *container.begin();
                    secondDisk.add(second);
                    container.remove(second->getId());

                    if (pVolume->modify(pVolume->getSsiStripSize(), SSI_Raid0, UNUSED_PARAMETER, secondDisk) != SSI_StatusOk) {
                        throw E_VOLUME_CREATE_FAILED;
                    }

                    waitUntilVolumeIsNormal(&pVolume);

                    if (pVolume->modify(pVolume->getSsiStripSize(), params.raidLevel, UNUSED_PARAMETER, container) != SSI_StatusOk) {
                        throw E_VOLUME_CREATE_FAILED;
                    }
                }
                break;

            case SSI_Raid1:
                if (shell("mdadm '/dev/" + pArray->getDevName() + "' --add '/dev/" + pEndDevice->getDevName() + "'") != 0) {
                    throw E_VOLUME_CREATE_FAILED;
                }
                break;

            default:
                throw E_INVALID_RAID_LEVEL;
            }
        }

        *volumeHandle = pVolume->getId();
    } catch (Exception ex) {
        pVolume->remove();
        delete pVolume;
        pArray->remove();
        delete pArray;
        switch (ex) {
        case E_NOT_AVAILABLE:
            return SSI_StatusNotInitialized;
        case E_INVALID_STRIP_SIZE:
            return SSI_StatusInvalidStripSize;
        case E_INVALID_HANDLE:
            return SSI_StatusInvalidHandle;
        case E_INVALID_NAME:
            return SSI_StatusInvalidString;
        case E_INVALID_RAID_LEVEL:
            return SSI_StatusInvalidRaidLevel;
        case E_BUFFER_TOO_SMALL:
            return SSI_StatusBufferTooSmall;
        case E_BUFFER_TOO_LARGE:
            return SSI_StatusBufferTooLarge;
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
        verifyVolumeName(params.volumeName, session.get());
        pVolume->setName(params.volumeName);
        pVolume->setStripSize(params.stripSize);
        pVolume->setRaidLevel(params.raidLevel);
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
    TemporarySession session;
    if (!session.isValid()) {
        return SSI_StatusNotInitialized;
    }

    Volume *pVolume = NULL;
    if (SSI_Status status = SsiGetItem(volumeHandle, &pVolume, getItem)) {
        return status;
    }

    try {
        verifyVolumeName(volumeName, session.get());
    } catch (Exception ex) {
        switch (ex) {
        case E_INVALID_NAME:
            return SSI_StatusInvalidString;

        default:
            return SSI_StatusFailed;
        }
    }

    return pVolume->rename(volumeName);
}

/* */
SSI_Status SsiExpandVolume(SSI_Handle volumeHandle, SSI_Uint64 newSizeMB)
{
    Volume *pVolume = NULL;
    if (SSI_Status status = SsiGetItem(volumeHandle, &pVolume, getItem)) {
        return status;
    }

    return pVolume->expand(newSizeMB*1024ULL);
}

/* */
SSI_Status SsiVolumeSetCachePolicy(SSI_Handle volumeHandle,
    SSI_VolumeCachePolicy policy)
{
    Volume *pVolume = NULL;
    if (SSI_Status status = SsiGetItem(volumeHandle, &pVolume, getItem)) {
        return status;
    }

    return pVolume->setCachePolicy(policy == SSI_VolumeCachePolicyOff);
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
