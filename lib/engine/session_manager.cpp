
/*
Copyright (c) 2011, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/




#if (HAVE_CONFIG_H == 1)
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
#include "session.h"
#include "unique_id_manager.h"
#include "session_manager.h"
#include "context_manager.h"

/* */
SessionManager::SessionManager()
    : m_pNullSession(0)
{
}

/* */
SessionManager::~SessionManager()
{
    delete m_pNullSession;

    for (Iterator<Session *> i = m_Sessions; *i != 0; ++i) {
        pContextMgr->releaseId(*i);
    }
}

/* */
Session * SessionManager::getSession(unsigned int id)
{
    Session *pSession;
    if (id == 0) {
        pSession = new Session();
        delete m_pNullSession;
        m_pNullSession = pSession;
    } else {
        pSession = m_Sessions.find(id);
    }
    return pSession;
}

/* */
unsigned int SessionManager::openSession()
{
    Session *pSession;
    try {
        pSession = new Session();
    } catch (...) {
        return 0; /* Out of memory, no more sessions allowed. */
    }
    try {
        pContextMgr->acquireId(pSession);
        m_Sessions.add(pSession);
        return pSession->getId();
    } catch (...) {
        delete pSession;
        return 0; /* Out of resources, no more sessions allowed. */
    }
}

/* */
SSI_Status SessionManager::closeSession(unsigned int id)
{
    if (id == 0) {
        return SSI_StatusInvalidParameter;
    }
    Session *pSession;
    try {
        pSession = m_Sessions.remove(id);
        pContextMgr->releaseId(pSession);
    } catch (...) {
        return SSI_StatusInvalidParameter;
    }
    return SSI_StatusOk;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
