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

#include <sys/stat.h>
#include <sys/types.h>
#include <cerrno>
#include <unistd.h>
#include <typeinfo>
#include <limits>
#include <log/log.h>

#include "raid_device.h"
#include "session.h"

using std::numeric_limits;
using boost::shared_ptr;
using boost::dynamic_pointer_cast;

/* */
RaidDevice::RaidDevice(const String &path)
    : StorageDevice(path)
{

}

/* */
RaidDevice::RaidDevice() : StorageDevice()
{
}

/* */
void RaidDevice::discover()
{
    m_DevName = m_Path.reverse_after("/");

    Directory dir(m_Path + "/slaves");
    std::list<Directory *> dirs = dir.dirs();
    foreach (i, dirs) {
        m_Components.push_back(shared_ptr<String>(new String((*i)->reverse_after("/"))));
    }
    update();
}

/* */
shared_ptr<RaidInfo> RaidDevice::getRaidInfo() const
{
    shared_ptr<RaidInfo> pinfo;
    foreach (i, m_BlockDevices) {
        if (shared_ptr<BlockDevice> block = (*i).lock()) {
            pinfo = block->getRaidInfo();
            if (pinfo) {
                break;
            }
        }
    }

    return pinfo;
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
        if (map.exists()) {
            return map;
        }
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
        if (create) {
            tmp = line.reverse_after("/");
        } else {
            tmp = line.left(" ");
        }

        try {
            tmp.find(name);
            break;
        } catch (...) {
        }
        map = map.after("\n");
        line = map.left("\n");
    }

    if (line == "") {
        throw E_ARRAY_CREATE_FAILED;
    }

    if (create) {
        m_DevName = line.left(" ");
    } else {
        m_Name = line.reverse_after("/");
    }
    line = line.reverse_left(" ");
    m_Uuid = line.reverse_after(" ");
    dlog(m_Name + " found in map: " + m_DevName + " " + m_Uuid);
}

/* */
bool RaidDevice::operator ==(const Object &object) const
{
    return typeid(*this) == typeid(object) &&
        static_cast<const RaidDevice *>(&object)->m_Uuid == m_Uuid;
}

/* */
void RaidDevice::addToSession(const shared_ptr<Session>& pSession)
{
    if (!pSession) {
        throw E_NULL_POINTER;
    }

    Container<EndDevice> endDevices;
    pSession->getEndDevices(endDevices, false);
    foreach (i, m_Components) {
        attachComponent(endDevices, *(*i));
    }
}

/* */
void RaidDevice::setEndDevices(const Container<EndDevice> &container)
{
    m_BlockDevices.clear();
    foreach (i, container) {
        shared_ptr<BlockDevice> pBlockDevice = dynamic_pointer_cast<BlockDevice>(*i);
        if (!pBlockDevice) {
            throw E_INVALID_OBJECT;
        } else if (pBlockDevice->isSystemDisk()) {
            throw E_SYSTEM_DEVICE;
        }
#if 0 /* APW */
        else if (pBlockDevice->getDiskUsage() != SSI_DiskUsagePassThru) {
            throw E_INVALID_USAGE;
        }
#endif /* APW */
        else if (pBlockDevice->getDiskState() != SSI_DiskStateNormal) {
            throw E_NOT_AVAILABLE;
        }

        m_BlockDevices.add(pBlockDevice);
    }
}

/* */
void RaidDevice::setName(const String &deviceName)
{
    const unsigned int npos = numeric_limits<unsigned int>::max();

    if (deviceName.length() > 16) {
        throw E_INVALID_NAME;
    }

    unsigned int pos = npos;
    try {
        String trimmed = deviceName;
        trimmed.trim();
        pos = trimmed.find(" ");
    } catch (...) {
        /* Not found */
    }

    if (pos != npos) {
        throw E_INVALID_NAME;
    }

    m_Name = deviceName;
}

/* */
void RaidDevice::attachComponent(const Container<EndDevice> &endDevices, const String &devName)
{
    foreach (i, endDevices) {
        shared_ptr<BlockDevice> pBlockDevice = dynamic_pointer_cast<BlockDevice>(*i);
        if (!pBlockDevice) {
            continue;
        }

        if (pBlockDevice->getDevName() == devName) {
            attachEndDevice(*i);
            return;
        }
    }

    throw E_INVALID_OBJECT;
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
