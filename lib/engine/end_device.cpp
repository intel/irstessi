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
#include "phy.h"
#include "port.h"
#include "remote_port.h"
#include "session.h"

/* */
EndDevice::EndDevice(const String &path)
    : StorageDevice(0, path)
{
    m_pPhy = new Phy(this, path, 0);
    m_pPort = new RemotePort(this, path);
    m_pPort->attachPhy(m_pPhy);
}

/* */
EndDevice::~EndDevice()
{
}

/* */
SSI_Status EndDevice::getInfo(SSI_EndDeviceInfo *pInfo) const
{
    if (pInfo == 0) {
        return SSI_StatusInvalidParameter;
    }
    pInfo->endDeviceHandle = getId();
    return SSI_StatusOk;
}

/* */
void EndDevice::getAddress(SSI_Address *pAddress) const
{
    if (pAddress) {
        /* TODO: this is temporary, the method will be moved to specialized classes. */
    }
}

/* */
void EndDevice::getPhys(Container &container) const
{
    container.clear();
    if (m_pPhy != 0) {
        container.add(reinterpret_cast<Object *>(m_pPhy));
    }
}

/* */
SSI_Status EndDevice::passthroughCmd(void *pInfo, void *pData, unsigned int dataSize,
    SSI_DataDirection dir)
{
    (void)pInfo;
    (void)pData;
    (void)dataSize;
    (void)dir;
    return SSI_StatusOk;
}

/* */
void EndDevice::acquireId(Session *pSession)
{
    pSession->addEndDevice(this);
    pSession->addPhy(m_pPhy);
    pSession->addPort(m_pPort);
}

/* */
bool EndDevice::equal(const Object *pObject) const
{
    return Object::equal(pObject) &&
        dynamic_cast<const EndDevice *>(pObject)->getSerialNum() == m_SerialNum;
}

/* */
void EndDevice::attachPhy(Object *pPhy)
{
    m_pPhy = dynamic_cast<Phy *>(pPhy);
}

/* */
void EndDevice::attachPort(Object *pPort)
{
    m_pPort = dynamic_cast<Port *>(pPort);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
