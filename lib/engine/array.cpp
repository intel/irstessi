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
#include "cache.h"
#include "storage_device.h"
#include "raid_device.h"
#include "array.h"
#include "volume.h"
#include "end_device.h"
#include "session.h"

/* */
Array::Array(StorageObject *pParent, const String &path)
    : RaidDevice(pParent, path)
{
}

/* */
Array::~Array()
{
}

/* */
SSI_Status Array::addSpare(const Container &container)
{
    (void)container;
    return SSI_StatusOk;
}

/* */
SSI_Status Array::addSpare(const EndDevice *pEndDevice)
{
    (void)pEndDevice;
    return SSI_StatusOk;
}

/* */
SSI_Status Array::getInfo(SSI_ArrayInfo *pInfo) const
{
    if (pInfo == 0) {
        return SSI_StatusInvalidParameter;
    }
    pInfo->arrayHandle = getId();
    m_Name.get(pInfo->name, sizeof(pInfo->name));
    pInfo->state = getState();
    pInfo->freeSize = getFreeSize();
    pInfo->totalSize = getSize();
    if (isCachable()) {
        pInfo->writeCachePolicy = SSI_WriteCachePolicyOn;
    } else {
        pInfo->writeCachePolicy = SSI_WriteCachePolicyOff;
    }
    pInfo->numVolumes = m_Volumes.count();
    pInfo->numDisks = m_EndDevices.count();
    return SSI_StatusOk;
}

/* */
SSI_Status Array::setWriteCacheState(bool enable)
{
    (void)enable;
    return SSI_StatusOk;
}

/* */
SSI_Status Array::removeSpare(const EndDevice *pEndDevice)
{
    (void)pEndDevice;
    return SSI_StatusOk;
}

/* */
void Array::getEndDevices(Container &container, bool __attribute__((unused)) all) const
{
    container = m_EndDevices;
}

/* */
void Array::getVolumes(Container &container) const
{
    container = m_Volumes;
}

/* */
SSI_Status Array::readStorageArea(void *pBuffer, unsigned int bufferSize)
{
    (void)bufferSize;
    (void)pBuffer;
    return SSI_StatusOk;
}

/* */
SSI_Status Array::writeStorageArea(void *pBuffer, unsigned int bufferSize)
{
    (void)bufferSize;
    (void)pBuffer;
    return SSI_StatusOk;
}

/* */
SSI_ArrayState Array::getState(void) const
{
    return SSI_ArrayStateUnknown;
}

/* */
unsigned long long Array::getSize(void) const
{
    return 0ULL;
}

/* */
unsigned long long Array::getFreeSize(void) const
{
    return 0ULL;
}

/* */
bool Array::isCachable(void) const
{
    return false;
}

/* */
void Array::attachEndDevice(Object *pEndDevice, bool direct)
{
    (void)direct;
    m_EndDevices.add(pEndDevice);
}

/* */
void Array::attachVolume(Object *pVolume)
{
    m_Volumes.add(pVolume);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
