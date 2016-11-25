/*
Copyright (c) 2011 - 2016, Intel Corporation
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

#include <cstdio>
#include <sys/types.h>
#include <csignal>

#include "event_manager.h"
#include "utils.h"

using std::nothrow;
using boost::shared_ptr;

/* */
EventManager::EventManager():
    m_notInitialized(true)
{
}

/* */
EventManager::~EventManager()
{
    /* unregisterEvent invalidates iterator - foreach loop is forbidden */
    Manager::iterator iter = m_events.begin();
    while (iter != m_events.end()) {
        unregisterEvent(iter->first);
        iter = m_events.begin();
    }

    stopEventMonitor();
}

/* */
void EventManager::startEventMonitor()
{
    pid_t pid = readPidFile("/var/run/ssieventmonitor.pid", "ssieventmonitor");

    if (pid <= 0) { /* Not found or invalid pid file */
        int ret = shell_command("ssieventmonitor --daemonise");
        if (ret == 0) {
            m_notInitialized = false;
        }
    }
}

/* */
void EventManager::stopEventMonitor()
{
    pid_t pid = readPidFile("/var/run/ssieventmonitor.pid", "ssieventmonitor");

    if (pid > 0) { /* Pid file found */
        kill(pid, SIGTERM);
    }
}

/* */
SSI_Handle EventManager::registerEvent()
{
    SSI_Handle eventHandle;
    if (m_events.size() == MAX_EVENT_HANDLES) {
        return SSI_NULL_HANDLE;
    }

    shared_ptr<Event> pEvent = shared_ptr<Event>(new(nothrow) Event());
    if (!pEvent) {
        return SSI_NULL_HANDLE; /* Out of memory */
    }

    if (!m_events.insert(pEvent).second) {
        return SSI_NULL_HANDLE; /* Out of resources */
    }

    if (pEvent->registerEvent()) {
        eventHandle = pEvent->getHandle();
    } else {
        unregisterEvent(pEvent->getHandle()); /* Failed to create semaphore */
        return SSI_NULL_HANDLE;
    }

    if (m_notInitialized) {
        startEventMonitor();
    }

    return eventHandle;
}

/* */
SSI_Status EventManager::unregisterEvent(SSI_Handle handle)
{
    if (handle == SSI_NULL_HANDLE) {
        return SSI_StatusInvalidParameter;
    }

    shared_ptr<Event> pEvent = m_events.remove(handle);

    if (!pEvent) {
        return SSI_StatusInvalidHandle;
    }

    return SSI_StatusOk;
}

/* */
shared_ptr<Event> EventManager::getEvent(SSI_Handle handle) const
{
    return m_events.at(handle);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
