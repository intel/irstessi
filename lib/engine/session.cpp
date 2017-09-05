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

#include <dirent.h>
#include <sys/types.h>
#include <cstdlib>
#include <set>
#include <log/log.h>

#include "session.h"
#include "filesystem.h"
#include "ahci.h"
#include "isci.h"
#include "vmd.h"
#include "raid_info.h"
#include "context_manager.h"
#include "end_device.h"
#include "enclosure.h"
#include "routing_device.h"
#include "phy.h"
#include "port.h"
#include "volume.h"
#include "array.h"
#include "mdadm_config.h"

using boost::shared_ptr;

/* */
Session::Session()
{

}

/* */
Session::~Session()
{
    foreach (i, m_Controllers) {
        pContextMgr->remove(*i);
    }

    foreach (i, m_Arrays) {
        pContextMgr->remove(*i);
    }

    foreach (i, m_Enclosures) {
        pContextMgr->remove(*i);
    }

    foreach (i, m_RaidInfo) {
        pContextMgr->remove(*i);
    }

    foreach (i, m_Phys) {
        pContextMgr->remove(*i);
    }

    foreach (i, m_Ports) {
        pContextMgr->remove(*i);
    }

    foreach (i, m_Volumes) {
        pContextMgr->remove(*i);
    }

    foreach (i, m_EndDevices) {
        pContextMgr->remove(*i);
    }

    foreach (i, m_RoutingDevices) {
        pContextMgr->remove(*i);
    }
}

/* */
void Session::initialize()
{
    Directory dir;
    std::list<Directory *> dirs;
    dlog("Open session");
    dir = "/sys/bus/pci/drivers/ahci";
    dirs = dir.dirs();
    foreach (i, dirs) {
        CanonicalPath path = *(*i) + "driver";
        if (path == dir) {
            File attr;
            String vendor;
            attr = *(*i) + "vendor";
            try {
                attr >> vendor;
                if (vendor != "0x8086") {
                    continue;
                }
            } catch (...) {
                /* TODO log that vendor cannot be read from filesystem */
                continue;
            }
            shared_ptr<AHCI> pAHCI = shared_ptr<AHCI>(new AHCI(CanonicalPath(*(*i))));
            pAHCI->discover();
            pAHCI->addToSession(shared_from_this());
        }
    }
    dir = "/sys/bus/pci/drivers/isci";
    dirs = dir.dirs();
    foreach (i, dirs) {
        CanonicalPath path = *(*i) + "driver";
        if (path == dir) {
            shared_ptr<ISCI> pISCI = shared_ptr<ISCI>(new ISCI(CanonicalPath(*(*i))));
            pISCI->discover();
            pISCI->addToSession(shared_from_this());
        }
    }

    dir = "/sys/bus/pci/drivers/vmd";
    dirs = dir.dirs();
    // all VMD controllers are presented as 1 controller
    shared_ptr<VMD> pVMD;
    foreach (i, dirs) {
        CanonicalPath path = *(*i) + "driver";
        if (path == dir) {
            if (!pVMD) {
                pVMD = shared_ptr<VMD>(new VMD(CanonicalPath(*(*i))));
            }
            pVMD->discover(*(*i));
        }
    }

    if (pVMD) {
        pVMD->addToSession(shared_from_this());
    }

    foreach (i, m_Controllers) {
        shared_ptr<RaidInfo> pRaidInfo = (*i)->findRaidInfo(m_RaidInfo);
        if (pRaidInfo) {
            pRaidInfo->addToSession(shared_from_this());
        }
    }

    if (m_EndDevices.size() > 0) {
        dir = "/sys/devices/virtual/block";
        dirs = dir.dirs();
        foreach (i, dirs) {
            __internal_attach_imsm_device(CanonicalPath(*(*i)));
        }
    }

    check_configuration();
}

/* */
bool Session::operator ==(const Object &object) const
{
    return &object == this;
}

/* */
shared_ptr<ScopeObject> Session::getObject(SSI_Handle handle)
{
    if (handle == SSI_NULL_HANDLE) {
        return shared_from_this();
    }

    shared_ptr<ScopeObject> pObject;
    try {
        pObject = getEndDevice(handle);
        if (pObject) {
            return pObject;
        }

        pObject = getArray(handle);
        if (pObject) {
            return pObject;
        }

        pObject = getEnclosure(handle);
        if (pObject) {
            return pObject;
        }

        pObject = getPhy(handle);
        if (pObject) {
            return pObject;
        }

        pObject = getVolume(handle);
        if (pObject) {
            return pObject;
        }

        pObject = getPort(handle);
        if (pObject) {
            return pObject;
        }

        pObject = getRoutingDevice(handle);
        if (pObject) {
            return pObject;
        }

        pObject = getRaidInfo(handle);
        if (pObject) {
            return pObject;
        }

        pObject = getController(handle);
        if (pObject) {
            return pObject;
        }
    } catch (...) {
        pObject = shared_ptr<ScopeObject>();
    }

    return pObject;
}

