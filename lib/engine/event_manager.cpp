
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
#include <cstdio>
#include <sys/types.h>
#include <csignal>

#include "exception.h"
#include "container.h"
#include "string.h"
#include "object.h"
#include "event.h"
#include "unique_id_manager.h"
#include "event_manager.h"
#include "context_manager.h"
#include "utils.h"

/* */
EventManager::EventManager()
{
}

/* */
EventManager::~EventManager()
{
    foreach (i, m_Events) {
        pContextMgr->remove(*i);
    }
}

/* */
int EventManager::startEventMonitor(void)
{
    pid_t pid = readPidFile("/var/run/ssieventmonitor.pid", "ssieventmonitor");

    if (pid <= 0)
	return shell("ssieventmonitor --daemonise");
    else
	return 1;
}

/* */
int EventManager::stopEventMonitor(void)
{
   pid_t pid = readPidFile("/var/run/ssieventmonitor.pid", "ssieventmonitor");
   if (pid <= 0) /* not found or invalid pid file */
       return 0;

   return kill(pid, SIGTERM);
}
/* */
unsigned int EventManager::registerEvent()
{
    Event *pEvent;
    unsigned int eventId;
    if (m_Events.size() == MAX_EVENT_HANDLES)
        return SSI_NULL_HANDLE;
    try {
        pEvent = new Event();
    } catch (...) {
        return SSI_NULL_HANDLE; /* Out of memory */
    }
    try {
        pContextMgr->add(pEvent);
        m_Events.add(pEvent);
    } catch (...) {
        delete pEvent;
        return SSI_NULL_HANDLE; /* Out of resources */
    }
    try {
        pEvent->registerEvent();
        eventId = pEvent->getId();
    } catch (...) {
        unregisterEvent(pEvent->getId()); /* failed to create semaphore*/
	return SSI_NULL_HANDLE;
    }
    /* everything went OK, so start event trigger */
    if (m_Events.size() != 0) {
	startEventMonitor();
    }
    return eventId;
}

/* */
SSI_Status EventManager::unregisterEvent(unsigned int id)
{
    if (id == 0) {
        return SSI_StatusInvalidHandle;
    }
    Event *pEvent;
    try {
        pEvent = m_Events.remove(id);
        pContextMgr->remove(pEvent);
    } catch (...) {
        return SSI_StatusInvalidHandle;
    }
    /* everything went OK, so stop event trigger */
    if (m_Events.size() == 0) {
	stopEventMonitor();
    }
    return SSI_StatusOk;
}

/* */
Event * EventManager::getEvent(unsigned int id) const
{
    return m_Events.find(id);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
