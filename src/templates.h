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
 *     may be used to endorse or promote products derived from this software
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

#ifndef __TEMPLATES_H__INCLUDED__
#define __TEMPLATES_H__INCLUDED__

#include <engine/context_manager.h>

#ifdef SSI_HAS_PRAGMA_ONCE
#pragma once
#endif

SSI_Status getTempSession(boost::shared_ptr<Session>& pSession);

SSI_Status getSession(SSI_Handle session, boost::shared_ptr<Session>& pSession);

template <class T>
SSI_Status SsiGetHandles(SSI_Handle session, SSI_ScopeType scopeType,
    SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount,
    void (*getItems)(const boost::shared_ptr<ScopeObject> &, SSI_ScopeType, Container<T> &))
{
    boost::shared_ptr<Session> pSession;
    if (SSI_Status status = getSession(session, pSession)) {
        return status;
    }

    boost::shared_ptr<ScopeObject> pScopeObject = pSession->getObject(scopeHandle);
    if (!pScopeObject) {
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
    void (*getItems)(const boost::shared_ptr<ScopeObject> &, SSI_ScopeType, Container<T> &))
{
    boost::shared_ptr<Session> session;
    if (SSI_Status status = getTempSession(session)) {
        return status;
    }

    boost::shared_ptr<ScopeObject> pScopeObject = session->getObject(scopeHandle);
    if (!pScopeObject) {
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
SSI_Status SsiGetInfo(SSI_Handle session, SSI_Handle itemHandle, T *info,
    boost::shared_ptr<T2> (*getItem)(const boost::shared_ptr<Session> &, SSI_Handle))
{
    boost::shared_ptr<Session> pSession;
    if (SSI_Status status = getSession(session, pSession)) {
        return status;
    }

    boost::shared_ptr<T2> pItem = getItem(pSession, itemHandle);
    if (!pItem) {
        return SSI_StatusInvalidHandle;
    }

    return pItem->getInfo(info);
}

template <class T, class T2>
SSI_Status SsiGetInfo(SSI_Handle itemHandle, T *info,
    boost::shared_ptr<T2>  (*getItem)(const boost::shared_ptr<Session> &, SSI_Handle))
{
    boost::shared_ptr<Session> session;
    if (SSI_Status status = getTempSession(session)) {
        return status;
    }

    boost::shared_ptr<T2> pItem = getItem(session, itemHandle);
    if (!pItem) {
        return SSI_StatusInvalidHandle;
    }

    return pItem->getInfo(info);
}

#endif /* __TEMPLATES_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
