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

#include <engine/event.h>
#include <engine/context_manager.h>

using boost::shared_ptr;

/* */
SSI_Status SsiGetEventHandle(SSI_Handle *eventHandle)
{
    if (pContextMgr == NULL) {
        return SSI_StatusNotInitialized;
    }

    if (eventHandle == NULL) {
        return SSI_StatusInvalidParameter;
    }

    *eventHandle = pContextMgr->registerEvent();

    if (*eventHandle == SSI_NULL_HANDLE) {
        return SSI_StatusInsufficientResources;
    }

    return SSI_StatusOk;
}

/* */
SSI_Status SsiFreeEventHandle(SSI_Handle eventHandle)
{
    if (pContextMgr == NULL) {
        return SSI_StatusNotInitialized;
    }

    return pContextMgr->unregisterEvent(eventHandle);
}

/* */
SSI_Status SsiEventWait(SSI_Uint32 timeout, SSI_Handle eventHandle)
{
    if (pContextMgr == NULL) {
        return SSI_StatusNotInitialized;
    }

    shared_ptr<Event> pEvent = pContextMgr->getEvent(eventHandle);

    if (!pEvent) {
        return SSI_StatusInvalidHandle;
    }

    return pEvent->wait(timeout);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
