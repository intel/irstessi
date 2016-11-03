/*
Copyright (c) 2011 - 2016, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __TEMPLATES_H__INCLUDED__
#define __TEMPLATES_H__INCLUDED__

#include <engine/context_manager.h>

#ifdef SSI_HAS_PRAGMA_ONCE
#pragma once
#endif

SSI_Status getSession(SSI_Handle session, Session **pSession);

template <class T>
SSI_Status SsiGetHandles(SSI_Handle session, SSI_ScopeType scopeType,
    SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount,
    void (*getItems)(ScopeObject *, SSI_ScopeType, Container<T> &))
{
    Session *pSession = NULL;
    if (SSI_Status status = getSession(session, &pSession)) {
        return status;
    }

    ScopeObject *pScopeObject = pSession->getObject(scopeHandle);
    if (pScopeObject == NULL) {
        return SSI_StatusInvalidScope;
    }

    if (!pScopeObject->scopeTypeMatches(scopeType)) {
        return SSI_StatusInvalidScope;
    }

    Container<T> container;
    try {
        getItems(pScopeObject, scopeType, container);
    } catch (...) {
        return SSI_StatusInvalidScope;
    }

    return container.getHandles(handleList, handleCount);
}

template <class T>
SSI_Status SsiGetHandles(SSI_ScopeType scopeType, SSI_Handle scopeHandle,
    SSI_Handle *handleList, SSI_Uint32 *handleCount,
    void (*getItems)(ScopeObject *, SSI_ScopeType, Container<T> &))
{
    TemporarySession session;
    if (!session.isValid()) {
        return SSI_StatusNotInitialized;
    }

    ScopeObject *pScopeObject = session->getObject(scopeHandle);
    if (pScopeObject == NULL) {
        return SSI_StatusInvalidScope;
    }

    if (!pScopeObject->scopeTypeMatches(scopeType)) {
        return SSI_StatusInvalidScope;
    }

    Container<T> container;
    try {
        getItems(pScopeObject, scopeType, container);
    } catch (...) {
        return SSI_StatusInvalidScope;
    }

    return container.getHandles(handleList, handleCount);
}

template <class T, class T2>
SSI_Status SsiGetItem(SSI_Handle session, SSI_Handle itemHandle, T **pItem,
    T2 * (*getItem)(Session *, SSI_Handle))
{
    Session *pSession = NULL;
    if (SSI_Status status = getSession(session, &pSession)) {
        return status;
    }

    *pItem = getItem(pSession, itemHandle);
    if (*pItem == NULL) {
        return SSI_StatusInvalidHandle;
    }

    return SSI_StatusOk;
}

template <class T, class T2>
SSI_Status SsiGetItem(SSI_Handle itemHandle, T **pItem, T2 * (*getItem)(Session *, SSI_Handle))
{
    TemporarySession session;
    if (!session.isValid()) {
        return SSI_StatusNotInitialized;
    }

    *pItem = getItem(session.get(), itemHandle);
    if (*pItem == NULL) {
        return SSI_StatusInvalidHandle;
    }

    return SSI_StatusOk;
}

template <class T, class T2>
SSI_Status SsiGetInfo(SSI_Handle session, SSI_Handle itemHandle, T *info,
    T2 * (*getItem)(Session *, SSI_Handle))
{
    T2 *pItem = NULL;
    if (SSI_Status status = SsiGetItem(session, itemHandle, &pItem, getItem)) {
        return status;
    }

    return pItem->getInfo(info);
}

template <class T, class T2>
SSI_Status SsiGetInfo(SSI_Handle itemHandle, T *info,
    T2 * (*getItem)(Session *, SSI_Handle))
{
    T2 *pItem = NULL;
    if (SSI_Status status = SsiGetItem(itemHandle, &pItem, getItem)) {
        return status;
    }

    return pItem->getInfo(info);
}

#endif /* __TEMPLATES_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
