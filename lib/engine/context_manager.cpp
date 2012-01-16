
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
#include <cstddef>

#include <ssi.h>
#include <log/log.h>

#include "exception.h"
#include "list.h"
#include "container.h"
#include "string.h"
#include "object.h"
#include "cache.h"
#include "session.h"
#include "event.h"
#include "context_manager.h"
#include "event_manager.h"
#include "unique_id_manager.h"
#include "session_manager.h"
#include "mdadm_config.h"

/* */
ContextManager::ContextManager()
    : m_pSessionMgr(0), m_pEventMgr(0), m_pUniqueIdMgr(0)
{
    m_pUniqueIdMgr = new UniqueIdManager;
    m_pEventMgr = new EventManager;
    m_pSessionMgr = new SessionManager;
}

/* */
ContextManager::~ContextManager()
{
    delete m_pSessionMgr;
    delete m_pEventMgr;
    delete m_pUniqueIdMgr;
}

/* */
SSI_Status ContextManager::getSystemInfo(SSI_SystemInfo *pInfo) const
{
    if (pInfo == 0) {
        return SSI_StatusInvalidParameter;
    }
    pInfo->interfaceVersionMajor = 1;
    pInfo->interfaceVersionMinor = 0;
    String ver = String(SSI_LIBRARY_VERSION);
    ver.get(pInfo->libraryVersion, sizeof(pInfo->libraryVersion));
    size_t offset = ver.length() + 1;
    if (offset < sizeof(pInfo->libraryVersion))
        get_mdadm_version(pInfo->libraryVersion + offset, sizeof(pInfo->libraryVersion) - offset);
    pInfo->maxSessions = -1U;
    pInfo->setVolCacheSizeSupport = SSI_FALSE;
    pInfo->passthroughCommandSupport = SSI_FALSE;

    return SSI_StatusOk;
}

/* */
Session * ContextManager::getSession(unsigned int id) const
{
    return m_pSessionMgr->getSession(id);
}

/* */
unsigned int ContextManager::openSession()
{
    return m_pSessionMgr->openSession();
}

SSI_Status ContextManager::closeSession(unsigned int id)
{
    return m_pSessionMgr->closeSession(id);
}

/* */
Event * ContextManager::getEvent(unsigned int id) const
{
    return m_pEventMgr->getEvent(id);
}

/* */
unsigned int ContextManager::registerEvent()
{
    return m_pEventMgr->registerEvent();
}

/* */
SSI_Status ContextManager::unregisterEvent(unsigned int id)
{
    return m_pEventMgr->unregisterEvent(id);
}

/* */
unsigned int ContextManager::acquireId(Object *pObject) {
    return m_pUniqueIdMgr->acquireId(pObject);
}

/* */
void ContextManager::releaseId(Object *pObject) {
    m_pUniqueIdMgr->releaseId(pObject);
}

/* */
void ContextManager::refresh()
{
    m_pUniqueIdMgr->refresh();
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
