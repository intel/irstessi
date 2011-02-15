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

#include "exception.h"
#include "list.h"
#include "container.h"
#include "string.h"
#include "object.h"
#include "storage_device.h"
#include "raid_device.h"
#include "array.h"
#include "volume.h"
#include "end_device.h"
#include "session.h"

/* */
Volume::Volume(StorageObject *pParent, const String &path, Session *pSession)
    : RaidDevice(pParent, path)
{
    if (pSession == 0) {
        throw E_NULL_POINTER;
    }
    pSession->addVolume(this);
}

/* */
Volume::~Volume()
{
}

/* */
SSI_Status Volume::initialize()
{
    return SSI_StatusOk;
}

/* */
SSI_Status Volume::rebuild(EndDevice *pEndDevice)
{
    (void)pEndDevice;
    return SSI_StatusOk;
}

/* */
SSI_Status Volume::expand(unsigned long long newSize)
{
    (void)newSize;
    return SSI_StatusOk;
}

/* */
SSI_Status Volume::rename(const String &newName)
{
    (void)newName;
    return SSI_StatusOk;
}

/* */
SSI_Status Volume::remove()
{
    return SSI_StatusOk;
}

/* */
SSI_Status Volume::markAsNormal()
{
    return SSI_StatusOk;
}

/* */
SSI_Status Volume::verify(bool repair)
{
    (void)repair;
    return SSI_StatusOk;
}

/* */
SSI_Status Volume::cancelVerify()
{
    return SSI_StatusOk;
}

/* */
SSI_Status Volume::modify(SSI_StripSize chunk, SSI_RaidLevel raidLevel,
    unsigned long long newSize, const Container &disks)
{
    (void)chunk;
    (void)raidLevel;
    (void)disks;
    (void)newSize;
    return SSI_StatusOk;
}

/* */
SSI_Status Volume::getInfo(SSI_VolumeInfo *pInfo)
{
    (void)pInfo;
    return SSI_StatusOk;
}

/* */
SSI_Status Volume::setCachePolicy(bool cacheOff)
{
    (void)cacheOff;
    return SSI_StatusOk;
}

/* */
SSI_Status Volume::writeStorageArea(void *pBuffer, unsigned int bufferSize)
{
    (void)bufferSize;
    (void)pBuffer;
    return SSI_StatusOk;
}

/* */
SSI_Status Volume::readStorageArea(void *pBuffer, unsigned int bufferSize)
{
    (void)bufferSize;
    (void)pBuffer;
    return SSI_StatusOk;
}

/* */
void Volume::attachEndDevice(Object *pEndDevice, bool direct)
{
    (void)direct;
    m_EndDevices.add(pEndDevice);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
