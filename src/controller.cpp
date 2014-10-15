
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

#include <ssi.h>

#include <engine/exception.h>
#include <engine/container.h>
#include <engine/string.h>
#include <engine/object.h>
#include <engine/controller.h>
#include <engine/session.h>
#include <engine/context_manager.h>

#include "templates.h"

static void getItems(ScopeObject *pScopeObject, SSI_ScopeType scopeType, Container<Controller> &container)
{
    pScopeObject->getControllers(container);
}

static Controller * getItem(Session *pSession, SSI_Handle handle)
{
    return pSession->getController(handle);
}

/* */
SSI_Status SsiGetControllerHandles(SSI_Handle session, SSI_ScopeType scopeType,
    SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount)
{
    return SsiGetHandles(session, scopeType, scopeHandle, handleList, handleCount, getItems);
}

/* */
SSI_Status SsiGetControllerInfo(SSI_Handle session, SSI_Handle controllerHandle,
    SSI_ControllerInfo *controllerInfo)
{
    return SsiGetInfo(session, controllerHandle, controllerInfo, getItem);
}

/* */
SSI_Status SsiReadPatrolSetState(SSI_Handle controllerHandle, SSI_Bool enable)
{
    if (pContextMgr == NULL) {
        return SSI_StatusNotInitialized;
    }
    Session *pSession;
    try {
        pSession = pContextMgr->getSession(SSI_NULL_HANDLE);
    } catch (...) {
        return SSI_StatusNotInitialized;
    }
    if (pSession == NULL) {
        return SSI_StatusInvalidSession;
    }
    Controller *pController = pSession->getController(controllerHandle);
    if (pController == NULL) {
        return SSI_StatusInvalidHandle;
    }
    return pController->readPatrolSetState(enable == SSI_TRUE);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
