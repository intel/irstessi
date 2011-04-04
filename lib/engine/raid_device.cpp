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

#include <sys/stat.h>
#include <sys/types.h>
#include <cerrno>
#include <unistd.h>

#include <ssi.h>

#include "exception.h"
#include "list.h"
#include "container.h"
#include "string.h"
#include "filesystem.h"
#include "object.h"
#include "storage_device.h"
#include "utils.h"
#include "end_device.h"
#include "block_device.h"
#include "session.h"
#include "raid_device.h"

/* */
RaidDevice::RaidDevice(const String &path)
    : StorageDevice(path)
{
    m_DevName = m_Path.reverse_after("/");

    Directory dir(m_Path + "/slaves");
    for (Iterator<Directory *> i = dir; *i != 0; ++i) {
        m_Components.add(new String((*i)->reverse_after("/")));
    }

    String buffer;
    if (shell_cap("mdadm --detail --export /dev/" + m_DevName, buffer) == 0) {
        m_Uuid = buffer.between("MD_UUID=", "\n");
        m_Name = buffer.between("MD_DEVNAME=", "\n");
    }
}

/* */
RaidDevice::RaidDevice() : StorageDevice()
{
}

/* */
RaidDevice::~RaidDevice()
{
    for (Iterator<String *> i = m_Components; *i != 0; ++i) {
        delete *i;
    }
}

/* */
void RaidDevice::update()
{
    CanonicalPath path = "/dev/md/" + m_Name;
    if (path == "") {
        throw E_ARRAY_CREATE_FAILED;
    }
    m_DevName = path.reverse_after("/");
    m_Path = CanonicalPath("/sys/block/" + m_DevName);

    String buffer;
    if (shell_cap("mdadm --detail --export /dev/" + m_DevName, buffer) == 0) {
        m_Uuid = buffer.between("MD_UUID=", "\n");
    }
}

/* */
bool RaidDevice::equal(const Object *pObject) const
{
    return Object::equal(pObject) &&
        dynamic_cast<const RaidDevice *>(pObject)->m_Uuid == m_Uuid;
}

/* */
void RaidDevice::acquireId(Session *pSession)
{
    if (pSession == 0) {
        throw E_NULL_POINTER;
    }
    Container<EndDevice> endDevices = pSession->getEndDevices();
    for (Iterator<String *> i = m_Components; *i != 0; ++i) {
        attachComponent(endDevices, *(*i));
    }
}

/* */
void RaidDevice::setEndDevices(const Container<EndDevice> &container)
{
    m_BlockDevices.clear();
    for (Iterator<EndDevice *> i = container; *i != 0; ++i) {
        BlockDevice *pBlockDevice = dynamic_cast<BlockDevice *>(*i);
        if (pBlockDevice == 0) {
            throw E_INVALID_OBJECT;
        }
        if (pBlockDevice->isSystemDisk()) {
            throw E_SYSTEM_DEVICE;
        }
#if 0 /* APW */
        if (pBlockDevice->getDiskUsage() != SSI_DiskUsagePassThru) {
            throw E_INVALID_USAGE;
        }
#endif /* APW */
        if (pBlockDevice->getDiskState() != SSI_DiskStateNormal) {
            throw E_NOT_AVAILABLE;
        }
        m_BlockDevices.add(pBlockDevice);
    }
}

/* */
void RaidDevice::setName(const String &deviceName)
{
    if (deviceName.length() > 16) {
        throw E_INVALID_NAME;
    }
    m_Name = deviceName;
}

/* */
void RaidDevice::attachComponent(const Container<EndDevice> &endDevices, const String &devName)
{
    Iterator<EndDevice *> i;
    for (i = endDevices; *i != 0; ++i) {
        BlockDevice *pBlockDevice = dynamic_cast<BlockDevice *>(*i);
        if (pBlockDevice == 0) {
            continue;
        }
        if (pBlockDevice->getDevName() == devName) {
            break;
        }
    }
    if (*i == 0) {
        throw E_INVALID_OBJECT;
    }
    attachEndDevice(*i);
}

/* */
void RaidDevice::determineDeviceName(const String &prefix)
{
    struct stat st;
    unsigned int id = 0;
    while (true) {
        m_Name = prefix + String(id++);
        if (stat("/dev/md/" + m_Name, &st) < 0 && errno == ENOENT) {
            break;
        }
    }
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