/* */
shared_ptr<RaidInfo> Session::getRaidInfo(SSI_Handle handle) const
{
    return m_RaidInfo.find(handle);
}

/* */
shared_ptr<Array> Session::getArray(SSI_Handle handle) const
{
    return m_Arrays.find(handle);
}

/* */
shared_ptr<Controller> Session::getController(SSI_Handle handle) const
{
    return m_Controllers.find(handle);
}

/* */
shared_ptr<StorageDevice> Session::getDevice(SSI_Handle handle) const
{
    shared_ptr<StorageDevice> pResult;

    pResult = m_Arrays.find(handle);
    if (pResult) {
        return pResult;
    }

    pResult = m_EndDevices.find(handle);
    if (pResult) {
        return pResult;
    }

    pResult = m_Volumes.find(handle);

    return pResult;
}

/* */
shared_ptr<Phy> Session::getPhy(SSI_Handle handle) const
{
    return m_Phys.find(handle);
}

/* */
shared_ptr<Port> Session::getPort(SSI_Handle handle) const
{
    return m_Ports.find(handle);
}

/* */
shared_ptr<RoutingDevice> Session::getRoutingDevice(SSI_Handle handle) const
{
    return m_RoutingDevices.find(handle);
}

/* */
shared_ptr<Volume> Session::getVolume(SSI_Handle handle) const
{
    return m_Volumes.find(handle);
}

/* */
shared_ptr<Enclosure> Session::getEnclosure(SSI_Handle handle) const
{
    return m_Enclosures.find(handle);
}

/* */
shared_ptr<EndDevice> Session::getEndDevice(SSI_Handle handle) const
{
    return m_EndDevices.find(handle);
}

/* */
void Session::addEndDevice(const shared_ptr<EndDevice>& pEndDevice)
{
    if (!pEndDevice) {
        throw E_NULL_POINTER;
    }

    pContextMgr->add(pEndDevice);
    m_EndDevices.add(pEndDevice);
}

/* */
void Session::addArray(const shared_ptr<Array>& pArray)
{
    if (!pArray) {
        throw E_NULL_POINTER;
    }

    pContextMgr->add(pArray);
    m_Arrays.add(pArray);
}

/* */
void Session::addRoutingDevice(const shared_ptr<RoutingDevice>& pRoutingDevice)
{
    if (!pRoutingDevice) {
        throw E_NULL_POINTER;
    }

    pContextMgr->add(pRoutingDevice);
    m_RoutingDevices.add(pRoutingDevice);
}

/* */
void Session::addEnclosure(const shared_ptr<Enclosure>& pEnclosure)
{
    if (!pEnclosure) {
        throw E_NULL_POINTER;
    }

    pContextMgr->add(pEnclosure);
    m_Enclosures.add(pEnclosure);
}

/* */
void Session::addPhy(const shared_ptr<Phy>& pPhy)
{
    if (!pPhy) {
        throw E_NULL_POINTER;
    }

    pContextMgr->add(pPhy);
    m_Phys.add(pPhy);
}

/* */
void Session::addVolume(const shared_ptr<Volume>& pVolume)
{
    if (!pVolume) {
        throw E_NULL_POINTER;
    }

    pContextMgr->add(pVolume);
    m_Volumes.add(pVolume);
}

/* */
void Session::addPort(const shared_ptr<Port>& pPort)
{
    if (!pPort) {
        throw E_NULL_POINTER;
    }

    pContextMgr->add(pPort);
    m_Ports.add(pPort);
}

/* */
void Session::addController(const shared_ptr<Controller>& pController)
{
    if (!pController) {
        throw E_NULL_POINTER;
    }

    pContextMgr->add(pController);
    m_Controllers.add(pController);
}

/* */
void Session::addRaidInfo(const shared_ptr<RaidInfo>& pRaidInfo)
{
    if (!pRaidInfo) {
        throw E_NULL_POINTER;
    }

    pContextMgr->add(pRaidInfo);
    m_RaidInfo.add(pRaidInfo);
}

/* */
void Session::__internal_attach_imsm_device(const String &path)
{
    CanonicalPath temp = path + "/md/metadata_version";
    if (temp) {
        String metadata;
        try {
            File attr = temp;
            attr >> metadata;
        } catch (...) {
        }
        if (metadata == "external:imsm") {
            __internal_attach_imsm_array(path);
        }
    }
}

/* */
void Session::__internal_attach_imsm_array(const String &path)
{
    try {
        shared_ptr<Array> pArray = shared_ptr<Array>(new Array(path));
        if (pArray) {
            pArray->discover();
            pArray->addToSession(shared_from_this());
        }
    } catch (Exception) {
        /* Invalid object */
    } catch (...) {
        /* TODO: log that there's not enough resources in the system. */
    }
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
