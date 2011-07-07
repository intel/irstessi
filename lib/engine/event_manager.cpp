
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
#include "event.h"
#include "unique_id_manager.h"
#include "event_manager.h"
#include "context_manager.h"

/* */
EventManager::EventManager()
{
}

/* */
EventManager::~EventManager()
{
    for (Iterator<Event *> i = m_Events; *i != 0; ++i) {
        pContextMgr->releaseId(*i);
    }
}

/* */
unsigned int EventManager::registerEvent()
{
    Event *pEvent;
    try {
        pEvent = new Event();
    } catch (...) {
        return SSI_NULL_HANDLE; /* Out of memory */
    }
    try {
        pContextMgr->acquireId(pEvent);
        m_Events.add(pEvent);
        return pEvent->getId();
    } catch (...) {
        delete pEvent;
        return SSI_NULL_HANDLE; /* Out of resources */
    }
    return SSI_NULL_HANDLE;
}

/* */
SSI_Status EventManager::unregisterEvent(unsigned int id)
{
    if (id == 0) {
        return SSI_StatusInvalidParameter;
    }
    Event *pEvent;
    try {
        pEvent = m_Events.remove(id);
        pContextMgr->releaseId(pEvent);
    } catch (...) {
        return SSI_StatusInvalidParameter;
    }
    return SSI_StatusOk;
}

/* */
Event * EventManager::getEvent(unsigned int id) const
{
    return m_Events.find(id);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
