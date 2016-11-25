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

#include <cstddef>
#include <log/log.h>

#include "context_manager.h"
#include "mdadm_config.h"

using boost::shared_ptr;

/* */
ContextManager::ContextManager()
{
}

/* */
ContextManager::~ContextManager()
{
}

/* */
SSI_Status ContextManager::getSystemInfo(SSI_SystemInfo *pInfo) const
{
    if (pInfo == NULL) {
        return SSI_StatusInvalidParameter;
    }

    pInfo->interfaceVersionMajor = 1;
    pInfo->interfaceVersionMinor = 0;
    String ver = String(SSI_LIBRARY_VERSION);
    ver.get(pInfo->libraryVersion, sizeof(pInfo->libraryVersion));
    size_t offset = ver.length() + 1;
    if (offset < sizeof(pInfo->libraryVersion)) {
        get_mdadm_version(pInfo->libraryVersion + offset, sizeof(pInfo->libraryVersion) - offset);
    }
    pInfo->maxSessions = -1U;
    pInfo->setVolCacheSizeSupport = SSI_FALSE;
    pInfo->passthroughCommandSupport = SSI_FALSE;

    return SSI_StatusOk;
}

/* */
shared_ptr<Session> ContextManager::getSession(SSI_Handle handle) const
{
    return m_SessionMgr.getSession(handle);
}

/* */
SSI_Handle ContextManager::openSession()
{
    return m_SessionMgr.openSession();
}

SSI_Status ContextManager::closeSession(SSI_Handle handle)
{
    return m_SessionMgr.closeSession(handle);
}

/* */
shared_ptr<Event> ContextManager::getEvent(SSI_Handle handle) const
{
    return m_EventMgr.getEvent(handle);
}

/* */
SSI_Handle ContextManager::registerEvent()
{
    return m_EventMgr.registerEvent();
}

/* */
SSI_Status ContextManager::unregisterEvent(SSI_Handle handle)
{
    return m_EventMgr.unregisterEvent(handle);
}

/* */
bool ContextManager::add(const object_ptr& pObject) {
    return m_HandleMgr.insert(pObject).second;
}

ContextManager::object_ptr ContextManager::remove(const object_ptr &pObject) {
    return remove(pObject.get());
}

/* */
ContextManager::object_ptr ContextManager::remove(Object *pObject) {
    return m_HandleMgr.remove(pObject);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
