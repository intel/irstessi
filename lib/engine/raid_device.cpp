
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

#include <sys/stat.h>
#include <sys/types.h>
#include <cerrno>
#include <unistd.h>

#include <ssi.h>
#include <log/log.h>

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
#include "raid_info.h"

/* */
RaidDevice::RaidDevice(const String &path)
    : StorageDevice(path)
{
    m_DevName = m_Path.reverse_after("/");

    Directory dir(m_Path + "/slaves");
    std::list<Directory *> dirs = dir.dirs();
    for (std::list<Directory *>::const_iterator i = dirs.begin(); i != dirs.end(); ++i) {
        m_Components.push_back(new String((*i)->reverse_after("/")));
    }
    update();

}

/* */
RaidDevice::RaidDevice() : StorageDevice()
{
}

/* */
RaidDevice::~RaidDevice()
{
    for (std::list<String *>::const_iterator i = m_Components.begin(); i != m_Components.end(); ++i) {
        delete *i;
    }
}

/* */
RaidInfo * RaidDevice::getRaidInfo() const
{
    RaidInfo *pinfo;
    for (std::list<BlockDevice *>::const_iterator i = m_BlockDevices.begin(); i != m_BlockDevices.end(); ++i) {
        pinfo = (*i)->getRaidInfo();
        if ( pinfo != 0)
            return pinfo;
    }
    return 0;
}

File RaidDevice::getMapFile()
{
    static const char *paths[] = {
            "/run/mdadm/map",
            "/var/run/mdadm/map",
            "/dev/.mdadm/map"
    };

    int n = sizeof(paths) / sizeof(*paths);

    for (int i = 0; i < n; i++) {
        File map(paths[i]);
        if (map.exists())
            return map;
    }

    throw E_NOT_FOUND;
}

/* */
void RaidDevice::update()
{
    String map;
    try {
        File mapFile = getMapFile();
        mapFile >> map;
        __internal_update(map);
    } catch (...) {
        dlog("failed to read map file");
        return;
    }
    m_Path = CanonicalPath("/sys/block/" + m_DevName);
}

/* */
void RaidDevice::__internal_update(String &map)
{
    String line, name, tmp;
    int create = 0;
    map += "\n";
    line = map.left("\n");
    if (m_Name != "") {
        name = m_Name;
        create = 1;
    } else {
        name = m_DevName;
    }
    dlog(name + " looking up in map");
    while (line != "") {
        if (create)
            tmp = line.reverse_after("/");
        else
            tmp = line.left(" ");
        try {
            tmp.find(name);
            break;
        } catch (...) {
        }
        map = map.after("\n");
        line = map.left("\n");
    }
    if (line == "")
        throw E_ARRAY_CREATE_FAILED;
    if (create)
        m_DevName = line.left(" ");
    else
        m_Name = line.reverse_after("/");
    line = line.reverse_left(" ");
    m_Uuid = line.reverse_after(" ");
    dlog(m_Name + " found in map: " + m_DevName + " " + m_Uuid);
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
    for (std::list<String *>::const_iterator i = m_Components.begin(); i != m_Components.end(); ++i) {
        attachComponent(endDevices, *(*i));
    }
}

/* */
void RaidDevice::setEndDevices(const Container<EndDevice> &container)
{
    m_BlockDevices.clear();
    for (std::list<EndDevice *>::const_iterator i = container.begin(); i != container.end(); ++i) {
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
    std::list<EndDevice *>::const_iterator i;
    for (i = endDevices.begin(); i != endDevices.end(); ++i) {
        BlockDevice *pBlockDevice = dynamic_cast<BlockDevice *>(*i);
        if (pBlockDevice == 0) {
            continue;
        }
        if (pBlockDevice->getDevName() == devName) {
            break;
        }
    }
    if (i == endDevices.end()) {
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
